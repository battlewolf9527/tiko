'    tiko editor - Programmer's Code Editor for the FreeBASIC Compiler
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'
'    This program is free software: you can redistribute it and/or modify
'    it under the terms of the GNU General Public License as published by
'    the Free Software Foundation, either version 3 of the License, or
'    (at your option) any later version.
'
'    This program is distributed in the hope that it will be useful,
'    but WITHOUT any WARRANTY; without even the implied warranty of
'    MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
'    GNU General Public License for more details.

type _GDBMessage as GDBMessage

type GDBMessage
    message   as string
    pNextNode as _GDBMessage ptr
end type

type VariableType
    VarName         as string
    VarNameDisplay  as string
    VarValue        as string
end type

type GDBSession
    hProcess                 as HANDLE
    hThread                  as HANDLE
    hStdInWrite              as HANDLE
    hStdOutRead              as HANDLE
    hStdErrRead              as HANDLE
    dwProcessId              as DWORD
    initialized              as boolean
    hThreadMessages          as any ptr
    KillMessageThread        as boolean
    hThreadMutex             as any ptr

    current_file_name        as string
    current_function_name    as string
    current_function_linenum as long
    variable_array(any)      as VariableType
    is_new_variables         as boolean 
    
    ' Message fifo queue
    head                     as GDBMessage ptr
    tail                     as GDBMessage ptr
    count                    as integer
end type

dim shared as GDBSession gdb_session

' Enqueue - add message to the end of the queue
sub gdb_enqueue_message( byref q as GDBSession, byref response as string )
    dim newNode as GDBMessage ptr = new GDBMessage
    newNode->message = response
    newNode->pNextNode = 0
    
    if q.tail = 0 then
        ' Queue is empty
        q.head = newNode
        q.tail = newNode
    else
        ' Add to end
        q.tail->pNextNode = newNode
        q.tail = newNode
    end if
    
    q.count += 1
end sub

' Dequeue - remove and return message from front of queue
function gdb_dequeue_message( byref q as GDBSession ) as string
    if q.head = 0 then
        return ""  ' Queue is empty
    end if
    
    dim tempNode as GDBMessage ptr = q.head
    dim msg as string = tempNode->message
    
    q.head = q.head->pNextNode
    if q.head = 0 then
        q.tail = 0  ' Queue is now empty
    end if
    
    delete tempNode
    q.count -= 1
    
    return msg
end function

' Check if queue is empty
function gdb_isMessageQueueEmpty( byref q as GDBSession ) as boolean
    dim as boolean isEmpty
    MutexLock( gdb_session.hThreadMutex )
    isEmpty = (q.head = 0)
    MutexUnLock( gdb_session.hThreadMutex )
    return isEmpty
end function

' Clear all messages from queue
sub clearQueue( byref q as GDBSession )
    while gdb_isMessageQueueEmpty(q) = false
        gdb_dequeue_message(q)
    wend
end sub

' Initialize GDB session
function gdb_init(byref session as GDBSession, byval executable as DWSTRING = "") as boolean
    dim as SECURITY_ATTRIBUTES sa
    dim as HANDLE hStdInRead, hStdInWrite
    dim as HANDLE hStdOutRead, hStdOutWrite
    dim as HANDLE hStdErrRead, hStdErrWrite
    dim as STARTUPINFO si
    dim as PROCESS_INFORMATION pi
    dim as string cmdline
    dim as integer ret
    
    ' Set up security attributes for pipe inheritance
    sa.nLength = sizeof(SECURITY_ATTRIBUTES)
    sa.bInheritHandle = TRUE
    sa.lpSecurityDescriptor = NULL
    
    ' Create pipes for stdin
    if CreatePipe(@hStdInRead, @hStdInWrite, @sa, 0) = 0 then return false
    if SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0) = 0 then return false
    
    ' Create pipes for stdout
    if CreatePipe(@hStdOutRead, @hStdOutWrite, @sa, 0) = 0 then return false
    if SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0) = 0 then return false
    
    ' Create pipes for stderr
    if CreatePipe(@hStdErrRead, @hStdErrWrite, @sa, 0) = 0 then return false
    if SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0) = 0 then return false
    
    ' Set up startup info
    si.cb = sizeof(STARTUPINFO)
    si.hStdError   = hStdErrWrite
    si.hStdOutput  = hStdOutWrite
    si.hStdInput   = hStdInRead
    si.dwFlags     = STARTF_USESTDHANDLES or STARTF_USESHOWWINDOW
    si.wShowWindow = SW_HIDE
    
    ' Build command line
    if len(executable) > 0 then
        cmdline = "gdb.exe -q -i=mi2 " & chr(34) & executable & chr(34)
    else
        cmdline = "gdb.exe -q -i=mi2"
    end if
    
    ' Create the GDB process
    ret = CreateProcess(NULL, cmdline, NULL, NULL, TRUE, _
                        CREATE_NO_WINDOW, NULL, NULL, @si, @pi)
    
    if ret = 0 then
        dim as DWORD err_code = GetLastError()
        print "CreateProcess failed with error code: "; err_code
        select case err_code
            case 2
                print "Error: gdb.exe not found. Please ensure GDB is installed and in PATH."
            case 3
                print "Error: Path not found."
            case 5
                print "Error: Access denied."
            case 87
                print "Error: Invalid parameter."
            case 193
                print "Error: Not a valid Win32 application."
            case else
                print "Error: Unknown error code."
        end select
        
        CloseHandle(hStdInRead)
        CloseHandle(hStdInWrite)
        CloseHandle(hStdOutRead)
        CloseHandle(hStdOutWrite)
        CloseHandle(hStdErrRead)
        CloseHandle(hStdErrWrite)
        return false
    end if
    
       
    ' Close handles we don't need in parent process
    CloseHandle(hStdInRead)
    CloseHandle(hStdOutWrite)
    CloseHandle(hStdErrWrite)
    
    ' Store session info
    session.hProcess    = pi.hProcess
    session.hThread     = pi.hThread
    session.hStdInWrite = hStdInWrite
    session.hStdOutRead = hStdOutRead
    session.hStdErrRead = hStdErrRead
    session.dwProcessId = pi.dwProcessId
    session.initialized = true

    return true
end function


' Clear any pending output from pipe
sub gdb_clear_pipe(byref session as GDBSession)
    if session.initialized = false then return 

    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    
    ' Read and discard any pending data
    do
        if session.KillMessageThread = true then exit do
        
        if PeekNamedPipe(session.hStdOutRead, NULL, 0, NULL, @bytes_avail, NULL) = 0 then exit do
        if bytes_avail = 0 then exit do
        
        if bytes_avail > 4096 then bytes_avail = 4096
        ReadFile(session.hStdOutRead, strptr(buffer), bytes_avail, @bytes_read, NULL)
    loop while bytes_avail > 0
end sub


' Send command to GDB (synchronous)
function gdb_send(byref session as GDBSession, byref cmd as string) as boolean
    if session.initialized = false then return false
    
    dim as string full_cmd = cmd + chr(13) + chr(10)
    dim as DWORD bytes_written
    dim as integer ret
    
    ret = WriteFile(session.hStdInWrite, strptr(full_cmd), len(full_cmd), @bytes_written, NULL)
    FlushFileBuffers(session.hStdInWrite)

    return iif(ret <> 0 andalso bytes_written > 0, true, false)
end function


' Receive response from GDB (synchronous with timeout)
function gdb_receive( byref session as GDBSession ) as boolean
    if session.initialized = false then return false
    
    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    dim as double start_time = timer
    dim as integer ret
    
    dim as integer timeout_ms = 500
    dim as string response = ""
    
    do
        if session.KillMessageThread = true then return false
        if session.initialized = false then return false
        
        ret = PeekNamedPipe(session.hStdOutRead, NULL, 0, NULL, @bytes_avail, NULL)
        
        if (ret <> 0) andalso (bytes_avail > 0) then
            if bytes_avail > 4096 then bytes_avail = 4096
            
            ret = ReadFile(session.hStdOutRead, strptr(buffer), bytes_avail, @bytes_read, NULL)
            
LM( "ret: " & ret & "  bytes_read: " & bytes_read )
            if (ret <> 0) andalso (bytes_read > 0) then
                response = response & left(buffer, bytes_read)
LM( "APPEND RESPONSE " & len(response) )                
                start_time = timer
            end if
        else
            exit do
        end if
        
        if (timer - start_time) * 1000 > timeout_ms then exit do
    loop

    if instr(response, "(gdb)") > 0 orelse instr(response, "^done") > 0 orelse _
       instr(response, "^running") > 0 orelse instr(response, "^error") > 0 orelse _
       instr(response, "*stopped") > 0 then
        ' Clear any old data first
        gdb_clear_pipe(session)

        ' Create a new message record and add it to the message queue. The WM_TIMER will
        ' check the message queue for any pending messages.
        MutexLock( gdb_session.hThreadMutex )
LM( "Message length queued: " & len(response) & chr(13,10) & response )
        gdb_enqueue_message( session, response )
        MutexUnLock( gdb_session.hThreadMutex )
    end if
        
    return false
end function


sub gdb_threadListener( byval userdata as any ptr )
    dim as GDBSession ptr session = cast(GDBsession ptr, userdata)
    do until session->KillMessageThread = true
        gdb_receive(*session ) 
    loop    
LM( "gdb_threadListener finished" )
end sub


' Close GDB session
sub gdb_close( byref session as GDBSession )
    if session.initialized then
        session.KillMessageThread = true

        ' Clear the message queue
        clearQueue( session )
    
        ' Try to quit gracefully
        gdb_send(session, "quit")
        sleep 200
        
        ' Check if process is still running
        dim as DWORD exit_code
        if GetExitCodeProcess(session.hProcess, @exit_code) <> 0 then
            if exit_code = STILL_ACTIVE then
                ' Send quit again with confirmation disabled
                gdb_send(session, "set confirm off")
                sleep 50
                gdb_send(session, "quit")
                sleep 200
                
                ' If still running, force terminate
                if GetExitCodeProcess(session.hProcess, @exit_code) <> 0 then
                    if exit_code = STILL_ACTIVE then
                        TerminateProcess(session.hProcess, 0)
                        sleep 100
                    end if
                end if
            end if
        end if

        CloseHandle(session.hStdInWrite)
        CloseHandle(session.hStdOutRead)
        CloseHandle(session.hStdErrRead)
        CloseHandle(session.hThread)
        CloseHandle(session.hProcess)

        session.hStdInWrite = 0
        session.hStdOutRead = 0
        session.hStdErrRead = 0
        session.hThread     = 0
        session.hProcess    = 0

        session.current_file_name     = ""
        session.current_function_name = ""
        erase session.variable_array
        
        session.initialized = false
    end if
end sub



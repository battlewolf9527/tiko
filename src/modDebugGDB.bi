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


' Enqueue - add message to the end of the queue
sub gdb_enqueue_message( byref response as string )
    dim newNode as GDBMessage ptr = new GDBMessage
    newNode->message = response
    newNode->pNextNode = 0
    
    if gdb_session.tail = 0 then
        ' Queue is empty
        gdb_session.head = newNode
        gdb_session.tail = newNode
    else
        ' Add to end
        gdb_session.tail->pNextNode = newNode
        gdb_session.tail = newNode
    end if
    
    gdb_session.count += 1
end sub

' Dequeue - remove and return message from front of queue
function gdb_dequeue_message() as string
    if gdb_session.head = 0 then
        return ""  ' Queue is empty
    end if
    
    dim tempNode as GDBMessage ptr = gdb_session.head
    dim msg as string = tempNode->message
    
    gdb_session.head = gdb_session.head->pNextNode
    if gdb_session.head = 0 then
        gdb_session.tail = 0  ' Queue is now empty
    end if
    
    delete tempNode
    gdb_session.count -= 1
    
    return msg
end function

' Check if queue is empty
function gdb_isMessageQueueEmpty() as boolean
    dim as boolean isEmpty
    MutexLock( gdb_session.hThreadMutex )
    isEmpty = (gdb_session.head = 0)
    MutexUnLock( gdb_session.hThreadMutex )
    return isEmpty
end function

' Clear all messages from queue
sub clearQueue()
    while gdb_isMessageQueueEmpty() = false
        gdb_dequeue_message()
    wend
end sub

' Initialize GDB session
function gdb_init( byval executable as DWSTRING = "" ) as boolean
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
    if CreatePipe( @hStdInRead, @hStdInWrite, @sa, 0 ) = 0 then return false
    if SetHandleInformation( hStdInWrite, HANDLE_FLAG_INHERIT, 0 ) = 0 then return false
    
    ' Create pipes for stdout
    if CreatePipe( @hStdOutRead, @hStdOutWrite, @sa, 0 ) = 0 then return false
    if SetHandleInformation( hStdOutRead, HANDLE_FLAG_INHERIT, 0 ) = 0 then return false
    
    ' Create pipes for stderr
    if CreatePipe( @hStdErrRead, @hStdErrWrite, @sa, 0 ) = 0 then return false
    if SetHandleInformation( hStdErrRead, HANDLE_FLAG_INHERIT, 0 ) = 0 then return false
    
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
    ret = CreateProcess( NULL, cmdline, NULL, NULL, TRUE, _
                         CREATE_NO_WINDOW, NULL, NULL, @si, @pi )
    
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
    gdb_session.hProcess    = pi.hProcess
    gdb_session.hThread     = pi.hThread
    gdb_session.hStdInWrite = hStdInWrite
    gdb_session.hStdOutRead = hStdOutRead
    gdb_session.hStdErrRead = hStdErrRead
    gdb_session.dwProcessId = pi.dwProcessId
    gdb_session.initialized = true

    return true
end function


' Clear any pending output from pipe
sub gdb_clear_pipe()
    if gdb_session.initialized = false then return 

    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    
    ' Read and discard any pending data
    do
        if gdb_session.KillMessageThread = true then exit do
        
        if PeekNamedPipe( gdb_session.hStdOutRead, NULL, 0, NULL, @bytes_avail, NULL ) = 0 then exit do
        if bytes_avail = 0 then exit do
        
        if bytes_avail > 4096 then bytes_avail = 4096
        ReadFile( gdb_session.hStdOutRead, strptr(buffer), bytes_avail, @bytes_read, NULL )
    loop while bytes_avail > 0
end sub


' Send command to GDB (synchronous)
function gdb_send( byref cmd as string ) as boolean
    if gdb_session.initialized = false then return false
    
    dim as string full_cmd = cmd + chr(13) + chr(10)
    dim as DWORD bytes_written
    dim as integer ret
    
    ret = WriteFile( gdb_session.hStdInWrite, strptr(full_cmd), len(full_cmd), @bytes_written, NULL )
    FlushFileBuffers( gdb_session.hStdInWrite )

    return iif(ret <> 0 andalso bytes_written > 0, true, false)
end function


' Receive response from GDB (synchronous with timeout)
function gdb_receive() as boolean
    if gdb_session.initialized = false then return false
    
    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    dim as double start_time = timer
    dim as integer ret
    
    dim as integer timeout_ms = 500
    dim as string response = ""
    
    do
        if gdb_session.KillMessageThread = true then return false
        if gdb_session.initialized = false then return false
        
        ret = PeekNamedPipe( gdb_session.hStdOutRead, NULL, 0, NULL, @bytes_avail, NULL )
        
        if (ret <> 0) andalso (bytes_avail > 0) then
            if bytes_avail > 4096 then bytes_avail = 4096
            
            ret = ReadFile( gdb_session.hStdOutRead, strptr(buffer), bytes_avail, @bytes_read, NULL )
            
            if (ret <> 0) andalso (bytes_read > 0) then
                response = response & left(buffer, bytes_read)
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
        gdb_clear_pipe()

        ' Create a new message record and add it to the message queue. The WM_TIMER will
        ' check the message queue for any pending messages.
        MutexLock( gdb_session.hThreadMutex )
        gdb_enqueue_message( response )
        MutexUnLock( gdb_session.hThreadMutex )
    end if
        
    return false
end function


sub gdb_threadListener( byval userdata as any ptr )
    do until gdb_session.KillMessageThread = true
        gdb_receive() 
    loop    
LM( "gdb_threadListener finished" )
end sub


' Close GDB session
sub gdb_close()
    if gdb_session.initialized then
        gdb_session.KillMessageThread = true

        ' Clear the message queue
        clearQueue()
    
        ' Try to quit gracefully
        gdb_send("quit")
        sleep 200
        
        ' Check if process is still running
        dim as DWORD exit_code
        if GetExitCodeProcess(gdb_session.hProcess, @exit_code) <> 0 then
            if exit_code = STILL_ACTIVE then
                ' Send quit again with confirmation disabled
                gdb_send("set confirm off")
                sleep 50
                gdb_send("quit")
                sleep 200
                
                ' If still running, force terminate
                if GetExitCodeProcess(gdb_session.hProcess, @exit_code) <> 0 then
                    if exit_code = STILL_ACTIVE then
                        TerminateProcess(gdb_session.hProcess, 0)
                        sleep 100
                    end if
                end if
            end if
        end if

        CloseHandle(gdb_session.hStdInWrite)
        CloseHandle(gdb_session.hStdOutRead)
        CloseHandle(gdb_session.hStdErrRead)
        CloseHandle(gdb_session.hThread)
        CloseHandle(gdb_session.hProcess)

        gdb_session = type<GDBSession>()
        
        gdb_session.initialized = false
    end if
end sub



'    tiko editor - Programmer's Code Editor for the FreeBASIC Compiler
'    Copyright (C) 2016-2025 Paul Squires, PlanetSquires Software
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

#include "fbthread.bi"


' Threaded GDB Interface

' ============================================================================
' GDB SESSION AND THREADING
' ============================================================================

type GDBSession
    hProcess    as HANDLE
    hThread     as HANDLE
    hStdInWrite as HANDLE
    hStdOutRead as HANDLE
    hStdErrRead as HANDLE
    dwProcessId as DWORD
    initialized as boolean
end type

type GDBThreadData
    session         as GDBSession
    command_queue   as string 
    response_queue  as string
    mutex           as any ptr
    command_cond    as any ptr
    response_cond   as any ptr
    should_exit     as boolean
    is_busy         as boolean
    hwnd_notify     as HWND
end type


declare function gdb_init( byref session as GDBSession, byval executable as CWSTR = "") as boolean
declare function gdb_send( byref session as GDBSession, byref cmd as string) as boolean
declare function gdb_receive( byref session as GDBSession, byref response as string, byval timeout_ms as integer = 500 ) as boolean
declare sub      gdb_close( byref session as GDBSession )
declare function gdb_thread_init( byval thread_data as GDBThreadData ptr, byval hwnd as HWND, byval executable as CWSTR = "" ) as boolean
declare sub      gdb_thread_proc( byval param as any ptr ) 
declare function gdb_thread_send( byval thread_data as GDBThreadData ptr, byref cmd as string) as boolean
declare function gdb_thread_get_response( byval thread_data as GDBThreadData ptr, byref response as string) as boolean
declare function gdb_thread_is_busy( byval thread_data as GDBThreadData ptr ) as boolean
declare sub      gdb_thread_close( byval thread_data as GDBThreadData ptr )

declare function replace_str( byref text as string, byref find_str as string, byref replace_with as string ) as string
declare function gdb_parse_console_output( byref response as string ) as string
declare function gdb_at_breakpoint( byref response as string ) as boolean
declare function gdb_is_stopped( byref response as string ) as boolean
declare function gdb_is_running( byref response as string ) as boolean
declare function gdb_parse_breakpoint_number( byref response as string ) as integer
declare function gdb_parse_stop_reason( byref response as string ) as string
declare function gdb_parse_current_function( byref response as string ) as string
declare function gdb_parse_current_file( byref response as string ) as string
declare function gdb_parse_current_line( byref response as string ) as integer
declare function gdb_has_exited( byref response as string ) as boolean
declare function gdb_parse_exit_code( byref response as string ) as integer
declare function gdb_parse_signal_name( byref response as string ) as string



' Initialize GDB session
function gdb_init( byref session as GDBSession, byval executable as CWSTR = "") as boolean
    dim as SECURITY_ATTRIBUTES sa
    dim as HANDLE hStdInRead, hStdInWrite
    dim as HANDLE hStdOutRead, hStdOutWrite
    dim as HANDLE hStdErrRead, hStdErrWrite
    dim as STARTUPINFO si
    dim as PROCESS_INFORMATION pi
    dim as string cmdline
    dim as integer ret
    
    ' Set up security attributes for pipe inheritance
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES)
    sa.bInheritHandle       = TRUE
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
    si.cb           = sizeof(STARTUPINFO)
    si.hStdError    = hStdErrWrite
    si.hStdOutput   = hStdOutWrite
    si.hStdInput    = hStdInRead
    si.dwFlags      = STARTF_USESTDHANDLES or STARTF_USESHOWWINDOW
    si.wShowWindow  = SW_HIDE
    
    dim as string gdbFilename = AfxGetExePathName & "bin\gdb\gdb.exe" 

    ' Build command line
    if len(executable) > 0 then
        cmdline = gdbFilename & " -q -i=mi " & chr(34) & executable & chr(34)
    else
        cmdline = gdbFilename & " -q -i=mi"
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
    session.hProcess    = pi.hProcess
    session.hThread     = pi.hThread
    session.hStdInWrite = hStdInWrite
    session.hStdOutRead = hStdOutRead
    session.hStdErrRead = hStdErrRead
    session.dwProcessId = pi.dwProcessId
    session.initialized = true
    
    return true
end function


function gdb_send( byref session as GDBSession, byref cmd as string) as boolean
    if session.initialized = false then return 0
    
    dim as string full_cmd = cmd + chr(13) + chr(10)
    dim as DWORD bytes_written
    dim as integer ret
    
    ret = WriteFile( session.hStdInWrite, strptr(full_cmd), len(full_cmd), @bytes_written, NULL )
    FlushFileBuffers( session.hStdInWrite )
    
    return iif(ret <> 0 andalso bytes_written > 0, true, false)
end function


function gdb_receive( byref session as GDBSession, byref response as string, byval timeout_ms as integer = 500 ) as boolean
    if session.initialized = false then return 0
    
    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    dim as double start_time = timer
    dim as integer ret
    
    response = ""
    
    do
        ret = PeekNamedPipe( session.hStdOutRead, NULL, 0, NULL, @bytes_avail, NULL )
        
        if ret <> 0 andalso bytes_avail > 0 then
            if bytes_avail > 4096 then bytes_avail = 4096
            
            ret = ReadFile( session.hStdOutRead, strptr(buffer), bytes_avail, @bytes_read, NULL )
            
            if ret <> 0 andalso bytes_read > 0 then
                response += left(buffer, bytes_read)
                start_time = timer
            end if
        else
            sleep 10
        end if
        
        if instr(response, "(gdb)") > 0 orelse instr(response, "^done") > 0 orelse _
           instr(response, "^running") > 0 orelse instr(response, "^error") > 0 then
            return true
        end if
        
        if (timer - start_time) * 1000 > timeout_ms then exit do
    loop

    
    return iif(len(response) > 0, true, false)
end function


sub gdb_close( byref session as GDBSession )
    if session.initialized then
        gdb_send(session, "quit")
        sleep 200
        
        dim as DWORD exit_code
        if GetExitCodeProcess(session.hProcess, @exit_code) <> 0 then
            if exit_code = STILL_ACTIVE then
                TerminateProcess(session.hProcess, 0)
            end if
        end if
        
        CloseHandle(session.hStdInWrite)
        CloseHandle(session.hStdOutRead)
        CloseHandle(session.hStdErrRead)
        CloseHandle(session.hProcess)

        session.initialized = false
    end if
end sub


' ============================================================================
' THREADED GDB WORKER
' ============================================================================

sub gdb_thread_proc( byval param as any ptr ) 
    dim as GDBThreadData ptr thread_data = param
    dim as string cmd, response
    
    do while thread_data->should_exit = false
        mutexlock(thread_data->mutex)
        
        ' Wait for command or exit signal
        do while thread_data->command_queue = "" andalso thread_data->should_exit = false
            condwait(thread_data->command_cond, thread_data->mutex)
        loop
        
        if thread_data->should_exit = true then
            mutexunlock(thread_data->mutex)
            exit do
        end if
        
        if thread_data->command_queue <> "" then
            cmd = thread_data->command_queue
            thread_data->command_queue = ""
            thread_data->is_busy = true
        end if
        
        mutexunlock(thread_data->mutex)
        
        if len(cmd) > 0 then
            if gdb_send(thread_data->session, cmd) then
                if gdb_receive(thread_data->session, response, 5000) then
                    mutexlock(thread_data->mutex)
                    thread_data->response_queue = response
                    thread_data->is_busy = false
                    condsignal(thread_data->response_cond)
                    mutexunlock(thread_data->mutex)
                    
                    ' Check for async notifications and send specific messages
                    if instr(response, "*stopped") > 0 then
                        if instr(response, "exited") > 0 then
                            PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_EXITED, 0, 0)
                        elseif instr(response, "breakpoint-hit") > 0 then
                            PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_BREAKPOINT_HIT, 0, 0)
                        else
                            PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_STOPPED, 0, 0)
                        end if
                    elseif instr(response, "*running") > 0 then
                        PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_RUNNING, 0, 0)
                    else
                        PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_RESPONSE, 0, 0)
                    end if
                else
                    mutexlock(thread_data->mutex)
                    thread_data->is_busy = false
                    mutexunlock(thread_data->mutex)
                    PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_ERROR, 0, 0)
                end if
            else
                mutexlock(thread_data->mutex)
                thread_data->is_busy = false
                mutexunlock(thread_data->mutex)
                PostMessage(thread_data->hwnd_notify, MSG_USER_GDB_ERROR, 0, 0)
            end if
            cmd = ""
        end if
    loop
    
print "THREAD EXITED gdb_thread_proc"

end sub


function gdb_thread_send( byval thread_data as GDBThreadData ptr, byref cmd as string) as boolean
    mutexlock(thread_data->mutex)

    thread_data->command_queue = cmd
    
    mutexunlock(thread_data->mutex)
    condsignal(thread_data->command_cond)
    return true
end function

function gdb_thread_get_response( byval thread_data as GDBThreadData ptr, byref response as string) as boolean
    mutexlock(thread_data->mutex)
    
    if thread_data->response_queue <> "" then
        response = thread_data->response_queue
        thread_data->response_queue = ""
        mutexunlock(thread_data->mutex)
        return true
    end if
    
    mutexunlock(thread_data->mutex)
    return false
end function

function gdb_thread_init( byval thread_data as GDBThreadData ptr, byval hwnd as HWND, byval executable as CWSTR = "" ) as boolean
    if gdb_init( thread_data->session, executable ) = false then
        return false
    end if
    
    thread_data->mutex          = MutexCreate()
    thread_data->command_cond   = condcreate()
    thread_data->response_cond  = condcreate()
    thread_data->command_queue  = ""
    thread_data->response_queue = ""
    thread_data->should_exit    = false
    thread_data->is_busy        = false
    thread_data->hwnd_notify    = hwnd

    dim as any ptr hThread = threadcreate( @gdb_thread_proc, cast(any ptr, thread_data) )

    if hThread = 0 then
        gdb_close(thread_data->session)
        mutexdestroy(thread_data->mutex)
        conddestroy(thread_data->command_cond)
        conddestroy(thread_data->response_cond)
        return false
    end if
    
    ' Send initial command to clear startup messages
    sleep 500
    gdb_thread_send( thread_data, "set pagination off" )
    
    return true
end function

function gdb_thread_is_busy( byval thread_data as GDBThreadData ptr ) as boolean
    dim as boolean result
    mutexlock(thread_data->mutex)
    result = thread_data->is_busy
    mutexunlock(thread_data->mutex)
    return result
end function

sub gdb_thread_close( byval thread_data as GDBThreadData ptr )
    thread_data->should_exit = true
    condsignal(thread_data->command_cond)
    sleep 200
    
    gdb_close(thread_data->session)

    mutexdestroy(thread_data->mutex)
    conddestroy(thread_data->command_cond)
    conddestroy(thread_data->response_cond)
end sub

' ============================================================================
' RESPONSE PARSING
' ============================================================================

function replace_str( byref text as string, byref find_str as string, byref replace_with as string ) as string
    dim as string result = text
    dim as integer position = 1
    
    do
        position = instr(position, result, find_str)
        if position = 0 then exit do
        
        result = left(result, position - 1) + replace_with + mid(result, position + len(find_str))
        position += len(replace_with)
    loop
    
    return result
end function

function gdb_parse_console_output( byref response as string ) as string
    dim as string result = ""
    dim as integer position = 1, start_pos, end_pos
    
    do
        start_pos = instr(position, response, "~" & chr(34))
        if start_pos = 0 then exit do
        
        start_pos += 2
        end_pos = instr(start_pos, response, chr(34) & chr(10))
        if end_pos = 0 then end_pos = instr(start_pos, response, chr(34))
        
        if end_pos > start_pos then
            result += mid(response, start_pos, end_pos - start_pos)
        end if
        
        position = end_pos + 2
    loop
    
    result = replace_str(result, "\n", chr(10))
    result = replace_str(result, "\t", chr(9))
    result = replace_str(result, "\\", "\")
    
    return result
end function

function gdb_at_breakpoint( byref response as string ) as boolean
    return iif(instr(lcase(response), "stopped") > 0 andalso _
               instr(lcase(response), "breakpoint") > 0, true, false)
end function

function gdb_is_stopped( byref response as string ) as boolean
    return iif(instr(response, "*stopped") > 0, true, false)
end function

function gdb_is_running( byref response as string ) as boolean
    return iif(instr(response, "^running") > 0 orelse instr(response, "*running") > 0, true, false)
end function

function gdb_parse_breakpoint_number( byref response as string ) as integer
    dim as integer position = instr(response, "Breakpoint ")
    if position > 0 then
        return val(mid(response, position + 11))
    end if
    
    ' Try MI format: bkptno="N"
    position = instr(response, "bkptno=" & chr(34))
    if position > 0 then
        return val(mid(response, position + 8))
    end if
    
    return 0
end function

function gdb_parse_stop_reason( byref response as string ) as string
    dim as integer position = instr(response, "reason=" & chr(34))
    if position = 0 then return ""
    
    dim as integer start_pos = position + 8
    dim as integer end_pos = instr(start_pos, response, chr(34))
    if end_pos > start_pos then
        return mid(response, start_pos, end_pos - start_pos)
    end if
    
    return ""
end function

function gdb_parse_current_function( byref response as string ) as string
    dim as integer position = instr(response, "func=" & chr(34))
    if position > 0 then
        dim as integer start_pos = position + 6
        dim as integer end_pos = instr(start_pos, response, chr(34))
        if end_pos > start_pos then
            return mid(response, start_pos, end_pos - start_pos)
        end if
    end if
    
    ' Fallback to text parsing
    position = instr(response, " in ")
    if position > 0 then
        dim as integer start_pos = position + 4
        dim as integer end_pos = start_pos
        do while end_pos <= len(response)
            dim as string ch = mid(response, end_pos, 1)
            if ch = " " orelse ch = "(" orelse ch = chr(10) orelse ch = chr(13) then exit do
            end_pos += 1
        loop
        if end_pos > start_pos then
            return mid(response, start_pos, end_pos - start_pos)
        end if
    end if
    
    return ""
end function

function gdb_parse_current_file( byref response as string ) as string
    dim as integer position = instr(response, "file=" & chr(34))
    if position > 0 then
        dim as integer start_pos = position + 6
        dim as integer end_pos = instr(start_pos, response, chr(34))
        if end_pos > start_pos then
            return mid(response, start_pos, end_pos - start_pos)
        end if
    end if
    return ""
end function

function gdb_parse_current_line( byref response as string ) as integer
    dim as integer position = instr(response, "line=" & chr(34))
    if position > 0 then
        return val(mid(response, position + 6))
    end if
    return 0
end function

function gdb_has_exited( byref response as string ) as boolean
    return iif(instr(response, "exited-normally") > 0 orelse _
               instr(response, "exited,exit-code") > 0 orelse _
               instr(response, "exited-signalled") > 0, true, false)
end function

function gdb_parse_exit_code( byref response as string ) as integer
    dim as integer position = instr(response, "exit-code=" & chr(34))
    if position > 0 then
        return val(mid(response, position + 11))
    end if
    
    ' Also check for text format
    position = instr(response, "exited with code ")
    if position > 0 then
        return val(mid(response, position + 17))
    end if
    
    ' If exited-normally, return 0
    if instr(response, "exited-normally") > 0 then
        return 0
    end if
    
    return -1
end function

function gdb_parse_signal_name( byref response as string ) as string
    dim as integer position = instr(response, "signal-name=" & chr(34))
    if position = 0 then return ""
    
    dim as integer start_pos = position + 13
    dim as integer end_pos = instr(start_pos, response, chr(34))
    if end_pos > start_pos then
        return mid(response, start_pos, end_pos - start_pos)
    end if
    
    return ""
end function


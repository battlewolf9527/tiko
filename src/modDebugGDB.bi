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

type GDBSession
    hProcess as HANDLE
    hThread as HANDLE
    hStdInWrite as HANDLE
    hStdOutRead as HANDLE
    hStdErrRead as HANDLE
    dwProcessId as DWORD
    initialized as boolean
end type

dim shared as GDBSession gdb_session

' Initialize GDB session
function gdb_init(byref session as GDBSession, byval executable as CWSTR = "") as boolean
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
    si.hStdError = hStdErrWrite
    si.hStdOutput = hStdOutWrite
    si.hStdInput = hStdInRead
    si.dwFlags = STARTF_USESTDHANDLES or STARTF_USESHOWWINDOW
    si.wShowWindow = SW_HIDE
    
    ' Build command line
    if len(executable) > 0 then
        cmdline = "gdb.exe -q -i=mi " & chr(34) & executable & chr(34)
    else
        cmdline = "gdb.exe -q -i=mi"
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
    session.hProcess = pi.hProcess
    session.hThread = pi.hThread
    session.hStdInWrite = hStdInWrite
    session.hStdOutRead = hStdOutRead
    session.hStdErrRead = hStdErrRead
    session.dwProcessId = pi.dwProcessId
    session.initialized = true
    
    return true
end function

' Send command to GDB (synchronous)
function gdb_send(byref session as GDBSession, cmd as string) as boolean
    if session.initialized = false then return false
    
    dim as string full_cmd = cmd + chr(13) + chr(10)
    dim as DWORD bytes_written
    dim as integer ret
    
    ret = WriteFile(session.hStdInWrite, strptr(full_cmd), len(full_cmd), @bytes_written, NULL)
    FlushFileBuffers(session.hStdInWrite)
    
    return iif(ret <> 0 andalso bytes_written > 0, true, false)
end function

' Receive response from GDB (synchronous with timeout)
function gdb_receive(byref session as GDBSession, byref response as string, timeout_ms as integer = 500) as boolean
    if session.initialized = false then return false
    
    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    dim as double start_time = timer
    dim as integer ret
    
    response = ""
    
    do
        ret = PeekNamedPipe(session.hStdOutRead, NULL, 0, NULL, @bytes_avail, NULL)
        
        if ret <> 0 andalso bytes_avail > 0 then
            if bytes_avail > 4096 then bytes_avail = 4096
            
            ret = ReadFile(session.hStdOutRead, strptr(buffer), bytes_avail, @bytes_read, NULL)
            
            if ret <> 0 andalso bytes_read > 0 then
                response += left(buffer, bytes_read)
                start_time = timer
            end if
        else
            sleep 10
        end if
        
        if instr(response, "(gdb)") > 0 orelse instr(response, "^done") > 0 orelse _
           instr(response, "^running") > 0 orelse instr(response, "^error") > 0 orelse _
           instr(response, "*stopped") > 0 then
            return true
        end if
        
        if (timer - start_time) * 1000 > timeout_ms then exit do
    loop
    
    return iif(len(response) > 0, true, false)
end function

' Close GDB session
sub gdb_close(byref session as GDBSession)
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
        CloseHandle(session.hThread)
        CloseHandle(session.hProcess)
        
        session.initialized = 0
    end if
end sub

' ============================================================================
' RESPONSE PARSING
' ============================================================================

function replace_str(text as string, find_str as string, replace_with as string) as string
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

'function gdb_parse_console_output(response as string) as string
'    dim as string result = ""
'    dim as integer position = 1, start_pos, end_pos
    
'    do
'        start_pos = instr(position, response, "~" & chr(34))
'        if start_pos = 0 then exit do
        
'        start_pos += 2
'        end_pos = instr(start_pos, response, chr(34) & chr(10))
'        if end_pos = 0 then end_pos = instr(start_pos, response, chr(34))
        
'        if end_pos > start_pos then
'            result += mid(response, start_pos, end_pos - start_pos)
'        end if
        
'        position = end_pos + 2
'    loop
    
'    result = replace_str(result, "\n", chr(10))
'    result = replace_str(result, "\t", chr(9))
'    result = replace_str(result, "\\", "\")
    
'    return result
'end function

function gdb_at_breakpoint(byref response as string) as boolean
    dim as string lresponse = lcase(response)
    return iif(instr(lresponse, "type=" & chr(34) & "breakpoint" & chr(34)), true, false)
end function

function gdb_is_running(byref response as string) as boolean
    dim as string lresponse = lcase(response)
    return iif(instr(lresponse, "^running") > 0 orelse instr(lresponse, "*running") > 0, true, false)
end function

function gdb_has_exited(byref response as string) as boolean
    dim as string lresponse = lcase(response)
    return iif(instr(lresponse, "exited-normally") > 0 orelse _
               instr(lresponse, "exit-code") > 0 orelse _
               instr(lresponse, "exited-signalled") > 0, true, false)
end function

function gdb_parse_breakpoint_number(byref response as string) as integer
    dim as string lresponse = lcase(response)
    dim as integer position = instr(lresponse, "number=" & chr(34))
    if position > 0 then
        return val(mid(lresponse, position + 8))
    end if
    return 0
end function

function gdb_parse_breakpoint_linenumber(byref response as string) as integer
    dim as string lresponse = lcase(response)
    dim as integer position = instr(lresponse, "line=" & chr(34))
    if position > 0 then
        return val(mid(lresponse, position + 6))
    end if
    return 0
end function

function gdb_parse_exit_code(byref response as string) as integer
    dim as string lresponse = lcase(response)
    dim as integer position = instr(lresponse, "exit-code=" & chr(34))
    if position > 0 then
        return val(mid(lresponse, position + 11))
    end if
    
    position = instr(lresponse, "exited with code ")
    if position > 0 then
        return val(mid(lresponse, position + 17))
    end if
    
    if instr(lresponse, "exited-normally") > 0 then
        return 0
    end if
    
    return -1
end function

function gdb_parse_current_function(byref response as string) as string
    dim as string lresponse = lcase(response)
    dim as integer position = instr(lresponse, "func=" & chr(34))
    if position > 0 then
        dim as integer start_pos = position + 6
        dim as integer end_pos = instr(start_pos, response, chr(34))
        if end_pos > start_pos then
            return mid(response, start_pos, end_pos - start_pos)
        end if
    end if
    
    position = instr(lresponse, " in ")
    if position > 0 then
        dim as integer start_pos = position + 4
        dim as integer end_pos = start_pos
        do while end_pos <= len(response)
            dim as string ch = mid(response, end_pos, 1)
            if ch = " " or ch = "(" or ch = chr(10) or ch = chr(13) then exit do
            end_pos += 1
        loop
        if end_pos > start_pos then
            return mid(response, start_pos, end_pos - start_pos)
        end if
    end if
    
    return ""
end function

function gdb_parse_current_filename(byref response as string) as string
    dim as string lresponse = lcase(response)
    dim as integer position = instr(lresponse, "file=" & chr(34))
    if position > 0 then
        dim as integer start_pos = position + 6
        dim as integer end_pos = instr(start_pos, response, chr(34))
        if end_pos > start_pos then
            dim as string file_name = mid(response, start_pos, end_pos - start_pos)
            return replace_str( file_name, "/", "\" )
        end if
    end if
    return ""
end function

' Send command and get response (synchronous)
function send_gdb_command( byref cmd as string, byref response as string ) as boolean
'    append_output("> " + cmd + chr(13) + chr(10))
    
    if gdb_send(gdb_session, cmd) = false then
'        append_output("ERROR: Failed to send command" + chr(13) + chr(10))
        return false
    end if
    
    if gdb_receive(gdb_session, response, 5000) = 0 then
'        append_output("ERROR: No response from GDB" + chr(13) + chr(10))
        return false
    end if
    
'    dim as string console_out = gdb_parse_console_output(response)
'    if len(console_out) > 0 then
'        append_output(console_out)
'    end if
    
    return true
end function


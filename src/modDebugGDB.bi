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


type GDBNameValuePair
    varname as string
    varvalue as string
end type

type GDBResult
    token as string
    resultClass as string
    resultCount as integer
    results(any) as GDBNameValuePair
    isComplete as boolean ' true = complete, false = incomplete
end type

declare sub gdb_parseResults(byval stringinput as string, results() as GDBNameValuePair, byref count as integer)


function gdb_isResultComplete(byval inputLine as string) as boolean
    ' Check if inputLine starts with a result record marker
    dim i as integer = 1
    
    ' Skip optional token
    while i <= len(inputLine) and (inputLine[i-1] >= asc("0") and inputLine[i-1] <= asc("9"))
        i += 1
    wend
    
    ' Must have '^' to be a result record
    if i > len(inputLine) or inputLine[i-1] <> asc("^") then
        return false
    end if
    
    ' Check for balanced braces/brackets
    dim braceCount as integer = 0
    dim bracketCount as integer = 0
    dim inString as boolean = false
    dim ch as ubyte
    
    for j as integer = i to len(inputLine)
        ch = inputLine[j-1]
        
        if ch = 34 andalso (j = 1 orelse inputLine[j-2] <> asc("\")) then
            inString = not inString
        elseif not inString then
            if ch = asc("{") then braceCount += 1
            if ch = asc("}") then braceCount -= 1
            if ch = asc("[") then bracketCount += 1
            if ch = asc("]") then bracketCount -= 1
        end if
    next
    
    return (braceCount = 0 and bracketCount = 0)
end function

function gdb_parseResultRecord(byval lines as string) as GDBResult
    dim result as GDBResult
    dim position as integer
    dim i as integer
    dim fullLine as string = lines
    
    result.resultCount = 0
    result.isComplete = gdb_isResultComplete(fullLine)
    
    ' Check if line starts with optional token (numeric)
    i = 1
    while i <= len(fullLine) andalso (fullLine[i-1] >= asc("0") andalso fullLine[i-1] <= asc("9"))
        result.token += chr(fullLine[i-1])
        i += 1
    wend
    
    ' Skip '^' character
    if i <= len(fullLine) andalso fullLine[i-1] = asc("^") then
        i += 1
    end if
    
    ' Parse result class (done, running, connected, error, exit)
    position = instr(i, fullLine, ",")
    if position > 0 then
        result.resultClass = mid(fullLine, i, position - i)
        ' Parse results into dynamic array
        gdb_parseResults(mid(fullLine, position + 1), result.results(), result.resultCount)
    else
        result.resultClass = mid(fullLine, i)
    end if
    
    return result
end function

function gdb_parseValue(byval stringinput as string, byref idx as integer) as string
    dim result as string
    dim ch as ubyte
    
    if idx > len(stringinput) then return ""
    
    ch = stringinput[idx-1]
    
    ' Handle quoted string
    if ch = 34 then
        idx += 1
        while idx <= len(stringinput)
            ch = stringinput[idx-1]
            if ch = 34 then
                idx += 1
                exit while
            elseif ch = asc("\") and idx < len(stringinput) then
                idx += 1
                ch = stringinput[idx-1]
                select case ch
                    case asc("n"): result += chr(10)
                    case asc("t"): result += chr(9)
                    case asc("r"): result += chr(13)
                    case asc("\"): result += chr(92)
                    case 34: result += chr(34)
                    case else: result += chr(ch)
                end select
                idx += 1
            else
                result += chr(ch)
                idx += 1
            end if
        wend
    ' Handle list
    elseif ch = asc("[") then
        dim depth as integer = 1
        result += chr(ch)
        idx += 1
        while idx <= len(stringinput) andalso depth > 0
            ch = stringinput[idx-1]
            result += chr(ch)
            if ch = asc("[") then depth += 1
            if ch = asc("]") then depth -= 1
            idx += 1
        wend
    ' Handle tuple
    elseif ch = asc("{") then
        dim depth as integer = 1
        result += chr(ch)
        idx += 1
        while idx <= len(stringinput) andalso depth > 0
            ch = stringinput[idx-1]
            result += chr(ch)
            if ch = asc("{") then depth += 1
            if ch = asc("}") then depth -= 1
            idx += 1
        wend
    ' Handle simple value
    else
        while idx <= len(stringinput)
            ch = stringinput[idx-1]
            if ch = asc(",") orelse ch = asc("}") orelse ch = asc("]") then
                exit while
            end if
            result += chr(ch)
            idx += 1
        wend
    end if
    
    return result
end function

sub gdb_parseResults(byval stringinput as string, results() as GDBNameValuePair, byref count as integer)
    dim i as integer = 1
    dim varName as string
    dim varValue as string
    dim ch as ubyte
    
    count = 0
    redim results(0 to 9) ' Initial allocation
    
    while i <= len(stringinput)
        ' Parse variable name
        varName = ""
        while i <= len(stringinput)
            ch = stringinput[i-1]
            if ch = asc("=") then
                i += 1
                exit while
            end if
            varName += chr(ch)
            i += 1
        wend
        
        ' Parse value
        varValue = gdb_parseValue(stringinput, i)
        
        ' Resize array if needed
        if count > ubound(results) then
            redim preserve results(0 to (count + 1) * 2 - 1)
        end if
        
        ' Store name-value pair
        results(count).varname = varName
        results(count).varvalue = varValue
        count += 1
        
        ' Skip comma
        if i <= len(stringinput) andalso stringinput[i-1] = asc(",") then
            i += 1
        end if
    wend
    
    ' Trim array to actual size
    if count > 0 then
        redim preserve results(0 to count - 1)
    end if
end sub

function gdb_findResultValue(results() as GDBNameValuePair, byval count as integer, byval searchName as string) as string
    for i as integer = 0 to count - 1
        if results(i).varname = searchName then
            return results(i).varvalue
        end if
    next
    return "" ' Not found
end function

function gdb_findResultIndex(results() as GDBNameValuePair, byval count as integer, byval searchName as string) as integer
    for i as integer = 0 to count - 1
        if results(i).varname = searchName then
            return i
        end if
    next
    return -1 ' Not found
end function

function gdb_resultExists(results() as GDBNameValuePair, byval count as integer, byval searchName as string) as boolean
    return (gdb_findResultIndex(results(), count, searchName) >= 0)
end function

function gdb_readMultilineRecord(testLines() as string, byref startIdx as integer) as string
    dim combined as string = ""
    dim lineCount as integer = ubound(testLines) - lbound(testLines) + 1
    
    while startIdx <= lineCount
        combined += testLines(startIdx)
        if gdb_isResultComplete(combined) then
            exit while
        end if
        startIdx += 1
    wend
    
    return combined
end function


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
function gdb_receive(byref session as GDBSession, timeout_ms as integer = 500) as string
    if session.initialized = false then return ""
    
    dim as string buffer = space(4096)
    dim as DWORD bytes_read, bytes_avail
    dim as double start_time = timer
    dim as integer ret
    
    dim as string response = ""
    
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
            return response
        end if
        
        if (timer - start_time) * 1000 > timeout_ms then exit do
    loop
    
    return ""
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

function gdb_positionEditor( byref result as GDBResult ) as boolean
    dim bkptValue as string = gdb_findResultValue(result.results(), result.resultCount, "bkpt")
    print "  [Lookup] Found 'bkpt': " + bkptValue
    
    dim as string file_name 
    dim as integer line_number
    
    ' Parse the bkpt tuple to find line number
    dim bkptResults() as GDBNameValuePair
    dim bkptCount as integer
    gdb_parseResults(mid(bkptValue, 2, len(bkptValue) - 2), bkptResults(), bkptCount) ' Strip { }
    
    if gdb_resultExists(bkptResults(), bkptCount, "line") then
        line_number = val( gdb_findResultValue(bkptResults(), bkptCount, "line") )
'        print "  [Lookup] Breakpoint line number: " + lineNum
    end if
    
'    if gdb_resultExists(bkptResults(), bkptCount, "file") then
'        dim fileName as string = gdb_findResultValue(bkptResults(), bkptCount, "file")
'        print "  [Lookup] Breakpoint file: " + fileName
'    end if

    if gdb_resultExists(bkptResults(), bkptCount, "fullname") then
        file_name = gdb_findResultValue(bkptResults(), bkptCount, "fullname")
        file_name = AfxStrReplace( file_name, "\\", "\" )
        file_name = AfxStrReplace( file_name, "/", "\" )
'        print "  [Lookup] Breakpoint full filename: " + fileName
    end if

LM("file_name: " & file_name)    
LM("line_number: " & str(line_number))
    
    dim as clsDocument ptr pDoc = OpenSelectedDocument( file_name, "", line_number )

    function = iif( pDoc > 0, true, false )
end function



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


' Send command and get response (synchronous)
function send_gdb_command( byref cmd as string ) as string
'    append_output("> " + cmd + chr(13) + chr(10))
    dim as string response
    
    if gdb_send(gdb_session, cmd) = false then
'        append_output("ERROR: Failed to send command" + chr(13) + chr(10))
        return ""
    end if
    
    response = gdb_receive(gdb_session, 5000) 
    if response = "" then
'        append_output("ERROR: No response from GDB" + chr(13) + chr(10))
        return ""
    end if
    
'    dim as string console_out = gdb_parse_console_output(response)
'    if len(console_out) > 0 then
'        append_output(console_out)
'    end if
    
    return response
end function


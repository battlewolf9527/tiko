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

#include once "frmDebug.bi"


function gdb_findResultValue( byref response as string , byref token as string ) as string
    ' Add to incoming token:   token="
    dim as string result
    
    token = token & "=" & chr(34)
    dim as integer iStart = instr( response, token )
    if iStart > 0 then
        iStart = iStart + len(token)
        dim as integer iEnd = instr( iStart, response, chr(34) )
        if iEnd > 0 then
            result = mid( response, iStart, iEnd - iStart ) 
        end if
    end if
    
    return result
end function


sub gdb_processResponse( byval response as string ) 
    dim as string file_name 
    dim as integer line_number

    if instr( response, "*stopped,reason=" & chr(34) & "breakpoint-hit" & chr(34)) orelse _
       instr( response, "*stopped,reason=" & chr(34) & "end-stepping-range" & chr(34)) then
        ' Line numbers in GDB are 1 based whereas Scintilla is 0 based.
        line_number = val( gdb_findResultValue( response, "line" ) ) - 1
        file_name = gdb_findResultValue( response, "fullname" )
        file_name = AfxStrReplace( file_name, "\\", "\" )
        file_name = AfxStrReplace( file_name, "/", "\" )

LM( "STOP: BREAKPOINT OR STEPPING" )
LM( "  file_name: " & file_name )
LM( "  line_number: " & str(line_number) )

        if (len(file_name) > 0) andalso (line_number >= 0) then
            dim as clsDocument ptr pDoc = OpenSelectedDocument( file_name, "", line_number)
            if pDoc then
                SciExec( pDoc->hWndActiveScintilla, SCI_MARKERDELETEALL, MARKER_DEBUGGER_CURLINE, 0 )
                pDoc->ToggleMarker( line_number, MARKER_DEBUGGER_CURLINE )
            end if    
        end if
    end if

    
'    '// MSG FIELD (error messages)
'    if gdb_resultExists(result.results(), result.resultCount, "msg") then
'        dim msgValue as string = gdb_findResultValue(result.results(), result.resultCount, "msg")
''LM("  [Lookup] Error message: " + msgValue)
'    end if
        
'    '// EXIT CODE (program exit code)
'    if gdb_resultExists(result.results(), result.resultCount, "exit-code") then
'        dim exitCode as string = gdb_findResultValue(result.results(), result.resultCount, "exit-code")
''LM("  [Lookup] Exit code: " + exitCode)
'        if exitCode = "0" then
''LM("           (Success)")
'        else
''LM("           (Error/Failure)")
'        end if
'    end if
        
'    '// STATUS field (program exit status)
'    if gdb_resultExists(result.results(), result.resultCount, "status") then
'        dim statusValue as string = gdb_findResultValue(result.results(), result.resultCount, "status")
'LM("  [Lookup] Status: " + statusValue)
'    end if
    
end sub


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmDebug
' ========================================================================================
function frmDebug_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT

    
    select case id
        
        case IDC_FRMDEBUG_CMDOK
            if codeNotify = BN_CLICKED then
                SendMessage( hwnd, WM_CLOSE, 0, 0 )
                exit function
            end if
        
        case IDCANCEL
            if codeNotify = BN_CLICKED then
                SendMessage( hwnd, WM_CLOSE, 0, 0 )
                exit function
            end if
    
        case IDC_BTN_RUN
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "run")
            end if
                
        case IDC_BTN_CONTINUE
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "continue")
            end if
        
        case IDC_BTN_BREAK
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "interrupt")
            end if
        
        case IDC_BTN_STOP
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "kill")
          end if
        
        case IDC_BTN_STEP_OVER
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "next")
            end if
        
        case IDC_BTN_STEP_INTO
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "step")
           end if
        
        case IDC_BTN_STEP_OUT
            if codeNotify = BN_CLICKED then
                gdb_send(gdb_session, "finish")
            end if

    end select

    function = 0
end function


' ========================================================================================
' frmDebug Window procedure
' ========================================================================================
function frmDebug_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        HANDLE_MSG (hwnd, WM_COMMAND,  frmDebug_OnCommand)
    
        case WM_CLOSE
            LogClose()
            KillTimer( HWND_FRMDEBUG, gApp.DebugTimerID )
            gdb_close( gdb_session )
            DestroyWindow( hwnd )
            
        case WM_NCDESTROY
            dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
            if pWindow then delete(pWindow)
            gdb_session.KillMessageThread = true
            ThreadWait( gdb_session.hThreadMessages )
            MutexDestroy( gdb_session.hThreadMutex )

            ' Remove any yellow arrow debugger current line marker
            dim pDoc as clsDocument ptr = gApp.pDocList
            do until pDoc = 0
                SciExec( pDoc->hWindow(0), SCI_MARKERDELETEALL, MARKER_DEBUGGER_CURLINE, 0 )
                pDoc = pDoc->pDocNext
            loop
        
        case WM_TIMER
            if wParam = gApp.DebugTimerID then
                ' Check for any pending message
                if gdb_isMessageQueueEmpty( gdb_session ) = false then
                    MutexLock( gdb_session.hThreadMutex )
                    dim as string message = gdb_dequeue_message( gdb_session )
                    MutexUnLock( gdb_session.hThreadMutex )
                    gdb_processResponse( message )    
                end if
            end if

    end select

    
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


function frmDebug_SetInitialBreakpoints( byref session as GDBSession ) as long
    'break main.c:25                    ' Line 25 in main.c
    'break utils.c:calculateSum         ' Function in utils.c
    'break "my file.c":10              ' File with spaces (use quotes)
    'break C:\path\to\file.c:42        ' Full path with line
    'break src/module.c:init           ' Relative path

    'tbreak main.c:50                  ' Temporary breakpoint
    'break main.c:25 if x > 10         ' Conditional breakpoint in file
    'rbreak main.c:.*                  ' Regex breakpoint (all functions in file)
    'info line main.c:25               ' Show address of line
    'list main.c:25                    ' Show source at line
    ' Iterate all pDoc in the project/files list
    dim as string sBreakpoints
    dim as string bp_cmd
    dim as string bp_file
    dim as string bp_line
    dim as long   nCount = 0
    
    dim pDoc as clsDocument ptr = gApp.pDocList
    do until pDoc = 0
        if pDoc->hWindow(0) <> null then 
            sBreakpoints = pDoc->GetBreakpoints()
        end if
        
        bp_file = pDoc->DiskFilename

        if len(sBreakpoints) then
            nCount = AfxStrParseCount( sBreakpoints, "," )
            for i as long = 1 to nCount
                ' Line numbers in GDB are 1 based whereas Scintilla is 0 based.
                bp_line = AfxStrParse( sBreakpoints, i, "," )
                bp_cmd = bp_cmd & "break " & chr(34) & bp_file & chr(34) & ":" & str(val(bp_line)+1) & chr(10)
            next
        end if
            
        pDoc = pDoc->pDocNext
    loop

LM( "Set Breakpoints:")
LM( bp_cmd & chr(13,10) )

    bp_cmd = rtrim(bp_cmd, chr(10))
    gdb_send( gdb_session, bp_cmd )

    function = 0    
end function
        
' ========================================================================================
' frmDebug_Show
' ========================================================================================
function frmDebug_Show( byval hWndParent as HWND, byval executable as string ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMDEBUG = pWindow->Create( hWndParent, "DEBUGGER", @frmDebug_WndProc, 0, 0, 0, 0, _
        WS_POPUP or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_DLGMODALFRAME or WS_EX_CONTROLPARENT or WS_EX_LEFT )
    pWindow->SetClientSize(625, 460)
    pWindow->Center(pWindow->hWindow, hWndParent)
    
    pWindow->AddControl("BUTTON", , IDC_BTN_RUN, "RUN", 5, 5, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_BTN_CONTINUE, "CONTINUE", 82, 5, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_BTN_BREAK, "BREAK", 159, 5, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_BTN_STOP, "STOP", 236, 5, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_BTN_STEP_OVER, "STEP OVER", 5, 35, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_BTN_STEP_INTO, "STEP INTO", 82, 35, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_BTN_STEP_OUT, "STEP OUT", 159, 35, 75, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_FRMDEBUG_CMDOK, L(0,"&OK"), 450, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_DEFPUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    pWindow->AddControl("BUTTON", , IDCANCEL, L(1,"&Cancel"), 535, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    LogInit( "c:\dev\gdb_interface\_debug.txt" )
    
    SetTimer(HWND_FRMDEBUG, gApp.DebugTimerID, 20, null)   ' 20 ms
    
    ' The debug executable has been sucessfully created so start the debug thread
    ' add feed any breakpoints into it.
    if gdb_init( gdb_session, executable ) then
        frmDebug_SetInitialBreakpoints( gdb_session )
        gdb_send(gdb_session, "set confirm off" )
        gdb_send(gdb_session, "set language pascal" )
        
        gdb_session.hThreadMutex = MutexCreate()
        gdb_session.hThreadMessages = ThreadCreate( @gdb_threadListener, @gdb_session )
        
    else
        MessageBox( HWND_FRMMAIN, "Failed to initialize GDB. Ensure gdb.exe is installed correctly.", _
                    "Error", MB_OK or MB_ICONERROR)
        DestroyWindow( HWND_FRMMAIN )
        exit function            
    end if
    
    
    ShowWindow( HWND_FRMDEBUG, SW_SHOW )
    
    return 0
end function






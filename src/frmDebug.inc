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


function frmDebug_PositionEditor( byval response as string ) as boolean
    dim as string file_name = gdb_parse_current_filename(response)
    dim as integer line_number = gdb_parse_breakpoint_linenumber(response)
                        
    LM("Filename: " + file_name)
    LM("line_number: " + str(line_number)) 
    
    dim as clsDocument ptr pDoc = OpenSelectedDocument( file_name, "", line_number )

    function = iif( pDoc > 0, true, false )
end function


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmDebug
' ========================================================================================
function frmDebug_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT

    dim as string response
    
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
'                update_status("Running program...")
    LM("Run...")
                if send_gdb_command("run", response) then

LM("RUN RESPONSE " & response)

                    if gdb_at_breakpoint(response) then
    LM(chr(13,10) + "*** BREAKPOINT HIT ***")
                        frmDebug_PositionEditor( response )

'    update_status("Stopped at breakpoint")
     '                   send_gdb_command("info locals", response)
                    elseif gdb_has_exited(response) then
                        dim as integer exit_code = gdb_parse_exit_code(response)
    LM(chr(13) + chr(10) + "*** PROGRAM EXITED ***" + chr(13) + chr(10))
    LM("Exit code: " + str(exit_code) + chr(13) + chr(10))
                    else
'                        update_status("Program running")
                    end if
                end if
            end if
                
        case IDC_BTN_CONTINUE
            if codeNotify = BN_CLICKED then
                if send_gdb_command("continue", response) then
LM("CONTINUE RESPONSE " & response)

                    if gdb_at_breakpoint(response) then
    LM(chr(13) + chr(10) + "*** BREAKPOINT HIT ***")
                        frmDebug_PositionEditor( response )

      '                  update_status("Stopped at breakpoint")
                    elseif gdb_has_exited(response) then
                        dim as integer exit_code = gdb_parse_exit_code(response)
    LM(chr(13) + chr(10) + "*** PROGRAM EXITED ***" + chr(13) + chr(10))
      '                  update_status("Program exited")
                    end if
                end if
            end if
        
        case IDC_BTN_BREAK
            if codeNotify = BN_CLICKED then
'                update_status("Breaking...")
LM("Interrupt")
                send_gdb_command("interrupt", response)
 '               update_status("Interrupted")
            end if
        
        case IDC_BTN_STOP
            if codeNotify = BN_CLICKED then
   '             update_status("Stopping...")
LM("Kill")
                send_gdb_command("kill", response)
   '             update_status("Stopped")
          end if
        
        case IDC_BTN_STEP_OVER
            if codeNotify = BN_CLICKED then
'                update_status("Stepping over...")
LM("Next")
                send_gdb_command("next", response)
                frmDebug_PositionEditor( response )
'                update_status("Ready")
            end if
        
        case IDC_BTN_STEP_INTO
            if codeNotify = BN_CLICKED then
'                update_status("Stepping into...")
LM("Step Into")
                send_gdb_command("step", response)
                frmDebug_PositionEditor( response )
 '               update_status("Ready")
           end if
        
        case IDC_BTN_STEP_OUT
            if codeNotify = BN_CLICKED then
'                update_status("Stepping out...")
LM("Step Out")
                send_gdb_command("finish", response)
                frmDebug_PositionEditor( response )
'                update_status("Ready")
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
            DestroyWindow( hwnd )
            
        case WM_NCDESTROY
            dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
            if pWindow then delete(pWindow)
            
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
    dim as long nCount = 0
    
    dim pDoc as clsDocument ptr = gApp.pDocList
    do until pDoc = 0
        if pDoc->hWindow(0) <> null then 
            sBreakpoints = pDoc->GetBreakpoints()
        end if
        
        bp_file = pDoc->DiskFilename

        if len(sBreakpoints) then
            nCount = AfxStrParseCount( sBreakpoints, "," )
            for i as long = 1 to nCount
                bp_line = AfxStrParse( sBreakpoints, i, "," )
                bp_cmd = bp_cmd & "break " & chr(34) & bp_file & chr(34) & ":" & bp_line & chr(10)
            next
        end if
            
        pDoc = pDoc->pDocNext
    loop

    bp_cmd = rtrim(bp_cmd, chr(10))
    send_gdb_command( bp_cmd, "" )

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
    
    ' The debug executable has been sucessfully created so start the debug thread
    ' add feed any breakpoints into it.
    if gdb_init( gdb_session, executable ) then
        frmDebug_SetInitialBreakpoints( gdb_session )
        gdb_send(gdb_session, "set confirm off" )
        gdb_send(gdb_session, "set language pascal" )
    else
        MessageBox( HWND_FRMMAIN, "Failed to initialize GDB. Ensure gdb.exe is installed correctly.", _
                    "Error", MB_OK or MB_ICONERROR)
        DestroyWindow( HWND_FRMMAIN )
        exit function            
    end if
    
    
    ShowWindow( HWND_FRMDEBUG, SW_SHOW )
    
    return 0
end function






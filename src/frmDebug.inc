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

    dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
    dim as GDBThreadData ptr gdb_data 
    if pWindow then gdb_data = cast(GDBThreadData ptr, pWindow->UserData(0))

    select case uMsg
        HANDLE_MSG (hwnd, WM_COMMAND,  frmDebug_OnCommand)
    
        case WM_CLOSE
            DestroyWindow( hwnd )
            
        case WM_NCDESTROY
            if pWindow then 
                delete(gdb_data)
                delete(pWindow)
            end if
            
        case WM_DESTROY
            if pWindow then 
                if (gdb_data <> 0) andalso (gdb_data->session.initialized = true) then 
                    gdb_thread_close(gdb_data)
                end if
            end if

        case MSG_USER_GDB_RESPONSE
            if gdb_data = 0 then exit function
            dim as string response
            dim as string console_out
            if gdb_thread_get_response(gdb_data, response) then
                console_out = gdb_parse_console_output(response)
print "WM_GDB_RESPONSE"
print "Response: "; response
print "console_out: "; console_out
                if len(console_out) > 0 then
'                    append_output(console_out)
                end if
            end if

      
        case MSG_USER_GDB_BREAKPOINT_HIT
print "WM_GDB_BREAKPOINT_HIT"
            dim as string response
            if gdb_thread_get_response(gdb_data, response) then
                dim as string  reason = gdb_parse_stop_reason(response)
                dim as integer bp_num = gdb_parse_breakpoint_number(response)
                dim as string  file_name = gdb_parse_current_file(response)
                dim as string  func_name = gdb_parse_current_function(response)
                dim as integer line_num = gdb_parse_current_line(response)
print "file_name: "; file_name
print "func_name: "; func_name
print "line_num: "; line_num
                OpenSelectedDocument( file_name, func_name, line_num )
            end if

            
        case MSG_USER_GDB_ERROR
        case MSG_USER_GDB_READY
        case MSG_USER_GDB_STOPPED
        case MSG_USER_GDB_RUNNING
        case MSG_USER_GDB_EXITED


    end select

    
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


function frmDebug_SetInitialBreakpoints( byval gdb_data as GDBThreadData ptr ) as long
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
                bp_cmd = "break " & chr(34) & bp_file & chr(34) & ":" & bp_line
                gdb_thread_send( gdb_data, bp_cmd )
            next
        end if
            
        pDoc = pDoc->pDocNext
    loop

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
    
    ' Create a pointer to the thread data structure and store it in the created window
    ' This manually created struture is deallocated in WM_NCDESTROY.
    dim as GDBThreadData ptr gdb_data = new GDBThreadData
    pWindow->UserData(0) = cast(LONG_PTR, gdb_data)
        
    pWindow->AddControl("BUTTON", , IDC_FRMDEBUG_CMDOK, L(0,"&OK"), 450, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_DEFPUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("BUTTON", , IDC_FRMDEBUG_CMDOK, L(0,"&OK"), 450, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_DEFPUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    pWindow->AddControl("BUTTON", , IDCANCEL, L(1,"&Cancel"), 535, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    
    ' The debug executable has been sucessfully created so start the debug thread
    ' add feed any breakpoints into it.
    if gdb_thread_init( gdb_data, HWND_FRMDEBUG, executable ) then
        sleep 500
        gdb_thread_send( gdb_data, "set confirm off" )
        gdb_thread_send( gdb_data, "set language pascal" )
        frmDebug_SetInitialBreakpoints( gdb_data )
    else
        MessageBox( HWND_FRMMAIN, "Failed to initialize GDB. Ensure gdb.exe is installed correctly.", _
                    "Error", MB_OK or MB_ICONERROR)
        DestroyWindow( HWND_FRMMAIN )
        exit function            
    end if
    
    
    ShowWindow( HWND_FRMDEBUG, SW_SHOW )
    
    return 0
end function






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

    select case uMsg
        HANDLE_MSG (hwnd, WM_COMMAND,  frmDebug_OnCommand)
    
        case WM_CLOSE
            DestroyWindow( hwnd )
            
        case WM_NCDESTROY
            dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
            if pWindow then delete(pWindow)

    end select

    
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' frmDebug_Show
' ========================================================================================
function frmDebug_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMDEBUG = pWindow->Create( hWndParent, "DEBUGGER", @frmDebug_WndProc, 0, 0, 0, 0, _
        WS_POPUP or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_DLGMODALFRAME or WS_EX_CONTROLPARENT or WS_EX_LEFT )
    pWindow->SetClientSize(625, 460)
    pWindow->Center(pWindow->hWindow, hWndParent)
        
    pWindow->AddControl("BUTTON", , IDC_FRMDEBUG_CMDOK, L(0,"&OK"), 450, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_DEFPUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    pWindow->AddControl("BUTTON", , IDCANCEL, L(1,"&Cancel"), 535, 417, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    ShowWindow( HWND_FRMDEBUG, SW_SHOW )
    
    ' Process Windows messages
'    function = pWindow->DoEvents(SW_SHOW)
    
    return 0
end function






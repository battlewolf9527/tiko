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

#include once "frmCommandLine.bi"


' ========================================================================================
' Process WM_CREATE message for window/dialog: frmCommandLine
' ========================================================================================
function frmCommandLine_OnCreate( _
            byval hwnd as HWND, _
            byval lpCreateStructPtr as LPCREATESTRUCT _
            ) as boolean

    ' This is a modal popup window so disable the parent window
    DisableAllModeless()

    '  Message cracker macro expects a True to be returned for a successful
    '  OnCreate handler even though returning -1 from a standard WM_CREATE
    '  call would stop creating the window. This is just one of those Windows
    '  inconsistencies.
    return true
end function


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmCommandLine
' ========================================================================================
function frmCommandLine_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT

    select case id
        case IDOK
            if codeNotify = BN_CLICKED then
                ' A commandline parameter is not saved for non-project files. These are
                ' set at the time when the user opens the file. For project related files the
                ' command line is saved to the project file.
                if gApp.IsProjectActive then
                    gApp.ProjectCommandLine = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMCOMMANDLINE_TEXTBOX1) )
                else
                    gApp.wszCommandLine = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMCOMMANDLINE_TEXTBOX1) )
                end if
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
' Process WM_CLOSE message for window/dialog: frmCommandLine
' ========================================================================================
function frmCommandLine_OnClose( byval hwnd as HWND ) as LRESULT
    ' Enables parent window keeping parent's zorder
    EnableAllModeless()
    DestroyWindow hwnd
    function = 0
end function


' ========================================================================================
' Process WM_DESTROY message for window/dialog: frmCommandLine
' ========================================================================================
function frmCommandLine_OnDestroy( byval hwnd as HWND ) as LRESULT
    PostQuitMessage(0)
    function = 0
end function


' ========================================================================================
' frmCommandLine Window procedure
' ========================================================================================
function frmCommandLine_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    Select case uMsg
        HANDLE_MSG (hwnd, WM_CREATE,   frmCommandLine_OnCreate)
        HANDLE_MSG (hwnd, WM_CLOSE,    frmCommandLine_OnClose)
        HANDLE_MSG (hwnd, WM_DESTROY,  frmCommandLine_OnDestroy)
        HANDLE_MSG (hwnd, WM_COMMAND,  frmCommandLine_OnCommand)
    End Select

    function = DefWindowProcW(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' frmCommandLine_Show
' ========================================================================================
function frmCommandLine_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    pWindow->Create( hWndParent, L(178,"Command Line"), @frmCommandLine_WndProc, 0, 0, 0, 0, _
        WS_POPUP or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_DLGMODALFRAME or WS_EX_CONTROLPARENT or WS_EX_LEFT)
    pWindow->SetClientSize(420, 100)
    pWindow->Center(pWindow->hWindow, hWndParent)
        
    pWindow->AddControl("LABEL", , IDC_FRMCOMMANDLINE_LABEL1, L(179,"Enter command line arguments:"), 10, 10, 400, 20, _
        WS_CHILD or WS_VISIBLE or SS_LEFT, WS_EX_LEFT or WS_EX_LTRREADING)

    dim as HWND hTextBox = _
    pWindow->AddControl("TEXTBOX", , IDC_FRMCOMMANDLINE_TEXTBOX1, "", 10, 33, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("BUTTON", , IDOK, L(0,"&OK"), 252, 63, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_DEFPUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    pWindow->AddControl("BUTTON", , IDCANCEL, L(1,"&Cancel"), 336, 63, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    if gApp.IsProjectActive then
        AfxSetWindowText( hTextBox, gApp.ProjectCommandLine )
    else   
        AfxSetWindowText( hTextBox, gApp.wszCommandLine )
    end if
    
    SetFocus hTextBox
    
    ' Process Windows messages
    function = pWindow->DoEvents(SW_SHOW)
    
    ' Delete the frmCommandLine CWindow class manually allocated memory 
    if pWindow then delete(pWindow)

end function

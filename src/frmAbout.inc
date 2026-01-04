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

#include once "frmAbout.bi"

' ========================================================================================
' Process WM_CREATE message for window/dialog: frmAbout
' ========================================================================================
function frmAbout_OnCreate( _
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
' Process WM_CLOSE message for window/dialog: frmAbout
' ========================================================================================
function frmAbout_OnClose( byval hwnd as HWND) as LRESULT
    ' Enables parent window keeping parent's zorder
    EnableAllModeless()
    DestroyWindow hwnd
    function = 0
end function


' ========================================================================================
' Process WM_DESTROY message for window/dialog: frmAbout
' ========================================================================================
function frmAbout_OnDestroy( byval hwnd as HWND) as LRESULT
    Dim As HFONT hFont = AfxGetWindowFont(GetDlgItem(hwnd, IDC_FRMABOUT_LBLAPPNAME))
    HWND_FRMABOUT = 0
    PostQuitMessage(0)
    function = 0
end function


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmAbout
' ========================================================================================
function frmAbout_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify As UINT _
            ) as LRESULT

    select case id
        case IDCANCEL  ' Close
            if codeNotify = BN_CLICKED then
                SendMessage( HWnd, WM_CLOSE, 0, 0)
                exit function
            end if
    end select
    
    function = 0
end function


' ========================================================================================
' frmAbout Window procedure
' ========================================================================================
function frmAbout_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        HANDLE_MSG (hwnd, WM_CREATE,  frmAbout_OnCreate)
        HANDLE_MSG (hwnd, WM_CLOSE,   frmAbout_OnClose)
        HANDLE_MSG (hwnd, WM_DESTROY, frmAbout_OnDestroy)
        HANDLE_MSG (hwnd, WM_COMMAND, frmAbout_OnCommand)
    end select

    ' for messages that we don't deal with
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)
end function


' ========================================================================================
' frmAbout_Show
' ========================================================================================
function frmAbout_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMABOUT = pWindow->Create( hWndParent, L(74,"About"), @frmAbout_WndProc, 0, 0, 0, 0, _
        WS_POPUP or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_DLGMODALFRAME or WS_EX_CONTROLPARENT or WS_EX_LEFT )
    pWindow->SetClientSize(580, 280)
    pWindow->Center(pWindow->hWindow, hWndParent)
        
    ' Add an image control                                                         
    dim pImageCtx as CImageCtx = CImageCtx(pWindow, IDC_FRMABOUT_IMAGE1, , 60, 50, 100, 100)
    pImageCtx.LoadImageFromResource( pWindow->InstanceHandle, "IMAGE_MAINICON" )

    dim as HWND hLabel = _
        pWindow->AddControl("LABEL", , IDC_FRMABOUT_LBLAPPNAME, APPNAME, 200, 40, 400, 30, _
            WS_CHILD or WS_VISIBLE or SS_LEFT or SS_CENTERIMAGE or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)
    Dim As HFONT hFont = pWindow->CreateFont("", 12, FW_BOLD)
    AfxSetWindowFont hLabel, hFont, True

    pWindow->AddControl("LABEL", , IDC_FRMABOUT_LBLAPPVERSION, "Version " & APPVERSION & APPBITS, 200, 71, 200, 20, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("LABEL", , IDC_FRMABOUT_LBLAPPCOPYRIGHT, APPCOPYRIGHT, 200, 115, 370, 20, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("LABEL", , IDC_FRMABOUT_LBLJOSE, "Special thanks to: José Roca (WinFBX Framework)", _
        200, 143, 300, 18, WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("LABEL", , IDC_FRMABOUT_LBLOTHERS, "Paul Doe and ImortisInglorian for ideas and testing", _
        200, 159, 350, 18, WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)

    ' Process Windows messages
    function = pWindow->DoEvents(SW_SHOW)
    
    ' Delete the frmAbout CWindow class manually allocated memory 
    if pWindow then delete(pWindow)

end function


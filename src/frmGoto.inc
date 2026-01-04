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

#include once "frmGoto.bi"


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmGoto
' ========================================================================================
function frmGoto_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT

    select case id
        case IDOK
            If codeNotify = BN_CLICKED then
                dim nLine as long
                dim swzText as wstring * MAX_PATH
                swzText = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMGOTO_TXTLINE) )
                nLine = val(swzText) - 1
                If nLine >= 0 then
                    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
                    if pDoc then 
                        dim as HWND hEdit = pDoc->hWndActiveScintilla
                        SciExec( hEdit, SCI_GOTOLINE, nLine, 0 )
                        pDoc->CenterCurrentLine
                    end if
                end if
                SendMessage hwnd, WM_CLOSE, 0, 0
                exit function
            End If
            
        case IDCANCEL
            if codeNotify = BN_CLICKED then
                SendMessage hwnd, WM_CLOSE, 0, 0
                exit function
            end if
    end select

    function = 0
end function


' ========================================================================================
' Process WM_CLOSE message for window/dialog: frmGoto
' ========================================================================================
function frmGoto_OnClose( byval hwnd as HWND ) as LRESULT
    ' Enables parent window keeping parent's zorder
    EnableAllModeless()
    DestroyWindow hwnd
    function = 0
end function


' ========================================================================================
' Process WM_DESTROY message for window/dialog: frmGoto
' ========================================================================================
function frmGoto_OnDestroy( byval hwnd as HWND ) as LRESULT
    PostQuitMessage(0)
    function = 0
end function


' ========================================================================================
' Process WM_CREATE message for window/dialog: frmGoto
' ========================================================================================
function frmGoto_OnCreate( _
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
' frmGoto Window procedure
' ========================================================================================
function frmGoto_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT
    Select case uMsg
        HANDLE_MSG (hwnd, WM_CREATE,   frmGoto_OnCreate)
        HANDLE_MSG (hwnd, WM_CLOSE,    frmGoto_OnClose)
        HANDLE_MSG (hwnd, WM_DESTROY,  frmGoto_OnDestroy)
        HANDLE_MSG (hwnd, WM_COMMAND,  frmGoto_OnCommand)
    End Select

    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' frmGoto_Show
' ========================================================================================
function frmGoto_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow
    pWindow->ClassStyle = CS_DROPSHADOW

    pWindow->Create(hWndParent, L(168,"Go to Line"), @frmGoto_WndProc, 0, 0, 287, 126, _
        WS_POPUP or WS_DLGFRAME or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_DLGMODALFRAME or WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->SetClientSize(281, 97)
    pWindow->Center
    
    pWindow->AddControl("TEXTBOX", pWindow->hWindow, IDC_FRMGOTO_TXTLINE, "", 105, 56, 67, 23, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->AddControl("BUTTON", pWindow->hWindow, IDOK, L(0,"OK"), 186, 14, 78, 30, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_DEFPUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("BUTTON", pWindow->hWindow, IDCANCEL, L(1,"Cancel"), 186, 51, 78, 30, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("LABEL", pWindow->hWindow, IDC_FRMGOTO_LBLCURRENTLINE, L(170,"Current line") & ":", 15, 15, 81, 17, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_RIGHT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("LABEL", pWindow->hWindow, IDC_FRMGOTO_LBLLASTLINE, L(169,"Last line") & ":", 15, 36, 81, 18, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_RIGHT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("LABEL", pWindow->hWindow, IDC_FRMGOTO_LBLGOTOLINE, L(171,"Go to line") & ":", 15, 59, 81, 20, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_RIGHT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("LABEL", pWindow->hWindow, IDC_FRMGOTO_LBLCURRENTVALUE, "", 107, 15, 65, 17, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_RIGHT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("LABEL", pWindow->hWindow, IDC_FRMGOTO_LBLLASTVALUE, "", 107, 36, 65, 18, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_RIGHT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    dim as long curPos, nLine, nLines
    
    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
    If pDoc then
        dim as HWND hEdit = pDoc->hWndActiveScintilla
        curPos  = SciExec(hEdit, SCI_GETCURRENTPOS, 0, 0)
        nLine   = SciExec(hEdit, SCI_LINEFROMPOSITION, curPos, 0) + 1
        nLines  = SciExec(hEdit, SCI_GETLINECOUNT, 0, 0)
        SetWindowText( GetDlgItem(pWindow->hWindow, IDC_FRMGOTO_LBLCURRENTVALUE), wstr(nLine) )
        SetWindowText( GetDlgItem(pWindow->hWindow, IDC_FRMGOTO_LBLLASTVALUE), wstr(nLines) )
    End If
        
    ShowWindow pWindow->hWindow, SW_SHOW
    SetFocus GetDlgItem(pWindow->hWindow, IDC_FRMGOTO_TXTLINE)

    ' Process Windows messages
    function = pWindow->DoEvents(SW_SHOW)
    
    ' Delete the frmGoto CWindow class manually allocated memory 
    if pWindow then delete(pWindow)

end function


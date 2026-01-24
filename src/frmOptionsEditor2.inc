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

#include "frmOptionsEditor2.bi"
#include "clsConfig.bi"


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmOptions
' ========================================================================================
function frmOptionsEditor2_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as hwnd, _
            byval codeNotify as UINT _
            ) as LRESULT

    select case id
        
    case IDC_FRMOPTIONSEDITOR2_CHKAUTOINDENTATION
        if codeNotify = BN_CLICKED then
            if Button_GetCheck( GetDlgItem(hwnd, IDC_FRMOPTIONSEDITOR2_CHKAUTOINDENTATION) ) then
                EnableWindow( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKFORNEXTVARIABLE), true )
            else
                EnableWindow( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKFORNEXTVARIABLE), false )
            end if   
        end if
    
    end select      

    function = 0
end function


' ========================================================================================
' frmOptionsEditor2 Window procedure
' ========================================================================================
function frmOptionsEditor2_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        HANDLE_MSG (hwnd, WM_COMMAND,  frmOptionsEditor2_OnCommand)
    end select

    ' for messages that we don't deal with
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' frmOptionsEditor2_Show
' ========================================================================================
function frmOptionsEditor2_Show( byval hwndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMOPTIONSEDITOR2 = pWindow->Create( hwndParent, "", @frmOptionsEditor2_WndProc, 0, 0, 0, 0, _
        WS_CHILD or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    ' Height and width of this child form is set in frmOptions_OnNotify when the treeview option is selected.
    
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKCODETIPS, L(115,"Enable CodeTips"), 0, 0, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKAUTOCOMPLETE, L(95,"Enable Autocomplete"), 0, 21, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKCHARAUTOCOMPLETE, L(417,"Enable Character Autocompletion"), 0, 42, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKAUTOINDENTATION, L(120,"Enable Auto Indentation"), 0, 63, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKFORNEXTVARIABLE, L(416,"Append loop variable to For/Next statement"), 30, 84, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKBRACEHIGHLIGHT, L(412,"Enable Brace Highlighting"), 0, 105, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR2_CHKOCCURRENCEHIGHLIGHT, L(419,"Enable Occurrences Highlighting"), 0, 126, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    Dim as hwnd hwnd = HWND_FRMOPTIONSEDITOR2
    
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKCODETIPS),            gConfig.CodeTips)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKAUTOCOMPLETE),        gConfig.AutoComplete)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKCHARAUTOCOMPLETE),    gConfig.CharacterAutoComplete)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKAUTOINDENTATION),     gConfig.AutoIndentation)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKFORNEXTVARIABLE),     gConfig.ForNextVariable)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKBRACEHIGHLIGHT),      gConfig.BraceHighlight)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKOCCURRENCEHIGHLIGHT), gConfig.OccurrenceHighlight)

    if Button_GetCheck( GetDlgItem(hwnd, IDC_FRMOPTIONSEDITOR2_CHKAUTOINDENTATION) ) then
        EnableWindow( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKFORNEXTVARIABLE), true )
    else
        EnableWindow( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR2_CHKFORNEXTVARIABLE), false )
    end if   

    function = 0
    
end function




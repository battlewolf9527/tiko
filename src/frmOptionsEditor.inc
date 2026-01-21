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

#include "frmOptionsEditor.bi"
#include "clsConfig.bi"


' ========================================================================================
' frmOptionsEditor Window procedure
' ========================================================================================
function frmOptionsEditor_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    ' for messages that we don't deal with
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' frmOptionsEditor_Show
' ========================================================================================
function frmOptionsEditor_Show( byval hwndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMOPTIONSEDITOR = pWindow->Create( hwndParent, "", @frmOptionsEditor_WndProc, 0, 0, 0, 0, _
        WS_CHILD or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    ' Height and width of this child form is set in frmOptions_OnNotify when the treeview option is selected.
    
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKSYNTAXHIGHLIGHTING, L(114,"Syntax highlighting"), 0, 0, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKCONFINECARET, L(116,"Confine caret to text"), 0, 21, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKCURRENTLINE, L(117,"Highlight current line"), 0, 42, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKTABTOSPACES, L(118,"Treat Tab as spaces"), 0, 63, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKSHOWLEFTMARGIN, L(122,"Show left margin"), 0, 84, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKSHOWFOLDMARGIN, L(123,"Show fold margin"), 0, 105, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKLINENUMBERING, L(124,"Show line numbers"), 0, 126, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKINDENTGUIDES, L(125,"Show Indentation guides"), 0, 147, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKSHOWRIGHTEDGE, L(306,"Show right edge column"), 0, 168, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSEDITOR_TXTRIGHTEDGE, "", 50, 189, 30, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL or ES_NUMBER, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSEDITOR_LBLRIGHTEDGE, L(307,"Position"), 85, 192, 100, 17, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
        
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKPOSITIONMIDDLE, L(407,"Position searches to middle of screen"), 0, 213, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSEDITOR_CHKTOGGLEBREAKPOINT, L(414,"Margin click toggles Breakpoint rather than Bookmark"), 0, 234, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSEDITOR_LBLTABSIZE, L(127,"Tab Size"), _
        0, 305, 100, 17, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSEDITOR_TXTTABSIZE, "", _
        0, 325, 30, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL or ES_NUMBER, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSEDITOR_LBLKEYWORDCASE, L(128,"Case"), _
        210, 305, 110, 16, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSEDITOR_COMBOCASE, "", _
        210, 325, 177, 20, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    
    dim as Long idx = 0
    dim as HWND hwnd = HWND_FRMOPTIONSEDITOR
    
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKSYNTAXHIGHLIGHTING), gConfig.SyntaxHighlighting)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKSHOWLEFTMARGIN),     gConfig.LeftMargin)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKSHOWFOLDMARGIN),     gConfig.FoldMargin)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKCONFINECARET),       gConfig.ConfineCaret)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKLINENUMBERING),      gConfig.LineNumbering)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKINDENTGUIDES),       gConfig.IndentGuides)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKCURRENTLINE),        gConfig.HighlightCurrentLine)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKTABTOSPACES),        gConfig.TabIndentSpaces)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKSHOWRIGHTEDGE),      gConfig.RightEdge)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKPOSITIONMIDDLE),     gConfig.PositionMiddle)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_CHKTOGGLEBREAKPOINT),   gConfig.ClickToggleBreakpoint)

    AfxSetWindowText( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_TXTRIGHTEDGE), gConfig.RightEdgePosition )
    AfxSetWindowText( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_TXTTABSIZE), gConfig.TabSize )
    
    ComboBox_AddString( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_COMBOCASE), @L(130,"Lower Case") )
    ComboBox_AddString( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_COMBOCASE), @L(131,"Upper Case") )
    ComboBox_AddString( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_COMBOCASE), @L(272,"Mixed Case") )
    ComboBox_AddString( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_COMBOCASE), @L(132,"Original Case") )
    ComboBox_SetCurSel( GetDlgItem( hwnd, IDC_FRMOPTIONSEDITOR_COMBOCASE), gConfig.KeywordCase )
    
    function = 0
    
end function




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

#include once "frmKeyboard.bi"


' ========================================================================================
' Generate the keybind string based on the selected controls
' ========================================================================================
function frmKeyboardEdit_GenerateKeyBinding() as DWSTRING
    dim as HWND hCtrl
    dim as DWSTRING wszKeys
    dim as DWSTRING wszAccel

    hCtrl = GetDlgItem( HWND_FRMKEYBOARDEDIT, IDC_FRMKEYBOARDEDIT_CHKCTRL )
    if Button_GetCheck(hCtrl) = BST_CHECKED then wszKeys = "Ctrl+"
    hCtrl = GetDlgItem( HWND_FRMKEYBOARDEDIT, IDC_FRMKEYBOARDEDIT_CHKALT )
    if Button_GetCheck(hCtrl) = BST_CHECKED then wszKeys = wszKeys & "Alt+"
    hCtrl = GetDlgItem( HWND_FRMKEYBOARDEDIT, IDC_FRMKEYBOARDEDIT_CHKSHIFT )
    if Button_GetCheck(hCtrl) = BST_CHECKED then wszKeys = wszKeys & "Shift+"

    hCtrl = GetDlgItem( HWND_FRMKEYBOARDEDIT, IDC_FRMKEYBOARDEDIT_COMBOACCEL )
    dim as long nCursel = ComboBox_GetCurSel( hCtrl )
    wszAccel = AfxGetComboBoxText( hCtrl, nCurSel )
    if wszAccel = "None" then 
        wszKeys = ""
    else   
        wszKeys = wszKeys & wszAccel
    end if   

    return wszKeys
end function

' ========================================================================================
' Process WM_CREATE message for window/dialog: frmKeyboardEdit
' ========================================================================================
function frmKeyboardEdit_OnCreate( _
            byval hwnd as HWND, _
            byval lpCreateStructPtr as LPCREATESTRUCT _
            ) as boolean

    ' This is a modal popup window so disable the parent window
    DisableAllModeless()
    EnableWindow( GetParent(hwnd), false )

    '  Message cracker macro expects a True to be returned for a successful
    '  OnCreate handler even though returning -1 from a standard WM_CREATE
    '  call would stop creating the window. This is just one of those Windows
    '  inconsistencies.
    Return True
end function


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmKeyboard
' ========================================================================================
function frmKeyboardEdit_OnCommand( _
            byval hwnd as HWND, _
            byval id as Long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT

    select case id

        case IDC_FRMKEYBOARDEDIT_CHKCTRL, IDC_FRMKEYBOARDEDIT_CHKALT, IDC_FRMKEYBOARDEDIT_CHKSHIFT
            if codeNotify = BN_CLICKED then
                dim as long nCurSel = ListView_GetSelection( HWND_FRMKEYBOARD_LISTVIEW )
                if nCurSel < 0 then exit function
                dim as DWSTRING wszKeys = frmKeyboardEdit_GenerateKeyBinding()
                frmKeyboard_CheckForKeyConflict( wszKeys, nCurSel )
            end if
            
        case IDC_FRMKEYBOARDEDIT_COMBOACCEL
            if codeNotify = CBN_SELCHANGE then
                dim as long nCurSel = ListView_GetSelection( HWND_FRMKEYBOARD_LISTVIEW )
                if nCurSel < 0 then exit function
                dim as DWSTRING wszKeys = frmKeyboardEdit_GenerateKeyBinding()
                frmKeyboard_CheckForKeyConflict( wszKeys, nCurSel )
            end if

        case IDOK
            If codeNotify = BN_CLICKED Then
                ' Update the gKeysEdit entry with the new UserKeys value & Enabled value
                dim as DWSTRING wszKeys = frmKeyboardEdit_GenerateKeyBinding()
                gKeysEdit.wszUserKeys = wszKeys
                dim as HWND hCtrl = GetDlgItem( HWND_FRMKEYBOARDEDIT, IDC_FRMKEYBOARDEDIT_CHKDISABLED )
                gKeysEdit.bDefaultDisabled = iif( Button_GetCheck(hCtrl) = BST_CHECKED, true, false )
                SendMessage( HWnd, WM_CLOSE, 0, 0 )
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
' Process WM_CLOSE message for window/dialog: frmKeyboardEdit
' ========================================================================================
function frmKeyboardEdit_OnClose( byval hwnd as HWND ) as LRESULT
    ' Enables parent window keeping parent's zorder
    EnableAllModeless()
    EnableWindow( GetParent(hwnd), true )
    DestroyWindow( hwnd )
    function = 0
end function


' ========================================================================================
' Process WM_DESTROY message for window/dialog: frmKeyboardEdit
' ========================================================================================
function frmKeyboardEdit_OnDestroy( byval hwnd as HWND) as LRESULT
    HWND_FRMKEYBOARDEDIT = 0
    PostQuitMessage(0)
    function = 0
end function


' ========================================================================================
' frmKeyboard Window procedure
' ========================================================================================
function frmKeyboardEdit_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        HANDLE_MSG (hwnd, WM_CREATE,      frmKeyboardEdit_OnCreate)
        HANDLE_MSG (hwnd, WM_CLOSE,       frmKeyboardEdit_OnClose)
        HANDLE_MSG (hwnd, WM_DESTROY,     frmKeyboardEdit_OnDestroy)
        HANDLE_MSG (hwnd, WM_COMMAND,     frmKeyboardEdit_OnCommand)
    end select

    ' for messages that we don't deal with
    function = DefWindowProc( hwnd, uMsg, wParam, lParam )

end function


' ========================================================================================
' Add shortcuts to the specified combobox.
' (1) Used in this form frmKeyboardEdit
' (2) User Tools
' ========================================================================================
function AddShortcutsToComboBox( byval hCombo as HWND ) as long
    dim as DWSTRING wszText

    ComboBox_AddString( hCombo, @wstr("None") ) 
    ComboBox_AddString( hCombo, @wstr("Backspace") ) 
    ComboBox_AddString( hCombo, @wstr("Tab") ) 
    ComboBox_AddString( hCombo, @wstr("Enter") ) 
    ComboBox_AddString( hCombo, @wstr("Escape") ) 
    ComboBox_AddString( hCombo, @wstr("Spacebar") ) 
    ComboBox_AddString( hCombo, @wstr("PageUp") ) 
    ComboBox_AddString( hCombo, @wstr("PageDn") ) 
    ComboBox_AddString( hCombo, @wstr("End") ) 
    ComboBox_AddString( hCombo, @wstr("Home") ) 
    ComboBox_AddString( hCombo, @wstr("Left") ) 
    ComboBox_AddString( hCombo, @wstr("Right") ) 
    ComboBox_AddString( hCombo, @wstr("Up") ) 
    ComboBox_AddString( hCombo, @wstr("Down") ) 
    ComboBox_AddString( hCombo, @wstr("DEL") ) 
    ComboBox_AddString( hCombo, @wstr("INS") ) 
    for i as long = 48 to 57    ' 0 to 9
        wszText = wchr(i)
        ComboBox_AddString( hCombo, wszText.sptr ) 
    next
    for i as long = 65 to 90    ' A to Z
        wszText = wchr(i)
        ComboBox_AddString( hCombo, wszText.sptr ) 
    next
    ComboBox_AddString( hCombo, @wstr("Numpad0") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad1") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad2") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad3") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad4") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad5") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad6") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad7") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad8") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad9") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad*") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad+") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad-") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad.") ) 
    ComboBox_AddString( hCombo, @wstr("Numpad/") ) 
    ComboBox_AddString( hCombo, @wstr("F1") ) 
    ComboBox_AddString( hCombo, @wstr("F2") ) 
    ComboBox_AddString( hCombo, @wstr("F3") ) 
    ComboBox_AddString( hCombo, @wstr("F4") ) 
    ComboBox_AddString( hCombo, @wstr("F5") ) 
    ComboBox_AddString( hCombo, @wstr("F6") ) 
    ComboBox_AddString( hCombo, @wstr("F7") ) 
    ComboBox_AddString( hCombo, @wstr("F8") ) 
    ComboBox_AddString( hCombo, @wstr("F9") ) 
    ComboBox_AddString( hCombo, @wstr("F10") ) 
    ComboBox_AddString( hCombo, @wstr("F11") ) 
    ComboBox_AddString( hCombo, @wstr("F12") ) 
    ComboBox_AddString( hCombo, @wstr("Tilde") ) 
    ComboBox_AddString( hCombo, @wstr("Plus") ) 
    ComboBox_AddString( hCombo, @wstr("Comma") ) 
    ComboBox_AddString( hCombo, @wstr("Minus") ) 
    ComboBox_AddString( hCombo, @wstr("Period") ) 
    ComboBox_AddString( hCombo, @wstr("SingleQuote") ) 
    ComboBox_AddString( hCombo, @wstr("SemiColon") ) 
    ComboBox_AddString( hCombo, @wstr("/") ) 
    ComboBox_AddString( hCombo, @wstr("\") ) 
    ComboBox_AddString( hCombo, @wstr("[") ) 
    ComboBox_AddString( hCombo, @wstr("]") ) 

    function = 0
end function



' ========================================================================================
' frmKeyboardEdit_Show
' ========================================================================================
function frmKeyboardEdit_Show( byval hWndParent as HWND ) as LRESULT

    dim hCtrl as HWND
    dim as DWSTRING wszText
    
    ' Get the components of the current user key binding
    dim as DWSTRING wszKeys = gKeysEdit.wszUserKeys
    dim as long nCount = AfxStrParseCount( wszKeys, "+" )
    dim as DWSTRING wszAccel
    dim as long isAlt = BST_UNCHECKED
    dim as long isShift = BST_UNCHECKED
    dim as long isCtrl = BST_UNCHECKED
    dim as long isDisabled = BST_UNCHECKED
    
    if gKeysEdit.bDefaultDisabled then
        isDisabled = BST_CHECKED
    end if   
    
    for ii as long = nCount to 1 step -1
        ' The accelerator key will always be the last parse regardless of 
        ' number of items in the string
        dim as DWSTRING wszString = AfxStrParse( wszKeys, ii, "+" )
        if ii = nCount then
            wszAccel = wszString
        else
            select case ucase(wszString)
                case "CTRL": isCtrl = BST_CHECKED
                case "SHIFT": isShift = BST_CHECKED
                case "ALT": isAlt = BST_CHECKED
            end select
        end if
    next
    
    
    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow
    pWindow->DPI = AfxCWindowOwnerPtr(hwndParent)->DPI

    HWND_FRMKEYBOARDEDIT = _
    pWindow->Create(hWndParent, L(220,"Keyboard Shortcuts"), _
        @frmKeyboardEdit_WndProc, 0, 0, 0, 0, _
        WS_POPUP or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_DLGMODALFRAME or WS_EX_CONTROLPARENT or WS_EX_LEFT )
    pWindow->SetClientSize( 330, 220 )
    pWindow->Center( pWindow->hWindow, hWndParent )

    hCtrl = pWindow->AddControl("LABEL", , IDC_FRMKEYBOARDEDIT_LABEL1, "", 20, 10, 350, 20, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    AfxSetWindowText( hCtrl, gKeysEdit.wszDescription )

    hCtrl = pWindow->AddControl("LABEL", , IDC_FRMKEYBOARDEDIT_LABEL2, "", 20, 30, 350, 20, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    wszText = gKeysEdit.wszDefaultKeys
    if len(wszText) = 0 then wszText = "(no key binding)"
    AfxSetWindowText( hCtrl, "Default: " & wszText  )

    hCtrl = pWindow->AddControl("CHECKBOX", , IDC_FRMKEYBOARDEDIT_CHKDISABLED, "Disable Default", 20, 50, 150, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    Button_SetCheck( hCtrl, isDisabled )

    hCtrl = pWindow->AddControl("LABEL", , IDC_FRMKEYBOARDEDIT_LABEL3, "", 20, 94, 350, 20, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    AfxSetWindowText( hCtrl, "User defined Keybinding" )

    hCtrl = pWindow->AddControl("CHECKBOX", , IDC_FRMKEYBOARDEDIT_CHKCTRL, "Ctrl", 20, 114, 48, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    Button_SetCheck( hCtrl, isCtrl )
    
    hCtrl = pWindow->AddControl("CHECKBOX", , IDC_FRMKEYBOARDEDIT_CHKALT, "Alt", 75, 114, 42, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    Button_SetCheck( hCtrl, isAlt )
    
    hCtrl = pWindow->AddControl("CHECKBOX", , IDC_FRMKEYBOARDEDIT_CHKSHIFT, "Shift", 127, 114, 48, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    Button_SetCheck( hCtrl, isShift )

    dim as HWND hCombo 
    hCombo = pWindow->AddControl("COMBOBOX", , IDC_FRMKEYBOARDEDIT_COMBOACCEL, "", 190, 114, 120, 200, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("BUTTON", , IDOK, L(0,"OK"), 154, 170, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("BUTTON", , IDCANCEL, L(1,"Cancel"), 236, 170, 74, 28, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
                                        
    AddShortcutsToComboBox( hCombo )
    
    if len(wszAccel) = 0 then wszAccel = "None"
    dim as long nCurSel 
    nCursel = ComboBox_FindStringExact( hCombo, -1, wszAccel.sptr )
    ComboBox_SetCurSel( hCombo, nCursel )

    SetFocus( GetDlgItem(HWND_FRMKEYBOARDEDIT, IDOK) )    
    
    ' Process Windows messages(modal)
    function = pWindow->DoEvents(SW_SHOW)
    
    ' Delete the CWindow class manually allocated memory 
    if pWindow then delete(pWindow)

end function


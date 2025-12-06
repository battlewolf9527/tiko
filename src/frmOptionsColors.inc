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

#include once "frmOptionsColors.bi"
#include once "clsConfig.bi"


' ========================================================================================
' Enumerate the names of all the fonts. Note the difference between how to enumerate them
' (%TMPF_FIXED_PITCH has the bit cleared).
' %TMPF_FIXED_PITCH for fixed pitch fonts (like in PB edit)
' %TMPF_trueTYPE or %TMPF_VECTOR for true type and vector fonts
' %TMPF_DEVICE for device fonts (like printer fonts)
' Exclude what you don't want to include in list.
' ========================================================================================
function frmOptionsColors_EnumFontName( _
            byref lf as LOGFONTW, _
            byref tm as TEXTMETRIC, _
            byval FontType as long, _
            hwnd as HWND _
            ) as long

    static wszPrevText as DWSTRING
    
    if (FontType and trueTYPE_FONTTYPE) then                      ' // true type fonts
        ComboBox_AddString( hwnd, @lf.lfFaceName )
    elseif (FontType and TMPF_FIXED_PITCH) = 0 then               ' <- check if bit is cleared!
        ComboBox_AddString( hwnd, @lf.lfFaceName )                        ' // fixed pitch fonts
    elseif (FontType and TMPF_VECTOR) then
        ComboBox_AddString( hwnd, @lf.lfFaceName )                        ' // vector fonts
    elseif (FontType and TMPF_DEVICE) then
        ComboBox_AddString( hwnd, @lf.lfFaceName )                        ' // device fonts
    else
        ComboBox_AddString( hwnd, @lf.lfFaceName )                        ' // system, others
    end if

    function = true

end function


' ========================================================================================
' Fill combobox with list of valid font names
' ========================================================================================
function frmOptionsColors_FillFontCombo( byval hwnd as HWND ) as long
    dim hDC as HDC = GetDC(0)
    SendMessage( hwnd, CB_RESETCONTENT, 0, 0)
    EnumFontFamilies hDC, byval 0, cast(FONTENUMPROCW, @frmOptionsColors_EnumFontName), cast(LPARAM, HWND)
    ReleaseDC 0, hDC
    function = 0
end function


' ========================================================================================
' WM_DRAWITEM procedure
' ========================================================================================
function frmOptionsColors_DrawFontCombo( _
            byval hwnd as HWND, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as long

    dim hFont as HFONT
    dim lpdis as DRAWITEMSTRUCT ptr
    dim wText as wstring * MAX_PATH
    dim wPrevText as wstring * MAX_PATH

    dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMMAIN)
    
    lpdis = cast(DRAWITEMSTRUCT ptr, lParam)
    if lpdis->itemID = &HFFFFFFFF then exit function   ' empty list, take a break..

    select case lpdis->itemAction
        case ODA_DRAWENTIRE, ODA_select
            SaveDC(lpdis->hDC)
            ' if not selected or if in edit part of the combobox
            if (lpdis->itemState and ODS_SELECTED) = 0 or _
                (lpdis->itemState and ODS_COMBOBOXEDIT) then
                SetBkColor lpdis->hDC, GetSysColor(COLOR_WINDOW)
                SetTextColor lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT)
                FillRect lpdis->hDC, @lpdis->rcItem, GetSysColorBrush(COLOR_WINDOW)
            else
                ' select text background
                SetBkColor lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT)
                SetTextColor lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT)
                FillRect lpdis->hDC, @lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT)
            end if

            ' Get item's text (fontname), create font and draw text
            wText = AfxGetComboBoxText(HWnd, lpdis->itemID)
            if len(wText) then
                hFont = pWindow->CreateFont( wText, 10 ) 
                if hFont then SelectObject(lpdis->hDC, hFont)
            end if
            DrawText( lpdis->hDC, @wText, len(wText), @lpdis->rcItem, DT_SINGLELINE or DT_LEFT or DT_VCENTER)

            ' Focus rect around selected item
            if (lpdis->itemState and ODS_SELECTED) then
                DrawFocusRect lpdis->hDC, @lpdis->rcItem
            end if
            
            RestoreDC(lpdis->hDC, -1)
            DeleteObject(hFont) 
            function = true

    end select

end function


' ========================================================================================
' Fill combobox with list of valid font sizes
' ========================================================================================
function frmOptionsColors_FillFontSizeCombo( _
            byval hCb as HWnd, _
            byval strFontName as wstring ptr _
            ) as long

    select case ucase(*strFontName)
        case "FIXEDSYS"
            ComboBox_ResetContent(hCb)
            ComboBox_AddString( hCb, @wstr("9"))
        case "TERMINAL"
            ComboBox_ResetContent(hCb)
            ComboBox_AddString( hCb, @wstr("5" ))
            ComboBox_AddString( hCb, @wstr("6" ))
            ComboBox_AddString( hCb, @wstr("12"))
            ComboBox_AddString( hCb, @wstr("14"))
        case Else                        
            ComboBox_ResetContent(hCb)
            ComboBox_AddString( hCb, @wstr("8" ))
            ComboBox_AddString( hCb, @wstr("9" ))
            ComboBox_AddString( hCb, @wstr("10"))
            ComboBox_AddString( hCb, @wstr("11"))
            ComboBox_AddString( hCb, @wstr("12"))
            ComboBox_AddString( hCb, @wstr("14"))
            ComboBox_AddString( hCb, @wstr("16"))
            ComboBox_AddString( hCb, @wstr("18"))
            ComboBox_AddString( hCb, @wstr("20"))
            ComboBox_AddString( hCb, @wstr("22"))
            ComboBox_AddString( hCb, @wstr("24"))
            ComboBox_AddString( hCb, @wstr("26"))
            ComboBox_AddString( hCb, @wstr("28"))
            ComboBox_AddString( hCb, @wstr("32"))
            ComboBox_AddString( hCb, @wstr("36"))
    end select

    function = 0
end function


' ========================================================================================
' Fill combobox with list of valid character sets
' ========================================================================================
function frmOptionsColors_FillFontCharSets( byval hCtl as HWND ) as long
    ComboBox_AddString( hCtl, @wstr("Default") )
    ComboBox_AddString( hCtl, @wstr("Ansi") )
    ComboBox_AddString( hCtl, @wstr("Arabic") )
    ComboBox_AddString( hCtl, @wstr("Baltic") )
    ComboBox_AddString( hCtl, @wstr("Chinese Big 5") )
    ComboBox_AddString( hCtl, @wstr("East Europe") )
    ComboBox_AddString( hCtl, @wstr("GB 2312") )
    ComboBox_AddString( hCtl, @wstr("Greek") )
    ComboBox_AddString( hCtl, @wstr("Hangul") )
    ComboBox_AddString( hCtl, @wstr("Hebrew") )
    ComboBox_AddString( hCtl, @wstr("Johab") )
    ComboBox_AddString( hCtl, @wstr("Mac") )
    ComboBox_AddString( hCtl, @wstr("OEM") )
    ComboBox_AddString( hCtl, @wstr("Russian") )
    ComboBox_AddString( hCtl, @wstr("Shiftjis") )
    ComboBox_AddString( hCtl, @wstr("Symbol") )
    ComboBox_AddString( hCtl, @wstr("Thai") )
    ComboBox_AddString( hCtl, @wstr("Turkish") )
    ComboBox_AddString( hCtl, @wstr("Vietnamese") )
    
    function = 0
end function


' ========================================================================================
' frmOptionsColors Window procedure
' ========================================================================================
function frmOptionsColors_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        
        case WM_MEASUREITEM
            if wParam = IDC_FRMOPTIONSCOLORS_COMBOFONTNAME then
                dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
                dim pMeasureItem as MEASUREITEMSTRUCT ptr = cast(MEASUREITEMSTRUCT ptr, lParam)
                pMeasureItem->itemHeight = pWindow->ScaleY(pMeasureItem->itemHeight)
                return true
            end if

        case WM_DRAWITEM  ' must pass this one on to ownerdrawn combo
            if (wParam = IDC_FRMOPTIONSCOLORS_COMBOFONTNAME) then 
                frmOptionsColors_DrawFontCombo( GetDlgItem(hwnd, IDC_FRMOPTIONSCOLORS_COMBOFONTNAME), wParam, lParam )
                return true 
            end if

    end select

    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' GetThemeDescription
' ========================================================================================
function GetThemeDescription( byval wszFilename as DWSTRING ) as string
    ' default to the theme description being the filename
    function = AfxGetFileName(wszFilename)

    dim pStream as CTextStream   
    if pStream.Open(wszFilename) <> S_OK then exit function  'error

    dim as DWSTRING wst, keyValue, keyData
        
    do until pStream.EOS
        wst = pStream.ReadLine

        wst = trim(AfxStrExtract( 1, wst, "#"))     ' remove comments
        if len(wst) = 0 then continue Do

        keyValue = trim(AfxStrParse(wst, 1, ":"))
        keyData = trim(AfxStrParse(wst, 2, ":"))
        
        ' is this a replaceable parameter
        if keyValue = "general.description" then
            function = keyData
            exit do
        end if

    loop

    pStream.Close

end function


' ========================================================================================
' frmOptionsColors_Show
' ========================================================================================
function frmOptionsColors_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMOPTIONSCOLORS = pWindow->Create( hWndParent, "", @frmOptionsColors_WndProc, 0, 0, 0, 0, _
        WS_CHILD or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    ' Height and width of this child form is set in frmOptions_OnNotify when the treeview option is selected.

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSCOLORS_LBLTHEMES, _
        L(422,"Themes") & ":", 0, 2, 375, 15, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    dim as HWND hListBox = _
    pWindow->AddControl("LISTBOX", , IDC_FRMOPTIONSCOLORS_LSTTHEMES, "", 0, 20, 386, 220, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or LBS_NOTIFY or LBS_NOINTEGRALHEIGHT, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    
    pWindow->AddControl("GROUPBOX",, IDC_FRMOPTIONSCOLORS_FRMFONT, L(136,"Font (applies to all styles)"), _
        0, 266, 385, 80, _
        WS_CHILD or WS_VISIBLE or BS_TEXT or BS_LEFT or BS_NOTIFY or BS_GROUPBOX, _
        WS_EX_TRANSPARENT or WS_EX_LEFT or WS_EX_LTRREADING)
    dim as HWND hComboFontname = _
        pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSCOLORS_COMBOFONTNAME, "", _
            11, 286, 310, 23, _
            WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST or CBS_OWNERDRAWFIXED or CBS_HASSTRINGS or CBS_SORT, _
            WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    dim as HWND hComboFontSize = _
        pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSCOLORS_COMBOFONTSIZE, "", _
            330, 286, 40, 23, _
            WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, _
            WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    dim as HWND hComboFontCharSet = _
        pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSCOLORS_COMBOFONTCHARSET, "", _
            11, 318, 150, 23, _
            WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, _
            WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->AddControl("LABEL",, IDC_FRMOPTIONSCOLORS_LBLEXTRASPACE, L(421,"Extra line spacing") & ":", _
        170, 320, 150, 18, _
        WS_CHILD or WS_VISIBLE or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or SS_RIGHT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSCOLORS_TXTEXTRASPACE, "0", _
        330, 318, 40, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    AfxSetWindowText( GetDlgItem( HWND_FRMOPTIONSCOLORS, IDC_FRMOPTIONSCOLORS_TXTEXTRASPACE), gConfig.FontExtraSpace)

    frmOptionsColors_FillFontCombo( hComboFontName )
    frmOptionsColors_FillFontSizeCombo( hComboFontSize, gConfig.EditorFontname )
    frmOptionsColors_FillFontCharSets( hComboFontCharSet )
    
    dim idx as long
    idx = SendMessage( hComboFontName, CB_FINDSTRING, 0, cast(LPARAM, *gConfig.EditorFontname))
    ComboBox_SetCurSel( hComboFontName, idx )
    idx = SendMessage( hComboFontSize, CB_FINDSTRING, 0, cast(LPARAM, *gConfig.EditorFontSize))
    ComboBox_SetCurSel( hComboFontSize, idx )
    idx = SendMessage( hComboFontCharSet, CB_FINDSTRING, 0, cast(LPARAM, *gConfig.EditorFontCharSet))
    ComboBox_SetCurSel( hComboFontCharSet, idx )
    
    ' Load the Themes listbox
    dim hSearch as HANDLE
    dim WFD as WIN32_FIND_DATAW
    dim wszPath as wstring * MAX_PATH
    dim wszCurPath as wstring * MAX_PATH
    dim wszFullPath as wstring * (MAX_PATH * 2)
    dim wszDisplayName as wstring * (MAX_PATH * 2)
    dim as long idxMatch = -1
    
    wszPath = AfxGetExePathName + "settings\themes\"
    wszCurPath = wszPath + "*"

    erase gThemeFilenames

    idx = 0
    hSearch = FindFirstFile( wszCurPath, @WFD )
    if hSearch <> INVALID_HANDLE_VALUE then
        do
            if (WFD.cFileName <> ".") andalso (WFD.cFileName <> "..") then
                wszFullPath = wszPath & WFD.cFileName
                wszDisplayName = GetThemeDescription(wszFullPath)
                idx = SendMessage( hListBox, LB_ADDSTRING, 0, cast(LPARAM, @wszDisplayName) )
                redim preserve gThemeFilenames(idx) as DWSTRING
                gThemeFilenames(idx) = WFD.cFileName
                if ucase(gConfig.ThemeShortFilename) = ucase(WFD.cFileName ) then
                    idxMatch = idx
                end if
            end if   
        loop while FindNextFile( hSearch, @WFD )
        FindClose( hSearch )
    end if
    SendMessage( hListBox, LB_SETCURSEL, idxMatch, 0 )

    function = 0
end function

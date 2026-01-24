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

#include once "frmOptionsCompiler.bi"
#include once "clsConfig.bi"


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmOptionsCompiler
' ========================================================================================
function frmOptionsCompiler_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT

    select case id

        case IDC_FRMOPTIONSCOMPILER_CMDFBHELPFILE

            if codeNotify = BN_CLICKED then
                ' Display the Open File Dialog
                dim pwszName as wstring ptr = AfxIFileOpenDialogW(hwnd, id)
                if pwszName then
                    SetWindowTextW( GetDlgItem(hwnd, IDC_FRMOPTIONSCOMPILER_TXTFBHELPFILE), pwszName )
                    CoTaskMemFree pwszName
                end if
                exit function
            end if

    end select

    function = 0
end function


' ========================================================================================
' frmOptionsCompiler Window procedure
' ========================================================================================
function frmOptionsCompiler_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        HANDLE_MSG (hwnd, WM_COMMAND, frmOptionsCompiler_OnCommand)
    end select

    function = DefWindowProcW(hwnd, uMsg, wParam, lParam)

end function


' ========================================================================================
' frmOptionsCompiler_Show
' ========================================================================================
function frmOptionsCompiler_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow
    
    HWND_FRMOPTIONSCOMPILER = pWindow->Create( hWndParent, "", @frmOptionsCompiler_WndProc, 0, 0, 0, 0, _
        WS_CHILD, WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    ' Height and width of this child form is set in frmOptions_OnNotify when the treeview option is selected.
    

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSCOMPILER_LBLTOOLCHAINS, _
        L(420,"Toolchains") & ":", 0, 2, 375, 15, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    dim as HWND hListBox = _
    pWindow->AddControl("LISTBOX", , IDC_FRMOPTIONSCOMPILER_LSTTOOLCHAINS, "", 0, 20, 386, 124, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or LBS_NOTIFY or LBS_NOINTEGRALHEIGHT, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSCOMPILER_LBLSWITCHES, _
        L(153,"Additional compiler option switches (optional)" & ":"), 0, 148, 375, 16, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSCOMPILER_TXTFBSWITCHES, "", 0, 167, 386, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSCOMPILER_LBLINCLUDES, _
        L(152,"Additional Include paths (semi colon separated)" & ":"), 0, 191, 375, 16, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSCOMPILER_TXTINCLUDES, "", 0, 211, 386, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSCOMPILER_CHKRUNVIACOMMANDWINDOW, _
        L(273,"Run compiled programs using command window"), 0, 241, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("CHECKBOX", , IDC_FRMOPTIONSCOMPILER_CHKDISABLECOMPILEBEEP, _
        L(413,"Disable successful compile sound"), 0, 262, 400, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSCOMPILER_LBLFBHELP, _
        L(155,"FreeBASIC Help file (*.chm)") & ":", 0, 300, 375, 18, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSCOMPILER_TXTFBHELPFILE, "", 0, 320, 356, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->AddControl("BUTTON", , IDC_FRMOPTIONSCOMPILER_CMDFBHELPFILE, "...", 362, 320, 24, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    
    gConfig.FBWINCompiler32  = ProcessFromCurdriveApp(gConfig.FBWINCompiler32)
    gConfig.FBWINCompiler64  = ProcessFromCurdriveApp(gConfig.FBWINCompiler64)
    gConfig.CompilerHelpfile = ProcessFromCurdriveApp(gConfig.CompilerHelpfile)

    ' Load the Toolchain listbox
    dim hSearch as HANDLE
    dim WFD as WIN32_FIND_DATAW
    dim wszPath as wstring * MAX_PATH
    dim wszCurPath as wstring * MAX_PATH
    dim wszFullPath as wstring * (MAX_PATH * 2)
    dim wszDisplayName as wstring * (MAX_PATH * 2)
    dim as long nCount = 0
    dim as long idx = 0
    dim as long idxMatch = -1
    
    wszPath = AfxGetExePathName + "toolchains\"
    wszCurPath = wszPath + "*"

    hSearch = FindFirstFile( wszCurPath, @WFD )
    if hSearch <> INVALID_HANDLE_VALUE then
        do
            if (WFD.dwFileAttributes AND FILE_ATTRIBUTE_DIRECTORY) = FILE_ATTRIBUTE_DIRECTORY then
                if (WFD.cFileName <> ".") andalso (WFD.cFileName <> "..") then
                    wszFullPath = ucase( wszPath & WFD.cFileName & "\fbc32.exe" )

                    ' Do check to see if path contains spaces as we will have to warn the user to fix it
                    wszDisplayName = WFD.cFileName
                    if instr( wszFullPath, " " ) then
                        wszDisplayName = wszDisplayName & "   (Warning: File path contains spaces)"
                    end if

                    idx = SendMessage( hListBox, LB_ADDSTRING, 0, cast(LPARAM, @wszDisplayName) )
                    if ucase(gConfig.FBWINCompiler32) = wszFullPath then
                        idxMatch = idx
                    end if
                    
                end if   
            end if
        loop while FindNextFile( hSearch, @WFD )
        FindClose( hSearch )
    end if
    SendMessage( hListBox, LB_SETCURSEL, idxMatch, 0 )
    
    dim as hwnd HWND = HWND_FRMOPTIONSCOMPILER
    AfxSetWindowText( GetDlgItem( hwnd, IDC_FRMOPTIONSCOMPILER_TXTFBSWITCHES),     gConfig.CompilerSwitches)
    AfxSetWindowText( GetDlgItem( hwnd, IDC_FRMOPTIONSCOMPILER_TXTINCLUDES),       gConfig.CompilerIncludes)
    AfxSetWindowText( GetDlgItem( hwnd, IDC_FRMOPTIONSCOMPILER_TXTFBHELPFILE),     gConfig.CompilerHelpfile)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSCOMPILER_CHKRUNVIACOMMANDWINDOW), gConfig.RunViaCommandWindow)
    Button_SetCheck( GetDlgItem( hwnd, IDC_FRMOPTIONSCOMPILER_CHKDISABLECOMPILEBEEP),  gConfig.DisableCompileBeep)

    function = 0 
end function

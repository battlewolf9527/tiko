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

#include once "frmMain.bi"
#include once "frmDebug.bi"
#include once "clsDocument.bi"


' ========================================================================================
' Search action Common actions
' ========================================================================================
function OnCommand_DebugCommon( byval id as long ) as LRESULT
            
    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
    if pDoc = 0 then return 0

    dim as HWND hEdit = pDoc->hWndActiveScintilla

    select case id
        case IDM_DEBUG_STARTDEBUGGING
            code_Compile(id)
            
        case IDM_DEBUG_STEPINTO
            
        case IDM_DEBUG_STEPOVER

        case IDM_DEBUG_TOGGLEBREAKPOINT
            pDoc->ToggleBreakPoint( pDoc->GetCurrentLineNumber )
            
        case IDM_DEBUG_DELETEALLBREAKPOINTS
            if MessageBox( HWND_FRMMAIN, L(315,"Do you want to delete all breakpoints?"), L(276,"Confirm"), _
                        MB_YESNOCANCEL or MB_ICONQUESTION or MB_DEFBUTTON1 ) <> IDYES then
                return true
            end if            
            dim pDoc as clsDocument ptr = gApp.pDocList
            do until pDoc = 0
                hEdit = pDoc->hWindow(0)
                SciExec( hEdit, SCI_MARKERDELETEALL, MARKER_BREAKPOINT, 0 )
                pDoc = pDoc->pDocNext
            loop

    end select
    
    function = 0
end function


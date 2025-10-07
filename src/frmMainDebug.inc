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
        
        case IDM_DEBUG_STEPINTO
            
        case IDM_DEBUG_STEPOVER

        case IDM_DEBUG_TOGGLEBREAKPOINT
            pDoc->ToggleBreakPoint( pDoc->GetCurrentLineNumber )

    end select
    
    function = 0
end function


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

#include once "frmDebug.bi"


function gdb_findResultValue( byref response as string, byref token as string ) as string
    ' Add to incoming token:   token="
    dim as string result
    
    token = token & "=" & chr(34)
    dim as integer iStart = instr( response, token )
    if iStart > 0 then
        iStart = iStart + len(token)
        dim as integer iEnd = instr( iStart, response, chr(34) )
        if iEnd > 0 then
            result = mid( response, iStart, iEnd - iStart ) 
        end if
    end if
    
    return result
end function


function gdb_findFunctionArguments( byref response as string, byref token as string ) as string
    ' Add to incoming token:   token="
    ' args=[{name="NUMHOURS$1",value="1500"},{name="NUMHOURS$1@entry",value="1500"}]
    dim as string result
    
    token = token & "=" & "["
    dim as integer iStart = instr( response, token )
    if iStart > 0 then
        iStart = iStart + len(token)
        dim as integer iEnd = instr( iStart, response, "]" )
        if iEnd > 0 then
            result = mid( response, iStart, iEnd - iStart ) 
        end if
    end if
    
    return result
end function


function gdb_getVariableDisplayName( byref session as GDBSession, byval varName as string ) as string
    dim as string varDisplayString = AfxStrExtract( 1, varName, "$" )
    if varDisplayString = varName then return varDisplayString
    if len(varName) = 0 then return ""
    
    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
    if pDoc = 0 then exit function

    dim as long r, startPos, endPos

    dim as HWND hEdit = pDoc->hWndActiveScintilla

    ' Try to find the variable within the source code starting at the function line
    startPos = SciExec( hEdit, SCI_POSITIONFROMLINE, session.current_function_linenum, 0)
    endPos = SciExec( hEdit, SCI_GETLENGTH, 0, 0)

    dim as string txtFind = varDisplayString
    SciExec( hEdit, SCI_SETTARGETSTART, startPos, 0)
    SciExec( hEdit, SCI_SETTARGETEND, endPos, 0)
    r = SciExec( hEdit, SCI_SEARCHINTARGET, len(txtFind), strptr(txtFind))
    if r then
        varDisplayString = pDoc->GetTextRange( r, r + len(txtFind) )
    end if

    function = varDisplayString
end function


function gdb_getVariableDisplayValue( byref session as GDBSession, byval varValue as string ) as string
    dim as string varDisplayString = varValue
    
    if left(varValue, 5) = "{data" then
        dim as string wszExtractString = AfxStrExtract( 1, varValue, "'", "'" )
        if len(wszExtractString) then
            varDisplayString = chr(34) & wszExtractString & chr(34) & "  (len = " & len(wszExtractString) & ")"
        end if    
    end if
    
    ' Try converting a Hex number to Decimal
    if left(varValue, 2) = "0x" then
        dim as string wszHexString = "&H" & mid(varValue, 3)
        varDisplayString = wszHexString & "  (dec = " & str(val(wszHexString)) & ")"
    end if

    function = varDisplayString
end function


function gdb_parseVariablesResponse( byref session as GDBSession, byref response as string ) as integer
    dim as integer i = 1
    dim as integer nItems = 0
    dim as integer p1 = 0
    dim as integer p2 = 0
    dim as string  keyValue
    dim as string  valValue

    dim hLV as HWND = HWND_FRMDEBUG_OUTPUT

    dim as DWSTRING wszText

    ' name="vr$29",value="<optimized out>"
    response = trim(response)
    if len(response) then
        do while i <= len(response)
            '' find name="
            p1 = instr(i, response, "name=" & chr(34))
            if p1 = 0 then exit do
            p1 += 6
            p2 = instr(p1, response, chr(34))
            if p2 = 0 then exit do
            keyValue = mid(response, p1, p2 - p1)

            '' find value="
            p1 = instr(p2, response, "value=" & chr(34))
            if p1 = 0 then exit do
            p1 += 7
            p2 = instr(p1, response, chr(34))
            if p2 = 0 then exit do
            valValue = mid(response, p1, p2 - p1)

            dim as boolean bAddVariable = true
            if left(keyValue, 3) = "vr$" then bAddVariable = false
            if left(keyValue, 4) = "TMP$" then bAddVariable = false
            if instr(valValue, "<optimized out>") then bAddVariable = false

            if (bAddVariable = true) andalso (session.is_new_variables = true) then
                redim preserve session.variable_array(nItems)
                session.variable_array(nItems).VarName  = keyValue
                session.variable_array(nItems).VarNameDisplay = gdb_getVariableDisplayName( session, keyValue )
                session.variable_array(nItems).VarValue = gdb_getVariableDisplayValue( session, valValue )
                nItems += 1
            else
                if (bAddVariable = true) andalso (session.is_new_variables = false) then
                    ' The variable list already exists so simply find the variable name and update its value
                    for i as long = lbound(session.variable_array) to ubound(session.variable_array)
                        if session.variable_array(i).varname = keyValue then
                            session.variable_array(i).varvalue = gdb_getVariableDisplayValue( session, valValue )
                            exit for
                        end if
                    next
                end if
            end if
            
            i = p2 + 1
        loop
    end if
    

    if session.is_new_variables = false then
        ' Simply update the value held in the ListView
        for i as long = lbound(session.variable_array) to ubound(session.variable_array) - 1
            for j as long = 0 to frmListView_GetCount( hLV ) - 1
                if frmListView_GetItemText(hLV, j, 1) = session.variable_array(i).VarName then
                    frmListView_SetItemText(hLV, j, 3, session.variable_array(i).VarValue)
                    exit for
                end if
            next
        next    
    end if
    
   
    if session.is_new_variables = true then
        ' Add all of new array to the ListView
        dim as VariableType temp

        ' Simple bubble sort the variables
        for i as long = lbound(session.variable_array) to ubound(session.variable_array) - 1
            for j as long = i + 1 to ubound(session.variable_array)
                if session.variable_array(i).VarName > session.variable_array(j).VarName then
                    temp = session.variable_array(i)
                    session.variable_array(i) = session.variable_array(j)
                    session.variable_array(j) = temp
                end if
            next 
        next 

        frmListView_DeleteAllItems( hLV )

        for i as long = lbound(session.variable_array) to ubound(session.variable_array)
            wszText = "" & chr(9) & _
                        session.variable_array(i).VarName & chr(9) & _
                        session.variable_array(i).VarNameDisplay & chr(9) & _
                        session.variable_array(i).VarValue
            frmListView_AddString( hLV, wszText )
        next
    end if


    ' Now set the flag that the array has variable names
    session.is_new_variables = false

    ' Set the column widths to the size of the max text widths
    dim as long nTextWidth
    dim as long nVarNameColWidth = getTextWidth( hLV, L(437, "Variable"), GUIFONT_9 )
    dim as long nVarNameDisplayColWidth = nVarNameColWidth
    dim as long nVarValueColWidth = getTextWidth( hLV, L(438, "Value"), GUIFONT_9 )
    
    for i as long = lbound(session.variable_array) to ubound(session.variable_array)
        ' Get the unscaled width of the text so that we can set the ListView column
        ' widths after all lines have been evaluated.
        nTextWidth = getTextWidth( hLV, session.variable_array(i).varname, GUIFONT_9 )
        nVarNameColWidth = MAX( nVarNameColWidth, nTextWidth )
        nTextWidth = getTextWidth( hLV, session.variable_array(i).VarNameDisplay, GUIFONT_9 )
        nVarNameDisplayColWidth = MAX( nVarNameDisplayColWidth, nTextWidth )
        nTextWidth = getTextWidth( hLV, session.variable_array(i).varvalue, GUIFONT_9 )
        nVarValueColWidth = MAX( nVarValueColWidth, nTextWidth )
    next
    ' Set the column widths based on the variable name and values
    ' Add a little extra padding to the widths
    nVarNameColWidth = nVarNameColWidth + 10
    nVarNameDisplayColWidth = nVarNameDisplayColWidth + 10
    nVarValueColWidth = nVarValueColWidth + 10
    frmListView_SetColumnWidth( hLV, 1, AfxScaleX(nVarNameColWidth) )
    frmListView_SetColumnWidth( hLV, 2, AfxScaleX(nVarNameDisplayColWidth) )
    frmListView_SetColumnWidth( hLV, 3, AfxScaleX(nVarValueColWidth) )

    dim as long nShow = iif( frmListView_GetCount(hLV) = 0, SW_HIDE, SW_SHOW )
    ShowWindow( hLV, nShow )
    frmListView_Refresh( hLV )

    return nItems
end function


sub gdb_processResponse( byref session as GDBSession, byval response as string ) 
    dim as string file_name 
    dim as string function_name 
    dim as string function_args
    dim as integer line_number

    ' Display value of local variables within scope
    if instr( response, "^done,variables=") then 
LM( "^done,variables RECEIVED  len=" & len(response) )
'LM( response )
        gdb_parseVariablesResponse( session, response )
    end if
    
    ' Process hitting a Breakpoint
    if instr( response, "*stopped,reason=" & chr(34) & "breakpoint-hit" & chr(34)) orelse _
       instr( response, "*stopped,reason=" & chr(34) & "end-stepping-range" & chr(34)) then
        ' Line numbers in GDB are 1 based whereas Scintilla is 0 based.
        line_number = val( gdb_findResultValue( response, "line" ) ) - 1
        file_name = gdb_findResultValue( response, "fullname" )
        file_name = AfxStrReplace( file_name, "\\", "\" )
        file_name = AfxStrReplace( file_name, "/", "\" )
        function_name = gdb_findResultValue( response, "func" )
        function_args = gdb_findFunctionArguments( response, "args" )

        ' If the current function name or filename has changed then reset the variable array.
        ' The new values for the array are set in gdb_parseVariablesResponse()
        if (file_name <> session.current_file_name) orelse _
            (function_name <> session.current_function_name) then
            erase session.variable_array
            session.is_new_variables = true
        end if    
        session.current_file_name = file_name
        session.current_function_name = function_name
        session.current_function_linenum = line_number
        
'LM( response )
LM( "STOP: BREAKPOINT OR STEPPING" )
LM( "  file_name:     " & file_name )
LM( "  function_name: " & function_name )
LM( "  function_args: " & function_args )
LM( "  line_number:   " & str(line_number) )

        if (len(file_name) > 0) andalso (line_number >= 0) then
            ' Get the filename as represented in the gApp doc list. That filename is in correct
            ' lettercase format whereas the filename returned via GDB is all uppercase. If that
            ' source file has not yet been loaded in the editor then simply use the GDB returned name.
            dim as DWSTRING wszFilename = file_name
            dim as clsDocument ptr pDoc = gApp.GetDocumentPtrByFilename(wszFilename)
            if pDoc then 
                wszFilename = pDoc->DiskFilename
            else
                dim as DWSTRING wszTemp = AfxFilenameOriginalCase( file_name )
                if AfxFileExists(wszTemp) then wszFilename = wszTemp
            end if
            pDoc = OpenSelectedDocument( wszFilename, "", line_number)
            if pDoc then
                SciExec( pDoc->hWndActiveScintilla, SCI_MARKERDELETEALL, MARKER_DEBUGGER_CURLINE, 0 )
                pDoc->ToggleMarker( line_number, MARKER_DEBUGGER_CURLINE )
                ' Ask to get list of variables and values for current context
                gdb_send( gdb_session, "-stack-list-variables 1" )
            end if    
        end if
    end if

    if instr( response, "*stopped,reason=" & chr(34) & "exited-normally" & chr(34)) orelse _
       instr( response, "*stopped,reason=" & chr(34) & "exited" & chr(34)) orelse _
       instr( response, "*stopped,reason=" & chr(34) & "exited-signalled" & chr(34)) then
        SendMessage( HWND_FRMDEBUG, IDM_DEBUG_STOPDEBUGGING, 0, 0 )
    end if

end sub


' ========================================================================================
' Position all child windows. Called manually and/or by WM_SIZE
' ========================================================================================
function frmDebug_PositionWindows() as LRESULT

    dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMDEBUG)
    if pWindow = 0 then exit function
    
    ' Get the entire client area
    dim as RECT rc: GetClientRect(HWND_FRMDEBUG, @rc)

    dim as long i = 0
    dim as long nSpacer     = pWindow->ScaleX(4)
    dim as long nLeft       = rc.left + pWindow->ScaleY(20)
    dim as long nHeight     = pWindow->ScaleY(TOPTABS_HEIGHT)
    dim as long nPadding    = 10   ' 5 left 5 right
    dim as long nImageWidth = DEBUG_BUTTON_IMAGE_WIDTH

    i = DEBUG_BUTTONS.STOP_DEBUGGING
    gDbgBtns(i).nID        = IDM_DEBUG_STOPDEBUGGING
    gDbgBtns(i).wszToolTip = getMenuText(gDbgBtns(i).nID)
    gDbgBtns(i).wszIcon    = wszIconStop
    gDbgBtns(i).wszText    = L(322,"Stop")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.CONTINUE_DEBUGGING
    gDbgBtns(i).nID        = IDM_DEBUG_STARTDEBUGGING
    gDbgBtns(i).wszToolTip = getMenuText(gDbgBtns(i).nID)
    gDbgBtns(i).wszIcon    = wszIconContinue
    gDbgBtns(i).wszText    = L(321,"Continue")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STEP_OVER
    gDbgBtns(i).nID        = IDM_DEBUG_STEPOVER
    gDbgBtns(i).wszToolTip = getMenuText(gDbgBtns(i).nID)
    gDbgBtns(i).wszIcon    = wszIconStepOver
    gDbgBtns(i).wszText    = L(312,"Step Over")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STEP_INTO
    gDbgBtns(i).nID        = IDM_DEBUG_STEPINTO
    gDbgBtns(i).wszToolTip = getMenuText(gDbgBtns(i).nID)
    gDbgBtns(i).wszIcon    = wszIconStepNext
    gDbgBtns(i).wszText    = L(311,"Step Into")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STEP_OUT
    gDbgBtns(i).nID        = IDM_DEBUG_STEPOUT
    gDbgBtns(i).wszToolTip = getMenuText(gDbgBtns(i).nID)
    gDbgBtns(i).wszIcon    = wszIconStepOut
    gDbgBtns(i).wszText    = L(319,"Step Out")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.RUN_TO_CURSOR
    gDbgBtns(i).nID        = IDM_DEBUG_RUNTOCURSOR
    gDbgBtns(i).wszToolTip = getMenuText(gDbgBtns(i).nID)
    gDbgBtns(i).wszIcon    = wszIconRunToCursor
    gDbgBtns(i).wszText    = L(320,"Run to Cursor")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    nLeft = gDbgBtns(i).rc.right + nSpacer

    AfxRedrawWindow( HWND_FRMDEBUG )

    dim as HWND hCtl 
    
    if HWND_FRMDEBUG_OUTPUT = 0 then
        hCtl = frmListView_Show( HWND_FRMDEBUG )
            frmListView_AddColumn( hCtl, 0, "", pWindow->ScaleX(10) )
            frmListView_AddColumn( hCtl, 1, L(437, "Variable"), pWindow->ScaleX(75) )
            frmListView_AddColumn( hCtl, 2, "", pWindow->ScaleX(75) )
            frmListView_AddColumn( hCtl, 3, L(438, "Value"), pWindow->ScaleX(250) )
            frmListView_SetForeColors( hCtl, ghOutput.ForeColor, ghOutput.ForeColorHot )
            frmListView_SetBackColors( hCtl, ghOutput.BackColor, ghOutput.ForeColorScrollBar )
            frmListView_SetScrollBarColors( hCtl, ghOutput.BackColorScrollBar, ghOutput.ForeColorScrollBar, ghOutput.ScrollBarDivider )

        HWND_FRMDEBUG_OUTPUT = hCtl
    end if
    GetClientRect(HWND_FRMDEBUG, @rc)
    SetWindowPos( HWND_FRMDEBUG_OUTPUT, 0, 0, nHeight, rc.right, rc.bottom - nHeight, SWP_NOZORDER )
    
    function = 0
end function


' ========================================================================================
' frmDebug Window procedure
' ========================================================================================
function frmDebug_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    static as long prev_idxHot = -1
    static as long curr_idxHot = -1

    static as DWSTRING wszPrevTooltip 
    static as HWND hTooltip
    if (uMsg = WM_DESTROY) andalso (IsWindow(hToolTip)) then DestroyWindow(hToolTip)

    select case uMsg

        case WM_CLOSE
            DestroyWindow( hwnd )
            
        case WM_DESTROY
            HWND_FRMDEBUG_OUTPUT = 0    
            KillTimer( HWND_FRMDEBUG, gApp.DebugTimerID )
            ' Kill the GDB process
            gdb_close( gdb_session )
            
        case WM_NCDESTROY
            dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
            if pWindow then delete(pWindow)
            gdb_session.KillMessageThread = true
            ThreadWait( gdb_session.hThreadMessages )
            MutexDestroy( gdb_session.hThreadMutex )

            gdb_session.hThreadMessages   = 0
            gdb_session.hThreadMutex      = 0
            gdb_session.KillMessageThread = false
            gdb_session.initialized       = false

            ' Remove any yellow arrow debugger current line marker
            dim pDoc as clsDocument ptr = gApp.pDocList
            do until pDoc = 0
                SciExec( pDoc->hWindow(0), SCI_MARKERDELETEALL, MARKER_DEBUGGER_CURLINE, 0 )
                pDoc = pDoc->pDocNext
            loop

            gApp.IsDebuggerActive = false
            
        case WM_TIMER
            if wParam = gApp.DebugTimerID then
                ' Check for any pending message
                if gdb_isMessageQueueEmpty( gdb_session ) = false then
                    MutexLock( gdb_session.hThreadMutex )
                    dim as string message = gdb_dequeue_message( gdb_session )
                    MutexUnLock( gdb_session.hThreadMutex )
                    gdb_processResponse( gdb_session, message )    
                end if
            end if

        case WM_SIZE
            ' Position all of the child windows
            if wParam <> SIZE_MINIMIZED then
                frmDebug_PositionWindows()
                return 0
            end if

        case WM_ERASEBKGND
            return true

        case WM_LBUTTONDOWN
            SetCapture( hwnd )
            
        case WM_MOUSEMOVE
            dim tme as TrackMouseEvent
            tme.cbSize = sizeof(TrackMouseEvent)
            tme.dwFlags = TME_HOVER or TME_LEAVE
            tme.hwndTrack = hwnd
            tme.dwHoverTime = 20    ' HOVER_DEFAULT is 500
            TrackMouseEvent(@tme) 

            if IsWindow(hTooltip) = 0 then hTooltip = AfxAddTooltip( hwnd, "", false, false )
            
            dim as point pt: GetCursorPos( @pt )
            MapWindowPoints( HWND_DESKTOP, hwnd, cast( point ptr, @pt ), 1 )

            ' Are we over one of the buttons
            curr_idxHot = -1
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                if PtInRect( @gDbgBtns(i).rc, pt ) then
                    ' Only make hot panels that have actual text
                    if len(gDbgBtns(i).wszText) then
                        curr_idxHot = i
                        gDbgBtns(i).isHot = true
                    end if   
                else   
                    gDbgBtns(i).isHot = false
                end if
            next
            if curr_idxHot <> prev_idxHot then
                AfxRedrawWindow( hwnd )
                prev_idxHot = curr_idxHot 
            end if

                
        case WM_MOUSELEAVE
            wszPrevTooltip = ""
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                gDbgBtns(i).isHot = false
            next
            prev_idxHot = -1
            curr_idxHot = -1
            DestroyWindow(hToolTip)
            AfxRedrawWindow( hwnd )


        case WM_LBUTTONUP
            dim as point pt: GetCursorPos( @pt )
            MapWindowPoints( HWND_DESKTOP, hwnd, cast( point ptr, @pt ), 1 )
            SetCursor( LoadCursor( null, IDC_ARROW ))
            ReleaseCapture
            ' Are we over one of the status bar panels
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                if PtInRect( @gDbgBtns(i).rc, pt ) then
                    PostMessage( HWND_FRMDEBUG, gDbgBtns(i).nID, 0, 0 )
                    exit for
                end if
            next   
            

        case WM_MOUSEHOVER
            dim as DWSTRING wszTooltip
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                if IsMouseOverRECT(hwnd,  gDbgBtns(i).rc ) then 
                    wszTooltip = gDbgBtns(i).wszToolTip
                    exit for
                end if
            next
            if (wszTooltip = "") andalso (IsWindow(hTooltip)) then
                DestroyWindow( hTooltip )
                exit function
            end if
            
            ' Display the tooltip
            if wszTooltip <> wszPrevTooltip then
                dim as DWSTRING wszText = AfxStrReplace( wszTooltip, wchr(9), " (" ) & ")"
                AfxSetTooltipText( hTooltip, hwnd, wszText )
                wszPrevTooltip = wszTooltip 
            end if    
            

        case WM_PAINT
            dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMDEBUG)
            dim as clsDoubleBuffer b

            b.BeginDoubleBuffer(hwnd)

            ' Set default back and fore colors
            b.SetForeColors( ghTopTabs.ForeColor, ghTopTabs.ForeColor )
            b.SetBackColors( ghTopTabs.BackColor, ghTopTabs.BackColor )

            ' Clear the entire back client area
            b.PaintClientRect()
          
            ' paint the panels
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                dim as COLORREF ForeColor = ghTopTabs.ForeColorHot
                if gDbgBtns(i).nID = IDM_DEBUG_STARTDEBUGGING then ForeColor = ghGeneral.iconsuccess
                if gDbgBtns(i).nID = IDM_DEBUG_STOPDEBUGGING then ForeColor = ghGeneral.iconfail
                
                if i = curr_idxHot then
                    b.SetBackColors( ghTopTabs.BackColorHot, ghTopTabs.BackColorHot )
                else
                    b.SetBackColors( ghTopTabs.BackColor, ghTopTabs.BackColor )
                end if

                b.PaintRect( @gDbgBtns(i).rc )
                
                ' Paint the image
                b.SetForeColors( ForeColor, ForeColor )
                dim as long nImageWidth = pWindow->ScaleX(DEBUG_BUTTON_IMAGE_WIDTH)
                dim as RECT rcIcon = gDbgBtns(i).rc
                rcIcon.right = rcIcon.left + nImageWidth
                b.SetFont( SYMBOLFONT_9 )
                b.PaintText( gDbgBtns(i).wszIcon, @rcIcon, DT_CENTER )
                
                ' Paint the text
                b.SetForeColors( ghTopTabs.ForeColorHot, ghTopTabs.ForeColorHot )
                dim as RECT rcText = gDbgBtns(i).rc
                rcText.left = rcIcon.right
                b.SetFont( GUIFONT_9 )
                b.PaintText( gDbgBtns(i).wszText, @rcText, DT_CENTER )
            next
                                    
            b.EndDoubleBuffer()

            return 0

        case IDM_DEBUG_STARTDEBUGGING
'            gdb_send(gdb_session, "continue")
            gdb_send(gdb_session, "-exec-continue")
        
        case IDM_DEBUG_STOPDEBUGGING
            gdb_send(gdb_session, "kill")
            'gdb_send(gdb_session, "-exec-kill")
            sleep 200
            PostMessage( HWND_FRMDEBUG, WM_CLOSE, 0, 0 )
        
        case IDM_DEBUG_STEPINTO
'            gdb_send(gdb_session, "step")
            gdb_send(gdb_session, "-exec-step")
        
        case IDM_DEBUG_STEPOVER
'            gdb_send(gdb_session, "next")
            gdb_send(gdb_session, "-exec-next")
        
        case IDM_DEBUG_STEPOUT
'            gdb_send(gdb_session, "finish")
            gdb_send(gdb_session, "-exec-finish")

        case IDM_DEBUG_RUNTOCURSOR
            'TODO: Set a temp breakpoint?
            ' Line numbers in GDB are 1 based whereas Scintilla is 0 based.
            dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
            if pDoc then
                dim as string bp_line = str(pDoc->GetCurrentLineNumber() + 1)
                dim as string bp_cmd = "tbreak " & chr(34) & pDoc->DiskFilename & chr(34) & ":" & bp_line
                gdb_send( gdb_session, bp_cmd )
                gdb_send(gdb_session, "-exec-continue")
            end if
            
    end select

    
    function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function


function frmDebug_SetInitialBreakpoints( byref session as GDBSession ) as long
    'break main.c:25                    ' Line 25 in main.c
    'break utils.c:calculateSum         ' Function in utils.c
    'break "my file.c":10              ' File with spaces (use quotes)
    'break C:\path\to\file.c:42        ' Full path with line
    'break src/module.c:init           ' Relative path

    'tbreak main.c:50                  ' Temporary breakpoint
    'break main.c:25 if x > 10         ' Conditional breakpoint in file
    'rbreak main.c:.*                  ' Regex breakpoint (all functions in file)
    'info line main.c:25               ' Show address of line
    'list main.c:25                    ' Show source at line
    
    ' Iterate all pDoc in the project/files list
    dim as string sBreakpoints
    dim as string bp_cmd
    dim as string bp_file
    dim as string bp_line
    dim as long   nCount = 0
    dim as long   numBreakpoints = 0
    
    dim pDoc as clsDocument ptr = gApp.pDocList
    do until pDoc = 0
        if pDoc->hWindow(0) <> null then 
            sBreakpoints = pDoc->GetBreakpoints()
        end if
        
        bp_file = pDoc->DiskFilename

        if len(sBreakpoints) then
            nCount = AfxStrParseCount( sBreakpoints, "," )
            for i as long = 1 to nCount
                ' Line numbers in GDB are 1 based whereas Scintilla is 0 based.
                bp_line = AfxStrParse( sBreakpoints, i, "," )
                bp_cmd = bp_cmd & "break " & chr(34) & bp_file & chr(34) & ":" & str(val(bp_line)+1) & chr(10)
'                bp_cmd = bp_cmd & "-break-insert --source " & chr(34) & bp_file & chr(34) & " --line " & str(val(bp_line)+1) & chr(10)
                numBreakpoints += 1
            next
        end if
            
        pDoc = pDoc->pDocNext
    loop

    ' If no breakpoints exist then set a temporary breakpoint in the main file
    if numBreakpoints = 0 then
'LM( "Set Temp Breakpoint on Main")
        bp_cmd = "tbreak main"
'        bp_cmd = "-break-insert -t main"
        gdb_send( gdb_session, bp_cmd )
        
    else    
'LM( "Set Breakpoints:")
'LM( bp_cmd & chr(13,10) )
        bp_cmd = rtrim(bp_cmd, chr(10))
        gdb_send( gdb_session, bp_cmd )
    end if

    function = 0    
end function
        
' ========================================================================================
' frmDebug_Show
' ========================================================================================
function frmDebug_Show( byval hWndParent as HWND, byval executable as string ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow

    HWND_FRMDEBUG = pWindow->Create( hWndParent, "DEBUGGER", @frmDebug_WndProc, 0, 0, 0, 0, _
        WS_POPUP or WS_THICKFRAME  or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT or WS_EX_LEFT )
    pWindow->SetClientSize(625, 460)
    pWindow->Center(pWindow->hWindow, hWndParent)

    ' Set the small and large icon for the main window (must be set after main window is created)
    pWindow->BigIcon   = LoadImage( pWindow->InstanceHandle, "IMAGE_AAA_MAINICON", IMAGE_ICON, 32, 32, LR_SHARED)
    pWindow->SmallIcon = LoadImage( pWindow->InstanceHandle, "IMAGE_AAA_MAINICON", IMAGE_ICON, 16, 16, LR_SHARED)

    dim as long nvalue = false
    if instr(gConfig.ThemeShortFilename, "_dark") then nvalue = true
    ' Attempt to apply the standard Windows dark theme to the non-client areas of the form.
    DwmSetWindowAttribute(HWND_FRMDEBUG, DWMWA_USE_IMMERSIVE_DARK_MODE, @nvalue, sizeof(nvalue))
    
    gApp.IsDebuggerActive = true
    
    frmDebug_PositionWindows()

    SetTimer(HWND_FRMDEBUG, gApp.DebugTimerID, 20, null)   ' 20 ms
    
    ' The debug executable has been sucessfully created so start the debug thread
    ' add feed any breakpoints into it.
    if gdb_init( gdb_session, executable ) then
        frmDebug_SetInitialBreakpoints( gdb_session )
        gdb_send(gdb_session, "set confirm off" )
        gdb_send(gdb_session, "set language pascal" )
        
        gdb_session.hThreadMutex = MutexCreate()
        gdb_session.hThreadMessages = ThreadCreate( @gdb_threadListener, @gdb_session )
        
    else
        MessageBox( HWND_FRMMAIN, "Failed to initialize GDB. Ensure gdb.exe is installed correctly.", _
                    "Error", MB_OK or MB_ICONERROR)
        DestroyWindow( HWND_FRMMAIN )
        exit function            
    end if
    
'    gdb_send(gdb_session, "run")
    gdb_send(gdb_session, "-exec-run")
    
    ShowWindow( HWND_FRMDEBUG, SW_SHOW )
    
    return 0
end function






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



sub gdb_processResponse( byval response as string ) 
    dim as string file_name 
    dim as string function_name 
    dim as string function_args
    dim as integer line_number


    if instr( response, "*stopped,reason=" & chr(34) & "breakpoint-hit" & chr(34)) orelse _
       instr( response, "*stopped,reason=" & chr(34) & "end-stepping-range" & chr(34)) then
        ' Line numbers in GDB are 1 based whereas Scintilla is 0 based.
        line_number = val( gdb_findResultValue( response, "line" ) ) - 1
        file_name = gdb_findResultValue( response, "fullname" )
        file_name = AfxStrReplace( file_name, "\\", "\" )
        file_name = AfxStrReplace( file_name, "/", "\" )
        function_name = gdb_findResultValue( response, "func" )
        function_args = gdb_findFunctionArguments( response, "args" )

LM( response )
LM( "STOP: BREAKPOINT OR STEPPING" )
LM( "  file_name:     " & file_name )
LM( "  function_name: " & function_name )
LM( "  function_args: " & function_args )
LM( "  line_number:   " & str(line_number) )

        if (len(file_name) > 0) andalso (line_number >= 0) then
            dim as clsDocument ptr pDoc = OpenSelectedDocument( file_name, "", line_number)
            if pDoc then
                SciExec( pDoc->hWndActiveScintilla, SCI_MARKERDELETEALL, MARKER_DEBUGGER_CURLINE, 0 )
                pDoc->ToggleMarker( line_number, MARKER_DEBUGGER_CURLINE )
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
    dim as long nLeft = rc.left
    dim as long nSpacer = pWindow->ScaleX(4)
    dim as long nHeight = pWindow->ScaleY(TOPTABS_HEIGHT)
    dim as long nPadding = 10   ' 5 left 5 right
    dim as long nImageWidth = 12

    i = DEBUG_BUTTONS.CONTINUE_DEBUGGING
    gDbgBtns(i).wszIcon    = wszIconContinue
    gDbgBtns(i).wszText    = L(321,"Continue")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    gDbgBtns(i).nID        = IDM_DEBUG_STARTDEBUGGING
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STOP_DEBUGGING
    gDbgBtns(i).wszIcon    = wszIconStop
    gDbgBtns(i).wszText    = L(322,"Stop")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    gDbgBtns(i).nID        = IDM_DEBUG_STOPDEBUGGING
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STEP_INTO
    gDbgBtns(i).wszIcon    = wszIconStepNext
    gDbgBtns(i).wszText    = L(311,"Step Next")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    gDbgBtns(i).nID        = IDM_DEBUG_STEPNEXT
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STEP_OVER
    gDbgBtns(i).wszIcon    = wszIconStepOver
    gDbgBtns(i).wszText    = L(312,"Step Over")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    gDbgBtns(i).nID        = IDM_DEBUG_STEPOVER
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.STEP_OUT
    gDbgBtns(i).wszIcon    = wszIconStepOut
    gDbgBtns(i).wszText    = L(319,"Step Out")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    gDbgBtns(i).nID        = IDM_DEBUG_STEPOUT
    nLeft = gDbgBtns(i).rc.right + nSpacer

    i = DEBUG_BUTTONS.RUN_TO_CURSOR
    gDbgBtns(i).wszIcon    = wszIconRunToCursor
    gDbgBtns(i).wszText    = L(320,"Run to Cursor")
    gDbgBtns(i).rc.Left    = nLeft
    gDbgBtns(i).rc.top     = rc.top
    gDbgBtns(i).rc.right   = nLeft + _
        pWindow->ScaleX( getTextWidth(HWND_FRMDEBUG, gDbgBtns(i).wszText, GUIFONT_9) + nPadding + nImageWidth)
    gDbgBtns(i).rc.bottom  = rc.top + nHeight
    gDbgBtns(i).nID        = IDM_DEBUG_RUNTOCURSOR
    nLeft = gDbgBtns(i).rc.right + nSpacer

    AfxRedrawWindow( HWND_FRMDEBUG )
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

    static as HWND hTooltip
    if (uMsg = WM_DESTROY) andalso (IsWindow(hToolTip)) then DestroyWindow(hToolTip)

    select case uMsg

        case WM_CLOSE
            LogClose()
            KillTimer( HWND_FRMDEBUG, gApp.DebugTimerID )
            gdb_close( gdb_session )
            DestroyWindow( hwnd )
            
        case WM_NCDESTROY
            dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
            if pWindow then delete(pWindow)
            gdb_session.KillMessageThread = true
            ThreadWait( gdb_session.hThreadMessages )
            MutexDestroy( gdb_session.hThreadMutex )

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
                    gdb_processResponse( message )    
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
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                gDbgBtns(i).isHot = false
            next
            prev_idxHot = -1
            curr_idxHot = -1
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
'            dim as DWSTR wszTooltip
'            if IsMouseOverRECT(hwnd,  gDbgBtns(STATUSBAR_IDPANEL.CONNECT_PANEL).rc ) then 
'                if gEClient.IsConnected() then
'                    wszTooltip = "Click to Disconnect"
'                end if    
'            end if
'            if (wszTooltip = "") andalso (IsWindow(hToolTip)) then 
'                DestroyWindow(hToolTip)
'            else    
'                AfxSetTooltipText( hTooltip, hwnd, wszTooltip )
'            end if
            

        case WM_PAINT
            dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMDEBUG)
            dim as clsDoubleBuffer b

            b.BeginDoubleBuffer(hwnd)

            ' Set default back and fore colors
            b.SetForeColors( ghTopTabs.ForeColor, ghTopTabs.ForeColorHot )
            b.SetBackColors( ghTopTabs.BackColor, ghTopTabs.BackColorHot )

            ' Clear the entire back client area
            b.PaintClientRect()
          
            ' paint the panels
            for i as long = lbound(gDbgBtns) to ubound(gDbgBtns)
                ' paint this tab based on active/inactive status
                if i = curr_idxHot then
                    b.SetForeColors( ghTopTabs.ForeColorHot, ghTopTabs.ForeColorHot )
                    b.SetBackColors( ghTopTabs.BackColorHot, ghTopTabs.BackColorHot )
                else
                    b.SetForeColors( ghTopTabs.ForeColor, ghTopTabs.ForeColor )
                    b.SetBackColors( ghTopTabs.BackColor, ghTopTabs.BackColor )
                end if

                b.PaintRect( @gDbgBtns(i).rc )
                
                dim as long nImageWidth = pWindow->ScaleX(12)
                
                dim as RECT rcIcon = gDbgBtns(i).rc
                rcIcon.right = rcIcon.left + nImageWidth
                
                dim as RECT rcText = gDbgBtns(i).rc
                rcText.left = rcIcon.right
                
                b.SetFont( SYMBOLFONT_9 )
                b.PaintText( gDbgBtns(i).wszIcon, @rcIcon, DT_CENTER )

                b.SetFont( GUIFONT_9 )
                b.PaintText( gDbgBtns(i).wszText, @rcText, DT_CENTER )
            next
                                    
            b.EndDoubleBuffer()

            return 0

        case IDM_DEBUG_STARTDEBUGGING
            gdb_send(gdb_session, "continue")
        
        case IDM_DEBUG_STOPDEBUGGING
            gdb_send(gdb_session, "kill")
            PostMessage( HWND_FRMDEBUG, WM_CLOSE, 0, 0 )
        
        case IDM_DEBUG_STEPNEXT
            gdb_send(gdb_session, "step")
        
        case IDM_DEBUG_STEPOVER
            gdb_send(gdb_session, "next")
        
        case IDM_DEBUG_STEPOUT
            gdb_send(gdb_session, "finish")

        case IDM_DEBUG_RUNTOCURSOR
            'TODO: Set a temp breakpoint?

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
                numBreakpoints += 1
            next
        end if
            
        pDoc = pDoc->pDocNext
    loop

    ' If no breakpoints exist then set a temporary breakpoint in the main file
    if numBreakpoints = 0 then
LM( "Set Temp Breakpoint on Main")
        bp_cmd = "tbreak main"
        gdb_send( gdb_session, bp_cmd )
        
    else    
LM( "Set Breakpoints:")
LM( bp_cmd & chr(13,10) )
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
        WS_POPUP or WS_CAPTION or WS_SYSMENU or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT or WS_EX_LEFT )
    pWindow->SetClientSize(625, 460)
    pWindow->Center(pWindow->hWindow, hWndParent)

    dim as long nvalue = false
    if instr(gConfig.ThemeShortFilename, "_dark") then nvalue = true
    ' Attempt to apply the standard Windows dark theme to the non-client areas of the form.
    DwmSetWindowAttribute(HWND_FRMDEBUG, DWMWA_USE_IMMERSIVE_DARK_MODE, @nvalue, sizeof(nvalue))
    
    gApp.IsDebuggerActive = true
    
    frmDebug_PositionWindows()

    LogInit( "_debug.txt" )
    
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
    

    'TODO: If no breakpoints exist then create temp breakpoint to start at beginning
    ' of the main file.
    
    gdb_send(gdb_session, "run")
    
    ShowWindow( HWND_FRMDEBUG, SW_SHOW )
    
    return 0
end function






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

#pragma once

#define IDC_FRMDEBUG_CMDOK      1000
#define IDC_BTN_RUN             1001
#define IDC_BTN_CONTINUE        1002
#define IDC_BTN_BREAK           1003
#define IDC_BTN_STOP            1004
#define IDC_BTN_STEP_OVER       1005
#define IDC_BTN_STEP_INTO       1006
#define IDC_BTN_STEP_OUT        1007
#define IDC_FRMDEBUG_TXTOUTPUT  1008

enum DEBUG_BUTTONS explicit
    BUTTON_FIRST         = 0
    CONTINUE_DEBUGGING   = 0
    STOP_DEBUGGING       = 1
    STEP_INTO            = 2
    STEP_OVER            = 3
    STEP_OUT             = 4
    RUN_TO_CURSOR        = 5
    BUTTON_LAST          = RUN_TO_CURSOR
end enum

type DEBUG_BUTTONS_TYPE
    wszToolTip as DWSTRING
    wszIcon    as DWSTRING
    wszText    as DWSTRING
    rc         as RECT            ' client coordinates 
    nID        as long            ' id to invoke if clicked on
    isHot      as boolean
end type

dim shared gDbgBtns(DEBUG_BUTTONS.BUTTON_LAST) as DEBUG_BUTTONS_TYPE

const as long DEBUG_BUTTON_IMAGE_WIDTH = 18

declare function frmDebug_Show( byval hWndParent as HWND, byval executable as string ) as LRESULT

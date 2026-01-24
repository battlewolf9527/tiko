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

#pragma once


#define IDC_FRMGOTO_LBLLASTLINE                     1000
#define IDC_FRMGOTO_LBLCURRENTLINE                  1001
#define IDC_FRMGOTO_LBLGOTOLINE                     1002
#define IDC_FRMGOTO_TXTLINE                         1003
#define IDC_FRMGOTO_LBLLASTVALUE                    1004
#define IDC_FRMGOTO_LBLCURRENTVALUE                 1005

declare function frmGoto_Show( byval hWndParent as HWND ) as LRESULT

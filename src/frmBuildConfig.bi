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


#define IDC_FRMBUILDCONFIG_LIST1                    1000
#define IDC_FRMBUILDCONFIG_LABEL1                   1001
#define IDC_FRMBUILDCONFIG_TXTDESCRIPTION           1002
#define IDC_FRMBUILDCONFIG_LABEL2                   1003
#define IDC_FRMBUILDCONFIG_TXTOPTIONS               1004
#define IDC_FRMBUILDCONFIG_CMDUP                    1005
#define IDC_FRMBUILDCONFIG_CMDDOWN                  1006
#define IDC_FRMBUILDCONFIG_CMDINSERT                1007
#define IDC_FRMBUILDCONFIG_CMDDELETE                1008
#define IDC_FRMBUILDCONFIG_OPT32                    1009
#define IDC_FRMBUILDCONFIG_OPT64                    1010
#define IDC_FRMBUILDCONFIG_LSTOPTIONS               1011
#define IDC_FRMBUILDCONFIG_CHKISDEFAULT             1012
#define IDC_FRMBUILDCONFIG_CHKEXCLUDEPOPUP          1013
#define IDC_FRMBUILDCONFIG_LABEL3                   1014
#define IDC_FRMBUILDCONFIG_CHKCTRL                  1015
#define IDC_FRMBUILDCONFIG_CHKALT                   1016
#define IDC_FRMBUILDCONFIG_CHKSHIFT                 1017
#define IDC_FRMBUILDCONFIG_COMBOKEY                 1018

#define FRMBUILDCONFIG_LISTBOX_LINEHEIGHT  20 

common shared as HACCEL ghAccelBuildConfigurations

declare function frmBuildConfig_CreateAcceleratorTable() as long
declare function frmBuildConfig_getActiveBuildIndex() as long
declare function frmBuildConfig_GetSelectedBuildDescription() as DWSTRING
declare function frmBuildConfig_GetSelectedBuildGUID() as string
declare function frmBuildConfig_GetDefaultBuildGUID() as string
declare function frmBuildConfig_Show( byval hWndParent as HWND ) as LRESULT


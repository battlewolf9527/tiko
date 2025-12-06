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

#include once "frmPanel.bi"

#define IDC_FRMLISTVIEW_HEADER               1000
#define IDC_FRMLISTVIEW_LISTBOX              1001
#define IDC_FRMLISTVIEW_VSCROLL              1002

type LISTVIEW_COLUMN_TYPE
    wszText as DWSTRING            ' row 0 is the header data
    nWidth as long
end type

type LISTVIEW_DATA_TYPE
    hVScroll as HWND            ' manually allocated needs to be delete
    hHeader as HWND             ' normal child control
    hListBox as HWND            ' normal child control
    ColData(any) as LISTVIEW_COLUMN_TYPE
    ForeColor as COLORREF
    ForeColorHot as COLORREF
    BackColor as COLORREF
    BackColorHot as COLORREF
    BackColorScrollBar as COLORREF
    ForeColorScrollBar as COLORREF
    DividerColorScrollBar as COLORREF
    VScrollData as PANEL_VSCROLL_TYPE
end type


declare function frmListView_Refresh( byval hLV as HWND ) as LRESULT
declare function frmListView_Show( byval hwndParent as hwnd ) as HWND


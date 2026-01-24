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


' =====================================================================================
' CPanelWindow class (extends the Afx base CWindow class)
' Creates a generic window with no extra GDI reources.
' =====================================================================================
type CPanelWindow extends CWindow

   private:
      m_dummy as byte  ' b/c can not have empty class  

   public:
      declare function Create (byval hParent as HWND = null, byval lpfnWndProc as WNDPROC = null, _
        byval x as long = CW_USEDEFAULT, byval y as long = CW_USEDEFAULT, byval nWidth as long = CW_USEDEFAULT, _
        byval nHeight as long = CW_USEDEFAULT, byval dwStyle as UINT = WS_CHILD or WS_CLIPCHILDREN or WS_CLIPSIBLINGS, _
        byval dwExStyle as UINT = WS_EX_CONTROLPARENT) as HWND
end type


private function CPanelWindow.Create (byval hParent as HWND = null, byval lpfnWndProc as WNDPROC = null, _
        byval x as long = CW_USEDEFAULT, byval y as long = CW_USEDEFAULT, byval nWidth as long = CW_USEDEFAULT, _
        byval nHeight as long = CW_USEDEFAULT, byval dwStyle as UINT = WS_CHILD or WS_CLIPCHILDREN or WS_CLIPSIBLINGS, _
        byval dwExStyle as UINT = WS_EX_CONTROLPARENT) as HWND
   if base.hWindow <> null then exit function
   dim as HINSTANCE InstanceHandle = base.InstanceHandle
   dim as ATOM wAtom = CWindow_RegisterClass("", InstanceHandle, lpfnWndProc)
   if wAtom = 0 then exit function
   dim as single rx = base.rxRatio
   dim as single ry = base.ryRatio
   ' // Create the window
   base.hWindow = .CreateWindowExW(dwExStyle, cast(LPCWSTR, cast(ULONG_PTR, cast(WORD, wAtom))), "", dwStyle, _
            IIF(x = CW_USEDEFAULT, CW_USEDEFAULT, x * rx), _
            IIF(y = CW_USEDEFAULT, CW_USEDEFAULT, y * ry), _
            IIF(nWidth = CW_USEDEFAULT, CW_USEDEFAULT, nWidth * rx), _
            IIF(nHeight = CW_USEDEFAULT, CW_USEDEFAULT, nHeight * ry), _
            hParent, null, InstanceHandle, cast(HANDLE, @this))
   if base.hWindow = null then exit function
   function = base.hWindow
   ' // Store the class pointer
   .SetWindowLongPtrW(base.hWindow, 0, cast(LONG_PTR, @this))
end function

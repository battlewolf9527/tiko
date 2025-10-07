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

declare function SetDocumentErrorPosition( byval hLV as HWnd, byval wID as long ) as long
declare function SetLogFileTextbox() as long
declare function ParseLogForError( _
            byref wsLogSt as CWSTR, _
            byval bAllowSuccessMessage as boolean, _
            byval wID as long, _
            byval fCompile64 as boolean, _
            byval fCompilingObjFiles as boolean _
            ) as boolean
declare function ResetScintillaCursors() as long
declare function RunEXE( byval pDocMain as clsDocument ptr, byref wszFileExe as CWSTR, byref wszParam as CWSTR ) as long
declare function SetCompileStatusBarMessage(byref wszText as wstring, byval hIconCompile as long) as LRESULT
declare function RedirConsoleToFile(byval wszExe as CWSTR, byval wszCmdLine as CWSTR, byref sConsoleText as string ) as long
declare function CreateTempResourceFile() as boolean


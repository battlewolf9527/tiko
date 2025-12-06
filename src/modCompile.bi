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

''
''  COMPILE_TYPE
''  Handle information related to the currnet compile process 
''
type COMPILE_TYPE
    MainFilename        as wstring * MAX_PATH   ' main source file (full path and file.ext)
    MainName            as wstring * MAX_PATH   ' main source file (Name only, no extension)
    MainFolder          as wstring * MAX_PATH   ' main source folder 
    ResourceFile        as wstring * MAX_PATH   ' full path and file.ext to resource file (if applicable) 
    TempResourceFile    as wstring * MAX_PATH   ' full path and file.ext to temporary resource file (if applicable) 
    OutputFilename      as wstring * MAX_PATH   ' resulting exe/dll/lib name 
    CompilerPath        as wstring * MAX_PATH   ' full path and file.ext to fbc.exe
    ObjFolder           as wstring * MAX_PATH   ' *.o for all modules (set depending on 32/64 bit) (full path)
    ObjFolderShort      as wstring * MAX_PATH   ' ".\" & APPEXTENSION & "\"
    ObjID               as wstring * MAX_PATH   ' "32" or "64" appended to object name
    LinkModules         as DWSTRING                ' From code embedded #LINKMODULES directive
    CompileFlags        as wstring * 2048
    CompileIncludes     as wstring * 2048       ' Additional user defined include paths
    wszFullCommandLine  as DWSTRING                ' Command line sent to the FB compiler
    wszFullLogFile      as DWSTRING                ' Full log file returned from the FB compiler
    wszOutputMsg        as DWSTRING                ' Additional info during compile process (time/filesize)
    RunAfterCompile     as boolean
    SystemTime          aS SYSTEMTIME           ' System time when compile finished
    StartTime           as double
    EndTime             as double
    CompileID           as long                 ' Type of compile (wID). Needed in case frmOutput listview later clicked on.
    bCompileThreadComplete as boolean
end type

type CompileThreadParams
    wszExe              as DWSTRING
    wszCmdLine          as DWSTRING
    sConsoleText        as string 
end type

declare sub compile_thread( byval userdata as any ptr )
declare function code_Compile( byval wID as long ) as boolean
declare function AddPathsToLinkModules( byval modules as DWSTRING ) as DWSTRING




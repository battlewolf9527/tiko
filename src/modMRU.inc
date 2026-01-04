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

#include once "modMRU.bi"

''
''
function clearMRUFilesItems( byval nParentID as long ) as long
    ' clear out any existing items that match the nParentID so that they can be
    ' reused when the MRU popup menu is created and shown. This function is used
    ' for Files and Projects and UserTools MRU lists.
    for i as long = lbound(gTopMenu) to ubound(gTopMenu)
        if gTopMenu(i).nParentID = nParentID then
            gTopMenu(i).nParentID = 0
            gTopMenu(i).nID = 0
            gTopMenu(i).nChildID = 0
        end if
    next
    function = 0
end function

''
''
function getNextFreeMRUindex() as long
    ' gets the first available free MRU index in the gTopMenu array. if there are
    ' none free then the gTopMenu array is extended and that index is returned. This
    ' function is used for both Files and Projects MRU lists.
    dim as long nFoundAt = -1
    for i as long = lbound(gTopMenu) to ubound(gTopMenu)
        if gTopMenu(i).nParentID = 0 then return i
    next
    if nFoundAt = -1 then
        nFoundAt = ubound(gTopMenu) + 1
        redim preserve gTopMenu(nFoundAt) as TOPMENU_TYPE
    end if
    function = nFoundAt
end function


''
''
function updateMRUFilesItems() as long
    ' clear MRU items already existing in the gTopMenu array. We overwrite and extend
    ' the gTopMenu array rather than erase it because existing menus depend on the 
    ' array index that aready exist. The function also returns the width to use for
    ' the resulting popup menu (based on text width metrics of each filename).
    clearMRUFilesItems( IDM_MRU )
    
    dim wszText as wstring * 256
    dim as boolean hasMRUfiles = false
    dim as long txtWidth = 0
    dim as long nMenuWidth = 0
    dim as long margin_width = 40

    for i as long = 0 To 9
        ' if the file no longer exists then remove it from the MRU list
        gConfig.MRU(i) = ProcessFromCurdriveApp( gConfig.MRU(i) )
        if AfxFileExists( gConfig.MRU(i) ) = 0 then
            gConfig.MRU(i) = ""
        else
            hasMRUfiles = true
            setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRU, IDM_MRUBASE + i, 0, false, false )
            wszText = gConfig.MRU(i)
            txtWidth = getTextWidth( HWND_FRMMAIN_MENUBAR, wszText, GUIFONT_10 )
            if txtWidth > nMenuWidth then nMenuWidth = txtWidth
        end if
    next
    
    if hasMRUfiles = false then
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRU, IDM_MRUFILES, 0, false, false )  ' (empty)
    else
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRU, IDM_MRUFILES, 0, false, true ) 
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRU, IDM_MRUCLEAR, 0, false, false )
    end if   
    
    function = nMenuWidth + (margin_width * 2)  ' unscaled value
end function


''
''
function updateMRUSessionFilesItems() as long
    ' clear MRU items already existing in the gTopMenu array. We overwrite and extend
    ' the gTopMenu array rather than erase it because existing menus depend on the 
    ' array index that aready eist.
    clearMRUFilesItems( IDM_MRUSESSION )
    
    dim wszText as wstring * 256
    dim as boolean hasMRUfiles = false
    dim as long txtWidth = 0
    dim as long nMenuWidth = 0
    dim as long margin_width = 40

    for i as long = 0 To 9
        ' if the file no longer exists then remove it from the MRU list
        gConfig.MRUSESSION(i) = ProcessFromCurdriveApp( gConfig.MRUSESSION(i) )
        if AfxFileExists( gConfig.MRUSESSION(i) ) = 0 then
            gConfig.MRUSESSION(i) = ""
        else
            hasMRUfiles = true
            setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUSESSION, IDM_MRUSESSIONBASE + i, 0, false, false )
            wszText = gConfig.MRUSESSION(i)
            txtWidth = getTextWidth( HWND_FRMMAIN_MENUBAR, wszText, GUIFONT_10 )
            if txtWidth > nMenuWidth then nMenuWidth = txtWidth
        end if
    next
    
    if hasMRUfiles = false then
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUSESSION, IDM_MRUSESSIONFILES, 0, false, false )  ' (empty)
    else
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUSESSION, IDM_MRUSESSIONFILES, 0, false, true ) 
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUSESSION, IDM_MRUSESSIONCLEAR, 0, false, false )
    end if   
    
    function = nMenuWidth + (margin_width * 2)  ' unscaled value
end function


''
''
function updateMRUProjectFilesItems() as long
    ' clear MRU items already existing in the gTopMenu array. We overwrite and extend
    ' the gTopMenu array rather than erase it because existing menus depend on the 
    ' array index that aready eist.
    clearMRUFilesItems( IDM_MRUPROJECT )
    
    dim wszText as wstring * 256
    dim as boolean hasMRUfiles = false
    dim as long txtWidth = 0
    dim as long nMenuWidth = 0
    dim as long margin_width = 40
    
    for i as long = 0 To 9
        ' if the file no longer exists then remove it from the MRU list
        gConfig.MRUPROJECT(i) = ProcessFromCurdriveApp( gConfig.MRUPROJECT(i) )
        if AfxFileExists( gConfig.MRUPROJECT(i) ) = 0 then
            gConfig.MRUPROJECT(i) = ""
        else
            hasMRUfiles = true
            setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUPROJECT, IDM_MRUPROJECTBASE + i, 0, false, false )
            wszText = gConfig.MRUPROJECT(i)
            txtWidth = getTextWidth( HWND_FRMMAIN_MENUBAR, wszText, GUIFONT_10 )
            if txtWidth > nMenuWidth then nMenuWidth = txtWidth
        end if
    next
    
    if hasMRUfiles = false then
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUPROJECT, IDM_MRUPROJECTFILES, 0, false, false )  ' (empty)
    else
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUPROJECT, IDM_MRUPROJECTFILES, 0, false, true ) 
        setTopMenuMRUItem( getnextFreeMRUindex(), IDM_MRUPROJECT, IDM_MRUPROJECTCLEAR, 0, false, false )
    end if   
    
    function = nMenuWidth + (margin_width * 2)   ' unscaled value
end function


''
''
function OpenMRUFile( _
            byval hwnd as HWND, _
            byval wID as long _
            ) as long
    dim pDoc as clsDocument ptr
    dim wszFile as wstring * MAX_PATH

    wszFile = gConfig.MRU(wID - IDM_MRUBASE)
    
    pDoc = OpenSelectedDocument( wszFile )
    if pDoc then pDoc->SetProjectFileType

    LoadExplorerFiles()
    LoadFunctionsFiles()

    function = 0
end function


''
''
function ClearMRUlist( byval wID as long ) as long
    Select Case wID
        Case IDM_MRUCLEAR
            for i as long = 0 To 9
                gConfig.MRU(i) = ""
            next
        Case IDM_MRUSESSIONCLEAR
            for i as long = 0 To 9
                gConfig.MRUSESSION(i) = ""
            next
        Case IDM_MRUPROJECTCLEAR
            for i as long = 0 To 9
                gConfig.MRUPROJECT(i) = ""
            next
    end Select
    function = 0
end function


''
''
function UpdateMRUList( byref wzFilename as wstring ) as long
    dim idxExistsAt as long = -1   ' index of where filename already exists 
    dim i           as long
    dim wzFile      as wstring * MAX_PATH
    
    if gApp.IsProjectLoading then exit function
    if gApp.IsProjectActive then exit function
     
    ' Search the 10 MRU items to see if the filename already exists and
    ' also for an available entry to put the filename in.
    for i = 0 To 9
        wzFile = gConfig.MRU(i)
        wzFile = ProcessFromCurdriveApp(wzFile)
        if ucase(wzFile) = ucase(wzFilename) then
            idxExistsAt = i:  exit for
        end if   
    next
    
    ' if does not already exist then we simply need to move all entries
    ' down and add at the beginning of the list. The original 10th entry
    ' will now drop off the list.
    if idxExistsAt = -1 then idxExistsAt = 9
    
    for i = idxExistsAt To 1 step -1
        gConfig.MRU(i) = gConfig.MRU(i-1)
    next
    gConfig.MRU(0) = wzFilename 
    
    ' Save the MRU to the configuration file. Only write the MRU items
    ' the ini file rather than overwriting the whole file. This enables
    ' the editor to work better with external tools that manually modify the
    ' settings.ini file.
    gConfig.WriteMRU

    function = 0
end function  



''  SESSIONS  ''

''
''
function OpenMRUSessionFile( _
            byval hwnd as HWND, _
            byval wID as long _
            ) as long
    dim wzFile as wstring * MAX_PATH
    wzFile = gConfig.MRUSESSION(wID - IDM_MRUSESSIONBASE)
    gConfig.LoadSessionFile( wzFile )
    function = 0
end function

    
''
''
function UpdateMRUSessionList( byval wszFilename as DWSTRING ) as long

    dim idxExistsAt as long = -1   ' index of where filename already exists 
    dim i           as long
    
    ' Search the 10 MRU items to see if the filename already exists and
    ' also for an available entry to put the filename in.
    for i = 0 To 9
        gConfig.MRUSession(i) = ProcessFromCurdriveApp(gConfig.MRUSession(i))
        if ucase(gConfig.MRUSession(i)) = ucase(wszFilename) then
            idxExistsAt = i:  exit For
        end if   
    next
    
    ' if does not already exist then we simply need to move all entries
    ' down and add at the beginning of the list. The original 10th entry
    ' will now drop off the list.
    if idxExistsAt = -1 then idxExistsAt = 9

    for i = idxExistsAt to 1 step -1
        gConfig.MRUSession(i) = gConfig.MRUSession(i-1)
    next
    gConfig.MRUSession(0) = wszFilename 

    ' Save the MRU to the configuration file. Only write the MRU items
    ' the ini file rather than overwriting the whole file. This enables
    ' the editor to work better with external tools that manually modify the
    ' settings.ini file.
    gConfig.WriteMRUSessions

    function = 0
end function  


''  PROJECTS  ''

''
''
function OpenMRUProjectFile( _
            byval hwnd as HWND, _
            byval wID as long _
            ) as long
    dim wzFile as wstring * MAX_PATH
    wzFile = gConfig.MRUPROJECT(wID - IDM_MRUPROJECTBASE)
    frmMain_OpenProjectSafely(hwnd, wzFile) 
    function = 0
end function

    
''
''
function UpdateMRUProjectList( byval wszFilename as DWSTRING ) as long

    dim idxExistsAt as long = -1   ' index of where filename already exists 
    dim i           as long
    
    ' Search the 10 MRU items to see if the filename already exists and
    ' also for an available entry to put the filename in.
    for i = 0 To 9
        gConfig.MRUProject(i) = ProcessFromCurdriveApp(gConfig.MRUProject(i))
        if ucase(gConfig.MRUProject(i)) = ucase(wszFilename) then
            idxExistsAt = i:  exit For
        end if   
    next
    
    ' if does not already exist then we simply need to move all entries
    ' down and add at the beginning of the list. The original 10th entry
    ' will now drop off the list.
    if idxExistsAt = -1 then idxExistsAt = 9

    for i = idxExistsAt To 1 step -1
        gConfig.MRUProject(i) = gConfig.MRUProject(i-1)
    next
    gConfig.MRUProject(0) = wszFilename 

    ' Save the MRU to the configuration file. Only write the MRU items
    ' the ini file rather than overwriting the whole file. This enables
    ' the editor to work better with external tools that manually modify the
    ' settings.ini file.
    gConfig.WriteMRUProjects

    function = 0
end function  




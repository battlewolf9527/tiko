' logging.bas – implementation
#include once "file.bi"

dim shared logFile as integer
dim shared logEnabled as boolean = 0

sub LogInit(filename as string)
    if logEnabled = false then
        logFile = freefile()
        open filename for output as #logFile
        logEnabled = true
    end if
end sub

sub LM( byref msg as string)
    if logEnabled = false then exit sub
    print #logFile, "[" & time & "] " & msg
    FileFlush (logfile)
end sub

sub LogClose()
    if logEnabled then
        close #logFile
        logEnabled = false
    end if
end sub

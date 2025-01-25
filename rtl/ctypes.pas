unit ctypes;

interface

type
    {$ifdef UNIX}
    cint = integer;
    clong = int64;
    {$ define(POSIX) }
    {$else}
        {$ifdef WINDOWS}
        cint = integer;
        clong = int64;
        {$endif}
    {$endif}
implementation
end.
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
        clong = integer;
        {$endif}
    {$endif}
implementation
end.
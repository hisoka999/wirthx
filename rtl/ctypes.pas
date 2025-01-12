unit ctypes;

interface

type
    {$ifdef UNIX}
    cint = integer;
    clong = int64;
    {$endif}
    {$ifdef WINDOWS}
    cint = integer;
    clong = int64;
    {$endif}
implementation
end.
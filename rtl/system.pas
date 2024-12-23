unit system;

type
    PChar = ^char;
    CFile = ^char;
    File = record
               pointer: CFile;
               isOpen: boolean;
            end;

    function fgetc( F : CFile) : char; external 'c';
    function fopen(fileName: PChar;mode : PChar) : CFile; external 'c';
    function fclose(F : CFile) : integer; external 'c';

    procedure Assign(var F: File;FileName: String);
    begin
        F.pointer := fopen(pchar(Filename),pchar('rw'));
        F.isOpen := true;
    end;

    procedure CloseFile(var F: File);
    begin
        if F.isOpen then
            fclose(F.pointer);
    end;

    Procedure inc(var value: integer); inline;
    begin
        value := value + 1;
    end;

    Procedure dec(var value: integer); inline;
    begin
        value := value - 1;
    end;

    procedure Readln(var F: File; var value: string);
    var
        c : char  = 10;
    begin

        if F.isOpen then
        begin

            repeat
            begin
                c := fgetc(F.pointer);
                if c >= 32 and c <= 125 then
                begin
                    value := value + c;
                end;
            end;
            until c = 10 or c = 13 or c = -1;
        end;
    end;


begin

end.
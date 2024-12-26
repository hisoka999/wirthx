unit system;

type
    PChar = ^char;
    CFile = ^char;
    File = record
               pointer: CFile;
               isOpen: boolean;
            end;

    function fgetc(var F : CFile) : char; external 'c';
    function fopen(fileName: PChar;mode : PChar) : CFile; external 'c';
    function fclose(var F : CFile) : integer; external 'c';

    procedure Assign(var F: File;FileName: String);
    begin
        F.pointer := fopen(pchar(Filename),pchar('rw'));
        if F.pointer != 0 then
            F.isOpen := true;
        else
            writeln('File not found: ',FileName);

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
        c : char  ;
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

    function CompareStr( S1,S2 : string) : integer;
    var
        idx : integer = 0;
        tmp : int64 = 0;
        length_S1 : int64;
        length_S2 : int64;
    begin
        length_S1 := length(S1);
        length_S2 := length(S2);
        CompareStr := length_S1 - length_S2;

        if CompareStr = 0 then
            for idx := 0 to length_S1 do
            begin
                tmp := S1[idx] - S2[idx];
                if tmp != 0 then
                begin
                    CompareStr := tmp;
                    break;
                end;
            end;
    end;


begin

end.
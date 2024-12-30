unit system;

type
    PChar = ^char;
    CFile = pointer;
    File = record
               ptr: pointer;
               isOpen: boolean;
            end;

    function fgetc(var F : CFile) : char; external 'c';
    function fopen(fileName: PChar;mode : PChar) : CFile; external 'c';
    function fclose(var F : CFile) : integer; external 'c';
    procedure cfree(F : PChar); external 'c' name 'free';

    procedure freemem(var F: PChar);inline;
    begin
        if F != 0 then
        begin
            cfree(F);
            F := 0;
        end;
    end;

    procedure Assign(var F: File;FileName: String);
    begin
        F.ptr := fopen(pchar(Filename),pchar('rw'));
        if F.ptr != 0 then
            F.isOpen := true;
        else
            writeln('File not found: ',FileName);

    end;

    procedure CloseFile(var F: File);
    begin
        if F.isOpen then
            fclose(F.ptr);
    end;

    Procedure inc(var value: integer); inline;
    begin
        value := value + 1;
    end;

    Procedure inc(var value: int64); inline;
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
        buffer : array [0..100] of char;
        offset : int64 = 0;
        bufferIndex : int64 = 0;
        i : int64;
        j: int64;
    begin

        if F.isOpen then
        begin

            repeat
            begin
                c := fgetc(F.ptr);

                if c >= 32 and c <= 125 then
                begin
                    //value := value + c;
                    buffer[bufferIndex] := c;
                    inc(bufferIndex);
                    // flush buffer
                    if high(buffer) = bufferIndex  then
                    begin
                        SetLength(value,offset + bufferIndex+1);
                        for i := 0 to bufferIndex - 1  do
                        begin
                            value[i+offset] := buffer[i];
                        end;
                        offset := offset + bufferIndex;
                        bufferIndex := 0;
                    end;

                end;

            end;
            until c = 10 or c = 13 or c = -1;

            SetLength(value,offset + bufferIndex);
            for j := 0 to bufferIndex - 1 do
            begin
                value[j+offset] := buffer[j];
            end;
            value[bufferIndex+offset] := 0;
        end;
    end;

    function CompareStr( S1,S2 : string) : integer;
    var
        idx : integer = 0;
        tmp : int64 = 0;
        length_S1 : int64;
        length_S2 : int64;
        max :int64;
    begin
        length_S1 := length(S1);
        length_S2 := length(S2);
        CompareStr := length_S1 - length_S2;
        max := length_S1 - 1;
        if CompareStr = 0 then
            for idx := 0 to max do
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
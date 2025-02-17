{
 Core System Unit
}
unit system;

interface
    type
        PChar = ^char;



    {
     Releases the given memory
    }
    procedure freemem(var F: PChar);
    procedure AssignFile(var F: File;FileName: String); external;
    procedure CloseFile(var F: File); external;
    Procedure inc(var value: integer);
    Procedure inc(var value: int64);
    Procedure dec(var value: integer);
    Procedure dec(var value: int64);
    {
        opens the file for reading
    }
    Procedure reset(var F: file);external;

    {
        @param(F file to read)
        @param(value string to read the file into)
    }
    procedure Readln(var F: File; var value: string); external;
    {
        returns 1 if the string S2 is greater then S1, -1 if the string is smaller and 0 if both are equal
        @param( S1 first string to compare)
        @param( S2 second string to compare with)
        @returns( 0 if equal)
    }
    function CompareStr( S1,S2 : string) : integer;

implementation
uses ctypes;


    procedure cfree(F : PChar); external 'c' name 'free';

    procedure freemem(var F: PChar);inline;
    begin
        if F != 0 then
        begin
            cfree(F);
            F := 0;
        end;
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
    Procedure dec(var value: int64); inline;
    begin
        value := value - 1;
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
end.
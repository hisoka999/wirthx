program problem4;

    function Str(value: integer):string;inline;
    var
        tmp : integer ;
        buffer: array [0..100] of char;
        i  :int64 = 0;
        k : int64 = 0;
        j : int64 = 0;
    begin
        tmp := value;
        if tmp < 0 then
            tmp := tmp * -1;

        while tmp > 0 do
        begin
            buffer[i] := tmp mod 10 + '0';
            tmp := tmp / 10;
            inc(i);
        end;

        if value < 0 then
        begin
            buffer[i] := '-';
            inc(i);
        end;
        SetLength(Str,i+1);
        Str[high(Str)] := 0;

        for j := i - 1 downto 0 do
        begin
            Str[k] := buffer[j];
            inc(k);
        end;
    end;


    function is_palindrom(product : integer) : boolean;inline;
    var
        tmp : string;
        h : int64;
    begin
        tmp := Str(product);
        is_palindrom := true;
        for i := 0 to length(tmp) / 2 do
        begin
            if tmp[i] != tmp[length(tmp) - 1 - i] then
            begin
                is_palindrom := false;
                break;
            end;
        end;
    end;

var
    product : integer;
    palindrom : integer = 0;
    tmp : string;
    rev : string;
begin

    for x := 100 to 1000 do
    begin
        for y := 100 to 1000 do
        begin
            product := x * y;
            if is_palindrom(product) and product > palindrom then
                palindrom := product;

        end;
    end;
    writeln('max palindrom = ',palindrom);
end.
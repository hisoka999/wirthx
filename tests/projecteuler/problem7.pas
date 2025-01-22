program problem7;


var
    count : integer = 2;
    prime : integer = 3;

    arr : array [0..205000] of boolean;
    k : integer ;
begin

    arr[0] := true;
    arr[1] := true;
    while true do
    begin
        k := 2 * prime;
        while k < length(arr) do
        begin
            arr[k] := true;
            k := k + prime;
        end;

        k := 2 + prime;

        while k < length(arr) and arr[k] do
        begin
            k := k + 2;
        end;

        if k < length(arr) then
        begin
            prime := k;
            inc(count);
            if count = 10001 then
            begin
                writeln(prime);
                break;
            end;
        end;
        else
            break;

    end;
end.
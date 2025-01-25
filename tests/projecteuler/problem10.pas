program problem10;


var
    count : integer = 2;
    prime : integer = 3;

    arr : array [0..2500000] of boolean;
    k : integer ;
    sum : int64 = 5;
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

            if prime >= 2000000 then
            begin
                writeln(sum);
                break;
            end;
            sum := sum + prime;
        end;
        else
            break;
    end;
end.
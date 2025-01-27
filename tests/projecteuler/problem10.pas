program problem10;


var
    prime : integer = 3;

    arr : array of boolean;
    k : int64  = 0 ;
    i : integer  = 0 ;
    sum : int64 = 5;
begin
    SetLength(arr,2500000);

    for i:= 2 to high(arr) do
        arr[i] := false;
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
        begin
            writeln('error');
            break;
        end;
    end;
end.
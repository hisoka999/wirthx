program arraytest;

type
    TValues = array [1..100] of integer;
var
    values : TValues;
    idx : integer;
begin

    for idx := 1 to 100 do
    begin
        values[idx] := idx * idx;
    end;
    for idx := 1 to 100 do
    begin
        writeln(values[idx]);
    end;
end.
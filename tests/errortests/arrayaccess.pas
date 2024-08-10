program arrayaccess;

type 
    TValues = array[1..100] of integer;
var 
    val : TValues;
    i: integer;
begin
    for i := low(val) to high(val) do
    begin
        val[i] := 0;
    end;

    writeln(val[0]);
    writeln(val[200]);
end.
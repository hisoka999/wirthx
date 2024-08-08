program customint;

type
    int64_t = int64;
var
    myintvalue : int64_t = 0;
begin
    writeln(myintvalue);
    for myintvalue := 0 to 500 do
    begin
     if myintvalue = 500 then
        writeln(myintvalue);
    end;

end.
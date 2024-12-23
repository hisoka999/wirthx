program whileloop;

var 
    a : integer;
begin
    a := 1;
    while a < 6 do
    begin
        write ('a = ',a,#10);
        inc(a);
    end;
end.
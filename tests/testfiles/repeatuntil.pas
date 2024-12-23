program repeatuntil;

var
    x : integer = 0;
    sum : integer = 0;
begin
    repeat
    begin
        inc(x);
        inc(x);
        sum:= sum + x;
        writeln('x = ',x);
    end;
    until x = 100;
    writeln('sum: ',sum);
end.
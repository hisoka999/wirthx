program forloop;

var
    sum :integer;
    count : integer;
    tmp: integer;
begin
    sum := 0;

    for count := 1 to 100 do
    begin
        sum := sum + count;
        if count >= 38 then 
            break;
    end;
    writeln('Count: ');
    writeln(count);
    writeln('Sum: ');
    writeln(sum);

end.
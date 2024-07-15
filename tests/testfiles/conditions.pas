program conditions;


var 
    con1 : integer;
    con2 : integer;
begin
    con1 := 100;
    con2 := 50;

    if con1 > con2 then
     begin
        writeln('100 > 50');
    end;

    if 50 >= con2 then
        writeln('50 >= con2');
    
    
    if 50 >= con1 then
    begin
        writeln('unreachable');
    end
    else
    begin
        writeln('con1 >= 50');;
    end;
    
end.
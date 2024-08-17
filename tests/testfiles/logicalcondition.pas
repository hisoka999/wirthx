program logicalcondition;

var
    mybool : boolean = true;
    demo : integer = 50;
    value : int64 = 50000;
begin
    if mybool and demo = 50 then
        writeln('mybool and demo = 50');
    
    mybool:= false;
    if not mybool then
        writeln('not mybool');
    else
        writeln('mybool');
    
    demo := 1;
    if demo > 10 or value >= 5000 then
        writeln('demo > 10 || value >= 5000');

end.
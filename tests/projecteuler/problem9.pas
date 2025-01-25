program problem9;

var
 a,b,c : integer;

begin 
    b := 2;
    c := 3;
    for a := 1 to 1000 do
    begin
        for b := a +1 to 1000 do
        begin
            for c:= b + 1 to 1000 do
            begin

                if (a + b + c) = 1000  and (a * a +  b * b) = (c*c) then
                begin
                    writeln(a,' + ',b,' + ',c,' = ',a + b + c);
                    writeln(a,' * ',b,' * ',c,' = ',a * b * c);
                    halt(0);
                end;

            end;
        end;

    end;

end.
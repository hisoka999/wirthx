Program helloWorld;

    Procedure println(param : string);
    begin
        writeln(param);
    end;
    
var
    myVar : integer;
    undefined: string;
begin
    myVar := 5+4;
    undefined := 'test';
    {my comment}
	println('Hello world!');
    if myVar = 90 then
    begin
        WriteLn(myVar);
        WriteLn(undefined);
    end
    else
    begin
        	println('my Else');
    end;
end.
Program helloWorld;

    Procedure println(param : string );
    begin
        writeln(param);
    end;

    function xadd(a : integer; b : integer): integer;
    begin
        xadd:=a+b;
    end;


    var
        myVar : integer;
        undefined: string;
        x : integer;
    begin
        myVar := 5+4;
        undefined := 'test';
        {my comment}
        println('Hello world!');
        if myVar = 90 then
        begin
            writeln(myVar);
            WriteLn(undefined);
        end
        else
        begin
                println('my Else');
        end;
        x := xadd(11,-5);
        writeln(x);

end.
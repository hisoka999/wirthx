Program functions;

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

    println('Print a line');
    x := xadd(11,-5);
    writeln(x);
end.
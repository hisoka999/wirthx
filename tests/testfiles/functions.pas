Program functions;

    Procedure println(param : string );
    begin
        writeln(param);
    end;

    function xadd(a : integer; b : integer): integer;
    begin
        xadd:=a+b;
    end;


    function xadd(x : int64; y : integer): int64;
    begin
        xadd:=x+y;
    end;

var
    myVar : integer;
    undefined: string;
    x : integer;
begin

    println('Print a line');
    x := xadd(11,-5);
    writeln(x);

    writeln(xadd(12,-7));

    writeln(xadd(120000000000,-7000));
end.
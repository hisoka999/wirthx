program externalfunction;

    function abs(value: integer): integer; external 'c';

    function myabs(x: integer): integer; external 'c' name 'abs';

begin

    writeln(abs(-1000));
    writeln(myabs(-1033));
end.
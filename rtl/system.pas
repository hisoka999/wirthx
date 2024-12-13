unit system;

    function Str(value: integer) : string;
    begin
        Str := ''; // TODO: implement the function
    end;

    Procedure write(param : string );
    begin
        puts(param); // TODO: puts expects a c string not a pascal string
    end;

    Procedure write(param : integer );
    begin
        write(str(param));
    end;

    procedure writeln(param: string);
    begin
        write(param);
        write('\n');
    end;

    Procedure writeln(param : integer );
    begin
        writeln(str(param));
    end;

    Procedure inc(var value: integer) inline;
    begin
        value := value + 1;
    end;

    Procedure dec(var value: integer) inline;
    begin
        value := value - 1;
    end;
begin

end.
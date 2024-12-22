program missing_return_type;

    {
        multi line comment to distract the lexer
        ....
    }

    function my_function(x,y : integer);
    begin
        my_function := 'test';
    end;
begin
    writeln(my_function(10,5);

end.
program wrong_return_type;

    //       multi line comment to distract the lexer
    //    ....

    function my_function(x,y : integer):string;
    begin
        my_function := x + y;
    end;
begin
    writeln(my_function(10,5);

end.
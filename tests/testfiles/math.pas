program math;

const
    MY_CONST = 234;
    TEST_CONST = 500;
var 
    test_var : integer;
    x : integer = 25;
begin
    test_var := -5000;
    writeln(test_var);

    test_var:= test_var * 2;

    writeln(test_var);

    test_var:= test_var + 4000;
    writeln(test_var);
    test_var:= 500;
    writeln(test_var);
    test_var:= 0;
    writeln(test_var);

    // test more complex integer math
    test_var := (TEST_CONST + 10) * x  + MY_CONST;
    writeln(test_var);
    test_var := TEST_CONST + 10 * x + MY_CONST;
    writeln(test_var);
    test_var := TEST_CONST + (10 * x) + MY_CONST;
    writeln(test_var);
end.
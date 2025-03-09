program singletest;

var
    number1 : single = 2.44;
    number2 : single = 3.55;

begin 

    if number1 - 2.44 <= 0.000001 then
        writeln('number1 is roughly ',number1);

    writeln('number1 + number2 = ', number1 + number2);
    writeln('number1 - number2 = ', number1 - number2);
    writeln('number1 * number2 = ', number1 * number2);
    writeln('number1 / number2 = ', number1 / number2);

end.
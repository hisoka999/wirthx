program problem2;

function fibonacci(n : int64) : int64;
begin
    if n <= 1 then
        fibonacci := n;
    else
        fibonacci := fibonacci(n - 1) + fibonacci(n - 2);
end;

var 
    sum : int64 = 0;
    i : int64 = 1;
        fib : int64;
begin
    for i:= 1 to 400 do
    begin
        fib := fibonacci(i);
        if fib > 4000000 then
            break;

        if fib mod 2 = 0 then
        begin
            sum:= sum + fib;
        end;

    end;
    writeln(sum);
end.
program problem1;
{
If we list all the natural numbers below 10
 that are multiples of 3 or 5, we get 3,5,6 and 9. 
 The sum of these multiples is 23.

Find the sum of all the multiples of 3 or 5 below 1000.
}

var 
    sum: integer = 0;
    i :int64;
begin
    for i:= 1 to 999 do
    begin
        if i mod 3 = 0 or i mod 5 = 0 then
            sum:= sum + i;
    end;
    
    writeln(sum);
end.
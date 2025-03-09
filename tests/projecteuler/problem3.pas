program problem3;

function largestPrimeFactor(value: int64) : int64;
var 
    i: int64 = 3;
    n : int64;
begin
    n := value;
    largestPrimeFactor := -1;
    
    // Check for factors of 2
    while n mod 2 = 0 do
    begin
        largestPrimeFactor := 2;
        n := n div 2;
    end;

    // Check for odd factors starting from 3
    while i * i <= n do
    begin
        while n mod i = 0 do
        begin
            largestPrimeFactor := i;
            n := n div i;
        end;

        i:= i + 2;
        
    end;

    // If n is still greater than 2, it is
    // a prime number
    if n > 2 then
        largestPrimeFactor := n;
end;


var

begin
    writeln(largestPrimeFactor(600851475143));
end.
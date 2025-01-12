program problem5;
var
    primes : array [0..7] of integer;
    divisor : integer;
    r : integer = 1;
begin
    primes[0] :=2;
    primes[1] :=3;
    primes[2] :=5;
    primes[3] :=7;
    primes[4] :=11;
    primes[5] :=13;
    primes[6] :=17;
    primes[7] :=19;

    for i := low(primes) to high(primes) do
    begin
        divisor := primes[i];
        while divisor <= 20 do
            divisor := divisor * primes[i];
        divisor := divisor div primes[i];
        r := r * divisor;
    end;
    writeln(r);
end.
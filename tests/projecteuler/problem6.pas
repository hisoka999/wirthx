program problem6;

    function square_sum(value : integer) : int64;
    var
        i : int64;
        tmp : int64;
    begin
        square_sum := 0;
        for i := 1 to value do
        begin
            tmp := i* i;
            square_sum := square_sum + tmp;
        end;
    end;

    function sum_and_square(value : integer) : int64;
    var
        i : int64;
    begin
        sum_and_square := 0;
        for i := 1 to value do
            sum_and_square := sum_and_square + i;

        sum_and_square := sum_and_square * sum_and_square;
    end;

var
    result : int64;
    s1 : int64;
    s2 : int64;
begin
    s1 := square_sum(100);
    s2 := sum_and_square(100);
    result := s2 - s1;
    writeln(s2,' - ',s1,' = ',result);
end.
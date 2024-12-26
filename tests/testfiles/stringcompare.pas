program stringcompare;

var
    test : String = 'Hello World';
    test2 : String;
    length_S1 : int64;
    length_S2 : int64;
begin
    test2 := test;

    length_S1 := length(test);
    length_S2 := length(test2);

    if test = test2 then
        writeln('test = test2');

    if test = 'New World' then
        writeln('test = New World');

    if test = 'Hello World' then
        writeln('test = Hello World');
    if test = 'Hello Wordd' then
        writeln('test = Hello Wordd');
    if CompareStr(test,test2) = 0 then
        writeln(test,' = ',test2);

end.
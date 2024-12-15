program stringtest;


var
  s, str1, str2, str3, str4: string;
  c: char;
  n: int64;
begin
    str1 := 'abc';     // assignment
    str2 := '123';     // string containing chars 1, 2 and 3
    str3 := #9#10;    // tab lf
    writeln(str1);
    writeln(str2);


    str4 := 'this is a ''quoted'' string';  // use of quotes within a string
    s := str1 + str2;  // concatenation
    c := s[1];         // use as index in array
    n := Length( s );  // length of string s
    

    writeln(str3);
    writeln(str4);
    writeln(s);
    writeln(c);
    writeln(n);
end.
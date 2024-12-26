program readfile;

var
    test : File;
    line : string ;
begin
    line := '';
    Assign(test,'testfiles/readfile.txt');
    Readln(test,line);
    writeln(line);
    CloseFile(test);
end.
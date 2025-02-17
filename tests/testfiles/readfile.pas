program readfile;

var
    test : File;
    line : string ;
begin
    line := '';
    AssignFile(test,'testfiles/readfile.txt');
    //reset(test);
    Readln(test,line);
    writeln(line);
    CloseFile(test);
end.
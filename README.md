# wirthx
Wirthx is an experimental pascal interpreter and compiler. 
The language is named after Nicolaus Wirth the creator of pascal.

## Compiler
The compiler is based on llvm and will generate a native binary for the target plattform. 
For now only `linux-x86-64` is supported.

## Interpreter

# Usage

```sh
wirthx testfiles/hello.pas
```
## Executing the compiler
The compiler will generate a native executable based on the program name defined in the program unit.

```sh
wirthx -c testfiles/hello.pas
```


# Examples

## Hello World
```pascal
program test

begin
    Writeln('Hello World');
end.
```
## Functions

```pascal
program test

    function addx(a : integer;b :integer): integer;
    begin
        addx := a + b;
    end;
var
    my_var : integer;
begin
    my_var := addx(1,2);
    Writeln(my_var);
end.
```
# wirthx
Wirthx is an experimental pascal compiler. 
The language is named after Nicolaus Wirth the creator of pascal.

## Compiler
The compiler is based on llvm and will generate a native binary for the target plattform. 
For now only `linux-x86-64` is supported.


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

## Records

```pascal
program test

    type Vec2 = record
        x : int64;
        y : int64;
    end;

    // pass the vector as a reference 
    procedure vec2_inc(var t : Vec2); 
    begin
        t.x := t.x + 1;
        t.y := t.y + 1;
    end;
var
    myvec : Vec2;
begin
    myvec.x := 2;
    myvec.y := 3;
    vec2_inc(myvec);
end.

```
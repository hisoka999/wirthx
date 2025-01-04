program pointer_test;
var
  a         : Integer = 100;
  ptrToInt  : PInteger;  // PInteger is a pointer type declared in the RTL
                         // which can only point to Integer value/variab
  ptrToInt2 : ^Integer;  // the general format for declaring a pointer type
begin
  ptrToInt := @a;
  Writeln(ptrToInt^);    // prints 100
  New(ptrToInt2);        // allocate an un-initialized Integer on the heap
  ptrToInt2^ := 222;     // assign a value to the heap allocated integer
  Writeln(ptrToInt2^);   // prints 222
end.
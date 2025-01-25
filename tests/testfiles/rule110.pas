program rule110;

const
  MAX_GEN = 48; // Number of generations
  SIZE = 50;   // Size of the cell array

type
  TCells = array[1..SIZE] of Integer;


// Function to calculate the next state of a cell based on its neighbors
//000 -> 0
//001 -> 1
//010 -> 1
//011 -> 1
//100 -> 0
//101 -> 1
//110 -> 1
//111 -> 0

function Rule(left, center, right: Integer): Integer;
var
    tmp : integer;
begin
    tmp := left * 4;
    tmp:= tmp + center * 2;
    tmp := tmp + right;
    if (tmp = 0) or (tmp = 4) or (tmp = 7) then
        Rule := 0
    else
        Rule := 1;
//  case (left * 4 + center * 2 + right) of
//    1, 2, 3, 5, 6: Rule := 1; // Rules for 110
//    else Rule := 0;
//  end;
end;

// Procedure to display the current state of the cells
procedure DisplayCells(var cells2: TCells);
var
  x: Integer;
begin
  for x := 1 to SIZE do
    if cells2[x] = 1 then
      Write('#')
    else
      Write(' ');
  Writeln();
end;

var
  cells, nextCells: TCells;
  i, gen: Integer;

begin
  // Initialize cells to have a single active cell in the middle
  for i := 1 to SIZE do
    cells[i] := 0;
  cells[SIZE] := 1;

  // Display initial state
  DisplayCells(cells);

  // Iterate through generations
  for gen := 1 to MAX_GEN do
  begin
    // Compute the next generation
    for i := 1 to SIZE do
    begin
      if i = 1 then
        nextCells[i] := Rule(0, cells[i], cells[i + 1])
      else if i = SIZE then
        nextCells[i] := Rule(cells[i - 1], cells[i], 0)
      else
        nextCells[i] := Rule(cells[i - 1], cells[i], cells[i + 1]);
    end;

    // Update cells and display the next generation
    cells := nextCells;
    DisplayCells(cells);
  end;
end.

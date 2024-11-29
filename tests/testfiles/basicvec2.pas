program basicvec2;

    type Vec2 = record
        x : int64;
        y : int64;
    end;

    function vec2_add( lhs,rhs : Vec2) : Vec2;
    begin
        vec2_add.x := lhs.x + rhs.x;
        vec2_add.y := lhs.y + rhs.y;
    end;

    procedure printvec( value: Vec2);
    begin
        writeln(value.x);
        writeln(value.y);
    end;

    procedure vec2_inc(var t : Vec2); 
    begin
        t.x := t.x + 1;
        t.y := t.y + 1;
    end;

var 
    myvec : Vec2;
    other : Vec2;
    res :Vec2;
begin
    myvec.x := 12;
    myvec.y := 13;
    printvec(myvec);   
     other.x := 15;
     other.y := 22;
     res := vec2_add(myvec,other);


     printvec(other);
     printvec(res);
     vec2_inc(other);
     printvec(other);
end.
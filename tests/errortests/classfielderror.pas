program classfielderror;

type
    TTest = class
    private
        FValue: Integer;
        FFloatValue: Double;
    public
        constructor Create(AValue: Integer);
    end;

    constructor TTest.Create(AValue: Integer);
    begin
        FValue := AValue;
        FFloatValue := 12.0; // Initialize Float value
    end;

var
    TestInstance: TTest;
begin
    TestInstance := TTest.Create(10);
    // The following lines will cause errors because FValue and FFloatValue are private
    WriteLn('Initial Value: ', TestInstance.FValue);
    WriteLn('Initial Float Value: ', TestInstance.FFloatValue);


end.
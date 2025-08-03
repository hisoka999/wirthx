program classtest;

type
  TTest = class
  private
    FValue: Integer;
    FFloatValue: Double;
  public
    constructor Create(AValue: Integer);
    function GetValue: Integer;
    procedure SetValue(AValue: Integer);

    function GetFloatValue: Double;
    procedure SetFloatValue(AValue: Double);
  end;

    constructor TTest.Create(AValue: Integer);
    begin
      FValue := AValue;
      FFloatValue := 12.0; // Initialize Float value
      //WriteLn('Constructor called with value: ', FValue);
    end;
    function TTest.GetValue: Integer;
    begin
      //result := FValue;
      exit(FValue);
        //WriteLn('GetValue called, returning: ', result);
    end;
    procedure TTest.SetValue(AValue: Integer);
    begin
      FValue := AValue;
        WriteLn('SetValue called, new value: ', FValue);
    end;

    function TTest.GetFloatValue: Double;
    begin
        result := FFloatValue;
    end;
    procedure TTest.SetFloatValue(AValue: Double);
    begin
        FFloatValue := AValue;
    end;

var
    TestInstance: TTest;
begin 
    TestInstance := TTest.Create(10);
    WriteLn('Initial Value: ', TestInstance.GetValue);
    TestInstance.SetValue(20);
    WriteLn('Updated Value: ', TestInstance.GetValue);


    WriteLn('Initial Float Value: ', TestInstance.GetFloatValue);
    TestInstance.SetFloatValue(3.14);
    WriteLn('Updated Float Value: ', TestInstance.GetFloatValue);
end.
program classvisibility;

type
  TTest = class
  private
    FValue: Integer;

  public
    // Public members can be accessed from anywhere
    constructor Create(AValue: Integer);
  protected
    // Protected members can be accessed by descendants

    function GetValue: Integer;

  end;

  constructor TTest.Create(AValue: Integer);
  begin
    FValue := AValue;
   end;
    function TTest.GetValue: Integer;
    begin
        exit(FValue);
        end;
var
    TestInstance: TTest;
begin 
    TestInstance := TTest.Create(10);
    // The following lines will cause errors because GetValue and SetValue are protected
    WriteLn('Initial Value: ', TestInstance.GetValue);
end.
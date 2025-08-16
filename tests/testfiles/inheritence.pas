program inheritence;

type
  TBase = class
  private
    FBaseValue: Integer;
  public
    constructor Create(AValue: Integer);
    function GetValue: Integer; virtual;
  end;

  TDerived = class(TBase)
  private
    FDerivedValue: Integer;
  public
    constructor Create(ABaseValue, ADerivedValue: Integer);
    function GetValue: Integer; override;
  end;
constructor TBase.Create(AValue: Integer);
begin
  FBaseValue := AValue;
end;
function TBase.GetValue: Integer;
begin
    result := FBaseValue;
end;

constructor TDerived.Create(ABaseValue, ADerivedValue: Integer);
begin
    inherited Create(ABaseValue);
    FDerivedValue := ADerivedValue;
end;
function TDerived.GetValue: Integer;
var tmp : Integer;
begin
    tmp := inherited GetValue(); // Call the base class method
    result := FDerivedValue + tmp;
end;
var
    BaseInstance: TBase;
    DerivedInstance: TDerived;
    Value: Integer;
begin
    BaseInstance := TBase.Create(10);
    DerivedInstance := TDerived.Create(20, 30);
    writeLn('Base Value: ', BaseInstance.GetValue); // Should output 10
    writeLn('Derived Value: ', DerivedInstance.GetValue); // Should output 60
end.
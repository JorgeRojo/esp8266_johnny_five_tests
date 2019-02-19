

unsigned int raw = 0;
float volt = 0.0;
float _res = 2;
float _maxVolt = 4.17;


void setup()
{
  Serial.begin(9600);
  pinMode(A0, INPUT);
  // analogReadResolution(32);
}

void loop()
{ 
  raw = analogRead(A0);

  // 2.08  --  3.3
  // x  --  1024
  
  volt = raw / 745.4; //1024 - 2.08  --  3.3
  volt = volt * 2;
  String v = String(volt);
  Serial.print("\t Raw: ");
  Serial.print(raw);
  Serial.print("\t Volts: ");
  Serial.println(v);
  delay(200);
}

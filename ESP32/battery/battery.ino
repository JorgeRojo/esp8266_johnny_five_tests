

unsigned int raw = 0;
float volt = 0.0;

void setup()
{
  Serial.begin(9600);
  pinMode(A0, INPUT);
  // analogReadResolution(32);
}

void loop()
{ 
  raw = analogRead(A0);
  volt = raw / 760.0;
  volt = volt * 2;
  String v = String(volt);
  Serial.print("\t Raw: ");
  Serial.print(raw);
  Serial.print("\t Volts: ");
  Serial.println(v);
  delay(100);
}

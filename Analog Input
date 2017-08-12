/* analogRead() converts the input voltage (0 to 5 volt range) to a digital value between 0 and 1023
 * Want a frequency variation of 0 to 120 Hz, so 
 */
 
#define inputPin A0
int analogIn = 0;
float frequency = 0.0; 
String str2;
String str1 = String("Frequency: ");

void setup() {
  pinMode(analogIn, INPUT);
}

void loop() {
  analogIn = analogRead(inputPin);
  if(analogIn < (1023/120)) //gives range to frequency to be zero
    frequency = 0.0;
  else 
    frequency = analogIn * (120/1023);
   
  str2 = str1 + frequency;
  Serial.println(str2);
}

/* analogRead() converts the input voltage (0 to 5 volt range) to a digital value between 0 and 1023
 * Want a frequency variation of 0 to 120 Hz, so 
 */
int analogIn = 0;
float frequency = 0.0; 
String str2;
String str1 = String("Frequency: ");

void setup() {
  Serial.begin(9600); //configure Baud rate
}

void loop() {
  analogIn = analogRead(A0);   //read analog data from pin A0
  frequency = (analogIn * 120.0)/1023.0; //convert to a frequency
  str2 = str1 + frequency + '\r'; //concatenate string
  Serial.print(str2); //Print to the console. Using PuTTY as the console
  delay(200); //delay for 200 ms
}

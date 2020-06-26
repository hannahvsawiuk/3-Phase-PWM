void setup() {
  Serial.begin(9600);
}
int input = 0;
char in_byte;


void loop() {
  if (Serial.available() > 0) {    /* is a character available?*/
    input = 0; /* clear previous value */
    while (1) {
      in_byte = Serial.read();
      if (in_byte == '\n') break;
      if (in_byte == -1) continue;
      input *= 10;
      input = in_byte - 48 + input;
      Serial.println(input);
    }
    Serial.println(input);
    
    /* check if a number was received*/
   /*if ((rx_byte > 0) && (rx_byte <= 120)) {
        Serial.print("Number received: ");
        Serial.println(rx_byte); 
      }
   else { 
      Serial.println("Not a valid entry");
    }*/
  }
}

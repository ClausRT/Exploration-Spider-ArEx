#include <Servo.h>

Servo motor;

int i = 0;


void setup() {
  motor.attach(3);

}

void loop() {
 
  motor.write(70);

  
  delay(2000);
  

 

}

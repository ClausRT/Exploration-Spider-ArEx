#include <Servo.h>

Servo motor;

int i = 0;


void setup() {
  motor.attach(3);

}

void loop() {
  for(i=70; i<130; i++){
  motor.write(i);
  delay(20);
  }
  
  delay(2000);
  
  for(i=130; i>70; i--){
    motor.write(i);
    delay(20); }
    
  delay(2000);
 

}

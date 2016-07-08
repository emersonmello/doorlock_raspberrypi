#include "wiring-gpio.h"

void doorlock(int open){
  
  if (open){
    digitalWrite (0, LOW); 
    digitalWrite (2, HIGH);
    //TODO - put here the conf for doorlock component
    delay (5000);
  }else{
    digitalWrite (0, HIGH); 
    digitalWrite (2, LOW);
    //TODO - put here the conf for doorlock component
  }
    
}

void setupWiring(){
    wiringPiSetup () ;
    
    // wiringPi pin 0 = Physical pin 11 (or BCM pin 17) on Raspberry Pi 2 B
    pinMode (0, OUTPUT) ; 
    // wiringPi pin 2 = Physical pin 13 (or BCM pin 27) on Raspberry Pi 2 B
    pinMode (2, OUTPUT) ;
    
    // red LED is always on when door is locked
    digitalWrite (0, HIGH) ;
    // green LED is always off when door is locked
    digitalWrite (2, LOW) ;
    
    //TODO - put here the conf for doorlock component
}
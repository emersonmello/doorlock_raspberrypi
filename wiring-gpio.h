/* 
 * File:   wiring-gpio.h
 * Author: Emerson Ribeiro de Mello <mello@ifsc.edu.br>
 *
 * Created on 08 July 2016, 12:07
 */

#ifndef WIRING_GPIO_H
#define WIRING_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wiringPi.h>

void doorlock(int open);
void setupWiring();
    
    
#ifdef __cplusplus
}
#endif

#endif /* WIRING_GPIO_H */


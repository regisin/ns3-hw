/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GPIOClass.h
 * Author: pregis
 *
 * Created on February 7, 2018, 3:25 PM
 */

#ifndef GPIOCLASS_H
#define GPIOCLASS_H

#include <iostream>

class GPIOClass {
public:
    GPIOClass();
    GPIOClass(std::string);
    virtual ~GPIOClass();
    
    int exportPin(); // exports GPIO
    int unexportPin(); // unexport GPIO
    int setDirection(std::string dir); // Set GPIO Direction
//    int setValue(std::string value); // Set GPIO Value (output pins)
    int getValue(std::string& value); // Get GPIO Value (input/output pins)
    std::string getPin(); // return the GPIO number associated with the instance of an object
    
private:
    std::string pin; // GPIO number associated with the instance of an object
};

#endif /* GPIOCLASS_H */


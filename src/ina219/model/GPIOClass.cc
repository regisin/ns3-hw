/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GPIOClass.cpp
 * Author: pregis
 * 
 * Created on February 7, 2018, 3:25 PM
 */

#include "GPIOClass.h"


GPIOClass::GPIOClass()
{
    this->pin = "40"; //GPIO4 is default
}

GPIOClass::GPIOClass(std::string pin)
{
    this->pin = pin;  //Instatiate GPIOClass object for GPIO pin number "gnum"
}

int GPIOClass::exportPin()
{
    std::string export_str = "/sys/class/gpio/export";
    std::ofstream exportgpio(export_str.c_str()); // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
    if (exportgpio < 0){
        std::cout << " OPERATION FAILED: Unable to export GPIO" << this->pin << "." << std::endl;
        return -1;
    }
    exportgpio << this->pin ; //write GPIO number to export
    exportgpio.close(); //close export file
    return 0;
}

int GPIOClass::unexportPin()
{
    std::string unexport_str = "/sys/class/gpio/unexport";
    std::ofstream unexportgpio(unexport_str.c_str()); //Open unexport file
    if (unexportgpio < 0){
        std::cout << " OPERATION FAILED: Unable to unexport GPIO" << this->pin << "." << std::endl;
        return -1;
    }
    unexportgpio << this->pin ; //write GPIO number to unexport
    unexportgpio.close(); //close unexport file
    return 0;
}

int GPIOClass::setDirection(std::string direction)
{

    std::string setdir_str ="/sys/class/gpio/gpio" + this->pin + "/direction";
    std::ofstream setdirgpio(setdir_str.c_str()); // open direction file for gpio
    if (setdirgpio < 0){
        std::cout << " OPERATION FAILED: Unable to set direction of GPIO" << this->pin << "." << std::endl;
        return -1;
    }
    
    if (direction.compare("in") || direction.compare("out")) {
        std::cout << " OPERATION FAILED: Invalid direction of GPIO" << this->pin << "." << std::endl;
        return -1;
    }

    setdirgpio << direction; //write direction to direction file
    setdirgpio.close(); // close direction file
    return 0;
}


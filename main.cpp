/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *  File:       main.cpp
 *  Author:     Motaz Khalifa
 *  Purpose:    A system that notifies the user if a certain audio loudness threshold have been
broken by sending vibrations and lighting up an LED and LCD display. System can be muted by pressing the push button.
 *  Extra modules/functions in file: mbed.h, 1802.h
 *  Assignment:  Project 3
 *  Inputs:      Audio Sensor, Push Button
 *  Outputs:     Vibration Motor, LED, LCD Display
 *  Constraints: Adjusting the audio sensor correctly
 *  References:  Lecture Videos
 **/


// Libraries
#include "mbed.h"
#include "1802.h"   //LCD

// Definitions
CSE321_LCD Screen( 16, 2, LCD_5x8DOTS, PF_0, PF_1);     //Setting up LCD Screen, 16x2, PF0 = SDA, PF1 =SCL


// Mutex Synchronization Lock Setup
Mutex lock;                                             //Create a lock, name: lock

// Watchdog Setup
Watchdog &watchMe = Watchdog::get_instance();           //Setup a WatchDog of 30 seconds
#define wdTimeout 30000

// Push Button Setup
InterruptIn pressed(BUTTON1);                           //Setup interrupt for push button on board
EventQueue buttonq(32 * EVENTS_EVENT_SIZE);
Thread buttonthread;

// Functions
void loud();    
void mute();

//Vibration Motor Setup
AnalogOut aout(PA_4);                                   //Initialize as an analog output at pin PA_4



int main()
{

//Start watchDog
    watchMe.start(wdTimeout);
    printf("-----------START----------\n");


//Initializing GPIO
    RCC->AHB2ENR|=0xA;              //RCC enabling ports D for LED, B for sensor 
//MODER /--------LED------/
    GPIOD->MODER &= ~(0x2);         //Set GPIO-D0 as an output
    GPIOD->MODER |= 0x1;
//MODER /-------SENSOR---/
    GPIOB->MODER &= ~(0x3);         //Set GPIO-B0 as an input for Audio sensor


//Start Screen
    Screen.begin();
    Screen.print("Quiet..");     


//Initializing Button
    buttonthread.start(callback(&buttonq, &EventQueue::dispatch_forever));
    pressed.rise(buttonq.event(&mute));


//Starts polling
    while (true)
    {  
    //If sound is detected in input
        if (GPIOB->IDR == 0x10){
            loud();                 //Call loud()
        }
    }


}




//************************
//Turns on alert that sound threshold have been broken
//Turns on an LED using bitwise driver configuration
//Prints new status to board
//Starts the vibration motor
//************************
void loud(void) {
//Use the Mutex Lock
    lock.lock();
//Start alert
     GPIOD->ODR |= 0x1;             //Turn on LED D0
//Print to board
    Screen.setCursor(0, 0);         //Clearing board
    Screen.clear();
    Screen.print("LOUD..");         //Print "LOUD.."
//Start the vibration motor
    aout.write(0.5);                //Supply power of 0.5 
//Unlock the mutex
    lock.unlock();
}




//************************
//Turns off alert (Activated by push button press)
//Turns off LED using bitwise driver configuration
//Prints new status to board
//Stops the vibration motor
//************************
void mute(void) {
//Stop alert        
    GPIOD->ODR &= ~(0x1);           //Turn off LED D0
//Print to board
    Screen.setCursor(0, 0);         //Clearing board
    Screen.clear();
    Screen.print("Quiet...");       //Print "Quiet..."
//Shut down vibration motor
    aout.write(0);                  //Supply power of 0
//Reset WatchDog Timer
    watchMe.kick();
}


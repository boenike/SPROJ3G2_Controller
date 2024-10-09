/**
 * Title: Semester Project 3 Group 2 - Car code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Car.c
 * Author: Bence Toth
 * Date: 08/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * RF library: https://github.com/andyrids/pico-nrf24
 * Servo library: https://www.codeproject.com/Articles/5360397/Raspberry-Pi-Pico-library-for-working-with-servos
 */

// Include necessary libraries
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "nrf24_driver.h"
#include "functions.h"
#include "servo.h"
//#include "hardware/i2c.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

uint8_t rx_pipe = 0 ;
payload_t Payload = { 1 , INIT_ANGLE } ;
nrf_client_t RF24 ;

// GPIO pin numbers for nRF24L01
pin_manager_t RF_Pins = { 
    .sck = 2,   // Serial Clock
    .copi = 3,  // Master Out - Slave In
    .cipo = 4,  // Master In - Slave Out
    .csn = 5,   // Chip Select Not
    .ce = 6     // Chip Enable
} ;

nrf_manager_t RF_Config = {
    // RF Channel 
    .channel = RF_CHANNEL ,

    // AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
    .address_width = AW_5_BYTES ,

    // dynamic payloads: DYNPD_ENABLE, DYNPD_DISABLE
    .dyn_payloads = DYNPD_DISABLE ,

    // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
    .data_rate = RF_DR_2MBPS ,

    // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
    .power = RF_PWR_0DBM ,

    // retransmission count: ARC_NONE...ARC_15RT
    .retr_count = ARC_NONE ,

    // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
    .retr_delay = ARD_250US 
} ;

int main ( void ) {

    stdio_init_all ( ) ;    // Initialize all present standard stdio types

    gpio_init ( PICO_DEFAULT_LED_PIN ) ;
    gpio_set_dir ( PICO_DEFAULT_LED_PIN , GPIO_OUT ) ;

    rc_servo Servo = rc_servo_init ( SERVO_PIN ) ;
    rc_servo_start ( &Servo , INIT_ANGLE ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , sizeof ( payload_t ) , RF24_RX , DYNPD_DISABLE , RF_ADDRESS , DATA_PIPE_0 ) ;

    while ( 1 ) {
        if ( RF24.is_packet ( &rx_pipe ) ) {
            RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;

            gpio_put ( PICO_DEFAULT_LED_PIN , Payload.direction ) ;
            rc_servo_set_angle ( &Servo , ( uint32_t ) Payload.servo_angle ) ;
        }
    }
    return 0 ;
}
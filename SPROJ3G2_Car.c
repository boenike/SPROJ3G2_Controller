/**
 * Title:           Semester Project 3 Group 2 - Car PRX source code
 * Institution:     University of Southern Denmark (SDU)
 * Campus:          Sønderborg
 * File:            SPROJ3G2_Car.c
 * Authors:         Bence Tóth
 * Date:            16/12/2024
 * Course:          BEng in Electronics
 * Semester:        3rd
 * Platform:        Raspberry Pi RP2040 C SDK
 * RF module:       nRF24L01 PA/LNA
 * RF library:      https://github.com/andyrids/pico-nrf24
 */

// Include necessary libraries
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "nrf24_driver.h"
#include "servo_control.h"
#include "functions.h"

// Initial values - middlepoint for servo - forward mode and halt for the motor
payload_t Payload = { .servo_angle = INIT_ANGLE , .speed_and_direction = 0 } ;

uint8_t car_stopped = 0 ;
uint32_t currentTime , prevTime = 0 ;

nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

int main ( void ) {

    hard_assert ( stdio_init_all ( ) ) ;    // Initialize all present standard stdio types

    hard_assert ( UART_Setup ( UART_ID , UART_BAUDRATE , UART_RX_PIN , UART_TX_PIN ) ) ;

    servo_init ( SERVO_PIN ) ;

    uint16_t rf_setup = nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , ACK_OFF ,
        PRX , sizeof ( payload_t ) , ( data_pipe_t ) payload_pipe , PAYLOAD_ADDRESS ) ;
        
    hard_assert ( rf_setup == RF_SETUP_OK ) ;

    while ( 1 ) {

        currentTime = to_ms_since_boot ( get_absolute_time ( ) ) ;

        // If the time delay between two consecutive data packets is
        // greater than the predetermined value, set the car into initial conditions

        if ( ( currentTime - prevTime ) > INTERVAL_LIMIT && !car_stopped ) {
            car_stopped = 1 ;
            set_servo_angle ( INIT_ANGLE , SERVO_PIN ) ;
            uart_putc_raw ( UART_ID , HALT ) ;
        }

        if ( RF24.is_packet ( &payload_pipe ) ) {
            car_stopped = 0 ;
            success = RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;
            prevTime = to_ms_since_boot ( get_absolute_time ( ) ) ;
            set_servo_angle ( Payload.servo_angle , SERVO_PIN ) ;
            uart_putc_raw ( UART_ID , Payload.speed_and_direction ) ;
        }
    }
    return 0 ;
}
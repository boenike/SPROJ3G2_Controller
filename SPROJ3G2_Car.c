/**
 * Title: Semester Project 3 Group 2 - Car code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Car.c
 * Authors: Bence Toth and Iliya Iliev
 * Date: 29/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * OLED module: SSD1306 128x32 I2C
 * RF library:   https://github.com/andyrids/pico-nrf24
 * OLED library: https://github.com/daschr/pico-ssd1306
 */

// Include necessary libraries
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "nrf24_driver.h"
#include "functions.h"
#include "servo_control.h"
#include "hardware/i2c.h"
//#include "ssd1306.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

payload_t Payload = { .direction = 1 , .servo_angle = INIT_ANGLE } ;
//echo_t Echo = { .echo = 0 } ;

//uint16_t rec_packets = 0 ;
uint8_t car_stopped = 0 ;
uint32_t currentTime , prevTime = 0 ;

nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

int main ( void ) {

    stdio_init_all ( ) ;    // Initialize all present standard stdio types

    gpio_init ( PICO_DEFAULT_LED_PIN ) ;
    gpio_set_dir ( PICO_DEFAULT_LED_PIN , GPIO_OUT ) ;
    gpio_put ( PICO_DEFAULT_LED_PIN , 0 ) ;

    servo_init ( SERVO_PIN ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , DYNPD_DISABLE ) ;
    nRF24_Comm_Dir_Setup ( &RF24 , RF24_RX , sizeof ( payload_t ) , sizeof ( echo_t ) , ( data_pipe_t ) payload_pipe , ( data_pipe_t ) echo_pipe , PAYLOAD_ADDRESS , ECHO_ADDRESS ) ;

    while ( 1 ) {
        
        currentTime = to_ms_since_boot ( get_absolute_time ( ) ) ;

        // If the time delay between two consecutive data packets is greater than the predetermined value, set the car into initial conditions
        if ( ( currentTime - prevTime ) > INTERVAL_LIMIT && !car_stopped ) {
            set_servo_angle(INIT_ANGLE , SERVO_PIN ) ;
            gpio_put ( PICO_DEFAULT_LED_PIN , 1 ) ;
            car_stopped = 1 ;
        }

        if ( RF24.is_packet ( &payload_pipe ) ) {
            RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;
            prevTime = to_ms_since_boot ( get_absolute_time ( ) ) ;
            car_stopped = 0 ;
            gpio_put ( PICO_DEFAULT_LED_PIN , Payload.direction ) ;
            set_servo_angle ( Payload.servo_angle , SERVO_PIN ) ;
            //printf("recieved a package\n");

            /*
            rec_packets++ ;
            if ( rec_packets == RET_LIMIT ) {
                rec_packets = 0 ;
                Echo.echo = ( Echo.echo ) ? 0 : 1 ;     // toggle LED

                // send packet to receiver's echo address
                //RF24.tx_destination ( ECHO_ADDRESS ) ;
                //RF24.dyn_payloads_disable ( ) ;
                success = RF24.standby_mode ( ) ;
                if ( success == NRF_MNGR_OK ) printf ( "Switched to TX mode\n" ) ;
                //printf("Switched to TX\n");        // Change to transmit mode for echo transmission

                success = RF24.send_packet ( &Echo , sizeof ( echo_t ) ) ;
                if ( success == NRF_MNGR_OK ) printf ( "Echo sent - %d\n" , Echo.echo ) ;

                success = RF24.receiver_mode ( ) ;        // Change back to receiver mode for normal operation 
                if ( success == NRF_MNGR_OK ) printf("Switched back to RX\n");
                printf ( "End of echo.\n\n" ) ;
            }*/
        }
    }
    return 0 ;
}
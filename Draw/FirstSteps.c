// Include necessary libraries
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "hardware/sync.h"

void LEDConfig();
void spiConfig();

// SPI data
uint16_t DAC_data_0 ; // output value

// DAC parameters (see the DAC datasheet)
// B-channel, 1x, active
#define DAC_config_chan_A 0b0011000000000000
#define DAC_config_chan_B 0b1011000000000000

//SPI configurations (note these represent GPIO number, NOT pin number)
#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  6
#define PIN_MOSI 7 //SDI
#define LDAC     8
#define LED      25
#define SPI_PORT spi0

// Core 0 entry point
int main() {
    // Initialize stdio/uart (printf won't work unless you do this!)
    stdio_init_all();

    spiConfig();

    // // Map LDAC pin to GPIO port, hold it low (could alternatively tie to GND)
    gpio_init(LDAC) ;
    gpio_set_dir(LDAC, GPIO_OUT) ;
    gpio_put(LDAC, 0) ;

    LEDConfig();

    #define MAXVoltage 2.5
    #define STEPS 1<<8
    uint16_t xPos; 
    uint i;
    bool direction = 0;

    while (1) {
        //xPos = 0;
        for (i = 0; i < STEPS-1; ++i) {
            if (direction == 0) {
                ++xPos;
            } else {
                --xPos;
            }
            //++xPos; //Line starting a 0, incrementing by 5/1000, to 5
            // Mask with DAC control bits
            DAC_data_0 = (DAC_config_chan_A | (xPos << 4 & 0x0fff))  ;
            // SPI write (no spinlock b/c of SPI buffer)
            spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;

            DAC_data_0 = (DAC_config_chan_B | (xPos << 4 & 0x0fff))  ;
            // SPI write (no spinlock b/c of SPI buffer)
            spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;
        }
        //sleep_ms(100);
        direction = !direction;
    }
}


void LEDConfig() {
    // Map LED to GPIO port, make it low
    gpio_init(LED) ;
    gpio_set_dir(LED, GPIO_OUT) ;
    gpio_put(LED, 0) ;
    
}

void spiConfig() {
    // Initialize SPI channel (channel, baud rate set to 20MHz)
    spi_init(SPI_PORT, 20000000) ;
    // Format (channel, data bits per transfer, polarity, phase, order)
    spi_set_format(SPI_PORT, 16, 0, 0, 0);

    // Map SPI signals to GPIO ports
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SPI) ;
}
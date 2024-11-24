// Include necessary libraries
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

void spiConfig();
void drawLine(int16_t x1, int16_t x2, int16_t y1, int16_t y2);

#define abs(a) ((a>0) ? a:-a) //if a greater than 0, return a else return negative a

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
#define SPI_PORT spi0

// Core 0 entry point
int main() {
    spiConfig();

    // // Map LDAC pin to GPIO port, hold it low (could alternatively tie to GND)
    gpio_init(LDAC) ;
    gpio_set_dir(LDAC, GPIO_OUT) ;
    gpio_put(LDAC, 0) ;

    while (1) {
        drawLine(0,255,255,0);
        drawLine(255,0,255,0);

        drawLine(0,255/4,255 - (255/4), 255);
        drawLine(255 - (255/4),255,  255, 255 - (255/4));

        drawLine(0,255/4, 255/4,0);
        drawLine(255-(255/4),255,0,255/4);

        //drawLine(0,255,0,64); //just to show fractional slope works
    }
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


void drawLine(int16_t x1, int16_t x2, int16_t y1, int16_t y2) {
    int16_t xCount = (x2 - x1); //Number of times we can increase the voltage to get from one point to next
    float m = (float)(y2 - y1)/ (float)xCount; //slope
    int16_t x = x1;
    int16_t i;
    float y;

    for(i = 0; i <= abs(xCount); ++i){
        x = (x2 < x1) ? x-1 : x+1;
        DAC_data_0 = (DAC_config_chan_A | ((uint16_t)x << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;
        
        y = m*((float)x - (float)x1) + (float)y1;
        DAC_data_0 = (DAC_config_chan_B | (( (uint16_t)y << 4) & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1);
    }
}
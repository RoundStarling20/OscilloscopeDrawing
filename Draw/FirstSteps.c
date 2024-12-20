// Include necessary libraries
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "math.h"

void spiConfig();
void drawSlopeLine(int16_t x1, int16_t x2, int16_t y1, int16_t y2);
void drawCircle(uint64_t x0, uint64_t y0, uint64_t r);
void drawHLine(uint16_t x1, uint16_t x2, uint16_t y);
void drawVLine(uint16_t y1, uint16_t y2, uint16_t x);
void drawRectangle(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);


#define abs(a) ((a>0) ? a:-a) //if a greater than 0, return a else return negative a

// SPI data
uint16_t DAC_data_0 ; // output value

// DAC parameters (see the DAC datasheet)
// B-channel, 1x, active
#define DAC_config_X_OUTPUT 0b0011000000000000
#define DAC_config_Y_OUTPUT 0b1011000000000000

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
        drawSlopeLine(0,255,255,0);
        drawSlopeLine(255,0,255,0);

        drawSlopeLine(0,255/4,255 - (255/4), 255);
        drawSlopeLine(255 - (255/4),255,  255, 255 - (255/4));

        drawSlopeLine(0,255/4, 255/4,0);
        drawSlopeLine(255-(255/4),255,0,255/4);

        //drawLine(0,255,0,64); //just to show fractional slope works

        drawCircle(255/2,255/2, 50);

        drawHLine(0, 255, 255/2);
        drawVLine(0,255,255/2);

        drawRectangle(0,255,0,255);
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


void drawSlopeLine(int16_t x1, int16_t x2, int16_t y1, int16_t y2) {
    int16_t xCount = (x2 - x1); //Number of times we can increase the voltage to get from one point to next
    float m = (float)(y2 - y1)/ (float)xCount; //slope
    int16_t x = x1;
    int16_t i;
    float y;

    for(i = 0; i <= abs(xCount); ++i){
        x = (x2 < x1) ? x-1 : x+1;
        DAC_data_0 = (DAC_config_X_OUTPUT | (x << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;
        
        y = m*((float)x - (float)x1) + (float)y1;
        DAC_data_0 = (DAC_config_Y_OUTPUT | (( (uint16_t)y << 4) & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1);
    }
}

void drawCircle(uint64_t x0, uint64_t y0, uint64_t r) {
    uint16_t x;
    float y;

    for(x = x0-r; x < x0 + r; ++x) {
        DAC_data_0 = (DAC_config_X_OUTPUT | (x << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;
        y = sqrt( (double)( r*r - (x - x0)*(x-x0))); //leave out y0 to avoid recomputing this
        DAC_data_0 = (DAC_config_Y_OUTPUT | (( (uint16_t)(y + y0) << 4) & 0x0fff));
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1);

        DAC_data_0 = (DAC_config_Y_OUTPUT | (( (uint16_t)(-y + y0) << 4) & 0x0fff));
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1);
    }
}

void drawHLine(uint16_t x1, uint16_t x2, uint16_t y) {
    int16_t distance = x1 - x2;
    uint16_t i;
    uint16_t x;
    for(i = 0; i < abs(distance); ++i) {
        x = (x2 < x1) ? x-1 : x+1;
        DAC_data_0 = (DAC_config_X_OUTPUT | (x << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;

        DAC_data_0 = (DAC_config_Y_OUTPUT | (y << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1);
    }

}

void drawVLine(uint16_t y1, uint16_t y2, uint16_t x) {
    int16_t distance = y1 - y2;
    uint16_t i;
    uint16_t y;
    for(i = 0; i < abs(distance); ++i) {
        DAC_data_0 = (DAC_config_X_OUTPUT | (x << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1) ;
        
        y = (y2 < y1) ? y-1 : y+1;
        DAC_data_0 = (DAC_config_Y_OUTPUT | (y << 4 & 0x0fff))  ;
        // SPI write (no spinlock b/c of SPI buffer)
        spi_write16_blocking(SPI_PORT, &DAC_data_0, 1);
    }

}

void drawRectangle(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) {
    drawVLine(y1, y2, x1);
    drawVLine(y1, y2, x2);
    drawHLine(x1,x2,y1);
    drawHLine(x1,x2,y2);
}
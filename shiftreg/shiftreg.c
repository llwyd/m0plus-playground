#include "gpio.h"
#include "util.h"
#include "systick.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>

#define SPI_PIN_SCK ( 5U )
#define SPI_PIN_MISO ( 6U )
#define SPI_PIN_MOSI ( 7U )

#define SPI_ALT_FUNC (5U)

static void Loop( void )
{
    static uint32_t lastBlink = 0U;
    static uint8_t counter = 0U;
    while( 1 )
    {
        while((SysTick_GetMS() - lastBlink) < 500U);
        //GPIO_Toggle(LED_PIN);
        counter++;
        SPI_WriteByte(counter);
        lastBlink = SysTick_GetMS();
    }
}

static void Init( void )
{
    GPIO_Init();
    GPIO_SetAlt(SPI_PIN_SCK, SPI_ALT_FUNC);
    GPIO_SetAlt(SPI_PIN_MISO, SPI_ALT_FUNC);
    GPIO_SetAlt(SPI_PIN_MOSI, SPI_ALT_FUNC);
    SPI_Init(); 
    /* SysTick Calibration value for 1ms tick as per ARM datasheet
     * 16MHz Clock / 1000Hz tick = 0x3e80
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x3e7f
     */
    SysTick_Init(0x3e7f);
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}


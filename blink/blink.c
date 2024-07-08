#include "gpio.h"
#include "util.h"
#include "systick.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 5U )

static void Loop( void )
{
    static uint32_t lastBlink = 0U;
    while( 1 )
    {
        while((SysTick_GetMS() - lastBlink) < 500U);
        GPIO_Toggle(LED_PIN);
        lastBlink = SysTick_GetMS();
    }
}

static void Init( void )
{
    GPIO_Init();
    GPIO_ConfigureOutput(LED_PIN);

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


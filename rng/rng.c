#include "gpio.h"
#include "util.h"
#include "systick.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 5U )

static volatile uint32_t no_init_data __attribute__((section(".no_init")));

static void Loop( void )
{
    static uint32_t lastBlink = 0U;
    while( 1 )
    {
        while((SysTick_GetMS() - lastBlink) < 100U);
//        no_init_data++;
        GPIO_Toggle(LED_PIN);
        lastBlink = SysTick_GetMS();
    }
}

static void Init( void )
{
    /* SysTick Calibration value for 1ms tick as per ARM datasheet
     * 16MHz Clock / 1000Hz tick = 0x3e80
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x3e7f
     */
    SysTick_Init(0x3e7f);
    
    GPIO_Init();
    GPIO_SetOutput(LED_PIN);

    no_init_data = 0x12345678;
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}


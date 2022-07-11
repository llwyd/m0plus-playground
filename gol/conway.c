/* A very minimalist blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "../common/util.h"
#include "../common/buffer.h"
#include "../common/clock.h"
#include "../common/i2c.h"
#include "../common/gpio.h"
#include "../common/fsm.h"
#include "../../../conway/life/life.h"
#include <stdarg.h>

/* Registers for GPIO Config */
#define PORT        ( *( ( volatile unsigned int *)0x41004400 ) )
#define PIN         ( *( ( volatile unsigned int *)0x41004410 ) )

#define LED_PIN ( 10U )

static volatile gpio_t * GPIO = ( gpio_t *) GPIO_BASE;
static volatile systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;

/* SysTick ISR */
void _sysTick( void )
{
    static uint16_t counter;
    counter++;
    if ( counter == 25 )
    {
    /* XOR Toggle of On-board LED */
        PIN ^= ( 1 << LED_PIN );
        counter = 0U;
    }
    Task_Add( &Life_Tick );
}

void SetupDisplay( int num, ... )
{
    uint8_t commands[4];
    va_list args;

    va_start( args, num );

    for( uint8_t idx = 0U; idx < num; idx++ )
    {
        commands[idx] = (uint8_t)va_arg( args, int );
    }

    va_end( args );

    while( !I2C_Write( 0x3C, commands, num ) );
}

void UpdateDisplay( void )
{
    uint8_t (*buffer)[LCD_COLUMNS] = Life_GetBuffer();
    uint8_t data[2] = { 0x40, 0x00};

    for( int i = 0; i < LCD_PAGES; i++ )
    {
        for( int j = 0; j < LCD_COLUMNS; j++ )
        {
            data[1] = buffer[i][j];
            I2C_Write( 0x3C, data, 2U );
        }
    }
}

void DisplayInit( void )
{
    SetupDisplay( 2U, 0x00, 0xAE );
    SetupDisplay( 3U, 0x00, 0x81, 0x7F );
    SetupDisplay( 3U, 0x00, 0xA8, 63 );
    SetupDisplay( 3U, 0x00, 0xD3, 0x00 );
    SetupDisplay( 2U, 0x00, 0x40 );
    SetupDisplay( 2U, 0x00, 0xa0 | 0x1 );
    SetupDisplay( 3U , 0x00, 0xda, 0x02 | ( 1 << 4 ) | ( 0 << 5 ) );
    SetupDisplay( 2U, 0x00, 0xc0 | ( 1 << 3 ) );
    SetupDisplay( 2U, 0x00, 0xA6 );
    SetupDisplay( 3U, 0x00, 0xD5, ( 15 << 4 | 0x00 ) );
    SetupDisplay( 3U, 0x00, 0x8D, 0x14 );
    SetupDisplay( 2U, 0x00, 0xA4 );
    SetupDisplay( 2U, 0x00, 0xAF );
    SetupDisplay( 3U, 0x00, 0x20, 0 );
    SetupDisplay( 4U, 0x00, 0x21, 0, 127);
    SetupDisplay( 4U, 0x00, 0x22, 0, 7);
}


static void Init ( void )
{
    Clock_Set48MHz();

    PORT |= ( 1 << LED_PIN );
    PIN |= ( 1 << LED_PIN );

    I2C_Init();
    DisplayInit();
    Life_Init( &UpdateDisplay );

    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 48MHz Clock / 1000Hz tick = 0xBB80
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x7527F
     */
    SYSTICK->CALIB = ( 0xBB7F );
    
    /* 1000 / 24fps = 42ish */
    SYSTICK->LOAD   = 0xBB7f * 42;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

}

int main ( void )
{
    Init();
    /* Endless Loop */
    while(1)
    {
        Task_Get();
    }

    return 0;
}


/* Conway's game of life for the Trinket m0+ and a small lcd display
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

#define LED_PIN ( 10U )

enum Signals
{
    signal_SysTick = signal_Count,
};

static volatile gpio_t * GPIO = ( gpio_t *) GPIO_BASE;
static volatile systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;
static fsm_events_t event;

/* SysTick ISR */
void _sysTick( void )
{
    FSM_AddEvent( &event, signal_SysTick );
}

static void SetupDisplay( int num, ... )
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

static void UpdateDisplay( void )
{
    uint8_t (*buffer)[LCD_COLUMNS] = Life_GetBuffer();
    uint8_t data[2] = { 0x40, 0x00};

    for( uint8_t i = 0; i < LCD_PAGES; i++ )
    {
        for( uint8_t j = 0; j < LCD_COLUMNS; j++ )
        {
            data[1] = buffer[i][j];
            I2C_Write( 0x3C, data, 2U );
        }
    }
}

static void DisplayInit( void )
{
    SetupDisplay( 2U, 0x00, 0xAE );
    SetupDisplay( 3U, 0x00, 0x81, 0x7F );
    SetupDisplay( 3U, 0x00, 0xA8, 63 );
    SetupDisplay( 3U, 0x00, 0xD3, 0x00 );
    SetupDisplay( 2U, 0x00, 0x40 );
    SetupDisplay( 2U, 0x00, 0xa0 | 0x1 );
    SetupDisplay( 3U, 0x00, 0xda, 0x02 | ( 1 << 4 ) | ( 0 << 5 ) );
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

    GPIO->DIRR |= ( 1 << LED_PIN );

    I2C_Init();
    DisplayInit();
    Life_Init( &UpdateDisplay );

    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 48MHz Clock / 1000Hz tick = 0xBB80
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0xBB7F
     */
    SYSTICK->CALIB = ( 0xBB7F );
    
    /* 1000 / 24fps = 4.2ish */
    SYSTICK->LOAD   = 0xBB7F * 4;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

}

/* Only state of the program */
static fsm_status_t Life( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;

    switch( s )
    {
        case signal_SysTick:
        {
            Life_Tick();
            ret = fsm_Handled;
        }
            break;
        case signal_Enter:
        {
            GPIO->OUT &= ~( 1 << LED_PIN );
        }
            break;
        case signal_Exit:
        case signal_None:
        default:
        {
            GPIO->OUT |= ( 1 << LED_PIN );
            ret = fsm_Ignored;
        }
            break;
    }

    return ret;
}

/* Main Event Loop */
static void Loop( void )
{
    fsm_t life;
    life.state = &Life;
    signal sig = signal_None;

    FSM_Init( &life, &event );

    while( 1 )
    {
        while( !FSM_EventsAvailable( &event ) );
        sig = FSM_GetLatestEvent( &event );
        FSM_Dispatch( &life, sig );
    }
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}


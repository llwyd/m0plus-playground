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
#include "../common/display.h"

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


static void UpdateLCD( void )
{
    const uint8_t (*buffer)[LCD_COLUMNS] = Life_GetBuffer();
    Display_Update( buffer );
}

static void Init ( void )
{
    Clock_Set48MHz();

    GPIO->DIRR |= ( 1 << LED_PIN );
    GPIO->OUT |= ( 1 << LED_PIN );

    I2C_Init();
    Display_Init();
    Life_Init( &UpdateLCD );

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


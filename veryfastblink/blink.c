/* A template blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */
#include "../common/clock.h"
#include "../common/gpio.h"
#include "../stateengine/src/fsm.h"
#include "../common/util.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 8U )

enum Signals
{
    signal_SysTick = signal_Count,
};

static systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;
static gpio_t * GPIO = ( gpio_t * ) GPIOA_BASE;
static fsm_events_t * event_ptr;

/* SysTick ISR */
void _sysTick( void )
{
    FSM_AddEvent( event_ptr, signal_SysTick );
}


static fsm_status_t Idle( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;

    switch( s )
    {
        case signal_SysTick:
        {
            GPIO->ODR ^= ( 1 << LED_PIN );
            ret = fsm_Handled;
        }
            break;
        case signal_Enter:
        case signal_Exit:
        case signal_None:
        default:
        {
            ret = fsm_Ignored;
        }
            break;
    }

    return ret;
}

static void Loop( void )
{
    fsm_t state;
    state.state = &Idle;
    signal sig = signal_None;
    fsm_events_t events;
    event_ptr = &events;

    FSM_Init( &state, &events );

    while( 1 )
    {
        while( !FSM_EventsAvailable( &events ) );
        sig = FSM_GetLatestEvent( &events );
        FSM_Dispatch( &state, sig );
    }
}

static void Init( void )
{
    Clock_Set64MHz();

    /* Lazy way of enabling gpio a */
    *((uint32_t *)0x40021034) |= ( 0x1 << 0 );

    GPIO->MODER &= ~( 1 << 17 );
    GPIO->MODER |= ( 1 << 16 );
 
    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    SYSTICK->CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    SYSTICK->LOAD   = 0x270F * 50 * 8;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

}

int main ( void )
{
    Init();
    Loop();

    return 0;
}


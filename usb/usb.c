/* A template blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */
#include "../common/gpio.h"
#include "../common/clock.h"
#include "../stateengine/src/fsm.h"
#include "../common/util.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 10U )
#define PM_APBB     ( *( ( volatile uint32_t *)0x4000041C ) )
#define USB_BASE ( 0x41005000 )

enum Signals
{
    signal_SysTick = signal_Count,
};

typedef struct
{
    uint8_t CTRLA:8;
    struct res8_t _RESERVED0;
    uint8_t SYNCBUSY:8;
    uint8_t QOSCTRL:8;
    struct res8_t _RESERVED1[9];
    uint8_t FSMSTATUS:8;
    struct res8_t _RESERVED2[22];
    uint32_t DESCADD:32;
    uint16_t PADCAL:16;
}
usb_device_t;

volatile static systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;
volatile static gpio_t * GPIO = ( gpio_t * ) GPIO_BASE;
volatile static usb_device_t * USB_DEVICE = ( usb_device_t * ) USB_BASE;

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
            GPIO->OUT ^= ( 1 << LED_PIN );
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
    /* set port 10 to output */
    GPIO->DIRR |= ( 1 << LED_PIN );
    GPIO->OUT |= ( 1 << LED_PIN );
 
    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    SYSTICK->CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    SYSTICK->LOAD   = 0x270F * 50;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

}


void Init_USB( void )
{
    Clock_Set48MHz();
    Clock_ConfigureGCLK( 0x7, 0x3, 0x6 );
    PM_APBB |= ( 0x1 << 5 );

    /* Configure PA24 and PA25 as USB D+ D- */
    /* Mux G */
    GPIO->PINCFG24 |= ( 0x1 << 0 );
    GPIO->PMUX12 |= ( 0x6 << 0 );
    
    GPIO->PINCFG25 |= ( 0x1 << 0 );
    GPIO->PMUX12 |= ( 0x6 << 4 );
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}


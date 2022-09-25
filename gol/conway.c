/* Conway's game of life for the Trinket m0+ and a small lcd display
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "../common/util.h"
#include "../common/buffer.h"
#include "../common/clock.h"
#include "../common/i2c.h"
#include "../common/gpio.h"
#include "../stateengine/src/fsm.h"
#include "../common/timer.h"
#include "../../../conway/life/life.h"
#include "../common/display.h"

#define LED_PIN ( 7U )

/* SysTick Calibration value for 10ms tick as per ARM datasheet
 * 48MHz Clock / 1000Hz tick = 0xBB80
 * Need to subtract 1 because count ends at 0 so
 * Calibration value is 0xBB7F
 */
#define SYSTICK_CALIB_VAL ( 0x9C3FFU );

_Static_assert( LCD_COLUMNS == DISPLAY_COLUMNS, "Mismatch of column size" );
_Static_assert( LCD_ROWS == DISPLAY_ROWS, "Mismatch of row size" );
_Static_assert( LCD_PAGES == DISPLAY_PAGES, "Mismatch of pages" );
_Static_assert( LCD_FULL_ROWS == DISPLAY_FULL_ROWS, "Mismatch of full row size" );
_Static_assert( sizeof( uint8_t ) == 1U, "uint8_t > 1 byte" );

enum Signals
{
    signal_Timer = signal_Count,
};

static volatile gpio_t * GPIO = ( gpio_t *) GPIOB_BASE;
static volatile systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;
static fsm_events_t event;

/* SysTick ISR */
void  __attribute__((interrupt("IRQ"))) _sysTick( void )
{
    //FSM_AddEvent( &event, signal_Timer );
}

void  __attribute__((interrupt("IRQ"))) _tim2( void )
{
    FSM_AddEvent( &event, signal_Timer );
    Timer_ClearInterrupt();
}

static void UpdateLCD( void )
{
    const uint8_t (*buffer)[LCD_COLUMNS] = Life_GetBuffer();
    Display_Update( buffer );
}

static void Init ( void )
{
    Clock_Set64MHz();

    /* Lazy way of enabling gpio b */
    *((uint32_t *)0x40021034) |= ( 0x1 << 1 );

    GPIO->MODER &= ~( 1 << 15 );
    GPIO->MODER |= ( 1 << 14 );
    GPIO->ODR |= ( 1 << LED_PIN );

    I2C_Init();
    Display_Init();
    Timer_Init(); 
    Life_Init( &UpdateLCD );

    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    SYSTICK->CALIB = SYSTICK_CALIB_VAL;
    
    /* 1000 / 17fps = 5.8ish */
    SYSTICK->LOAD = 0xFFFF;

    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF"); 
    Timer_Start();
}

/* Only state of the program */
static fsm_status_t Life( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;

    switch( s )
    {
        case signal_Timer:
        {
            Life_Tick();
            ret = fsm_Handled;
        }
        break;

        case signal_Enter:
        {
            GPIO->ODR &= ~( 1 << LED_PIN );
            ret = fsm_Handled;
        }
        break;

        case signal_Exit:
        case signal_None:
        default:
        {
            GPIO->ODR |= ( 1 << LED_PIN );
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
    life.state = Life;
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


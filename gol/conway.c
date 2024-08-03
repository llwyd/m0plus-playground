/* Conway's game of life for the Trinket m0+ and a small lcd display
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "../common/util.h"
#include "../common/buffer.h"
#include "../common/clock.h"
#include "../common/i2c.h"
#include "../common/gpio.h"
#include "state.h"
#include "events.h"
#include "../common/timer.h"
#include "../../../conway/life/life.h"
#include "../common/display.h"
#include "../common/systick.h"

#define LED_PIN ( 7U )

#define CORE_CLOCK ( 64000000U )
#define SYSTICK_1MS ( ( CORE_CLOCK / 1000U ) - 1U )

_Static_assert( LCD_COLUMNS == DISPLAY_COLUMNS, "Mismatch of column size" );
_Static_assert( LCD_ROWS == DISPLAY_ROWS, "Mismatch of row size" );
_Static_assert( LCD_PAGES == DISPLAY_PAGES, "Mismatch of pages" );
_Static_assert( LCD_FULL_ROWS == DISPLAY_FULL_ROWS, "Mismatch of full row size" );
_Static_assert( sizeof( uint8_t ) == 1U, "uint8_t > 1 byte" );

DEFINE_STATE(Life);

#define SIGNALS(SIG) \
    SIG(Timer) \

GENERATE_SIGNALS(SIGNALS);

static volatile gpio_t * GPIO = ( gpio_t *) GPIOB_BASE;
static volatile event_fifo_t events;

void  __attribute__((interrupt("IRQ"))) _tim2( void )
{
    FIFO_Enqueue( &events, EVENT(Timer));
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
    SysTick_Init( SYSTICK_1MS );
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF"); 

    /* microcontroller starts faster than the power on for LCD
     * so need brief delay on startup */
    const uint32_t delay = SysTick_GetMS();
    while( (SysTick_GetMS() - delay) < 60U );

    /* Lazy way of enabling gpio b */
    *((uint32_t *)0x40021034) |= ( 0x1 << 1 );

    GPIO->MODER &= ~( 1 << 15 );
    GPIO->MODER |= ( 1 << 14 );
    GPIO->ODR |= ( 1 << LED_PIN );

    I2C_Init();
    Display_Init();
    Timer_Init(); 
    Life_Init( &UpdateLCD );
}

/* Only state of the program */
static state_ret_t State_Life( state_t * this, event_t s )
{
    state_ret_t ret;

    switch( s )
    {
        case EVENT(Timer):
        {
            Life_Tick();
            ret = HANDLED();
            break;
        }
        case EVENT(Enter):
        {
            GPIO->ODR &= ~( 1 << LED_PIN );
            Timer_Start();
            ret = HANDLED();
            break;
        }
        case EVENT(Exit):
        default:
        {
            GPIO->ODR |= ( 1 << LED_PIN );
            ret = HANDLED();
        }
        break;
    }

    return ret;
}

/* Main Event Loop */
static void Loop( void )
{
    state_t life;
    life.state = STATE(Life);
    event_t sig = EVENT(None);

    FIFO_Enqueue(&events, EVENT(Enter));
    
    while( 1 )
    {
        while( FIFO_IsEmpty( (fifo_base_t*)&events ) );
        sig = FIFO_Dequeue( &events );
        STATEMACHINE_FlatDispatch( &life, sig );
    }
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}


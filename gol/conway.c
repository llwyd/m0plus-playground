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
#include "../common/adc.h"
#include "../common/timer.h"
#include "../../../conway/life/life.h"
#include "../common/display.h"

#define LED_PIN ( 10U )

/* NVIC */
#define NVIC_ISER0      ( *((volatile unsigned int *) 0xE000E100 ) )
#define NVIC_ICPR0      ( *((volatile unsigned int *) 0xE000E280 ) )

_Static_assert( LCD_COLUMNS == DISPLAY_COLUMNS, "Mismatch of column size" );
_Static_assert( LCD_ROWS == DISPLAY_ROWS, "Mismatch of row size" );
_Static_assert( LCD_PAGES == DISPLAY_PAGES, "Mismatch of pages" );
_Static_assert( LCD_FULL_ROWS == DISPLAY_FULL_ROWS, "Mismatch of full row size" );
_Static_assert( sizeof( uint8_t ) == 1U, "uint8_t > 1 byte" );

#define MAX_FRAMERATE  ( 16U )
#define ADC_WINDOW_INC ( 16U )

#define CALC_FRAMERATE( X, Y ) ( (uint8_t)( ( ( (uint16_t)(X) * (uint16_t)(Y) ) >> 8U ) + 1U ) )

enum Signals
{
    signal_SysTick = signal_Count,
    signal_ADCWindow,
};
    
/* SysTick Calibration value for 10ms tick as per ARM datasheet
 * 48MHz Clock / 1000Hz tick = 0xBB80
 * Need to subtract 1 because count ends at 0 so
 * Calibration value is 0xBB7F
 */
static const uint32_t calib_val = 0xBB7F;

static volatile gpio_t * GPIO = ( gpio_t *) GPIO_BASE;
static volatile systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;
static fsm_events_t event;

/* SysTick ISR */
void _sysTick( void )
{
    FSM_AddEvent( &event, signal_SysTick );
}

void _adc( void )
{
    FSM_AddEvent( &event, signal_ADCWindow );
    NVIC_ICPR0 |= ( 0x1 << 23U );
    ADC_ClearInterrupt();
}

void _tcc0( void )
{
    GPIO->OUT ^= ( 0x1 << LED_PIN );
    NVIC_ICPR0 |= ( 0x1 << 15U );
    Timer_ClearInterrupt();
}

static void UpdateLCD( void )
{
    const uint8_t (*buffer)[LCD_COLUMNS] = Life_GetBuffer();
    Display_Update( buffer );
}

/* M0+ doesnt have a div operator, only need a rough division */
uint32_t UnsignedDIV( uint32_t num, uint32_t denom )
{
    uint32_t result = 0;
    if( num && denom )
    {
        while( num >= denom )
        {
            num -= denom;
            result++;
        }
    }
    else
    {
        result = 1;
    }

    return result;
}

static void UpdateFramerate( void )
{
    uint8_t raw_adc = ADC_Read();
    uint8_t new_framerate = CALC_FRAMERATE( MAX_FRAMERATE, raw_adc );

    uint32_t upper_lim = new_framerate * 16U;
    uint32_t lower_lim = upper_lim - 16U;
    upper_lim--;

    uint32_t new_systick = ( 0xFFFFFF >> new_framerate );

    SYSTICK->CTRL &= ~0x7;
    SYSTICK->LOAD = new_systick;
    SYSTICK->VAL = 0x0;
    SYSTICK->CTRL |= 0x7;    

    ADC_UpdateWindow( (uint8_t)upper_lim, (uint8_t)lower_lim );
}

static void InitialiseADCWindow( void )
{
    ADC_Start();
    UpdateFramerate();
    NVIC_ISER0 |= ( 1 << 23U ); 
}

static void Init ( void )
{
    Clock_Set48MHz();

    GPIO->DIRR |= ( 1 << LED_PIN );
    GPIO->OUT |= ( 1 << LED_PIN );

    I2C_Init();
    Display_Init();
    Timer_Init();
    NVIC_ISER0 |= ( 1 << 15U );

    ADC_Init();
    Life_Init( &UpdateLCD );

    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    SYSTICK->CALIB = calib_val;
    
    /* 1000 / 17fps = 5.8ish */
    SYSTICK->LOAD = ( 0xFFFFFF );

    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF"); 
    InitialiseADCWindow();
    Timer_Start();
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
        
        case signal_ADCWindow:
        {
            UpdateFramerate();
            ret = fsm_Handled;
        }
        break;

        case signal_Enter:
        {
            GPIO->OUT &= ~( 1 << LED_PIN );
            ret = fsm_Handled;
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


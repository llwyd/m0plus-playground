/* A very minimalist blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "../common/util.h"
#include "timer.h"

/* Registers for GPIO Config */
#define PORT        ( *( ( volatile unsigned int *)0x41004400 ) )
#define PIN         ( *( ( volatile unsigned int *)0x41004410 ) )

/* SysTick registers */
#define STK_CTRL    ( *( ( volatile unsigned int *)0xE000E010 ) )
#define STK_LOAD    ( *( ( volatile unsigned int *)0xE000E014 ) )
#define STK_VAL     ( *( ( volatile unsigned int *)0xE000E018 ) )
#define STK_CALIB   ( *( ( volatile unsigned int *)0xE000E01C ) )

/* SysCtrl */
#define SYSCTRL_8MHZ ( *( ( volatile unsigned int *)0x40000820 ) )

#define LED_PIN ( 10U )

typedef enum
{
    colour_red,
    colour_orange,
    colour_yellow,
    colour_green,
    colour_blue,
    colour_violet,
    colour_None,
} colour_t;

typedef enum
{
    st_GreenIncrement,
    st_RedDecrement,
    st_BlueIncrement,
    st_GreenDecrement,
    st_RedIncrement,
    st_BlueDecrement,
} colour_state_t;

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char global;
} rgb_t;

union rgb_raw_t
{
    rgb_t bytes;
    unsigned int raw;
};


typedef struct
{
    colour_t colour;
    unsigned int code;
} colour_code_t;

typedef struct
{
    unsigned int start;
    unsigned int colour;
    unsigned int stop;
} led_t;

colour_code_t ledColours [7] =
{
    {colour_red,        0xFF000087},
    {colour_orange,     0x3F02c087},
    {colour_yellow,     0x3FBFc087},
    {colour_green,      0x00FF0087},
    {colour_blue,       0x0000FF87},
    {colour_violet,     0x0F000F87},
    {colour_None,       0x00000087},
};

static led_t led;
static unsigned int ledIndex;


static colour_state_t rgb_update( unsigned int * colour, colour_state_t state )
{
    static rgb_t rgb =
    {
        .red = 0xff,
        .green = 0x00,
        .blue = 0x00,
        .global = 0xe1,
    };
   
    colour_state_t ret = state;

    switch( state )
    {
        case st_GreenIncrement:
            rgb.green++;
            ret = ( rgb.green == 0xFF ) ? st_RedDecrement : state;
            break;
        case st_RedDecrement:
            rgb.red--;
            ret = ( rgb.red == 0x00 ) ? st_BlueIncrement : state;
            break;
        case st_BlueIncrement:
            rgb.blue++;
            ret = ( rgb.blue == 0xFF ) ? st_GreenDecrement : state;
            break;
        case st_GreenDecrement:
            rgb.green--;
            ret = ( rgb.green == 0x00 ) ? st_RedIncrement : state;
            break;
        case st_RedIncrement:
            rgb.red++;
            ret = ( rgb.red == 0xFF ) ? st_BlueDecrement : state;
            break;
        case st_BlueDecrement:
            rgb.blue--;
            ret = ( rgb.blue == 0x00 ) ? st_GreenIncrement : state;
            break;
    }

    union rgb_raw_t raw;
    raw.bytes = rgb;
    *colour = raw.raw;
    return ret;
}

/* SysTick ISR */
void _sysTick( void )
{
    /* XOR Toggle of On-board LED */
//    TOG( PIN, 0x1, LED_PIN );
    static colour_state_t state = st_GreenIncrement;
    unsigned int colour; 
    if( !Timer_Active() )
    {
        state = rgb_update( &colour, state );
        led.colour = colour;
        if( ledIndex == 6 )
        {
            ledIndex = 0U;
        }
        Timer_Start();
    }
}

static void Init( void )
{
    /* 8Mhz Clock */
    CLR( SYSCTRL_8MHZ, 0x3, 8U );
    
    /* set port 10 to output */
    SET( PORT, 1, LED_PIN );
    CLR( PIN, 1, LED_PIN );

    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;
    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 8MHz Clock / 100Hz tick = 0x13880
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x1387F
     */
    STK_CALIB = ( 0x1387F );
    
    /* Made this timer faster than 10ms to make the colour transitions smoother */
    STK_LOAD   = 0x1387F >> 3;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SET( STK_CTRL, 0x7, 0U);
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}

int main ( void )
{
    led.start = 0x00000000;
    led.colour = ledColours[0].code;
    led.stop = 0xFFFFFFFF;

    unsigned int * raw_led = (unsigned int * )&led;

    Init();    
    Timer_Init( raw_led, 3 );

    /* Endless Loop */
    while(1);
    return 0;
}


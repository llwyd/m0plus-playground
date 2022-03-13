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

colour_code_t ledColours [3] =
{
    {colour_red,    0xFF000087},
    {colour_green,  0x00FF0087},
    {colour_red,    0x0000FF87},
};

/* SysTick ISR */
void _sysTick( void )
{
    /* XOR Toggle of On-board LED */
    TOG( PIN, 0x1, LED_PIN );
}

static void Init( void )
{
    /* 8Mhz Clock */
    CLR( SYSCTRL_8MHZ, 0x3, 8U );
    
    /* set port 10 to output */
    SET( PORT, 1, LED_PIN );
    SET( PIN, 1, LED_PIN );

    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;
    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 8MHz Clock / 100Hz tick = 0x13880
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x1387F
     */
    STK_CALIB = ( 0x1387F );
    
    /* 500ms Blink is previous value * 50 */
    STK_LOAD   = 0x1387F * 50;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SET( STK_CTRL, 0x7, 0U);
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}

int main ( void )
{
    led_t led = 
    {
        .start = 0x00000000,
        .colour = ledColours[2].code,
        .stop = 0xFFFFFFFF,
    };
    unsigned int * raw_led = (unsigned int * )&led;

    Init();    
    Timer_Init( raw_led, 3 );

    /* Endless Loop */
    while(1);
    return 0;
}


/* A very minimalist blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "../common/util.h"
#include "../common/spi.h"

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
    unsigned int start;
    unsigned int colour;
    unsigned int stop;
} led_t;

static led_t led;

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


void UpdateLed( void )
{
    static colour_state_t state = st_GreenIncrement;
    unsigned int colour; 
    state = rgb_update( &colour, state );
    
    led.colour = colour;    

    uint8_t * colour_ptr = ( uint8_t *)&led;

    SPI_WriteMSB( colour_ptr, 12U );
}

/* SysTick ISR */
void _sysTick( void )
{
    /* XOR Toggle of On-board LED */
    PIN ^= ( 1 << LED_PIN );

    UpdateLed();
}

int main ( void )
{
    /* 8Mhz Clock */
    CLR( SYSCTRL_8MHZ, 0x3, 8U );
    
    led.start = 0x00000000;
    led.colour = 0xe10000ff;
    led.stop = 0xFFFFFFFF;
    
    /* set port 10 to output */
    PORT |= ( 1 << LED_PIN );
    PIN |= ( 1 << LED_PIN );

    SPI_Init();
    SPI_Write();
    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    STK_CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    STK_LOAD   = 0x13875 >> 2 ;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    STK_CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

    /* Endless Loop */
    while(1);

    return 0;
}


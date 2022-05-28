/* A very minimalist blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */
#include "../common/util.h"
#include "../common/gpio.h"

/* SysTick registers */
#define STK_CTRL    ( *( ( volatile unsigned int *)0xE000E010 ) )
#define STK_LOAD    ( *( ( volatile unsigned int *)0xE000E014 ) )
#define STK_VAL     ( *( ( volatile unsigned int *)0xE000E018 ) )
#define STK_CALIB   ( *( ( volatile unsigned int *)0xE000E01C ) )

#define LED_PIN ( 10U )
#define INPUT_PIN ( 7U )

static gpio_t * GPIO = ( gpio_t *) GPIO_BASE;
static eic_t * EIC   = ( eic_t *) EIC_BASE;

void _eic( void )
{
    GPIO->OUT |= ( 1 << LED_PIN );
}

/* SysTick ISR */
void _sysTick( void )
{
    if( ( GPIO->IN & ( 1 << INPUT_PIN ) ) )
    {
        GPIO->OUT &= ~( 1 << LED_PIN );
    }
    else
    {
        GPIO->OUT |= ( 1 << LED_PIN );
    }
}

void ConfigureInput( void )
{
    /* PA 7 as input switch, pulled up internally, switch connected to GND */

    /* DIR defaults to input */
    GPIO->OUT |= ( 1 << INPUT_PIN );

    /* Set pullup */
    GPIO->PINCFG7 |= ( 1  << 2U );

    /* Input enable */
    GPIO->PINCFG7 |= ( 1 << 1U );
}

int main ( void )
{
    /* set port 10 to output */
    GPIO->DIRR |= ( 1 << LED_PIN );
    GPIO->OUT &= ~( 1 << LED_PIN );

    ConfigureInput();

    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    STK_CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    STK_LOAD   = 0x270F * 5;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    STK_CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

    /* Endless Loop */
    while(1);

    return 0;
}


/* A very minimalist blinky program for the Trinkey m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */


/* Registers for GPIO Config */
#define PORT        ( *( ( volatile unsigned int *)0x41004400 ) )
#define PIN         ( *( ( volatile unsigned int *)0x41004410 ) )

/* SysTick registers */
#define STK_CTRL    ( *( ( volatile unsigned int *)0xE000E010 ) )
#define STK_LOAD    ( *( ( volatile unsigned int *)0xE000E014 ) )
#define STK_VAL     ( *( ( volatile unsigned int *)0xE000E018 ) )
#define STK_CALIB   ( *( ( volatile unsigned int *)0xE000E01C ) )

#define LED_PIN ( 10U )

/* SysTick ISR */
void _sysTick( void )
{
    /* XOR Toggle of On-board LED */
    PIN ^= ( 1 << LED_PIN );
}

int main ( void )
{
    /* set port 10 to output */
    PORT |= ( 1 << LED_PIN );
    PIN |= ( 1 << LED_PIN );
 
    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    STK_CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    STK_LOAD   = 0x270E * 50;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    STK_CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

    /* Endless Loop */
    while(1);

    return 0;
}


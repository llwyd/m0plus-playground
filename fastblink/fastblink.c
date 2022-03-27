/* A very minimalist blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */
#include "../common/util.h"

/* Registers for GPIO Config */
#define PORT        ( *( ( volatile unsigned int *)0x41004400 ) )
#define PIN         ( *( ( volatile unsigned int *)0x41004410 ) )

/* SysTick registers */
#define STK_CTRL    ( *( ( volatile unsigned int *)0xE000E010 ) )
#define STK_LOAD    ( *( ( volatile unsigned int *)0xE000E014 ) )
#define STK_VAL     ( *( ( volatile unsigned int *)0xE000E018 ) )
#define STK_CALIB   ( *( ( volatile unsigned int *)0xE000E01C ) )

/* Generic Clock Controller */
#define GCLK            ( *( ( volatile unsigned int *)0x40000C00 ) )
#define GCLK_STATUS     ( *( ( volatile unsigned int *)0x40000C00 ) )
#define GCLK_CTRL       ( *( ( volatile unsigned int *)0x40000C00 ) )
#define GCLK_GENCTRL    ( *( ( volatile unsigned int *)0x40000C04 ) )
#define GCLK_GENDIV     ( *( ( volatile unsigned int *)0x40000C08 ) )

/* SysCtrl */
#define SYSCTRL         ( *( ( volatile unsigned int *)0x40000800 ) )
#define SYSCTRL_STATUS  ( *( ( volatile unsigned int *)0x4000080C ) )
#define DFL_CTRL        ( *( ( volatile unsigned int *)0x40000824 ) )
#define DFL_VAL         ( *( ( volatile unsigned int *)0x40000828 ) )
#define NVM_CALIB       ( *( ( volatile unsigned int *)0x00806024 ) )

/* NVM */
#define NVM_CTRL       ( *( ( volatile unsigned int *)0x41004004 ) )

#define LED_PIN ( 10U )

/* SysTick ISR, 2Hz blink */
void _sysTick( void )
{
    static unsigned int counter;

    counter++;
    if ( counter == 500 )
    {
        /* XOR Toggle of On-board LED */
        PIN ^= ( 1 << LED_PIN );
        counter = 0U;
    }
}

void Configure48MHz( void )
{
    /* Higher clock requires a wait state */
    NVM_CTRL |= ( 0x1 << 1 ); 

    /* Get coarse calibration value from NVM */
    unsigned int calib = NVM_CALIB;
    calib = ( calib >> 26 );
    calib &= 0x3F;

    /* Configure 48MHz reference in open loop */
    CLR( DFL_CTRL, 1, 7 );
    DFL_VAL |= ( calib << 10U ); 
    DFL_CTRL |= ( 1 << 1U );

    /* Configure GCLK 0 to use 48MHz as main clock */ 
    unsigned int val = ( 1 << 16U ) | ( 0x7 << 8U );
    GCLK_GENCTRL = val;

    while( GCLK_STATUS & ( 1 << 15 ) );
}

int main ( void )
{
    /* set port 10 to output */
    PORT |= ( 1 << LED_PIN );

    Configure48MHz();
    
    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 48MHz Clock / 100Hz tick = 0x75300
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x752FF
     */
    STK_CALIB = ( 0x752FF );
    
    /* 1ms Blink is previous value / 10 */
    STK_LOAD   = 0x752ff / 10;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    STK_CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");

    /* Endless Loop */
    while(1);

    return 0;
}


#include "timer.h"
#include "../common/util.h"

/* Registers for GPIO Config */
#define PORT        ( *( ( volatile unsigned int *)0x41004400 ) )
#define PIN         ( *( ( volatile unsigned int *)0x41004410 ) )
#define PA1CFG      ( *( ( volatile unsigned int *)0x41004440 ) )
#define MUX         ( *( ( volatile unsigned int *)0x41004430 ) )

/* Registers for Timing */
#define TCC2        ( *( ( volatile unsigned int *)0x42002800 ) )
#define TCC2_EVCTRL ( *( ( volatile unsigned int *)0x42002820 ) )
#define TCC2_INT    ( *( ( volatile unsigned int *)0x42002828 ) )
#define TCC2_PER    ( *( ( volatile unsigned int *)0x42002840 ) )
#define TCC2_CC     ( *( ( volatile unsigned int *)0x42002844 ) )
#define TCC2_CC1     ( *( ( volatile unsigned int *)0x42002848 ) )
#define TCC2_INTFLAG     ( *( ( volatile unsigned int *)0x4200282C ) )
#define TCC2_WAVEGEN     ( *( ( volatile unsigned int *)0x4200283C ) )
#define TCC2_WEXCTRL     ( *( ( volatile unsigned int *)0x42002814 ) )
#define TCC2_STATUS    ( *( ( volatile unsigned int *)0x42002830 ) )

/* Generic Clock Controller */
#define GCLK            ( *( ( volatile unsigned int *)0x40000C00 ) )
#define GCLK_STATUS     ( *( ( volatile unsigned int *)0x40000C00 ) )
#define GCLK_CTRL       ( *( ( volatile unsigned int *)0x40000C00 ) )
#define GCLK_GENCTRL    ( *( ( volatile unsigned int *)0x40000C04 ) )
#define GCLK_GENDIV     ( *( ( volatile unsigned int *)0x40000C08 ) )

/* Power Manager */
#define PM              ( *( ( volatile unsigned int *)0x40000400 ) )
#define PM_APBC         ( *( ( volatile unsigned int *)0x40000420 ) )

/* NVIC */
#define NVIC_ISER0      ( *((volatile unsigned int *) 0xE000E100 ) )
#define NVIC_ICPR0      ( *((volatile unsigned int *) 0xE000E280 ) )

volatile static unsigned int idx = 0U;
volatile static unsigned int tcc_count = 0U;

volatile static unsigned int * timerData;
volatile static unsigned int timerDataLength;

/* tcc2 ISR */
void _tcc2( void )
{
    if( TCC2_INTFLAG & ( 1 << 16 ))
    {
        unsigned int pin_mask = ( 1 << tcc_count );        
        if( timerData[idx] & pin_mask )
        {
            PIN |= (1 << 0 );
        }
        else
        {
            PIN &= ~(1 << 0 );
        }

        tcc_count++;

        if( tcc_count == 32 )
        {
            idx++;
            tcc_count = 0;
            if( idx == timerDataLength )
            {
                idx = 0;
                TCC2 &= ~(1 << 1 );
            }
        }
        
        TCC2_INTFLAG |= ( 1 << 16 );
    }
    
    NVIC_ICPR0 |= ( 0x1 << 17 );
}


extern bool Timer_Active( void )
{
    bool timerActive = (bool)!( TCC2_STATUS & 0x1 );
    return timerActive;
} 

extern void Timer_Start( void )
{
    TCC2 |= ( 1 << 1 );
}

static void ConfigureClock( void )
{
    /* Use 8MHz oscillator as source for GCLK1, enable generation */
    unsigned int val = 0U;
    val = ( 1 << 16 ) | ( 0x6 << 8 ) | ( 1 << 0 );
    GCLK_GENCTRL |= val;
    while( GCLK_STATUS & ( 1 << 15 ) ) ;
    
    /* Enable CLK 1 */
    val = ( 1 << 14) | ( 1 << 8 ) | ( 0x1B << 0);
    GCLK_CTRL |= ( val << 16 );
    while( GCLK_STATUS & ( 1 << 15 )) ;

    /* enable APBC bridge for TCC2 */
    SET( PM_APBC, 1, 10 );
}

extern void Timer_Init( unsigned int * data, unsigned int len )
{
    timerData = data;
    timerDataLength = len;

    ConfigureClock();

    /*PA0 = Data In */
    SET( PORT, 0x1, 0x0 );
    CLR(  PIN, 0x1, 0x0 );
    
    /*PA1 = Clock In TCC2/WO[1]
     * This needs to be a multiplexed output signal */
    SET( PORT, 0x1, 0x1 );
    CLR(  PIN, 0x1, 0x1 );
    SET( PA1CFG, 0x1, 8 );
    
    /* Configure to use peripheral E */
    SET( MUX, 0x4, 0x4 );

    SET( NVIC_ISER0, 0x1, 17 );

    SET( TCC2_INT, 1, 16 ); 
    
    TCC2_CC1 = ( 0x2 >> 0 );
    TCC2_CC = ( 0x1 >> 0 );
    TCC2_PER = 0x3;
  
    /* Normal PWM */ 
    SET( TCC2_WAVEGEN, 0x2, 0 );

    TCC2 |= (0x0<<12)| (1<<11) | (0x5 <<8) | ( 1 << 1);
}

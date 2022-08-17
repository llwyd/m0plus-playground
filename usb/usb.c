/* A template blinky program for the Trinket m0+
 * T.L. 2022
 * */

/* ATSAMD21E18 */
#include "../common/gpio.h"
#include "../common/clock.h"
#include "../stateengine/src/fsm.h"
#include "../common/util.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 10U )
#define PM_APBB     ( *( ( volatile uint32_t *)0x4000041C ) )
#define PM_AHB     ( *( ( volatile uint32_t *)0x40000414 ) )
#define USB_BASE        ( 0x41005000 )
#define ENDPOINT_BASE   ( 0x41005100 )

/* NVM */
#define NVM_CTRL       ( *( ( volatile uint32_t *)0x41004004 ) )
#define NVM_CALIB       ( *( ( volatile uint32_t *)0x00806024 ) )

/* NVIC */
#define NVIC_ISER0      ( *((volatile unsigned int *) 0xE000E100 ) )
#define NVIC_ICPR0      ( *((volatile unsigned int *) 0xE000E280 ) )

enum Signals
{
    signal_SysTick = signal_Count,
};

typedef struct
{
    uint8_t CTRLA:8;
    struct res8_t _RESERVED0;
    uint8_t SYNCBUSY:8;
    uint8_t QOSCTRL:8;
    struct res8_t _RESERVED1[9];
    uint8_t FSMSTATUS:8;
    struct res8_t _RESERVED2[22];
    uint32_t DESCADD:32;
    uint16_t PADCAL:16;
}
usb_comms_t;

typedef struct
{
    struct res8_t _RESERVED0[8];
    uint16_t CTRLB:16;
    uint8_t DADD:8;
    struct res8_t _RESERVED1;
    uint8_t STATUS:8;
    struct res8_t _RESERVED2[3];
    uint16_t FNUM:16;
    struct res8_t _RESERVED3[2];
    uint16_t INTENCLR:16;
    struct res8_t _RESERVED4[2];
    uint16_t INTENSET:16;
    struct res8_t _RESERVED5[2];
    uint16_t INTFLAG:16;
    struct res8_t _RESERVED6[2];
    uint16_t EPINTSMRY:16;
}
usb_device_t;

typedef struct
{
    uint8_t EPCFGn:8;
    struct res8_t _RESERVED0[3];
    uint8_t EPSTATUSCLRn:8;
    uint8_t EPSTATUSSETn:8;
    uint8_t EPSTATUSn:8;
    uint8_t EPINTFLAGn:8;
    uint8_t EPINTENCLRn:8;
    uint8_t EPINTENSETn:8;
}
usb_endpoint_t;

typedef struct
{
    uint32_t ADDR:32;
    uint32_t PCKSIZE:32;
    uint16_t EXTREG:16;
    uint8_t STATUS_BK:8;
    struct res8_t _RESERVED0[5];
}
__attribute__((aligned(8)))descriptor_t;


__attribute__((aligned(4))) static uint8_t usb_raw_recv[64];
__attribute__((aligned(4))) static uint8_t usb_raw_send[64];

static descriptor_t bank[16];

volatile static systick_t * SYSTICK = ( systick_t * ) SYSTICK_BASE;
volatile static gpio_t * GPIO = ( gpio_t * ) GPIO_BASE;

volatile static usb_comms_t *   USB_COMMS =   ( usb_comms_t * ) USB_BASE;
volatile static usb_device_t *  USB_DEVICE = ( usb_device_t * ) USB_BASE;
volatile static usb_endpoint_t *  USB_ENDPOINT = ( usb_endpoint_t * ) ENDPOINT_BASE;

static fsm_events_t * event_ptr;

void ep_reset( void );

/* SysTick ISR */
void _sysTick( void )
{
    FSM_AddEvent( event_ptr, signal_SysTick );
}

void _usb( void )
{
    uint32_t device_intflag = USB_DEVICE->INTFLAG;
    uint32_t ep_summary = USB_DEVICE->EPINTSMRY;

    /* Handle USB Reset */
    if ( ( USB_DEVICE->INTFLAG & ( 0x1 << 3 ) ) )
    {
        USB_DEVICE->INTFLAG = ( 0x1 << 3 );
        ep_reset();
        USB_DEVICE->INTENCLR = ( 0x1 << 4 ) | ( 0x1 << 0 );

   //     USB_DEVICE->DADD |= ( 0x1 << 7 );
   /*
        if( usb_raw_recv[1] == 0x5 )
        {
            ep_reset();
            USB_DEVICE->DADD = ( usb_raw_recv[2] );
            USB_DEVICE->DADD |= ( 0x1 << 7 );
            for( int i =0; i < sizeof(usb_raw_recv);i++ )
            {
            //    usb_raw_recv[i]=0x0;
                //bank[1].PCKSIZE &= ~( 0x3FFF );
            }
            bank[0].PCKSIZE &= ~( 0x3FFF );
        }   
        */
    }

    if( USB_ENDPOINT[0].EPINTFLAGn )
    {
        while(1);
    }
    if( ep_summary )
    { 
        while(1);
    }

   NVIC_ICPR0 |= ( 0x1 << 7U );
}

void ep_reset( void )
{
    
    bank[0].ADDR = (uint32_t)usb_raw_recv;
    bank[1].ADDR = (uint32_t)usb_raw_send;
    bank[0].PCKSIZE = (0x3 << 28) | (64 <<14);
    bank[1].PCKSIZE = (0x3 << 28) | (64 <<14);
    //bank[0].PCKSIZE &= ~( 0x3FFF );
    //bank[1].PCKSIZE &= ~( 0x3FFF );
    
    /* Control IN */
    USB_ENDPOINT[0].EPCFGn = ( 0x1 << 4 ) | ( 0x1 );    

    /* Configure ENDPOINT 0 for control */
    USB_ENDPOINT[0].EPINTENSETn = ( 0x1 << 4 ) | ( 0x1 << 1 ) | ( 0x1 << 0 ); 
}

static fsm_status_t Idle( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;

    switch( s )
    {
        case signal_SysTick:
        {
            GPIO->OUT ^= ( 1 << LED_PIN );
            ret = fsm_Handled;
        }
            break;
        case signal_Enter:
        case signal_Exit:
        case signal_None:
        default:
        {
            ret = fsm_Ignored;
        }
            break;
    }

    return ret;
}

static void Loop( void )
{
    fsm_t state;
    state.state = &Idle;
    signal sig = signal_None;
    fsm_events_t events;
    event_ptr = &events;

    FSM_Init( &state, &events );

    while( 1 )
    {
        while( !FSM_EventsAvailable( &events ) );
        sig = FSM_GetLatestEvent( &events );
        FSM_Dispatch( &state, sig );
    }
}

static void Init( void )
{
    /* set port 10 to output */
    GPIO->DIRR |= ( 1 << LED_PIN );
    GPIO->OUT |= ( 1 << LED_PIN );
 
    /* Reset SysTick Counter and COUNTFLAG */
    SYSTICK->VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    SYSTICK->CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    SYSTICK->LOAD   = 0xFFFFFF;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    SYSTICK->CTRL |= 0x7;
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}


void Init_USB( void )
{
    Clock_Set48MHz();
    
    /* Configure PA24 and PA25 as USB D+ D- */
    /* Mux G */
    GPIO->DIRR |= ( 0x1 << 24 );
    GPIO->OUT &= ~( 0x1 << 24 );
    GPIO->PMUX12 |= ( 0x6 << 0 );
    GPIO->PINCFG24 |= ( 0x1 << 0 );
    
    GPIO->DIRR |= ( 0x1 << 25 );
    GPIO->OUT &= ~( 0x1 << 25 );
    GPIO->PMUX12 |= ( 0x6 << 4 );
    GPIO->PINCFG25 |= ( 0x1 << 0 );
    
    PM_APBB |= ( 0x1 << 5 );
    PM_AHB |= ( 0x1 << 6 );
    Clock_ConfigureGCLK( 0x7, 0x1, 0x6 );
   
    /* Software Reset USB */ 
    USB_COMMS->CTRLA |= ( 0x1 << 0 );
    WAITSET( USB_COMMS->SYNCBUSY, 0 );
    WAITCLR( USB_COMMS->SYNCBUSY, 0 );

    /* Get and Set Calibration bits */
    uint32_t usb_transn = ( NVM_CALIB >> 13 ) & 0x1F;
    uint32_t usb_transp = ( NVM_CALIB >> 18 ) & 0x1F;
    uint32_t usb_trim   = ( NVM_CALIB >> 23 ) & 0x7;

    USB_COMMS->PADCAL |= usb_transp;
    USB_COMMS->PADCAL |= ( usb_transn << 6U );
    USB_COMMS->PADCAL |= ( usb_trim << 12 );

    USB_COMMS->DESCADD = (uint32_t)bank;
    
    /* Full speed, attach */
    USB_DEVICE->INTENSET = ( 0x1 << 3 );
    
    
    USB_COMMS->CTRLA |= ( 0x1 << 1 ) | ( 0x1 << 2 );
    WAITCLR( USB_COMMS->SYNCBUSY, 1 ); 
    
    /* Initialisation, device mode, run in standby, enable */
    USB_DEVICE->CTRLB &= ~( 0x1 );

    //ep_reset();
    
    NVIC_ISER0 |= ( 0x1 << 7 );
}

int main ( void )
{
    Init();
    Init_USB();
    Loop();

    return 0;
}


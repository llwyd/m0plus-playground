/* A very minimalist blinky program for the Trinket m0+
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

#define SERCOM_BASE ( 0x42000800 )
#define GCLK_BASE ( 0x40000C00 );
#define PM_APBC         ( *( ( volatile unsigned int *)0x40000420 ) )

/* NVIC */
#define NVIC_ISER0      ( *((volatile unsigned int *) 0xE000E100 ) )
#define NVIC_ICPR0      ( *((volatile unsigned int *) 0xE000E280 ) )

typedef struct
{
    unsigned int CTRLA:32;
    unsigned int CTRLB:32;
    unsigned int RESERVED_0:32;
    unsigned int BAUD:32;
    unsigned int RESERVED_1:32;
    unsigned char INTENCLR:8;
    unsigned char RESERVED_2:8;
    unsigned char INTENSET:8;
    unsigned char RESERVED_3:8;
    unsigned char INTFLAG:8;
    unsigned char RESERVED_4:8;
    unsigned short STATUS:16;
    unsigned int SYNCBUSY:32;
    unsigned int RESERVED_5:32;
    unsigned int ADDR:32;
    unsigned short DATA:16;
    unsigned char RESERVED_6:8;
    unsigned char RESERVED_7:8;
    unsigned char RESERVED_8:8;
    unsigned char RESERVED_9:8;
    unsigned char RESERVED_10:8;
    unsigned char RESERVED_11:8;
    unsigned char DBGCTRL:8;
} i2c_t;

typedef struct
{
    unsigned char CTRL:8;
    unsigned char STATUS:8;
    unsigned short CLKCTRL:16;
    unsigned int GENCTRL:32;
    unsigned int GENDEV:32;
} gclk_t;

typedef struct
{
    unsigned int DIRR:32;
    unsigned int DIRCLR:32;
    unsigned int DIRSET:32;
    unsigned int DIRTGL:32;
    unsigned int OUT:32;
    unsigned int OUTCLR:32;
    unsigned int OUTSET:32;
    unsigned int OUTTGL:32;
    unsigned int IN:32;
    unsigned int CTRL:32;
    unsigned int WRCONFIG:32;
    unsigned int RESERVED_0:32;
    unsigned char PMUX0:8;
    unsigned char PMUX1:8;
    unsigned char PMUX2:8;
    unsigned char PMUX3:8;
    unsigned char PMUX4:8;
    unsigned char PMUX5:8;
    unsigned char PMUX6:8;
    unsigned char PMUX7:8;
    unsigned char PMUX8:8;
    unsigned char PMUX9:8;
    unsigned char PMUX10:8;
    unsigned char PMUX11:8;
    unsigned char PMUX12:8;
    unsigned char PMUX13:8;
    unsigned char PMUX14:8;
    unsigned char PMUX15:8;
    unsigned char PINCFG0:8;
    unsigned char PINCFG1:8;
    unsigned char PINCFG2:8;
    unsigned char PINCFG3:8;
    unsigned char PINCFG4:8;
    unsigned char PINCFG5:8;
    unsigned char PINCFG6:8;
    unsigned char PINCFG7:8;
    unsigned char PINCFG8:8;
    unsigned char PINCFG9:8;
    unsigned char PINCFG10:8;
    unsigned char PINCFG11:8;
    unsigned char PINCFG12:8;
    unsigned char PINCFG13:8;
    unsigned char PINCFG14:8;
    unsigned char PINCFG15:8;
    unsigned char PINCFG16:8;
    unsigned char PINCFG17:8;
    unsigned char PINCFG18:8;
    unsigned char PINCFG19:8;
    unsigned char PINCFG20:8;
    unsigned char PINCFG21:8;
    unsigned char PINCFG22:8;
    unsigned char PINCFG23:8;
    unsigned char PINCFG24:8;
    unsigned char PINCFG25:8;
    unsigned char PINCFG26:8;
    unsigned char PINCFG27:8;
    unsigned char PINCFG28:8;
    unsigned char PINCFG29:8;
    unsigned char PINCFG30:8;
    unsigned char PINCFG31:8;
} gpio_t;

volatile i2c_t * SERCOM = ( i2c_t * ) SERCOM_BASE; 
volatile gpio_t * GPIO = ( gpio_t * ) 0x41004400;
volatile gclk_t * GCLK = ( gclk_t * ) GCLK_BASE;

/* SysTick ISR */
void _sysTick( void )
{
    /* XOR Toggle of On-board LED */
    GPIO->OUT ^= ( 1 << LED_PIN );
}

void Init( void )
{
    /* GCLK */
    /* Set main clk as source for gclk1 */
    unsigned int val = 0U;
    val = ( 1 << 16 ) | ( 1 << 0x6 ) | ( 1 << 0 );
    GCLK->GENCTRL |= val;
    while( GCLK->STATUS & ( 1 << 7 ) ); 

    /* Set GCLK1 as clock for SERCOM0 */
    unsigned short short_val = ( 1 << 14 ) | ( 1 << 8 ) | ( 0x14 << 0 );
    GCLK->CLKCTRL |= short_val;
    while( GCLK->STATUS & ( 1 << 7 ) ); 

    PM_APBC |= ( 0x1 << 2U );

    /* GPIO */
    /* PA8 = SDA, MUX C */
    GPIO->DIRR |= ( 0x3 << 8 );
    GPIO->OUTSET |= ( 0x3 << 8 );
    GPIO->PINCFG8 |= ( 0x1 << 0 );
    GPIO->PMUX4 |= 0x2;
    
    /* PA9 = SCL, MUX C */
    GPIO->PINCFG9 |= ( 0x1 << 0 );
    GPIO->PMUX4 |= ( 0x2 << 4 );

    /* SERCOM 0 */
    /* Set I2C Host Mode */
    SERCOM->CTRLA |= ( 0x5 << 2U );

    /* Enable Smart mode */
    SERCOM->CTRLB |= ( 0x1 << 8U );

    /* Bus timeout time */
    SERCOM->CTRLA |= ( 0x3 << 28U);

    /* Baud */
    SERCOM->BAUD |= ( 0x4 << 0U );

    /* Enable */
    SERCOM->CTRLA |= ( 0x1 << 1 );
    while( ( SERCOM->SYNCBUSY & ( 1 << 1 ) ) );
}

void Read( void )
{
    unsigned char address = 0x48;
    unsigned int addr = 0x91;
    volatile unsigned char data[2] = {0x00, 0x00};

    /* Force IDLE state */
    SERCOM->STATUS |= ( 0x1 << 4 );
    while( ( SERCOM->SYNCBUSY & ( 1 << 2 ) ) );
   
    /* Send ACK after each data read */ 
    SERCOM->CTRLB &= ~( 0x1 << 18 );
    while( ( SERCOM->STATUS & ( 1 << 2 ) ) );

    /* Send Address */
    SERCOM->ADDR = ( ( address << 1 ) | 0x1);
    while( ( SERCOM->SYNCBUSY & ( 1 << 2 ) ) );
    while( !( SERCOM->STATUS & ( 1 << 7 ) ) );
    
    data[0] = SERCOM->DATA;
    
    /* Send NACK after final byte */
    SERCOM->CTRLB |= ( 0x1 << 18 );
    while( ( SERCOM->SYNCBUSY & ( 1 << 2 ) ) );
    while( !( SERCOM->STATUS & ( 1 << 7 ) ) );
    data[1] = SERCOM->DATA;
   
    GPIO->OUT ^= ( 1 << LED_PIN );
    
}

int main ( void )
{
    /* set port 10 to output */
    GPIO->DIRR  |= ( 1 << LED_PIN );
    GPIO->OUT   |= ( 1 << LED_PIN );

    Init();
    /* Reset SysTick Counter and COUNTFLAG */
    STK_VAL = 0x0;

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    STK_CALIB = ( 0x270F );
    
    /* 500ms Blink is previous value * 50 */
    STK_LOAD   = 0x270F * 50;
     
    /* Enable SysTick interrupt, counter 
     * and set processor clock as source */
    STK_CTRL |= 0x7;

    /* Globally Enable Interrupts */
    asm("CPSIE IF");
    Read();

    /* Endless Loop */
    while(1)
    {
        Read();
    }

    return 0;
}


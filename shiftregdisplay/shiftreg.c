#include "gpio.h"
#include "util.h"
#include "systick.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>

#define SPI_PIN_SCK ( 5U )
#define SPI_PIN_MISO ( 6U )
#define SPI_PIN_MOSI ( 7U )

#define SHIFT_REG_RCLK ( 9U )

#define DISPLAY_RS_PIN ( 0U )
#define DISPLAY_E_PIN ( 1U )

#define SPI_ALT_FUNC (5U)

static volatile gpio_t * gpio_a = ( gpio_t *) GPIOA_BASE;

static void Strobe(void);

static void Loop( void )
{
    static uint32_t lastBlink = 0U;
    static uint8_t counter = 0U;
    while( 1 )
    {
        while((SysTick_GetMS() - lastBlink) < 100U);
        counter++;
        lastBlink = SysTick_GetMS();
    }
}

static void Delay_MS(uint32_t delay)
{
    uint32_t tick = SysTick_GetMS();
    while((SysTick_GetMS() - tick) < delay);
}

static void Strobe(void)
{
    GPIO_SetOutput(gpio_a, DISPLAY_E_PIN);
    Delay_MS(1U);
    GPIO_ClearOutput(gpio_a, DISPLAY_E_PIN);
    Delay_MS(1U);
}

static void WriteCommand(uint8_t command)
{
    GPIO_ClearOutput(gpio_a,DISPLAY_RS_PIN);
    
    GPIO_ClearOutput(gpio_a,SHIFT_REG_RCLK);
    SPI_WriteByte(command);
    GPIO_SetOutput(gpio_a,SHIFT_REG_RCLK);
    
//    Delay_MS(1U); 
    Strobe();
}

static void WriteCharacter(uint8_t character)
{
    GPIO_SetOutput(gpio_a,DISPLAY_RS_PIN);
    
    GPIO_ClearOutput(gpio_a,SHIFT_REG_RCLK);
    SPI_WriteByte(character);
    GPIO_SetOutput(gpio_a,SHIFT_REG_RCLK);
   
 //   Delay_MS(1U); 
    Strobe();
}

static void InitDisplay(void)
{
    /* Wait 100ms for power supply to settle */
    uint32_t tick = SysTick_GetMS();
    while((SysTick_GetMS() - tick) < 100U);
    
    /* Clear Display */
    WriteCommand(0x01);

    /* Function Set */
    WriteCommand(0x30);

    /* Turn on display and no cursor */
    WriteCommand(0x0c);

    /* Entry Set */
    WriteCommand(0x06);
    
    /* Function Set */
    WriteCommand(0x38);

    WriteCharacter('H');
    WriteCharacter('e');
    WriteCharacter('l');
    WriteCharacter('l');
    WriteCharacter('o');
    
    WriteCommand(0xC0);
    WriteCharacter('W');
    WriteCharacter('o');
    WriteCharacter('r');
    WriteCharacter('l');
    WriteCharacter('d');
    
}


static void Init( void )
{
    /* SysTick Calibration value for 1ms tick as per ARM datasheet
     * 16MHz Clock / 1000Hz tick = 0x3e80
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x3e7f
     */
    SysTick_Init(0x3e7f);
    
    GPIO_Init();
    GPIO_ConfigureOutput(gpio_a, SHIFT_REG_RCLK);
    GPIO_SetOutput(gpio_a, SHIFT_REG_RCLK);
    
    GPIO_ConfigureOutput(gpio_a, DISPLAY_RS_PIN);
    GPIO_ConfigureOutput(gpio_a,DISPLAY_E_PIN);

    GPIO_ClearOutput(gpio_a,DISPLAY_RS_PIN);
    GPIO_ClearOutput(gpio_a,DISPLAY_E_PIN);

    GPIO_SetAlt(gpio_a,SPI_PIN_SCK, SPI_ALT_FUNC);
    GPIO_SetAlt(gpio_a,SPI_PIN_MISO, SPI_ALT_FUNC);
    GPIO_SetAlt(gpio_a,SPI_PIN_MOSI, SPI_ALT_FUNC); 
    
    
    SPI_Init(); 
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}

int main ( void )
{
    Init();
    InitDisplay();
    Loop();

    return 0;
}


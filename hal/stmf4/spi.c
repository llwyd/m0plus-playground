#include "spi.h"

#define SPI1_BASE   ( 0x40013000 )

typedef struct
{
    uint32_t CR1:32;
    uint32_t CR2:32;
    uint32_t SR:32;
    uint32_t DR:32;
    uint32_t CRCPR:32;
    uint32_t RXCRCR:32;
    uint32_t TXCRCR:32;
    uint32_t I2SCFGR:32;
    uint32_t I2SPR:32;
}
spi_t;

static spi_t * SPI = ( spi_t * ) SPI1_BASE;

extern void SPI_Init(void)
{
    /* Enable SPI bus */
    *((uint32_t *)0x40023844) |= ( 0x1 << 12 );

    /* Master mode */
    SPI->CR1 |= (1<<2U);

    /* LSB first */
//    SPI->CR1 |= (1<<7U);
    /* Software slave management */
    SPI->CR2 |= (1<<2);
    /* Enable */
    SPI->CR1 |= (1<<6U);
}

extern void SPI_WriteByte(uint8_t data)
{
    while(SPI->SR&(1<<7U));
    SPI->DR = data;
    /* Wait to transmit */
    while((SPI->SR&(1<<1U)) != (1<<1U));

}


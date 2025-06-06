/* Conway's game of life for the Trinket m0+ and a small lcd display
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "util.h"
#include "systick.h"
#include "event_heap.h"
#include "state.h"
#include "events.h"
#include <stdbool.h>

#define CORE_CLOCK ( 16000000U )

#define TIM2_BASE   ( 0x40000000 )
#define GPIOA_BASE  ( 0x48000000 )
#define GPIOB_BASE  ( 0x48000400 )
#define GPIOC_BASE  ( 0x48000800 )
#define GPIOD_BASE  ( 0x48000C00 )
#define GPIOE_BASE  ( 0x48001000 )

DEFINE_STATE(Idle);

#define EVENTS(EVNT) \
    EVNT(PQEvent) \

GENERATE_EVENTS(EVENTS);

typedef struct
{
    uint32_t CR1:32;
    uint32_t CR2:32;
    uint32_t SMCR:32;
    uint32_t DIER:32;
    uint32_t SR:32;
    uint32_t EGR:32;
    uint32_t CCMR1:32;
    uint32_t CCMR2:32;
    uint32_t CCER:32;
    uint32_t CNT:32;
    uint32_t PSC:32;
    uint32_t ARR:32;
    uint32_t RESVD:32;
    uint32_t CCR1:32;
    uint32_t CCR2:32;
    uint32_t CCR3:32;
    uint32_t CCR4:32;
    uint32_t RESERVED0:32;
    uint32_t DCR:32;
    uint32_t DMAR:32;
    uint32_t TIM2_OR1:32;
    uint32_t TIM2_OR2:32;
}
timer_t;

typedef struct
{
    uint32_t MODER:32;
    uint32_t OTYPER:32;
    uint32_t OSPEEDR:32;
    uint32_t PUPDR:32;
    uint32_t IDR:32;
    uint32_t ODR:32;
    uint32_t BSRR:32;
    uint32_t LCKR:32;
    uint32_t AFRL:32;
    uint32_t AFRH:32;
    uint32_t BRR:32;
} gpio_t;

static volatile timer_t * TIM2 = ( timer_t * ) TIM2_BASE;

static volatile gpio_t * GPIO_B = ( gpio_t * ) GPIOB_BASE;
static heap_t heap;
static event_fifo_t events;
static void EnqueueEventAfter(event_t e, uint32_t time_ms);

#define PIN (3U)

void  __attribute__((interrupt("IRQ"))) _tim2( void )
{
    ENTER_CRITICAL();
    heap_data_t data = Heap_PopFull(&heap);     
    FIFO_Enqueue( &events, data.event);

    if(Heap_IsEmpty(&heap))
    {
        TIM2->DIER &= ~( 0x1 << 1U );
    }
    else
    {
        uint32_t peek = Heap_Peek(&heap);
        TIM2->CCR1 = peek & 0xFFFF;
    }
    EXIT_CRITICAL(); 
    NVIC_ICPR |= ( 0x1 << 28U );
    TIM2->SR &= ~( 0x1 << 1U );
}

static state_ret_t State_Idle( state_t * this, event_t s )
{
    state_ret_t ret;

    switch( s )
    {
        case EVENT(PQEvent):
        {
            GPIO_B->ODR ^= (1 << PIN);
            //EnqueueEventAfter(EVENT(PQEvent), 1000U);
            ret = HANDLED();
            break;
        }
        case EVENT(Enter):
        case EVENT(Exit):
        {
            ret = HANDLED();
            break;
        }
        default:
        {
            ret = NO_PARENT(this);
        }
        break;
    }

    return ret;
}

static void ConfigureGPIO(void)
{
    *((uint32_t *)0x4002104C) |= ( 0x1 << 1 );
    
    GPIO_B->MODER &= ~( 1 << ((PIN << 1U) + 1U) );
    GPIO_B->MODER |= ( 1 << ((PIN << 1U)) );
    
    GPIO_B->ODR &= ~(1 << PIN);
}

static void EnqueueEventAfter(event_t e, uint32_t time_ms)
{

    uint32_t current_tick = TIM2->CNT;
    uint32_t timeout = current_tick + time_ms;
    heap_data_t data =
    {
        .key = timeout,
        .event = e,
    };

    ENTER_CRITICAL();
    Heap_Push(&heap, data);
    EXIT_CRITICAL();

    ENTER_CRITICAL();
    uint32_t peek = Heap_Peek(&heap);

    TIM2->CCR1 = peek & 0xFFFF;
    EXIT_CRITICAL();
    TIM2->DIER |= ( 0x1 << 1U );
}

static void ConfigureTimer(void)
{
    /* Enable TIM2 */
    *((uint32_t *)0x40021058) |= ( 0x1 << 0U );
    
    
    TIM2->PSC = 0x0FFF;
    TIM2->ARR = 0xFFFF;
    TIM2->CR1 |= ( 0x1 << 0U );
    GPIO_B->ODR |= (1 << PIN);
   
    /* Force update */
    TIM2->EGR |= (0x1 << 0U);

    NVIC_ISER |= ( 1 << 28U );
    NVIC_ICPR |= ( 0x1 << 28U );
    TIM2->SR = 0U;
     
}

int main ( void )
{
    state_t state;
    Events_Init(&events);
    Heap_Init(&heap);
    STATEMACHINE_Init(&state, STATE(Idle));
    ConfigureGPIO();
    ConfigureTimer();
    
    /* Globally Enable Interrupts */
    asm("CPSIE IF"); 
   
    EnqueueEventAfter(EVENT(PQEvent), 2000U);
    EnqueueEventAfter(EVENT(PQEvent), 125U);
    EnqueueEventAfter(EVENT(PQEvent), 1500U);
    EnqueueEventAfter(EVENT(PQEvent), 250U);

    while(1)
    {
        while( FIFO_IsEmpty( (fifo_base_t*)&events ) )
        {
            __asm("wfi");
        }
        event_t e = FIFO_Dequeue( &events );
        STATEMACHINE_Dispatch(&state, e);
    }
    return 0;
}


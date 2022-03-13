#ifndef __TIMER_H_
#define __TIMER_H_

#include "../common/util.h"

extern void Timer_Init( unsigned int * data, unsigned int len );
extern bool Timer_Active( void );
extern void Timer_Start( void );

#endif /* __TIMER_H_ */


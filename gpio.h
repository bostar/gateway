#ifndef __GPIO_H__
#define __GPIO_H__

#include "stdbool.h"

void gpio_init(void);
void reset_zm516x(void);
void sleep_zm516x(unsigned char state);

#endif

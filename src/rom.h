#ifndef UBOY_ROM_H_
#define UBOY_ROM_H_
#include <stdint.h>
#include <stdbool.h>


bool loadrom(const uint8_t* data);
void freerom(void);



#endif


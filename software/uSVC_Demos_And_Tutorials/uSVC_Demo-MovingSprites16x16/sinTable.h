/*
 * sinTable.h
 *
 * Created: 30/08/2019 11:44:20
 *  Author: PETN
 */ 


#ifndef SINTABLE_H_
#define SINTABLE_H_

// Put the following lines in a header file!
#include <stdint.h>
#define NUMBER_OF_SINTABLE_ENTRIES 1024
#define SINTABLE_ENTRY_MASK 0x3FF
extern const int16_t sinTable[NUMBER_OF_SINTABLE_ENTRIES];



#endif /* SINTABLE_H_ */
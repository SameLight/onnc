#include <stdio.h>
#include <stdlib.h>
#include "arm_math.h"

typedef int8_t q7_t;
q7_t inputA[36] = {1, 0, 0, 0, 0, 1, 
				0, 1, 0, 0, 1, 0, 
				0, 0, 1, 1, 0, 0, 
				1, 0, 0, 0, 1, 0, 
				0, 1, 0, 0, 1, 0, 
				0, 0, 1, 0, 1, 0, 
				};
q7_t inputB[36] = {2,2,2,2,2,2,
				2,2,2,2,2,2,
				2,2,2,2,2,2,
				2,2,2,2,2,2,
				2,2,2,2,2,2,
				2,2,2,2,2,2};
q7_t *inputA_buffer = inputA;
q7_t *inputB_buffer = inputB;

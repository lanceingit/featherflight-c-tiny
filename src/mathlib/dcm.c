#include "dcm.h"
#include <stdint.h>

float dcm_trace(Dcm m)
{
    float res=0;
    for(uint8_t i = 0; i < 3; i++) {
        res += m[i][i];
    }
    return res;       
}

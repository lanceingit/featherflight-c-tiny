#pragma once

#include "board.h"

struct i2c_s 
{
    bool inited;
#ifdef F3_EVO    
    I2C_TypeDef* I2Cx;
#endif    
};


#ifdef F3_EVO    
struct i2c_s* i2c_open(I2C_TypeDef* I2Cx);
#endif    
int8_t i2c_read(struct i2c_s* i2c, uint8_t addr, uint8_t reg, uint8_t len, uint8_t* buf);
int8_t i2c_write(struct i2c_s* i2c, uint8_t addr, uint8_t reg, uint8_t data);




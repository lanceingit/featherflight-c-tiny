#pragma once

#include "board.h"

typedef struct 
{
    bool inited;
    I2C_TypeDef* I2Cx;
} i2c_s;


i2c_s* i2c_open(I2C_TypeDef* I2Cx);
int8_t i2c_read(i2c_s* self, uint8_t addr, uint8_t reg, uint8_t len, uint8_t* buf);
int8_t i2c_write(i2c_s* self, uint8_t addr, uint8_t reg, uint8_t data);




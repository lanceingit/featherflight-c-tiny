#pragma once

#include "board.h"

struct i2c_s 
{
    bool inited;
    I2C_TypeDef* I2Cx;
};


struct i2c_s* i2c_open(I2C_TypeDef* I2Cx);
int8_t i2c_read(struct i2c_s* i2c, uint8_t addr, uint8_t reg, uint8_t len, uint8_t* buf);
int8_t i2c_write(struct i2c_s* i2c, uint8_t addr, uint8_t reg, uint8_t data);




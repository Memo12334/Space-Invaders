#pragma once
#include "../8080/types.h"

class Memory
{
    public:
    virtual u8 read_byte(u16 addr) const = 0;
    virtual u16 read_word(u16 addr) const = 0;

    virtual void write_byte(u16 addr, u8 data) = 0;
    virtual void write_word(u16 addr, u16 data) = 0;

    virtual u8 read_port(u8 port) = 0;
    virtual void write_port(u8 port, u8 data) = 0; 
};
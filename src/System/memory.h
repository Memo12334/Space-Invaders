#pragma once
#include <vector>
#include <array>
#include "../8080/types.h"


class Memory
{
    public:
    Memory(const char* file_name);

    public:
    u8 read_byte(u16 addr) const;
    u16 read_word(u16 addr) const;

    void write_byte(u16 addr, u8 data);
    void write_word(u16 addr, u16 data);

    u8 read_port(int port);
    void write_port(u16 addr, u8 data);

    //void load_test(const char* file_name);

    private:
    void load_rom(const char* file_name);

    std::vector<u8> rom;
    std::array<u8, 0x2000> ram; // ram + vram
    
};
#include <fstream>
#include <iterator>
#include <string.h>
#include "memory.h"

Memory::Memory(const char* file_name)
{
}

u8 Memory::read_byte(u16 addr) const
{
    if (addr >= 0x2000 && addr < 0x4000)
        return ram[addr-0x2000];
    
    else if (addr >= 0x0000 && addr < 0x2000)
        return rom[addr];
    
    return 0xFF;
    
   //return rom[addr];
}

u16 Memory::read_word(u16 addr) const
{
    return read_byte(addr) | static_cast<u16>(read_byte(addr+1) << 8);
}

void Memory::write_byte(u16 addr, u8 data)
{
    
    if (addr < 0x2000 || addr >= 0x4000)
        return;
    
    ram[addr] = data;

    //rom[addr] = data;
}

void Memory::write_word(u16 addr, u16 data)
{
    write_byte(addr, data & 0xFF);
    write_byte(addr+1, data >> 8);
}

void Memory::load_rom(const char* file_name)
{
    std::ifstream file(file_name, std::ios::binary);
    std::copy(std::istream_iterator<u8>(file), std::istream_iterator<u8>(),
            std::back_inserter(rom));
    printf("rom\n");
    file.close();
}

/*
void Memory::load_test(const char* file_name)
{
    rom.resize(0x10000);
	FILE *f;
    size_t file_size = 0;

    f = fopen(file_name, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: can't open file '%s'\n", file_name);
    }

    // obtain file size
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    rewind(f);

    u8 buffer[file_size];
    size_t result = fread(buffer, 1, file_size, f);
    if (result != file_size) {
        fprintf(stderr, "error: while reading file '%s'\n", file_name);
    }    
    
    for (size_t i = 0; i < file_size; i++)
    {
        rom[0x100 + i] = buffer[i];
    }

    fclose(f);
}
*/
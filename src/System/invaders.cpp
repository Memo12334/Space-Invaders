#include <fstream>
#include <iterator>
#include "invaders.h"

Invaders::Invaders()
    :
    cpu{*this}
{
}

void Invaders::execute_instruction()
{
    for (int i = 0; i < 2; i++) {
        
        while (cpu.get_cycles() < cycles_per_interrupt)
            cpu.execute_instruction();
    
    cpu.set_cycles(cpu.get_cycles() - cycles_per_interrupt);

    cpu.interrupt(i ? 0x10 : 0x08);
    }
}

void Invaders::handle_event(sf::Event& ev)
{
    if (ev.key.code == sf::Keyboard::Left)
        port1i |= 0x20;
    else
        port1i &= ~0x20;
    
    if (ev.key.code == sf::Keyboard::Right)
        port1i |= 0x40;
    else 
        port1i &= ~0x40;
    
    if (ev.key.code == sf::Keyboard::Space)
        port1i |= 0x10;
    else 
        port1i &= ~0x10;
}

void Invaders::render(sf::RenderWindow& window)
{
    int offset = 0x2400 - 0x2000;
    for (int i = 0, j = 0; i < WIDTH * HEIGHT / 8; i++, j += 4)
    {
        if (ram[offset+i] != 0)
        {
            display[i]   = 255;
            display[i+1] = 255;
            display[i+2] = 255;
            display[i+3] = 255;
        }
        else
        {
            display[i]   = 0;
            display[i+1] = 0;
            display[i+2] = 0;
            display[i+3] = 0;
        }
    } 

    sf::Image image;
	image.create(WIDTH, HEIGHT, display.data());

	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite;
	sprite.setTexture(texture, true);

	window.clear();
	window.draw(sprite);
	window.display(); 
}


u8 Invaders::read_byte(u16 addr) const 
{
    if (addr >= 0x2000 && addr < 0x4000)
        return ram[addr-0x2000];
    
    else if (addr >= 0x0000 && addr < 0x2000)
        return rom[addr];

    return 0xFF;
    
   //return rom[addr];
}

u16 Invaders::read_word(u16 addr) const 
{
    return read_byte(addr) | static_cast<u16>(read_byte(addr+1) << 8);
}

void Invaders::write_byte(u16 addr, u8 data) 
{  
    if (addr < 0x2000 || addr >= 0x4000)
        return;

    ram[addr-0x2000] = data;

    //rom[addr] = data;
}

void Invaders::write_word(u16 addr, u16 data) 
{
    write_byte(addr, data & 0xFF);
    write_byte(addr+1, data >> 8);
}

u8 Invaders::read_port(u8 port) 
{
    u8 x;

    switch (port)
    {
        case 1:
            x = port1i | 0x08;
            break;
        case 2:
            x = port2i;
            break;
        case 3:
            x = static_cast<u8>(((((port4hi << 8) | port4lo) << port2o) >> 8));
            break;
    }

    return x;
}
    
void Invaders::write_port(u8 port, u8 data)
{ 
    switch (port)
    {
        case 2:
            port2o = data & 0x07;
            break;
        case 3:
            // sound
            break;
        case 4:
              port4lo = port4hi;
              port4hi = data;
              break;
        case 5:
            // sound
            break;
    }
}

void Invaders::load_rom(const char* file_name)
{
    std::ifstream file(file_name, std::ios::binary);
    std::copy(std::istream_iterator<u8>(file), std::istream_iterator<u8>(),
            std::back_inserter(rom));
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

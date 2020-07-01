#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "../8080/types.h"
#include "../8080/cpu.h"
#include "memory.h"

class Invaders : public Memory
{
    public:
    Invaders();

    public:
    void execute_instruction();
    void handle_event(sf::Event& ev);
    void render(sf::RenderWindow& window);

    void load_rom(const char* file_name);
    //void load_test(const char* file_name);

    u8 read_byte(u16 addr) const override;
    u16 read_word(u16 addr) const override;

    void write_byte(u16 addr, u8 data) override;
    void write_word(u16 addr, u16 data) override;

    u8 read_port(u8 port) override;
    void write_port(u8 port, u8 data) override;

    private:
    Cpu cpu;

    std::vector<u8> rom = {};
    std::array<u8, 0x2000> ram = {}; // ram + vram
    
    static constexpr int WIDTH = 256;
    static constexpr int HEIGHT = 224;
    std::array<u8, WIDTH * HEIGHT * 4> display = {}; // RGBA 
    static constexpr int cycles_per_interrupt = 2000000 / (60 * 2); // cycles per interrupt

    u8 port1i  = 0;
    u8 port2i  = 0;
    u8 port2o  = 0;
    u8 port3o  = 0;
    u8 port4lo = 0;
    u8 port4hi = 0;
    u8 port5o  = 0;
};
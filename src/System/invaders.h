#pragma once
#include <memory>

class Cpu;
class Memory;

class Invaders
{
    public:
    Invaders(const char* rom);
    ~Invaders();

    Invaders(const Invaders&) = default;
    Invaders(Invaders&&) = default;
    Invaders& operator=(const Invaders&) = default;
    Invaders& operator=(Invaders&&) = default;

    void execute_instruction();

    private:
    std::unique_ptr<Memory> memory;
    std::unique_ptr<Cpu> cpu;
};
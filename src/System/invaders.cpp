#include "invaders.h"
#include "../8080/cpu.h"
#include "memory.h"

Invaders::Invaders(const char* rom)
    :
    memory{std::make_unique<Memory>(rom)},
    cpu{std::make_unique<Cpu>(*memory)}
{
}

Invaders::~Invaders() = default;

void Invaders::execute_instruction()
{
}
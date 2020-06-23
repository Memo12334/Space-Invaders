#include "invaders.h"
#include "../8080/cpu.h"


int main(int argc, char** argv)
{
    Invaders invader{argv[1]};
    
    invader.execute_instruction();
    

}
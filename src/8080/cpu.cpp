#include "cpu.h"
#include "../System/memory.h"
#include <iostream>


Cpu::Cpu(Memory& _memory)
    :
    memory{_memory}
{
    reset();
    set_flags(Reserved1, 1);
}

void Cpu::reset()
{
    A = 0x00;
    B = 0x00;
    C = 0x00;
    D = 0x00;
    E = 0x00;
    H = 0x00;
    L = 0x00;
    F = 0x00;
    pc = 0x0000;
    sp = 0x0000;

    halted = false;
    cycles = 0;
    interrupted = false;
}

int Cpu::get_cycles() const
{
    return cycles;
}

void Cpu::set_cycles(int val)
{
    cycles = val;
}

void Cpu::interrupt(u16 addr)
{
    push(pc);
    pc = addr;
    interrupted = false;
}

u8 Cpu::read_byte(u16 addr) const
{
    return memory.read_byte(addr);
}

u16 Cpu::read_word(u16 addr) const
{
    return memory.read_word(addr);
}

void Cpu::write_byte(u16 addr, u8 data)
{
    memory.write_byte(addr, data);
}

void Cpu::write_word(u16 addr, u16 data)
{
    memory.write_word(addr, data);
}

u16 Cpu::read_next_word()
{
    u16 temp = read_word(pc);
    pc += 2;
    return temp;
}

void Cpu::set_flags(Flags flgs, bool x)
{
    if (x)
        F |= flgs;
    else
        F &= ~flgs;
}

bool Cpu::parity_check(u8 byte)
{
    int c = 0;
    for (int i = 0; i < 8; i++)
        c += (byte >> i) & 0x01;
    
    return (c % 2) == 0 ? true : false;
}

void Cpu::push(u16 data)
{
    write_byte(--sp, data >> 8);
    write_byte(--sp, data & 0xFF);
}

u16 Cpu::pop()
{
    const u8 lo = read_byte(sp++);
    const u8 hi = read_byte(sp++);
    return lo | static_cast<u16>(hi) << 8;    
}

void Cpu::xthl()
{
    const u16 temp = get_HL();
    set_HL(read_word(sp));
    write_word(sp, temp);
}

void Cpu::xchg()
{
    const u16 temp = get_HL();
    set_HL(get_DE());
    set_DE(temp);
}

u8 Cpu::add(u8 data, u8 cf)
{
    const u16 u16res = A + data + cf;
    const u8 u8res = u16res;

    set_flags(Carry, u16res >= 0x100);
    set_flags(Parity, parity_check(u8res));
    set_flags(HalfCarry, ((A & 0xF) + (data & 0xF)) & 0x10);
    set_flags(Zero, u8res == 0);
    set_flags(Sign, u16res & 0x80);

    return u8res;
}

u8 Cpu::sub(u8 data, u8 cf)
{
    const u8 res = A - data - cf;

    set_flags(Carry, A < data);
    set_flags(Parity, parity_check(res));
    set_flags(HalfCarry, ((A & 0xF) - (data & 0xF) >= 0));
    set_flags(Zero, res == 0);
    set_flags(Sign, res & 0x80);
    
    return res;
}

u8 Cpu::inr(u8 data)
{
    data++;
    set_flags(Parity, parity_check(data));
    set_flags(HalfCarry, !(data & 0x0F));
    set_flags(Zero, data == 0);
    set_flags(Sign, data & 0x80);

    return data;
}

u8 Cpu::dcr(u8 data)
{
    set_flags(HalfCarry, (data & 0x0F) == 0) ;
    data--;
    set_flags(Parity, parity_check(data));
    set_flags(Zero, data == 0);
    set_flags(Sign, data & 0x80);

    return data;
}

u8 Cpu::ana(u8 data)
{
    const u8 res = A & data;

    set_flags(Carry, false);
    set_flags(Parity, parity_check(res));
    set_flags(HalfCarry, (A & 0x08) | (data & 0x08));
    set_flags(Zero, res == 0);
    set_flags(Sign, res & 0x80);

    return res;
}

u8 Cpu::ora(u8 data)
{
    const u8 res = A | data;

    set_flags(Carry, false);
    set_flags(Parity, parity_check(res));
    set_flags(HalfCarry, false);
    set_flags(Zero, res == 0);
    set_flags(Sign, res & 0x80);

    return res;
}

u8 Cpu::xra(u8 data)
{
    const u8 res = A ^ data;

    set_flags(Carry, false);
    set_flags(Parity, parity_check(res));
    set_flags(HalfCarry, false);
    set_flags(Zero, res == 0);
    set_flags(Sign, res & 0x80);

    return res;
}

void Cpu::daa()
{
    if (((A & 0x0F) > 9) || (F & HalfCarry))
    {
        A += 0x06;
        F |= HalfCarry;
    }
    else
        F &= ~HalfCarry;

    if ((A > 0x9F) || (F & Carry))
    {
        A += 0x60;
        F |= Carry;
    }
    
    set_flags(Parity, parity_check(A));
    set_flags(Sign, A & 0x80);
    set_flags(Zero, A == 0);
}

void Cpu::dad(u16 data)
{
    const u32 u32res = static_cast<u32>(get_HL()) + static_cast<u32>(data);
    const u16 u16res = static_cast<u16>(u32res);
    set_flags(Carry, u32res > 0xFFFF);
    set_HL(u16res);
}

void Cpu::jmp(const u16 addr)
{
    pc += 2;
    pc = addr;
}

void Cpu::call(const u16 addr)
{
    pc += 2;
    push(pc);
    pc = addr;
}

void Cpu::rlc()
{
    A = (A << 1) | (A >> 7);
    set_flags(Carry, A & 0x01);
}

void Cpu::rrc()
{
    set_flags(Carry, A & 0x01);
    A = (A >> 1) | (A << 7);
}

void Cpu::ral()
{
    u8 a = A;
    A <<= 1;
    if (F & Carry) A |= 0x01;
    set_flags(Carry, a & 0x80);
}

void Cpu::rar()
{
    u8 a = A;
    A >>= 1;
    if (F & Carry) A |= 0x80;
    set_flags(Carry, a & 0x01);
}

void Cpu::execute_instruction()
{
    u8 opcode = read_byte(pc++);
    u16 temp;
    i8080_debug_output();

    switch(opcode)
    {
       // NOP
       case 0x00: cycles += 4; break;       case 0x08: cycles += 4; break;
       case 0x10: cycles += 4; break;       case 0x18: cycles += 4; break;
       case 0x20: cycles += 4; break;       case 0x28: cycles += 4; break;
       case 0x30: cycles += 4; break;       case 0x38: cycles += 4; break;

       // MOV
       case 0x40: cycles += 5; break;
       case 0x50: cycles += 5; D = B; break;
       case 0x60: cycles += 5; H = B; break;
       case 0x70: cycles += 7; write_byte(get_HL(), B); break;
       case 0x41: cycles += 5; B = C; break;
       case 0x51: cycles += 5; D = C; break;
       case 0x61: cycles += 5; H = C; break;
       case 0x71: cycles += 7; write_byte(get_HL(), C); break;
       case 0x42: cycles += 5; B = D; break;
       case 0x52: cycles += 5; break;
       case 0x62: cycles += 5; H = D; break;
       case 0x72: cycles += 5; write_byte(get_HL(), D); break;
       case 0x43: cycles += 5; B = E; break;
       case 0x53: cycles += 5; D = E; break;
       case 0x63: cycles += 5; H = E; break;
       case 0x73: cycles += 7; write_byte(get_HL(), E); break;
       case 0x44: cycles += 5; B = H; break;
       case 0x54: cycles += 5; D = H; break;
       case 0x64: cycles += 5; break;
       case 0x74: cycles += 7; write_byte(get_HL(), H); break;
       case 0x45: cycles += 5; B = L; break;
       case 0x55: cycles += 5; D = L; break;
       case 0x65: cycles += 5; H = L; break;
       case 0x75: cycles += 7; write_byte(get_HL(), L); break;
       case 0x46: cycles += 7; B = read_byte(get_HL()); break;
       case 0x56: cycles += 7; D = read_byte(get_HL()); break;
       case 0x66: cycles += 7; H = read_byte(get_HL()); break;
       case 0x47: cycles += 5; B = A; break;
       case 0x57: cycles += 5; D = A; break;
       case 0x67: cycles += 5; H = A; break;
       case 0x77: cycles += 7; write_byte(get_HL(), A); break;
       case 0x48: cycles += 5; C = B; break;
       case 0x58: cycles += 5; E = B; break;
       case 0x68: cycles += 5; L = B; break;
       case 0x78: cycles += 5; A = B; break;
       case 0x49: cycles += 5; break;
       case 0x59: cycles += 5; E = C; break;
       case 0x69: cycles += 5; L = C; break;
       case 0x79: cycles += 5; A = C; break;
       case 0x4A: cycles += 5; C = D; break;
       case 0x5A: cycles += 5; E = D; break;
       case 0x6A: cycles += 5; L = D; break;
       case 0x7A: cycles += 5; A = D; break;
       case 0x4B: cycles += 5; C = E; break;
       case 0x5B: cycles += 5; break;
       case 0x6B: cycles += 5; L = E; break;
       case 0x7B: cycles += 5; A = E; break;
       case 0x4C: cycles += 5; C = H; break;
       case 0x5C: cycles += 5; E = H; break;
       case 0x6C: cycles += 5; L = H; break;
       case 0x7C: cycles += 5; A = H; break;
       case 0x4D: cycles += 5; C = L; break;
       case 0x5D: cycles += 5; E = L; break;
       case 0x6D: cycles += 5; break;
       case 0x7D: cycles += 5; A = L; break;
       case 0x4E: cycles += 7; C = read_byte(get_HL()); break;
       case 0x5E: cycles += 7; E = read_byte(get_HL()); break;
       case 0x6E: cycles += 7; L = read_byte(get_HL()); break;
       case 0x7E: cycles += 7; A = read_byte(get_HL()); break;
       case 0x4F: cycles += 5; C = A; break;
       case 0x5F: cycles += 5; E = A; break;
       case 0x6F: cycles += 5; L = A; break;
       case 0x7F: cycles += 5; break;

       // MVI
       case 0x06: cycles += 7; B = read_byte(pc++); break;
       case 0x16: cycles += 7; D = read_byte(pc++); break;
       case 0x26: cycles += 7; H = read_byte(pc++); break;
       case 0x36: cycles += 10; write_byte(get_HL(), read_byte(pc++)); break;
       case 0x0E: cycles += 7; C = read_byte(pc++); break;
       case 0x1E: cycles += 7; E = read_byte(pc++); break;
       case 0x2E: cycles += 7; L = read_byte(pc++); break;
       case 0x3E: cycles += 7; A = read_byte(pc++); break;

       case 0x3A: cycles += 13; A = read_byte(read_word(pc)); pc += 2; break;   // LDA
       case 0x32: cycles += 13; write_byte(read_word(pc), A); pc += 2; break;   // STA

       // LDAX
       case 0x0A: cycles += 7; A = read_byte(get_BC()); break;
       case 0x1A: cycles += 7; A = read_byte(get_DE()); break;

       // STAX
       case 0x02: cycles += 7; write_byte(get_BC(), A); break;
       case 0x12: cycles += 7; write_byte(get_DE(), A); break;

       case 0x2A: cycles += 16; set_HL(read_word(read_word(pc))); pc += 2; break;  // LHLD 
       case 0x22: cycles += 16; write_word(read_word(pc), get_HL()); pc += 2; break;  // SHLD

       // LXI
       case 0x01: cycles += 10; set_BC(read_word(pc)); pc += 2; break;
       case 0x11: cycles += 10; set_DE(read_word(pc)); pc += 2; break;
       case 0x21: cycles += 10; set_HL(read_word(pc)); pc += 2; break;
       case 0x31: cycles += 10; sp = read_word(pc); pc += 2; break;

       // PUSH
       case 0xC5: cycles += 11; push(get_BC()); break;
       case 0xD5: cycles += 11; push(get_DE()); break;
       case 0xE5: cycles += 11; push(get_HL()); break;
       case 0xF5: cycles += 11; push(get_AF()); break;

       // POP
       case 0xC1: cycles += 10; set_BC(pop()); break;
       case 0xD1: cycles += 10; set_DE(pop()); break;
       case 0xE1: cycles += 10; set_HL(pop()); break;
       case 0xF1: cycles += 10; set_AF(pop());  break;

       case 0xE3: cycles += 18; xthl(); break; // XTHL
       case 0xF9: cycles += 5; sp = get_HL(); break; // SPHL
       case 0xE9: cycles += 5; pc = get_HL(); break; // PCHL
       case 0xEB: cycles += 5; xchg(); break; // XCHG

       // ADD
       case 0x80: cycles += 4; A = add(B, 0); break;
       case 0x81: cycles += 4; A = add(C, 0); break;
       case 0x82: cycles += 4; A = add(D, 0); break;
       case 0x83: cycles += 4; A = add(E, 0); break;
       case 0x84: cycles += 4; A = add(H, 0); break;
       case 0x85: cycles += 4; A = add(L, 0); break;
       case 0x86: cycles += 7; A = add(read_byte(get_HL()), 0); break;
       case 0x87: cycles += 4; A = add(A, 0); break;

       // SUB
       case 0x90: cycles += 4; A = sub(B, 0); break;
       case 0x91: cycles += 4; A = sub(C, 0); break;
       case 0x92: cycles += 4; A = sub(D, 0); break;
       case 0x93: cycles += 4; A = sub(E, 0); break;
       case 0x94: cycles += 4; A = sub(H, 0); break;
       case 0x95: cycles += 4; A = sub(L, 0); break;
       case 0x96: cycles += 7; A = sub(read_byte(get_HL()), 0); break;
       case 0x97: cycles += 4; A = sub(A, 0); break;
       
       // INR
       case 0x04: cycles += 5; B = inr(B); break;
       case 0x14: cycles += 5; D = inr(D); break;
       case 0x24: cycles += 5; H = inr(H); break;
       case 0x34: cycles += 10; write_byte(get_HL(), inr(read_byte(get_HL()))); break;
       case 0x0C: cycles += 5; C = inr(C); break;
       case 0x1C: cycles += 5; E = inr(E); break;
       case 0x2C: cycles += 5; L = inr(L); break;
       case 0x3C: cycles += 5; A = inr(A); break;

       // DCR
       case 0x05: cycles += 5; B = dcr(B); break;
       case 0x15: cycles += 5; D = dcr(D); break;
       case 0x25: cycles += 5; H = dcr(H); break;
       case 0x35: cycles += 10; write_byte(get_HL(), dcr(read_byte(get_HL()))); break;
       case 0x0D: cycles += 5; C = dcr(C); break;
       case 0x1D: cycles += 5; E = dcr(E); break;
       case 0x2D: cycles += 5; L = dcr(L); break;
       case 0x3D: cycles += 5; A = dcr(A); break;

       // CMP
       case 0xB8: cycles += 4; sub(B, 0); break; 
       case 0xB9: cycles += 4; sub(C, 0); break; 
       case 0xBA: cycles += 4; sub(D, 0); break; 
       case 0xBB: cycles += 4; sub(E, 0); break; 
       case 0xBC: cycles += 4; sub(H, 0); break; 
       case 0xBD: cycles += 4; sub(L, 0); break; 
       case 0xBE: cycles += 7; sub(read_byte(get_HL()), 0); break; 
       case 0xBF: cycles += 4; sub(A, 0); break; 

       // ANA
       case 0xA0: cycles += 4; A = ana(B); break;
       case 0xA1: cycles += 4; A = ana(C); break;
       case 0xA2: cycles += 4; A = ana(D); break;
       case 0xA3: cycles += 4; A = ana(E); break;
       case 0xA4: cycles += 4; A = ana(H); break;
       case 0xA5: cycles += 4; A = ana(L); break;
       case 0xA6: cycles += 7; A = ana(read_byte(get_HL())); break;
       case 0xA7: cycles += 4; A = ana(A); break;

       // ORA
       case 0xB0: cycles += 4; A = ora(B); break;
       case 0xB1: cycles += 4; A = ora(C); break;
       case 0xB2: cycles += 4; A = ora(D); break;
       case 0xB3: cycles += 4; A = ora(E); break;
       case 0xB4: cycles += 4; A = ora(H); break;
       case 0xB5: cycles += 4; A = ora(L); break;
       case 0xB6: cycles += 7; A = ora(read_byte(get_HL())); break;
       case 0xB7: cycles += 4; A = ora(A); break;

       // XRA
       case 0xA8: cycles += 4; A = xra(B); break;
       case 0xA9: cycles += 4; A = xra(C); break;
       case 0xAA: cycles += 4; A = xra(D); break;
       case 0xAB: cycles += 4; A = xra(E); break;
       case 0xAC: cycles += 4; A = xra(H); break;
       case 0xAD: cycles += 4; A = xra(L); break;
       case 0xAE: cycles += 7; A = xra(read_byte(get_HL())); break;
       case 0xAF: cycles += 4; A = xra(A); break;

       case 0xC6: cycles += 7; A = add(read_byte(pc++), 0); break; // ADI
       case 0xD6: cycles += 7; A = sub(read_byte(pc++), 0); break; // SUI
       case 0xE6: cycles += 7; A = ana(read_byte(pc++)); break;    // ANI
       case 0xF6: cycles += 7; A = ora(read_byte(pc++)); break;    // ORI
       case 0xEE: cycles += 7; A = xra(read_byte(pc++)); break;    // XRI
       case 0xFE: cycles += 7; sub(read_byte(pc++), 0); break;     // CPI
       case 0x27: cycles += 4; daa(); break;                       // DAA

       // ADC
       case 0x88: cycles += 4; A = add(B, F & Carry); break;
       case 0x89: cycles += 4; A = add(C, F & Carry); break;
       case 0x8A: cycles += 4; A = add(D, F & Carry); break;
       case 0x8B: cycles += 4; A = add(E, F & Carry); break;
       case 0x8C: cycles += 4; A = add(H, F & Carry); break;
       case 0x8D: cycles += 4; A = add(L, F & Carry); break;
       case 0x8E: cycles += 7; A = add(read_byte(get_HL()), F & Carry); break;
       case 0x8F: cycles += 4; A = add(A, F & Carry); break;

       case 0xCE: cycles += 7; A = add(read_byte(pc++), F & Carry); break; // ACI

       // SBB
       case 0x98: cycles += 4; A = sub(B, F & Carry); break;
       case 0x99: cycles += 4; A = sub(C, F & Carry); break;
       case 0x9A: cycles += 4; A = sub(D, F & Carry); break;
       case 0x9B: cycles += 4; A = sub(E, F & Carry); break;
       case 0x9C: cycles += 4; A = sub(H, F & Carry); break;
       case 0x9D: cycles += 4; A = sub(L, F & Carry); break;
       case 0x9E: cycles += 7; A = sub(read_byte(get_HL()), F & Carry); break;
       case 0x9F: cycles += 4; A = sub(A, F & Carry); break;

       case 0xDE: cycles += 7; A = sub(read_byte(pc++), F & Carry); break; // SBI

       // DAD
       case 0x09: cycles += 10; dad(get_BC()); break;
       case 0x19: cycles += 10; dad(get_DE()); break;
       case 0x29: cycles += 10; dad(get_HL()); break;
       case 0x39: cycles += 10; dad(sp); break;

       // INX
       case 0x03: cycles += 5; B++, C++; break;
       case 0x13: cycles += 5; D++, E++; break;
       case 0x23: cycles += 5; H++, L++; break;
       case 0x33: cycles += 5; sp++; break;

       // DCX
       case 0x0B: cycles += 5; B--, C--; break;
       case 0x1B: cycles += 5; D--, E--; break;
       case 0x2B: cycles += 5; H--, L--; break;
       case 0x3B: cycles += 5; sp--; break;    
      
       // JMP
       case 0xC3: cycles += 10; jmp(read_word(pc)); break;
       case 0xCB: cycles += 10; jmp(read_word(pc)); break;

       // CALL
       case 0xCD: cycles += 17; call(read_word(pc)); break;
       case 0xDD: cycles += 17; call(read_word(pc)); break;
       case 0xED: cycles += 17; call(read_word(pc)); break;
       case 0xFD: cycles += 17; call(read_word(pc)); break;

       // RET
       case 0xC9: cycles += 10; pc = pop(); break;
       case 0xD9: cycles += 10; pc = pop(); break;
       
       case 0xC2: cycles += 10; temp = read_next_word(); if (!(F & Zero)) { pc = temp; } break;     // JNZ
       case 0xCA: cycles += 10; temp = read_next_word(); if (F & Zero) { pc = temp; } break;       // JZ
       case 0xD2: cycles += 10; temp = read_next_word(); if (!(F & Carry)) { pc = temp; } break;   // JNC
       case 0xDA: cycles += 10; temp = read_next_word(); if (F & Carry) { pc = temp; } break;      // JC
       case 0xE2: cycles += 10; temp = read_next_word(); if (!(F & Parity)) { pc = temp; } break;  // JPO
       case 0xEA: cycles += 10; temp = read_next_word(); if (F & Parity) { pc = temp; } break;     // JPE
       case 0xF2: cycles += 10; temp = read_next_word(); if (!(F & Sign)) { pc = temp; } break;    // JP
       case 0xFA: cycles += 10; temp = read_next_word(); if (F & Sign) { pc = temp; } break;       // JM
       case 0xC4: if (!(F & Zero)) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;    // CNZ
       case 0xCC: if (F & Zero) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;       // CZ
       case 0xD4: if (!(F & Carry)) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;   // CNC
       case 0xDC: if (F & Carry) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;      // CC
       case 0xE4: if (!(F & Parity)) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;  // CPO
       case 0xEC: if (F & Parity) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;     // CPE
       case 0xF4: if (!(F & Sign)) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;    // CP
       case 0xFC: if (F & Sign) { cycles += 17; call(read_word(pc)); } else { cycles += 11; pc += 2; } break;       // CM
       case 0xC0: if (!(F & Zero)) { cycles += 11; pc = pop(); } else { cycles += 5; } break;   // RNZ
       case 0xC8: if (F & Zero) { cycles += 11; pc = pop(); } else { cycles += 5; } break;      // RZ
       case 0xD0: if (!(F & Carry)) { cycles += 11; pc = pop(); } else { cycles += 5; } break;  // RNC
       case 0xD8: if (F & Carry) { cycles += 11; pc = pop(); } else { cycles += 5; } break;     // RC
       case 0xE0: if (!(F & Parity)) { cycles += 11; pc = pop(); } else { cycles += 5; } break; // RPO
       case 0xE8: if (F & Parity) { cycles += 11; pc = pop(); } else { cycles += 5; } break;    // RPE
       case 0xF0: if (!(F & Sign)) { cycles += 11; pc = pop(); } else { cycles += 5; } break;   // RP
       case 0xF8: if (F & Sign) { cycles += 11; pc = pop(); } else { cycles += 5; } break;      // RM
       case 0x07: cycles += 4; rlc(); break;    // RLC
       case 0x0F: cycles += 4; rrc(); break;    // RRC
       case 0x17: cycles += 4; ral(); break;    // RAL
       case 0x1F: cycles += 4; rar(); break;    // RAR
       case 0xF3: cycles += 4; interrupted = false; break;  // EI
       case 0xFB: cycles += 4; interrupted = true; break;   // DI
       case 0x3F: cycles += 4; F ^= Carry; break;           // CMC
       case 0x37: cycles += 4; set_flags(Carry, 1); break;  // STC
       case 0x2F: cycles += 4; A ^= 0xFF; break;            // CMA
       case 0x76: cycles += 7; halted = 1; pc--; break;     // HLT

       // RST
       case 0xC7: cycles += 11; call(0x00); break;
       case 0xCF: cycles += 11; call(0x08); break;
       case 0xD7: cycles += 11; call(0x10); break;
       case 0xDF: cycles += 11; call(0x18); break;
       case 0xE7: cycles += 11; call(0x20); break;
       case 0xEF: cycles += 11; call(0x28); break;
       case 0xF7: cycles += 11; call(0x30); break;
       case 0xFF: cycles += 11; call(0x38); break;

       case 0xDB: cycles += 10; A = memory.read_port(pc++); break;            // IN
       case 0xD3: cycles += 10; memory.write_port(read_byte(pc++), A); break; // OUT
    }  
}



static const char* DISASSEMBLE_TABLE[] = {
    "nop", "lxi b,#", "stax b", "inx b", "inr b", "dcr b", "mvi b,#", "rlc",
    "ill", "dad b", "ldax b", "dcx b", "inr c", "dcr c", "mvi c,#", "rrc",
    "ill", "lxi d,#", "stax d", "inx d", "inr d", "dcr d", "mvi d,#", "ral",
    "ill", "dad d", "ldax d", "dcx d", "inr e", "dcr e", "mvi e,#", "rar",
    "ill", "lxi h,#", "shld", "inx h", "inr h", "dcr h", "mvi h,#", "daa",
    "ill", "dad h", "lhld", "dcx h", "inr l", "dcr l", "mvi l,#", "cma",
    "ill", "lxi sp,#","sta $", "inx sp", "inr M", "dcr M", "mvi M,#", "stc",
    "ill", "dad sp", "lda $", "dcx sp", "inr a", "dcr a", "mvi a,#", "cmc",
    "mov b,b", "mov b,c", "mov b,d", "mov b,e", "mov b,h", "mov b,l",
    "mov b,M", "mov b,a", "mov c,b", "mov c,c", "mov c,d", "mov c,e",
    "mov c,h", "mov c,l", "mov c,M", "mov c,a", "mov d,b", "mov d,c",
    "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,M", "mov d,a",
    "mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l",
    "mov e,M", "mov e,a", "mov h,b", "mov h,c", "mov h,d", "mov h,e",
    "mov h,h", "mov h,l", "mov h,M", "mov h,a", "mov l,b", "mov l,c",
    "mov l,d", "mov l,e", "mov l,h", "mov l,l", "mov l,M", "mov l,a",
    "mov M,b", "mov M,c", "mov M,d", "mov M,e", "mov M,h", "mov M,l", "hlt",
    "mov M,a", "mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h",
    "mov a,l", "mov a,M", "mov a,a", "add b", "add c", "add d", "add e",
    "add h", "add l", "add M", "add a", "adc b", "adc c", "adc d", "adc e",
    "adc h", "adc l", "adc M", "adc a", "sub b", "sub c", "sub d", "sub e",
    "sub h", "sub l", "sub M", "sub a", "sbb b", "sbb c", "sbb d", "sbb e",
    "sbb h", "sbb l", "sbb M", "sbb a", "ana b", "ana c", "ana d", "ana e",
    "ana h", "ana l", "ana M", "ana a", "xra b", "xra c", "xra d", "xra e",
    "xra h", "xra l", "xra M", "xra a", "ora b", "ora c", "ora d", "ora e",
    "ora h", "ora l", "ora M", "ora a", "cmp b", "cmp c", "cmp d", "cmp e",
    "cmp h", "cmp l", "cmp M", "cmp a", "rnz", "pop b", "jnz $", "jmp $",
    "cnz $", "push b", "adi #", "rst 0", "rz", "ret", "jz $", "ill", "cz $",
    "call $", "aci #", "rst 1", "rnc", "pop d", "jnc $", "out p", "cnc $",
    "push d", "sui #", "rst 2", "rc", "ill", "jc $", "in p", "cc $", "ill",
    "sbi #", "rst 3", "rpo", "pop h", "jpo $", "xthl", "cpo $", "push h",
    "ani #", "rst 4", "rpe", "pchl", "jpe $", "xchg", "cpe $", "ill", "xri #",
    "rst 5", "rp", "pop psw", "jp $", "di", "cp $", "push psw","ori #",
    "rst 6", "rm", "sphl", "jm $", "ei", "cm $", "ill", "cpi #", "rst 7"
};

// outputs a debug trace of the emulator state to the standard output,
// including registers and flags
void Cpu::i8080_debug_output() {
    char flags[] = "......";

    if (F & Zero) flags[0] = 'z';
    if (F & Sign) flags[1] = 's';
    if (F & Parity) flags[2] = 'p';
    if (F & HalfCarry) flags[3] = 'a';
    if (F & Carry) flags[4] = 'c';

    // registers + flags
    printf("af\tbc\tde\thl\tpc\tsp\tflags\tcycles\n");
    printf("%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%s\t%i\n",
           get_AF(), get_BC(), get_DE(), get_HL(), pc,
           sp, flags, cycles);

    // current address in memory
    printf("%04X: ", pc);

    // current opcode + next two
    printf("%02X %02X %02X", read_byte(pc), read_byte(pc+1),
           read_byte(pc + 2));

    // disassembly of the current opcode
    printf(" - %s", DISASSEMBLE_TABLE[read_byte(pc)]);

    printf("\n================================");
    printf("==============================\n");
}

/*
void Cpu::run_testrom() {
    pc = 0x100; // the test roms all start at 0x100
    write_byte(5, 0xC9); // inject RET at 0x5 to handle "CALL 5", needed
                           // for the test roms

    printf("*******************\n");

    while (true) {
        const u16 cur_pc = pc;

        if (read_byte(pc) == 0x76) { // RET
            printf("HLT at %04X\n", pc);
        }

        if (pc == 5) {
            // prints characters stored in memory at (DE)
            // until character '$' (0x24 in ASCII) is found
            A = 0XFF;
            if (C == 9) {
                u16 i = get_DE();
                do {
                    printf("%c", read_byte(i));
                    i += 1;
                } while (read_byte(i) != 0x24);
            }
            // prints a single character stored in register E
            if (C == 2) {
                printf("%c", E);
            }
        }

        // uncomment following line to have a debug output of machine state
        // warning: will output multiple GB of data for the whole test suite
        //i8080_debug_output();
        execute_instruction();
        
        if (pc == 0) {
            printf("\nJumped to 0x0000 from 0x%04X\n\n", cur_pc);
            break;
        }
    }
}
*/
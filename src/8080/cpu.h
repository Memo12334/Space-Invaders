#pragma once
#include "types.h"

class Memory;

class Cpu 
{
    public:
    Cpu(Memory& _memory);

    void execute_instruction();
    void reset();
    int get_cycles() const;
    void set_cycles(int val);
    void interrupt(u16 addr);

    private:
    Memory& memory;
    int cycles;
    bool halted;
    bool interrupted;

    enum Flags {
        Carry       = 0x01,
        Reserved1   = 0x02,
        Parity      = 0x04,
        Reserved2   = 0x08,
        HalfCarry   = 0x10,
        Reserved3   = 0x20,
        Zero        = 0x40,
        Sign        = 0x80
    };

    u8 A, B, C, D, E, H, L; // Registers
    u8 F; // Flag register
    u16 pc; // Program counter
    u16 sp; // Stack pointer

    inline u16 get_HL() const {
        return (static_cast<u16>(H) << 8) | L;
    }

    inline u16 get_DE() const {
        return (static_cast<u16>(D) << 8) | E;
    }

    inline u16 get_BC() const {
        return (static_cast<u16>(B) << 8) | C;
    }

    inline u16 get_AF() const {
        return (static_cast<u16>(A) << 8) | F;
    }

    inline void set_HL(u16 data) {
        H = data >> 8;
        L = data & 0xFF;
    }

    inline void set_DE(u16 data) {
        D = data >> 8;
        E = data & 0xFF;
    }

    inline void set_BC(u16 data) {
        B = data >> 8;
        C = data & 0xFF;
    }

    inline void set_AF(u16 data) {
        A = data >> 8;
        F = data & 0xFF;
    }

    friend inline Flags operator|(Flags a, Flags b) {
        return static_cast<Flags>(static_cast<int>(a) | static_cast<int>(b));
    }

    friend inline Flags operator&(Flags a, Flags b) {
        return static_cast<Flags>(static_cast<int>(a) & static_cast<int>(b));
    }

    private:
    u8 read_byte(u16 addr) const;
    u16 read_word(u16 addr) const;

    void write_byte(u16 addr, u8 data);
    void write_word(u16 addr, u16 data);
    u16 read_next_word();

    void set_flags(Flags flgs, bool x);
    bool parity_check(u8 byte);

    void push(u16 data);
    u16 pop();
    void xthl();
    void xchg();
    u8 add(u8 data, u8 cf);
    u8 sub(u8 data, u8 cf);
    u8 inr(u8 data);
    u8 dcr(u8 data);
    u8 ana(u8 data);
    u8 ora(u8 data);
    u8 xra(u8 data);
    void daa();
    void dad(u16 data);
    void jmp(const u16 addr);
    void call(const u16 addr);
    void rlc();
    void rrc();
    void ral();
    void rar();


    //public:
    //void run_testrom();
    void i8080_debug_output();
};


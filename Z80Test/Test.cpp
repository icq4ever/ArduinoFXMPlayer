#ifdef TESTING
#include <stdio.h>
#include <stdlib.h>
#include "Z80.h"

static uint8 mem[0x10000] = { 
    0xcd, 0, 0, /* call init */ // 1, 2
/* label */
    0x76,       /* halt */
    0xcd, 0, 0, /* call int */ // 5, 6
    0xc3, 3, 0, /* jmp 3 */
};

static uint8 memtype[0x10000] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

static Z80 z80;
static int time = 0;

static const char *regs[16] = 
{
    "FAl",
    "FAh",
    "FBl",
    "FBh",
    "FCl",
    "FCh",
    "FN",
    "Set",
    "LA",
    "LB",
    "LC",
    "FEl",
    "FEh",
    "SE",
    "DA",
    "DB"
};

uint8 peek(uint16 addr)
{
    uint8 res = mem[addr]; 
    printf("%x: [%x] -> %x\n", z80.registers.pc, addr, res);
    return res;
}

void poke(uint16 addr, uint8 c)
{
    printf("%x: [%x] <- %x\n", z80.registers.pc, addr, c);
//    printf("%X %X\n", addr, c);
    mem[addr] = c;
    memtype[addr] = 2;
}

void handle_output(uint a, uint b)
{
    /*
OUT (0xfffd)   - Select a register 0-14
IN  (0xfffd)   - Read the value of the selected register
OUT (0xbffd)   - Write to the selected register
    */
    if (a == 0xfffd)
//        printf("%d: REG %s", time, regs[b]);
        printf("%X ", b);
    else if (a == 0xbffd)
        printf(" %X\n", b);
//        printf(" = %d\n", b);
    else
        printf("OUT %x, %x\n", a, b);
}

uint handle_input(uint a)
{
	uint8 b = 0xff;
    printf("IN %x\n", a);
	return b;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: %s <file> <address> <intaddr>\n", argv[0]);
        return 0;
    }
    FILE *f;
    fopen_s(&f, argv[1], "rb");
    if (!f)
        return 0;
    unsigned int start, addr, intaddr;
    sscanf_s(argv[2], "%d", &start);
    sscanf_s(argv[3], "%d", &intaddr);
    addr = start;

    while (!feof(f) && addr < 0x10000)
    {
        uint8 b;
        if (fread(&b, 1, 1, f) != 1)
            break;
        memtype[addr] = 1;
        mem[addr++] = b;
    }
    fclose(f);

    mem[1] = start;
    mem[2] = start >> 8;
    mem[5] = intaddr;
    mem[6] = intaddr >> 8;

    z80.registers.pc = 0;
    z80.registers.sp = 0x0;
    int count = 10000000;
    while (count)
    {
        count -= z80.run(1);
        if (z80.halt)
        {
            z80.halt = 0;
            ++z80.registers.pc;
            ++time;
        }
    }
    printf("stopped at %x\n", z80.registers.pc);
    int last = memtype[0];
    printf("%d: 0", last);
    int rom = 0;
    int ram = 0;
    for (int i = 0 ; i < 0x10000 ; ++i)
    {
        if (memtype[i] == 1)
            ++rom;
        else if (memtype[i] == 2)
            ++ram;
        if (memtype[i] == last)
            continue;
        last = memtype[i];
        printf("-%x\n%d: %x", i - 1, last, i);
    }
    printf("-0x10000\n");
    printf("ROM: %d\nRAM: %d\n", rom, ram);
}
#endif

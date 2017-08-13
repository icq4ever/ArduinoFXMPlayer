#ifdef TESTING
/*	Copyright  (c)	Günter Woigk 1996 - 2015
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.


	Z80 Emulator	version 2.2.5
					initially based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	1995-03-28 kio	Started work on C version
	1995-04-01 kio	Finished, yee it works!
	1995-04-06 kio	Removed "final bug": im2 interrupt handling
	1995-06-12 kio	revised EI_WITH_DELAY handling
	1995-06-12 kio	all Info-Calls and profiling added
	1995-06-12 kio	exact r register emulation completed
	1995-06-13 kio	All opcodes covered, no unsupported opcodes!
	1995-11-01 kio	Paged memory supported
	1995-11-26 kio	Engine now completely hardware independent!
	1996-03-28 kio	Core2Core() works even if src and dest block overlap at both ends
	1996-03-28 kio	removed option to switch off latency of 'EI' instruction
	1996-05-02 kio	LOAD_CC after IN and OUT added to make extra wait states possible
	1998-12-24 kio	started port to linux, rework for c++
	2000-02-12 kio	removed bug in LDDR emulation
	2002-02-06 kio	started work on cocoa version
	2004-11-08 kio	cleaned up source. removed 'class cpu' overhead and disfunct stuff
	2004-11-11 kio	wait cycle management and crt video callback finished
	2005-01-16 kio	wait testing cpu cycles validated against the real machine
	2006-10-06 kio	removed the 'class Cpu' layer, as it contained a lot of Z80 stuff...
	2008-06-09 kio	reworked run loop and exit codes; added instruction counting exit condition
	2008-06-14 kio	copyRamToBuffer(), ReadRamFromFile(), saveToFile() and vice versa functions
	2008-06-22 kio	korr: im=0 after reset
	2009-05-28 kio	Qt support, #defines for some classes, UnmapMem(), new initialization model
	2013-06-12 kio	added bits 3 and 5 to zlog_table[]  ((thanks to Rob Probin for the hint))
	2015-06-05 kio	stripped complications and made a sample project to be used a starter for own projects
*/

#define  SAFE 2
#define  LOG  1
#define	 Z80_SOURCE			// -> include other class header files or typedefs only
#include "Z80.h"			// major header file
#include "Z80macros.h"		// required and optional macros
#include "Z80opcodes.h"		// opcode enumeration


// ==================================================================================
//							ctor, dtor
// ==================================================================================


Z80::Z80()
{
	Z80_INFO_CTOR;
}


Z80::~Z80 ( )
{
	Z80_INFO_DTOR;
}


void Z80::init(/*cc=0*/)
{
	//cc = 0;
	//nmi = 0;
	reset_registers();
}


void Z80::reset()
{
	reset_registers();
}


void Z80::reset_registers()
{
// reset registers:
	BC=DE=HL=IX=IY=
	PC=SP=BC2=DE2=HL2=0;
	RA=RA2=RF=RF2=RR=RI=0;

// reset other internal state:
	IFF1=IFF2 = disabled;	// disable interrupts
	registers.im = 0;		// interrupt mode := 0
	//nmi &= ~2;				// clear nmi FF
	halt = no;
}



// ======================================================================
//							helpers
// ======================================================================

uint16 peek2( uint16 addr )
{
  return peek(addr) + (peek(addr+1)<<8);
//  return uint16(peek(addr)) + uint16(uint16(peek(addr+1))<<8);
}

void poke2 ( uint16 addr, uint16 n )
{
  poke( addr,   uint8(n) );
  poke( addr+1, uint8(n>>8) );
}

uint16 Z80::pop2 ( )
{
	registers.sp += 2;
	return peek2( registers.sp - 2 );
}

void Z80::push2 ( uint16 n )
{
	registers.sp -= 2;
	poke2( registers.sp, n );
}

/*
void Z80::copyRamToBuffer( uint16 q, uint8* z, uint16 cnt ) throw()
{
	uint16 n = CPU_PAGESIZE-(q&CPU_PAGEMASK);
	do
	{	if( n>cnt && cnt ) n = cnt;
		c2b( rdPtr(q), z, n );
		q+=n; z+=n; cnt-=n;
		n = CPU_PAGESIZE;
	}
	while( cnt );
}

void Z80::copyBufferToRam( uint8 const* q, uint16 z, uint16 cnt ) throw()
{
	uint16 n = CPU_PAGESIZE-(z&CPU_PAGEMASK);
	do
	{	if( n>cnt && cnt ) n = cnt;
		b2c( q, wrPtr(z), n );
		q+=n; z+=n; cnt-=n;
		n = CPU_PAGESIZE;
	}
	while( cnt );
}

void Z80::copyRamToBuffer( uint16 q, CoreByte* z, uint16 cnt ) throw()
{
	uint16 n = CPU_PAGESIZE-(q&CPU_PAGEMASK);
	do
	{	if( n>cnt && cnt ) n = cnt;
		c2c( rdPtr(q), z, n );
		q+=n; z+=n; cnt-=n;
		n = CPU_PAGESIZE;
	}
	while( cnt );
}

void Z80::copyBufferToRam( CoreByte const* q, uint16 z, uint16 cnt ) throw()
{
	uint16 n = CPU_PAGESIZE-(z&CPU_PAGEMASK);
	do
	{	if( n>cnt && cnt ) n = cnt;
		c2c( q, wrPtr(z), n );
		q+=n; z+=n; cnt-=n;
		n = CPU_PAGESIZE;
	}
	while( cnt );
}


void Z80::readRamFromFile( FD& fd, uint16 z, uint16 cnt ) throw(file_error)
{
	uint8 bu[CPU_PAGESIZE];
	uint16 n = CPU_PAGESIZE-(z&CPU_PAGEMASK);
	do
	{	if( n>cnt && cnt ) n = cnt;
		fd.read_bytes(bu,n);
		b2c( bu, wrPtr(z), n );
		z+=n; cnt-=n;
		n = CPU_PAGESIZE;
	}
	while( cnt );
}

void Z80::writeRamToFile( FD& fd, uint16 q, uint16 cnt ) throw(file_error)
{
	uint8 bu[CPU_PAGESIZE];
	uint16 n = CPU_PAGESIZE-(q&CPU_PAGEMASK);
	do
	{	if( n>cnt && cnt ) n = cnt;
		c2b( rdPtr(q), bu, n );
		fd.write_bytes(bu,n);
		q+=n; cnt-=n;
		n = CPU_PAGESIZE;
	}
	while( cnt );
}
*/

// ----	load and store local variables from/to global registers ------------
#define	LOAD_REGISTERS										\
	r		= registers.r;	/* refresh counter R		*/	\
	pc		= registers.pc;	/* program counter PC		*/	\
	ra		= registers.a;	/* register A				*/	\
	rf		= registers.f;	/* register F				*/	\

#define	SAVE_REGISTERS															\
	registers.r		= (registers.r&0x80)|(r&0x7f);	/* refresh counter R	*/	\
	registers.pc	= pc;							/* program counter PC	*/	\
	registers.a		= ra;							/* register A			*/	\
	registers.f		= rf;							/* register F			*/	\


// ----	jumping -----------------------------------------------------------
#define	LOOP				goto nxtcmnd						// LOOP to next instruction
#define POKE_AND_LOOP(W,C)	{ w=W; c=C; goto poke_and_nxtcmd; }	// POKE(w,c) and goto next instr.
#define EXIT				  goto x							// exit from cpu



// ===========================================================================
// ====	The Z80 Engine =======================================================
// ===========================================================================

int Z80::run(int count)
{
	uint16	pc;					// z80 program counter
	uint8	ra;					// z80 a register
	uint8	rf;					// z80 flags
	uint8	r;					// z80 r register bit 0...6

    int res = 0;

// load local variables from data members:
	LOAD_REGISTERS;

slow_loop:
	irpt = off;

#if 0
// ---- NMI TEST ---------------

// test non-maskable interrupt:
// the NMI is edge-triggered and automatically cleared

	if(nmi&2/*pending*/)
	{								// 11 T cycles, probably: M1:5T, M2:3T, M3:3T
		nmi &= ~2;					// processing no longer pending
		if(halt) { halt=no; pc++; }
	//	IFF2 = IFF1;				// save current irpt enable bit
		IFF1 = disabled;			// disable irpt, preserve IFF2
		INCR_R();
		INCR_CC(5);					// M1: 5 T: interrupt acknowledge cycle
		Z80_INFO_NMI();
		PUSH(pc>>8);				// M2: 3 T: push pch
		PUSH(pc);					// M3: 3 T: push pcl
		pc = 0x0066;				// jump to 0x0066
		//LOOP;
	}
#endif
// ---- INTERRUPT TEST -----------------

// test maskable interrupt:
// note: the /INT signal is not cleared by int ack
//		 the /INT signal is sampled once per instruction at the end of instruction, during refresh cycle
//		 if the interrupt is not started until the /INT signal goes away then it is lost!
	if(!irpt)		   LOOP;		// no interrupt asserted
	if(IFF1==disabled) LOOP;		// irpt disabled?

	if(halt) { halt=no; pc++; }
	IFF1 = IFF2 = disabled;			// disable interrupt
	INCR_R();						// M1: 2 cc + standard opcode timing (min. 4 cc)
	INCR_CC(6);						// /HALT test and busbyte read in cc+4
	Z80_INFO_IRPT();

	switch(registers.im)
	{
	uint16 w;
	case 0:
	/*	mode 0: read instruction from bus
		NOTE:	docs say that M1 is always 6 cc in mode 0, but that is not conclusive:
				the 2 additional T cycles are before opcode fetch, and thus they must always add to instruction execution
				the timing from the moment of instruction fetch (e.g. cc +2 in a normal M1 cycle, cc +4 in int ack M1 cycle)
				and can't be shortended.
				to be tested somehow.	kio 2004-11-12
	*/
		switch(int_ack_byte)
		{
		case RST00: case RST08:		//	timing: M1: 2+5 cc: opcode RSTxx  ((RST is 5 cc))
		case RST10: case RST18:		//			M2:	3 cc:   push pch
		case RST20: case RST28:		//			M3: 3 cc:   push pcl
		case RST30: case RST38:
			INCR_CC(1);
			w = int_ack_byte-RST00;	// w = address to call
			goto irpt_xw;

		case CALL:					//	timing:	M1:   2+4 cc: opcode CALL
			INCR_CC(7);				//			M2+3: 3+4 cc: get NN
			w = int0_call_address;	//  		M4+5: 3+3 cc: push pch, pcl		w = address to call
			goto irpt_xw;

		default:					//	only RSTxx and CALL NN are supported
			TODO();					//  any other opcode is of no real use.		((throws internal_error))
		};

	case 1:
	/*	Mode 1:	RST38
		NOTE:	docs say, timing is 13 cc for implicit RST38 in mode 1 and 12 cc for fetched RST38 in mode 0.
				maybe it is just vice versa? in mode 1 the implicitely known RST38 can be executed with the start of the M1 cycle
				and finish after 5 cc, prolonged to the irpt ackn M1 cycle min. length of 6 cc. Currently i'll stick to 13 cc as doc'd.
				to be tested somehow.	kio 2004-11-12
		TODO:	does the cpu a dummy read in M1?
				at least ZX Spectrum videoram waitcycles is no issue because irpt is off-screen.
	*/
		INCR_CC(1);				//	timing:	M1:	7 cc: int ack cycle (as doc'd)
		w = 0x0038;				//			M2: 3 cc: push pch					w = address to call
		goto irpt_xw;			//			M3: 3 cc: push pcl

irpt_xw: PUSH(pc>>8);			//	M2: 3 cc: push pch
		 PUSH(pc);				//	M3: 3 cc: push pcl and load pc
		 pc = w;				// w = address to call
		 LOOP;

	case 2:
	/*	Mode 2:	jump via table
	*/
		INCR_CC(1);				//	timing:	M1: 7 cc: int ack  and  read interrupt vector from bus
		PUSH(pc>>8);			//			M2: 3 cc: push pch
		PUSH(pc);				//			M3: 3 cc: push pcl
		w = registers.i*256+int_ack_byte;
		PEEK(PCL,w);			//			M4: 3 cc: read low byte from table
		PEEK(PCH,w+1);			//			M5: 3 cc: read high byte and jump
		pc = PC;
		LOOP;

	default:
		IERR();					// bogus irpt mode
	};

	IERR();


// ==========================================================================
// MAIN INSTRUCTION DISPATCHER
// ==========================================================================


uint8	c;							// general purpose byte register
uint16	w;							// general purpose word register
#define	wl		(uint8)w			// access low byte of w
#define	wh		(w>>8)				// access high byte of w


poke_and_nxtcmd:
	POKE(w,c);				// --> CPU_WAITCYCLES, CPU_READSCREEN, cc+=3, Poke(w,c)

nxtcmnd:
    if (halt)
        return res;
    if (res < count)
	{
        ++res;
loop_ei:
		GET_INSTR(c);
		#include "Z80core.h"
		IERR();					// all opcodes decoded!
	}

// ---- EXIT TEST ----

	SAVE_REGISTERS;

    return res;
}
#endif

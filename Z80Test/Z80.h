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

	Z80 cpu emulation version 2.2.4
*/

#ifndef	_Z80_h_
#define	_Z80_h_

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
#define IERR()
#define TODO()

#define disabled 0
#define yes 1
#define enabled 1
#define no 0
#define off 0
#define on 1

//#ifndef COREBYTE_DEFINED
//typedef uint16 CoreByte;		// 8 bit for the CPU plus 8 bit for options
//#endif

// ----	helper ----

inline uint8  lowbyte(uint n)  { return n; }
inline uint8& lowbyte(uint8& n)  { return n; }
#ifdef _BIG_ENDIAN			// m68k, ppc
inline uint8& lowbyte(uint16& n) { return *(((uint8*)&n)+1); }
inline uint8& lowbyte(uint32& n) { return *(((uint8*)&n)+3); }
#else						// i386, pdp
inline uint8& lowbyte(uint16& n) { return (uint8&)n; }
inline uint8& lowbyte(uint32& n) { return (uint8&)n; }
#endif

uint8 peek(uint16 addr);
void poke(uint16 addr, uint8 c);
void handle_output(uint a, uint b);
uint handle_input(uint a);

uint16 peek2( uint16 addr );
void poke2 ( uint16 addr, uint16 n );

// ----	z80 registers ----

union Z80Regs
{
	uint16	nn[16];
	struct	 { uint16 af,bc,de,hl, af2,bc2,de2,hl2, ix,iy,pc,sp, iff, ir; };
   #ifdef _BIG_ENDIAN			// m68k, ppc
	struct	 { uint8 a,f,b,c,d,e,h,l, a2,f2,b2,c2,d2,e2,h2,l2, xh,xl,yh,yl,pch,pcl,sph,spl, iff1,iff2, i,r, im,xxx; };
   #else						// i386, pdp
	struct	 { uint8 f,a,c,b,e,d,l,h, f2,a2,c2,b2,e2,d2,l2,h2, xl,xh,yl,yh,pcl,pch,spl,sph, iff1,iff2, r,i, im,xxx; };
   #endif
};


struct Z80
{
	Z80Regs		registers;			// z80 registers
	//int32		cc;					// cpu clock cycle	(inside run() a local variable cc is used!)
	//uint		nmi;				// idle=0, flipflop set=1, execution pending=2
	bool		halt;				// HALT instruction executed
  bool    irpt;         // interrupt asserted
  uint8    int_ack_byte;     // byte read in int ack cycle (0 = no interupts)
  uint16    int0_call_address;    // mostly unused

  Z80();
  ~Z80();
  //void    handle_output (uint a, uint b);
  //uint    handle_input  (uint a);
  void    reset_registers ();

// Item interface:
 void	init();
 void	reset();
//virtual bool	input			(int32 cc, uint16 addr, uint8& byte);
//virtual bool	output			(int32 cc, uint16 addr, uint8 byte);
//virtual void	update			(int32);
//virtual void	shift_cc		(int32, int32);


// Run the Cpu:
	int run(int count);

	//void		setNmi			()		{ if(nmi==0) { nmi=3; } nmi|=1; }
	//void		clearNmi		()		{ nmi &= ~1; }		// clear FF.set only

//	uint8		peek			(uint16 addr) const;
//	void		poke			(uint16 addr, uint8 c);
//	uint16		peek2			(uint16 addr);
//	void		poke2			(uint16 addr, uint16 n);
	uint16		pop2			();
	void		push2			(uint16 n);
/*
	void		copyBufferToRam	(uint8 const* q, uint16 z_addr, uint16 cnt)		throw();
	void		copyRamToBuffer	(uint16 q_addr, uint8* z, uint16 cnt)			throw();
	void		copyBufferToRam	(const CoreByte* q, uint16 z_addr, uint16 cnt)	throw();
	void		copyRamToBuffer	(uint16 q_addr, CoreByte *z, uint16 cnt)		throw();
	void		readRamFromFile	(FD& fd, uint16 z_addr, uint16 cnt)				throw(file_error);
	void		writeRamToFile	(FD& fd, uint16 q_addr, uint16 cnt)				throw(file_error);
*/
//static inline void c2b(CoreByte const* q, uint8*    z, uint n) { for( uint i=0; i<n; i++ ) z[i] = q[i]; }
//static inline void b2c(uint8    const* q, CoreByte* z, uint n) { for( uint i=0; i<n; i++ ) lowbyte(z[i]) = q[i]; }
//static inline void c2c(CoreByte const* q, CoreByte* z, uint n) { for( uint i=0; i<n; i++ ) lowbyte(z[i]) = q[i]; }
};



#endif
















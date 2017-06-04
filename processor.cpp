#include "processor.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std ;

#define SIGN(x)	((x)&0x80000000)
#define SIGNED_ADD(z,x,y)  (z)=(x)+(y);
#define SIGNED_SUB(z,x,y)  SIGNED_ADD(z,x,-(y));

#define REG_WRITE(r,x)	if((r)!=0)reg[r]=x;


// instruction decoder
class Instruction{
private:
	unsigned int bin ;

public:
	// constructor
	Instruction( unsigned int bin ){
		this->bin = bin ;
	}

	// interpreter
	unsigned char opcode() const {
		return (unsigned char)(bin>>26) ;
	}
	unsigned char rs() const {
		return (unsigned char)(bin>>21) & 0x1F ;
	}
	unsigned char rt() const {
		return (unsigned char)(bin>>16) & 0x1F ;
	}
	unsigned char rd() const {
		return (unsigned char)(bin>>11) & 0x1F ;
	}
	unsigned char shamt() const {
		return (unsigned char)(bin>>6) & 0x1F ;
	}
	unsigned char funct() const {
		return (unsigned char)bin & 0x3F ;
	}
	unsigned short immediate() const {
		return (unsigned short)bin ;
	}
	unsigned int address() const {
		return bin & ((1<<26)-1) ;
	}
} ;


// constructor
Single_Cycle::Single_Cycle(
	ostream *snapshot, istream *iimage, istream *dimage,
	int argc, char **argv, ostream *report, ostream *trace) : Register(snapshot) {
		
	// read iimage with bias
	this->imem = new Memory( 1024, iimage, true ) ;
	pc = imem->get_init_value() ;

	// read dimage
	this->dmem = new Memory( 1024, dimage ) ;
	sp() = dmem->get_init_value() ;

	this->cmp = new CMP_I_D( argc, argv, report, trace ) ;
}


// execute next instruction
bool Single_Cycle::next_cycle( int cycle ){
	bool re = true ;

	// instruction fetch
	unsigned int my_inst = *(unsigned int*)imem->fetch_4_bytes(pc);
	my_inst = (my_inst>>24 & 0x000000ff) |
			  (my_inst>>8  & 0x0000ff00) |
			  (my_inst<<8  & 0x00ff0000) |
			  (my_inst<<24);

	Instruction inst( my_inst ) ;
	 cmp->access_imem( pc, cycle ) ;
	int pc_4 ;
	SIGNED_ADD( pc_4, pc, 4 ) ;
	pc = pc_4 ;

	// register decode
	int s = reg[inst.rs()] ;
	int t = reg[inst.rt()] ;
	int d = reg[inst.rd()] ;

	// trace varible
	string intr_name ;

	// instruction decode
	switch( inst.opcode() ){
	case 0x00:	// R-type
		if( inst.funct() == 0x08 ){
			// jr
			pc = s ;
			intr_name = "jr" ;
		}
		else{

			switch( inst.funct() ){
			case 0x20:	// add
				SIGNED_ADD( d, s, t ) ;
				intr_name = "add" ;
				break ;
			case 0x21: // addu, 2016 feature
				// the result is same as MIPS add (our project do not define carry flag)
				SIGNED_ADD( d, s, t ) ;
				intr_name = "addu" ;
				break ;

			case 0x22:	// sub
				SIGNED_SUB( d, s, t ) ;
				intr_name = "sub" ;
				break ;

			case 0x24:	// and
				d = (int) ((unsigned int)s & (unsigned int)t) ;
				intr_name = "and" ;
				break ;

			case 0x25:	// or
				d = (int) ((unsigned int)s | (unsigned int)t) ;
				intr_name = "or" ;
				break ;

			case 0x26:	// xor
				d = (int) ((unsigned int)s ^ (unsigned int)t) ;
				intr_name = "xor" ;
				break ;

			case 0x27:	// nor
				d = (int) ~((unsigned int)s | (unsigned int)t) ;
				intr_name = "nor" ;
				break ;

			case 0x28:	// nand
				d = (int) ~((unsigned int)s & (unsigned int)t) ;
				intr_name = "nand" ;
				break ;

			case 0x2A:	// slt
				d = (s<t)?1:0 ;
				intr_name = "slt" ;
				break ;

			case 0x00:	// sll
				d = (int) ((unsigned int)t << inst.shamt()) ;
				intr_name = "sll" ;
				break ;

			case 0x02:	// srl
				d = (int) ((unsigned int)t >> inst.shamt()) ;
				intr_name = "srl" ;
				break ;

			case 0x03:	// sra
				d = t >> inst.shamt() ;
				intr_name = "sra" ;
				break ;

			case 0x18: //mult

			    int64_t product;
			    int32_t nrs; 
    			int32_t nrt; 
    			nrs = (int32_t)reg[inst.rs()];
    			nrt = (int32_t)reg[inst.rt()];
    			product = ((int64_t)nrs * (int64_t)nrt);
    			Hi = (uint32_t)((product >> 32) & 0xffffffff);
    			Lo = (uint32_t)(product & 0xffffffff);
    			intr_name = "mult" ;
				break;

			case 0x19: //multu
			  	uint32_t rsu; 
    			uint32_t rtu; 
    			uint64_t productu;
    			rsu = (uint32_t)reg[inst.rs()];
    			rtu = (uint32_t)reg[inst.rt()];
    			productu = ((uint64_t)rsu * (uint64_t)rtu);
    			Hi = (uint32_t)((productu >> 32) & 0xffffffff); 
    			Lo = (uint32_t)(productu & 0xffffffff);
				intr_name = "multu" ;
				break;

			case 0x10: //mfhi
				d = Hi;
				intr_name = "mfhi" ;
				break;

			case 0x12: //mflo
			    intr_name = "mflo" ;
				d = Lo;
				break;
			}

			REG_WRITE( inst.rd(), d ) ;
		}
		break ;

	case 0x08:	// addi
		SIGNED_ADD( t, s, (int)(short)inst.immediate() ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "addi" ;
		break ;

	case 0x09: // addiu, 2016 feature
		SIGNED_ADD( t, s, (int)(short)inst.immediate() ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "addiu" ;
		break ;

	case 0x23:	// lw
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		t = *(int*)( dmem->fetch_4_bytes(d) ) ;
		//convert to bigendian
		t = (t>>24 & 0x000000ff) |
			(t>>8  & 0x0000ff00) |
			(t<<8  & 0x00ff0000) |
			(t<<24) ;

		cmp->access_dmem( d, cycle ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "lw" ;
		break ;

	case 0x21:	// lh
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		t = *(short*)dmem->fetch_2_bytes(d) ;

		//convert to bigendian
		t = (t>>8 & 0x000000ff) |
			(t<<8 & 0x0000ff00);
		if( t & 0x00008000)//signed extention
			t = t | 0xffff0000;

		cmp->access_dmem( d, cycle ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "lh" ;
		break ;

	case 0x25:	// lhu
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		t = *(unsigned short*)dmem->fetch_2_bytes(d) ;
		//convert to bigendian
		t = (t>>8 & 0x000000ff) |
			(t<<8 & 0x0000ff00);

		cmp->access_dmem( d, cycle  ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "lhu" ;
		break ;

	case 0x20:	// lb
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		t = *(char*)dmem->fetch_byte(d) ;
		cmp->access_dmem( d, cycle  ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "lb" ;
		break ;

	case 0x24:	// lbu
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		t = *(unsigned char*)dmem->fetch_byte(d) ;
		cmp->access_dmem( d, cycle  ) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "lbu" ;
		break ;

	case 0x2B:	// sw
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		//bigendian store
		*(  unsigned char*)dmem->fetch_4_bytes(d)     = (unsigned char)( t>>24 & 0x000000ff );
		*(((unsigned char*)dmem->fetch_4_bytes(d))+1) = (unsigned char)( t>>16 & 0x000000ff );
		*(((unsigned char*)dmem->fetch_4_bytes(d))+2) = (unsigned char)( t>>8  & 0x000000ff );
		*(((unsigned char*)dmem->fetch_4_bytes(d))+3) = (unsigned char)( t     & 0x000000ff );

		cmp->access_dmem( d, cycle  ) ;
		intr_name = "sw" ;
		break ;

	case 0x29:	// sh
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		//bigendian store
		*(  unsigned char*)dmem->fetch_2_bytes(d)     = (unsigned char)( t>>8  & 0x000000ff );
		*(((unsigned char*)dmem->fetch_2_bytes(d))+1) = (unsigned char)( t     & 0x000000ff );

		cmp->access_dmem( d, cycle  ) ;
		intr_name = "sh" ;
		break ;

	case 0x28:	// sb
		SIGNED_ADD( d, s, (int)(short)inst.immediate() ) ;
		*(unsigned char*)dmem->fetch_byte(d) = (unsigned char)t ;
		cmp->access_dmem( d, cycle  ) ;
		intr_name = "sb" ;
		break ;

	case 0x0F:	// lui
		t = ((int)inst.immediate()) << 16 ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "lui" ;
		break ;

	case 0x0C:	// andi
		t = (int) ((unsigned int)s & (unsigned int)inst.immediate()) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "andi" ;
		break ;

	case 0x0D:	// ori
		t = (int) ((unsigned int)s | (unsigned int)inst.immediate()) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "ori" ;
		break ;

	case 0x0E:	// nori
		t = (int) ~((unsigned int)s | (unsigned int)inst.immediate()) ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "nori" ;
		break ;

	case 0x0A:	// slti
		t = (s<(int)(short)inst.immediate())?1:0 ;
		REG_WRITE( inst.rt(), t ) ;
		intr_name = "slti" ;
		break ;

	case 0x04:	// beq
		intr_name = "beq" ;
		if( s == t ){
			s = pc ;
			t = 4 * (int)(short)inst.immediate() ;
			SIGNED_ADD( pc, s, t ) ;
		}
		break ;

	case 0x05:	// bne
		intr_name = "bne" ;
		if( s != t ){
			s = pc ;
			t = 4 * (int)(short)inst.immediate() ;
			SIGNED_ADD( pc, s, t ) ;
		}

		break ;
	case 0x07: // bgtz
		intr_name = "bgtz" ;
		// 2016 feature, only the condition is different
		if ( s > 0 ){
			s = pc ;
			t = 4 * (int)(short)inst.immediate() ;
			SIGNED_ADD( pc, s, t ) ;
		}
		break ;
	case 0x03:	// jal
		intr_name = "jal" ;
		reg[31] = pc ;
		pc &= 0xF0000000 ;
		pc |= 4 * inst.address() ;
		break;
		
	case 0x02:	// j
		intr_name = "j" ;
		pc &= 0xF0000000 ;
		pc |= 4 * inst.address() ;
		break ;

	case 0x3F:	// halt
		intr_name = "halt" ;
		re = false ;
		break;
	default:
		cout<<"illegal, invalid op code\n"<<endl;
		exit(EXIT_FAILURE);
	}
	//
	// trace implementation
	//
	intr_name = intr_name ;
	cmp->trace_dump(cycle, intr_name) ;

	return re ;
}

void Single_Cycle::dump_report(){
	cmp->dump() ;
}


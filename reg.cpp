#include "reg.h"
// 
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std ;


// constructor
Register::Register( ostream *os ){
	this->os = os ;

	memset( reg, 0, sizeof(reg) ) ;
	pc = 0;
	Hi = 0;
	Lo = 0;
}

// reference of sp
int &Register::sp(){
	return reg[29] ;
}

// dump regs
int reg_pre[32]={-1} , pre_hi=-1, pre_lo=-1;
void Register::dump( int cycle ){
	*os << "cycle " << dec << cycle << endl ;
	*os << uppercase ;

	for( int i=0 ; i<32 ; i++ )
		if(cycle==0)
			*os << "$" << setfill('0') << setw(2) << dec << i << ": 0x" << setw(8) << hex << reg[i] << endl ;
		else if(reg[i]!=reg_pre[i])
			*os << "$" << setfill('0') << setw(2) << dec << i << ": 0x" << setw(8) << hex << reg[i] << endl ;

	if(pre_hi!= Hi) *os << "$HI: 0x" << setw(8) << hex << Hi << endl ;
	if(pre_lo!= Lo)	*os << "$LO: 0x" << setw(8) << hex << Lo << endl ;

	*os << "PC: 0x" << setw(8) << hex << pc << endl ;
	*os << endl << endl << flush ;

	//copy pre cycle info.
	pre_lo= Lo;
	pre_hi= Hi;
	memcpy(reg_pre, reg, sizeof(reg));
}

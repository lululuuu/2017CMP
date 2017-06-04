#ifndef __REG_H__
#define __REG_H__

#include <iostream>

using namespace std ;


// register file
class Register{
protected:
	int reg[32] ;
	int pc, Hi, Lo;

	ostream *os ;

	// reference of sp
	int &sp() ;

public:
	// constructor
	Register( ostream *os ) ;

	// dump regs
	void dump( int cycle ) ;
} ;

#endif


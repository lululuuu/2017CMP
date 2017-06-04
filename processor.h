#ifndef __SINGLE_CYCLE_H__
#define __SINGLE_CYCLE_H__
#include "reg.h"
#include "mem.h"
#include "cmp.h"

#include <iostream>

using namespace std ;


// single cycle cpu
class Single_Cycle : public Register {
protected:
	Memory *imem ;
	Memory *dmem ;
	CMP_I_D *cmp ;

public:
	// constructor
	Single_Cycle(
		ostream *snapshot,
		istream *iimage,
		istream *dimage,
		int argc,
		char **argv,
		ostream *report,
		ostream	*trace) ;

	// execute next instruction
	bool next_cycle( int cycle ) ;

	void dump_report() ;
} ;

#endif


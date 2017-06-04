#ifndef __MEM_H__
#define __MEM_H__

#include <vector>
#include <iostream>
using namespace std ;


// Memory Unit
class Memory{
protected:
	int init_value ;
	vector<unsigned char> byte ;

	unsigned int out_of_range ;

public:
	// constructor
	Memory( int size, istream *is/*input file*/, bool bias=false ) ;

	int get_init_value() ;

	void* fetch_byte( int address ) ;
	void* fetch_2_bytes( int address ) ;
	void* fetch_4_bytes( int address ) ;
} ;

#endif


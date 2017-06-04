#ifndef __ERROR_DUMP_H__
#define __ERROR_DUMP_H__

#include <iostream>

using namespace std ;


// Handler to write error_dump
class ErrorDump{
protected:
	ostream *os ;

	bool is_write_to_zero ;
	bool is_number_overflow ;
	bool is_address_overflow ;
	bool is_misalign ;

	// clear flags
	void clear() ;

public:
	// constructor
	ErrorDump( ostream *os ) ;

	// write errors
	void write_errors( int cycle ) ;

	// errors
	void write_to_zero() ;
	void number_overflow() ;
	void address_overflow() ;
	void misalign() ;
} ;

#endif


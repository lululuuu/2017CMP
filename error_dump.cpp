#include "error_dump.h"

#include <cstdlib>

using namespace std ;


// constructor
ErrorDump::ErrorDump( ostream *os ){
	this->os = os ;
	clear() ;
}

// clear flags
void ErrorDump::clear(){
	is_write_to_zero = false ;
	is_number_overflow = false ;
	is_address_overflow = false ;
	is_misalign = false ;
}

// set cycle
void ErrorDump::write_errors( int cycle ){
	if( is_write_to_zero )
		*os << "Write $0 error in cycle: " << cycle << endl << flush ;
	if( is_number_overflow )
		*os << "Number overflow in cycle: " << cycle << endl << flush ;
	if( is_address_overflow )
		*os << "Address overflow in cycle: " << cycle << endl << flush ;
	if( is_misalign )
		*os << "Misalignment error in cycle: " << cycle << endl << flush ;

	if( is_address_overflow || is_misalign )
		exit( 0 ) ;

	clear() ;
}

void ErrorDump::write_to_zero(){ 
	is_write_to_zero = true ;
}

void ErrorDump::number_overflow(){
	is_number_overflow = true ;
}

void ErrorDump::address_overflow(){
	is_address_overflow = true ;
}

void ErrorDump::misalign(){
	is_misalign = true ;
}


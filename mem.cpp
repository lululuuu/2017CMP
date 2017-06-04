#include "mem.h"

#include <iostream>
#include <stdlib.h>
using namespace std ;


// constructor
Memory::Memory( int size, istream *is, bool bias ){
	byte.resize( size, 0 ) ;
	out_of_range = 0xFFFFFFFF ;

	is->read( (char*)&init_value, 4 ) ;
	//convert to big_endian
	init_value = (init_value>>24 & 0x000000ff) |
				 (init_value>>8  & 0x0000ff00) | 
				 (init_value<<8  & 0x00ff0000) |
				 (init_value<<24);

	int word_count ;
	is->read( (char*)&word_count, 4 ) ;
	//convert to big_endian
	word_count = (word_count>>24 & 0x000000ff) |
				 (word_count>>8  & 0x0000ff00) | 
				 (word_count<<8  & 0x00ff0000) |
				 (word_count<<24) ;
	
	
	
	if( bias ){//load iimage.bin
		if(word_count*4+init_value > 1024){
			cout<<"illegal Iimage..., loaded address is over 1K\n"<<endl;
			exit(EXIT_FAILURE);
		}
		is->read( (char*)&byte[init_value], word_count *4 ) ;
		if(is->gcount() != word_count*4 ){
			cout<<"illegal Iimage..., loaded instructions isn't a complete word\n"<<endl;
			exit(EXIT_FAILURE);
		}
	}
	else{//load dimage.bin
		if(word_count*4 > 1024){
			cout<<"illegal Dimage..., loaded address is over 1K\n"<<endl;
			exit(EXIT_FAILURE);
		}
		is->read( (char*)&byte[0], word_count *4 ) ;
		if(is->gcount() != word_count*4 ){
			cout<<"illegal Dimage..., loaded instructions isn't a complete word\n"<<endl;
			exit(EXIT_FAILURE);
		}
	}
}

int Memory::get_init_value(){
	return init_value ;
}

void* Memory::fetch_byte( int address ){
	if( (address < 0) || (address >= 1024) ){
		cout<<"illegal , memory address overflow\n"<<endl;
		exit(EXIT_FAILURE);
	}
	return &byte[address] ;
}

void* Memory::fetch_2_bytes( int address ){
	if( (address < 0) || (address+1 >= 1024) ){
		cout<<"illegal , memory address overflow\n"<<endl;
		exit(EXIT_FAILURE);
	}
	if( address&1 ){
		cout<<"illegal , memory address missaline\n"<<endl;
		exit(EXIT_FAILURE);
	}
	return &byte[address];
}

void* Memory::fetch_4_bytes( int address ){
	if( (address < 0) || (address+3 >= 1024) ){
		cout<<"illegal , memory address overflow\n"<<endl;
		exit(EXIT_FAILURE);
	}
	if( address&3 ){
		cout<<"illegal , memory address missaline\n"<<endl;
		exit(EXIT_FAILURE);
	}
	return &byte[address];
}


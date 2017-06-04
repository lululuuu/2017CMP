#ifndef __CMP_H__
#define __CMP_H__

#include <vector>
#include <iostream>
#include <bitset>
#include <string>
using namespace std ;


class CMP_Single{
private:
	int disk_size ;
	int mem_size ;
	int page_size ;
	int cache_size ;
	int cache_block ;
	int cache_asso ;
	
	// page
	vector<int> page_tag ;
	vector<int> page_atime ;

	// TLB
	vector<int> TLB_tag ;
	vector<int> TLB_vadd ;
	vector<int> TLB_atime ;

	// cache
	int cache_bank ;
	vector<int> cache_tag ;
	vector<int> cache_atime ;
	vector<int> MRU_bit;

	int access_page( int address, int cycle ) ;
	void access_cache( int address, int cycle ) ;

public:
	// counter
	int cache_hit, cache_miss ;
	int TLB_hit, TLB_miss ;
	int page_hit, page_miss ;
	int cache_h, page_h, tlb_h, disk_h;

	CMP_Single( int disk_size, int mem_size, int page_size, int cache_size, int cache_block, int cache_asso ) ;

	void access( int address, int cycle ) ;
} ;


class CMP_I_D{
private:
	CMP_Single *imem, *dmem ;
	ostream *os ;
	ostream *trace ;

public:
	CMP_I_D( int argc, char **argv, ostream *os, ostream *trace ) ;
	~CMP_I_D() ;

	void access_imem( int address, int cycle ) ;
	void access_dmem( int address, int cycle ) ;

	void dump() ;
	void trace_dump(int cycle, string intr_name);

} ;

#endif


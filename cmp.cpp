#include "cmp.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <iomanip>

using namespace std ;

#define INVALID -1
#define MRU_bit_ZERO 0
#define MRU_bit_ONE 1

CMP_Single::CMP_Single( int disk_size, int mem_size, int page_size, int cache_size, int cache_block, int cache_asso ){
	if( cache_asso <= 0 )//it won't happen
		cache_asso = cache_size/cache_block ;

	// input
	this->disk_size = disk_size ;
	this->mem_size = mem_size ;
	this->page_size = page_size ;
	this->cache_size = cache_size ;
	this->cache_block = cache_block ;//cache block size
	this->cache_asso = cache_asso ;

	// page(actually,no page table => it directly search the memory)
	page_tag.assign( mem_size/page_size, INVALID ) ;   //store virtual page number
	page_atime.assign( mem_size/page_size, INVALID ) ;

	// TLB
	TLB_tag.assign( disk_size/page_size/4, INVALID ) ; //store virtual page number
	TLB_vadd.assign( disk_size/page_size/4, INVALID ) ;//store physical page number
	TLB_atime.assign( disk_size/page_size/4, INVALID ) ;

	// cache
	cache_bank = cache_size/cache_block/cache_asso ;     //number of sets (total sets)
	cache_tag.assign( cache_size/cache_block, INVALID ) ;//block's number
	cache_atime.assign( cache_size/cache_block, INVALID ) ;
	MRU_bit.assign( cache_size/cache_block, MRU_bit_ZERO ) ;

	// counter
	cache_hit = cache_miss = 0 ;
	TLB_hit = TLB_miss = 0 ;
	page_hit = page_miss = 0 ;
	cache_h = page_h = tlb_h = disk_h = 0 ;

}


void CMP_Single::access( int address, int cycle ){
	address = access_page( address, cycle ) ;
	access_cache( address, cycle ) ;
}


int CMP_Single::access_page( int address, int cycle ){
	page_h = tlb_h = disk_h = 0 ;

	// std::cout << "===============" << endl;
	// std::cout << "cycle "<<cycle << endl;
	 // std::cout << "[Access] "<< "VA:" << address << endl;
	int page_id = address / page_size ;

	// TLB
	size_t TLB_seek ;
	for( TLB_seek=0 ; TLB_seek<TLB_tag.size() ; TLB_seek++ ){
		if( TLB_tag[TLB_seek] == page_id )
			break ;
	}
	if( TLB_seek < TLB_tag.size() ){
		TLB_hit ++ ;
		tlb_h = 1;
		TLB_atime[TLB_seek] = cycle ;
		return TLB_vadd[TLB_seek]*page_size + address%page_size;
	}

	TLB_miss ++ ;

	// page
	size_t page_seek ;
	for( page_seek=0 ; page_seek<page_tag.size() ; page_seek++ ){
		if( page_tag[page_seek] == page_id )
			break ;
	}
	if( page_seek < page_tag.size() )
	{
		page_hit ++ ;
		page_h = 1 ;
	}
	else{
		page_miss ++ ;
		page_seek = 0 ;
		// find the least use index to replace
		for( size_t i=1 ; i<page_tag.size() ; i++ ){
			if( page_atime[page_seek] > page_atime[i] )
				page_seek = i ;
		}

		// SWAP OUT(Page Fault)
		if( page_atime[page_seek] != INVALID ){
			// erase duplicated cache
			//(Page Fault swap out effect corresponding caches)
			for( size_t i=0 ; i<cache_tag.size() ; i++ ){
				if( cache_tag[i]*cache_block/page_size == (int)page_seek ){
					cache_tag[i] = INVALID ;
					MRU_bit[i] = MRU_bit_ZERO ;
					// cache_tag[i] = cache_atime[i] = INVALID ;
					// std::cout << "PF: invalid cache block #" << i <<endl;
				}
			}
		}

		page_tag[page_seek] = page_id ;
	}
	page_atime[page_seek] = cycle ;

	// erase duplicated TLB
	for( size_t i=0 ; i<TLB_tag.size() ; i++ ){
		if( TLB_vadd[i] == (int)page_seek )
			TLB_vadd[i] = TLB_tag[i] = TLB_atime[i] = INVALID ;
	}

	// TLB insert
	TLB_seek = 0 ;
	for( size_t i=1 ; i<TLB_tag.size() ; i++ ){
		if( TLB_atime[TLB_seek] > TLB_atime[i] )
			TLB_seek = i ;
	}

	TLB_tag[TLB_seek] = page_id ;
	TLB_vadd[TLB_seek] = page_seek ;
	TLB_atime[TLB_seek] = cycle ;
	disk_h = 1;

	return TLB_vadd[TLB_seek]*page_size + address%page_size;
}

void CMP_Single::access_cache( int address, int cycle ){
	cache_h = 0 ;
	int cache_id = address / cache_block ; // offset
	int bank_id = cache_id % cache_bank ; // determine which set(bank)

	// cache
	size_t cache_seek ;
	const size_t start = bank_id * cache_asso ;
	const size_t end = start + cache_asso ;
	for( cache_seek=start ; cache_seek<end ; cache_seek++ ){
		if( cache_tag[cache_seek] == cache_id )
			break ;
	}
	if( cache_seek < end ){
		cache_hit ++ ;
		cache_h = 1;
		if (cache_asso != 1)
			MRU_bit[cache_seek] = MRU_bit_ONE ; // update hit block
		// std::cout << ",[HIT]cache_seek:" << cache_seek ;
	}
	else{
		cache_miss ++ ;

		cache_seek = start ;
		for( size_t i=start ; i<end; i++){
			// Second Prority: only MRU = 0
			if( MRU_bit[i] == MRU_bit_ZERO && cache_tag[i] != INVALID ){
				cache_seek = i ;
				break;
			}
		}
		for( size_t i=start ; i<end ; i++ ){

			// First Prority: Invalid block
			if( cache_tag[i] == INVALID ){
				cache_seek = i ;
				break;
			}
		}


		cache_tag[cache_seek] = cache_id ;

		if (cache_asso != 1)
			MRU_bit[cache_seek] = MRU_bit_ONE ;

	}

	int sum=0 ;
	if (cache_asso != 1){
		for( size_t i=start ; i<end ; i++){// check all the bits are 1
			sum+=MRU_bit[i] ;
		}
		if( sum == cache_asso ){
				for( size_t i=start ; i<end ; i++ ){
					MRU_bit[i] = MRU_bit_ZERO;
				}
				MRU_bit[cache_seek] = MRU_bit_ONE ;
		}
	}

    cache_atime[cache_seek] = cycle ;

   /* std::cout<<"Cycle:"<< cycle<<endl;
    std::cout<<"---------------------------------------------------"<<endl;
    std::cout << "[TLB]" << endl  << "tag: ";
    std::cout << "[TLB]" << endl  << "tag: ";
    for (std::vector<int>::const_iterator i = TLB_tag.begin(); i != TLB_tag.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl << "PPN: ";
    for (std::vector<int>::const_iterator i = TLB_vadd.begin(); i != TLB_vadd.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl << "LRU: ";
    for (std::vector<int>::const_iterator i = TLB_atime.begin(); i != TLB_atime.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl ;
    std::cout  << "hit: "<< TLB_hit << ", "<< "miss: "<< TLB_miss << endl; 
    std::cout << endl ;


    std::cout << "[PPE]" << endl << "tag: ";
    for (std::vector<int>::const_iterator i = page_tag.begin(); i != page_tag.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl << "LRU: ";
    for (std::vector<int>::const_iterator i = page_atime.begin(); i != page_atime.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl ;
    std::cout << "hit: "<< page_hit << ", "<< "miss: "<< page_miss << endl;
    std::cout << endl ;

    std::cout << "[Access] "<< "PA:" << address << ", "
        << "tag+index: " << cache_id << ", "
        << "index: " << bank_id << endl << endl;

    std::cout << "[Cache] " << "(asso: "<<cache_asso<<")"<< endl;
    std::cout << "[victim]cache_seek:" << cache_seek << endl;
    std::cout <<"MRU:" ;
    for (std::vector<int>::const_iterator i = MRU_bit.begin(); i != MRU_bit.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl;
    std::cout <<"tag:" ;
    for (std::vector<int>::const_iterator i = cache_tag.begin(); i != cache_tag.end(); ++i)
        std::cout << setw(3) << left << *i << ' ' ;
    std::cout << endl;

    std::cout  << "hit: "<< cache_hit << ", "<< "miss: "<< cache_miss << ", "  << endl;*/

}


CMP_I_D::CMP_I_D( int argc, char **argv, ostream *os, ostream *trace ){
	this->os = os ;
	this->trace = trace ;

	if( argc < 11 ){
		imem = new CMP_Single( 1024, 64,  8, 16, 4, 4 ) ;
		dmem = new CMP_Single( 1024, 32, 16, 16, 4, 1 ) ;
	}
	else{
		imem = new CMP_Single( 1024,
			atoi(argv[1]), atoi(argv[3]),
			atoi(argv[5]), atoi(argv[6]), atoi(argv[7]) ) ;
		dmem = new CMP_Single( 1024,
			atoi(argv[2]), atoi(argv[4]),
			atoi(argv[8]), atoi(argv[9]), atoi(argv[10]) ) ;
	}
}
CMP_I_D::~CMP_I_D(){
	delete imem ;
	delete dmem ;
}


void CMP_I_D::access_imem( int address, int cycle ){
	// std::cout << "I_$:" ;
	imem->access( address, cycle ) ;
}


void CMP_I_D::access_dmem( int address, int cycle ){
	// std::cout << "D_$:" ;
	dmem->access( address, cycle ) ;
}

void CMP_I_D::dump(){
	*os << "ICache :" << endl ;
	*os << "# hits: " << imem->cache_hit << endl ;
	*os << "# misses: " << imem->cache_miss << endl ;
	*os << endl << flush ;

	*os << "DCache :" << endl ;
	*os << "# hits: " << dmem->cache_hit << endl ;
	*os << "# misses: " << dmem->cache_miss << endl ;
	*os << endl << flush ;

	*os << "ITLB :" << endl ;
	*os << "# hits: " << imem->TLB_hit << endl ;
	*os << "# misses: " << imem->TLB_miss << endl ;
	*os << endl << flush ;

	*os << "DTLB :" << endl ;
	*os << "# hits: " << dmem->TLB_hit << endl ;
	*os << "# misses: " << dmem->TLB_miss << endl ;
	*os << endl << flush ;

	*os << "IPageTable :" << endl ;
	*os << "# hits: " << imem->page_hit << endl ;
	*os << "# misses: " << imem->page_miss << endl ;
	*os << endl << flush ;

	*os << "DPageTable :" << endl ;
	*os << "# hits: " << dmem->page_hit << endl ;
	*os << "# misses: " << dmem->page_miss << endl ;
	*os << endl << flush ;
}

void CMP_I_D::trace_dump(int cycle, string intr_name){
	*trace << cycle << ", " << setiosflags( ios::left ) << setw(5)<<intr_name << ":";
	if(imem->cache_h)
		*trace << " ICache ";
	if(imem->tlb_h) 
		*trace << " ITLB ";
	if(imem->page_h)
		*trace << " IPageTable ";
	if(!imem->cache_h && !imem->tlb_h && !imem->page_h)
		*trace << " Disk ";

	*trace << " ; ";

	if(dmem->cache_h)
		*trace << " DCache ";
	if(dmem->tlb_h) 
		*trace << " DTLB ";
	if(dmem->page_h)
		*trace << " DPageTable ";
	if(!dmem->cache_h && !dmem->tlb_h && !dmem->page_h)
		*trace << " Disk ";
	*trace << endl ;

	imem->cache_h = imem->tlb_h = imem->page_h = imem->disk_h = 0;
	dmem->cache_h = dmem->tlb_h = dmem->page_h = dmem->disk_h = 0;

}

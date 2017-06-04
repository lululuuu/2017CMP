#include "processor.h"
#include <fstream>
#include <stdlib.h>
using namespace std ;


int main(int argc, char **argv){
	ofstream snapshot( "snapshot.rpt" ) ;
	ofstream report( "report.rpt" ) ;
	ofstream trace( "trace.rpt" ) ;
	ifstream iimage( "iimage.bin", std::ifstream::binary ) ;
	ifstream dimage( "dimage.bin", std::ifstream::binary ) ;
	if(iimage.fail()){//read iimage.bin fail
		cout<<"No iimage.bin"<<endl;
		exit(EXIT_FAILURE);
	}
	if(dimage.fail()){//read dimage.bin fail
		cout<<"No dimage.bin"<<endl;
		exit(EXIT_FAILURE);
	}
	Single_Cycle single_cycle( &snapshot, &iimage, &dimage, argc, argv, &report, &trace ) ;

	for( int cycle=0 ; cycle<500001; ){
		single_cycle.dump( cycle ) ;

		if( !single_cycle.next_cycle( ++cycle ) )
			break ;
	}

	single_cycle.dump_report() ;

	return 0 ;
}


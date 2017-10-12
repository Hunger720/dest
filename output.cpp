#include <fstream>
#include <iostream>
#include "output.h"


void shape_output(const char *path, float landmark[], int number)
{

	std::ofstream file(path,std::ios::out);
	
	if(file){

		file<<number<<"\n";

		for(int i=0; i<number; ++i)
			file<<get(landmark,i,0)<<' '<<get(landmark,i,1)<<'\n';

		std::cout<<"output file "<<path<<" succeeded.\n";
		
	}else{

		std::cerr<<"output file "<<path<<" failed.\n";

	}

	file.close();
}
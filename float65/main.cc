//
//  main.cc
//  float65
//
//  Created by SpacedCowboy on 1/21/23.
//

#include <iostream>
#include "Float65.h"

int main(int argc, const char * argv[])
	{
	Float65 f65;
	
	for (int i=1; i<argc; i++)
		{
		if ((argv[i][0] == '0') && (argv[i][1] == 'x'))
			{
			f65.set(argv[i] + 2);
			printf("Float: %f\n", f65.toFloat());
			}
		else
			{
			f65.set(atof(argv[i]));
			printf("Hex  : %s\n", f65.toString().c_str());
			}
		}
			
	return 0;
	}
	
	
	

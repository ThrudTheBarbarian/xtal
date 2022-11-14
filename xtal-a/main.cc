//
//  main.cc
//  xtal-a
//
//  Created by Thrud The Barbarian on 10/27/22.
//
// Takes the assembly output from the xtal compiler and produces an
// executable ready to run on the extended hardware

#include "Assembler.h"

int main(int argc, const char * argv[])
	{
	Assembler assembler;
	return assembler.main(argc, argv);
	}

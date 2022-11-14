//
//  OutputBlock.cc
//  as8
//
//  Created by Thrud The Barbarian on 10/31/22.
//

#include "Token.h"
#include "OutputBlock.h"

/****************************************************************************\
|* Constructor
\****************************************************************************/
OutputBlock::OutputBlock(int baseAddress)
			:_baseAddress(baseAddress)
	{
	_data.push_back(0xFF);
	_data.push_back(0xFF);
	_data.push_back(baseAddress & 0xFF);
	_data.push_back((baseAddress >> 8) & 0xFF);
	_data.push_back(0);
	_data.push_back(0);
	}

/****************************************************************************\
|* Add a token to the stream
\****************************************************************************/
void OutputBlock::add(Token &token)
	{
	if (token.type() == T_6502)
		{
		_data.push_back(token.opcode());
		if (token.bytes() > 1)
			_data.push_back(token.op1());
		if (token.bytes() > 2)
			_data.push_back(token.op2());
		}
	else if (token.which() == P_BYTE)
		{
		_data.insert(std::end(_data),
					 std::begin(token.data()),
					 std::end(token.data()));
		}
	else
		LOG("Attempt to add Token '%s' to an output block",
			token.toString().c_str());
	}

/****************************************************************************\
|* Finalise this block
\****************************************************************************/
void OutputBlock::finalise(void)
	{
	int end = (int)_data.size() - 1 + _baseAddress - 6;
	_data[4] = end & 0xFF;
	_data[5] = (end >> 8) & 0xFF;
	}

/****************************************************************************\
|* Write data to the provided file
\****************************************************************************/
void OutputBlock::write(FILE *fp)
	{
	if (_data.size() > 6)
		if (fwrite(_data.data(), 1, _data.size(), fp) != _data.size())
			FATAL(ERR_OUTPUT, "Couldn't write block to file completely");
	}

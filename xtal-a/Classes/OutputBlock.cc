//
//  OutputBlock.cc
//  xtal-a
//
//  Created by Thrud The Barbarian on 10/31/22.
//

#include "Token.h"
#include "OutputBlock.h"

#include "sharedDefines.h"

/****************************************************************************\
|* Constructor
\****************************************************************************/
OutputBlock::OutputBlock(int baseAddress)
			:_baseAddress(baseAddress)
			,_isData(false)
	{
	clear();
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
|* Add raw data to the block
\****************************************************************************/
void OutputBlock::add(uint8_t *data, int len)
	{
	const uint8_t * bytes = const_cast<uint8_t *>(data);
	add(bytes, len);
	}
	
void OutputBlock::add(char *data, int len)
	{
	const char * bytes = const_cast<char *>(data);
	add(bytes, len);
	}
	
void OutputBlock::add(const char *data, int len)
	{
	const uint8_t * bytes = reinterpret_cast<const uint8_t *>(data);
	add(bytes, len);
	}
	
void OutputBlock::add(const uint8_t *data, int len)
	{
	for (int i=0; i<len; i++)
		_data.push_back(data[i]);
	}
	
	
/****************************************************************************\
|* Checksum this block
\****************************************************************************/
void OutputBlock::addChecksum(void)
	{
	uint16_t value = 0;
	for (int i=6; i<_data.size(); i++)
		value += _data[i];
	_data.push_back(value & 0xFF);
	_data.push_back(value >> 8);
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
|* Return the size of the block data
\****************************************************************************/
int OutputBlock::size(void)
	{
	return (int)(_data.size() - 6);
	}

/****************************************************************************\
|* Reset the block
\****************************************************************************/
void OutputBlock::clear(void)
	{
	_data.clear();
	_data.push_back(0xFF);
	_data.push_back(0xFF);
	_data.push_back(_baseAddress & 0xFF);
	_data.push_back((_baseAddress >> 8) & 0xFF);
	_data.push_back(0);
	_data.push_back(0);
	}
	
/****************************************************************************\
|* Write data to the provided file
\****************************************************************************/
void OutputBlock::write(FILE *fp)
	{
	if (_data.size() > 6)
		if (fwrite(_data.data(), 1, _data.size(), fp) != _data.size())
			FATAL(ERR_OUTPUT, "Couldn't write block to file completely\n");
	}

/****************************************************************************\
|* Write hex-data to the provided file
\****************************************************************************/
void OutputBlock::writeHex(FILE *fp)
	{
	if (_data.size() > 6)
		{
		int at = _data[2] + ((int)_data[3]) * 256;
		
		for (int i=6; i<_data.size(); i+=8)
			{
			fprintf(fp, "%04X:", at);
			
			for (int j=0; j<8; j++)
				{
				if (i+j < _data.size())
					{
					fprintf(fp, " %02X", _data[i+j]);
					at ++;
					}
				}
			fprintf(fp, "\n");
			}
		}
	}

//
//  Float65.cc
//  float65
//
//  Created by Thrud The Barbarian on 1/21/23.
//
//
//  Maths floating point routines
//  =============================
//
//  Format:
//  -------
//
// Laid out in memory as below:
//
//          +--------+  +--------+  +---------+  +--------+
//          |  LSB   |  |Mantissa|  |   MSB   |  |Exponent|
//          |   (n)  |  |  (n+1) |  |  (n+2)  |  |  (n+3) |
//          |MMMMMMMM|  |MMMMMMMM|  |S.MMMMMMM|  |SEEEEEEE|
//          +--------+  +--------+  +---------+  +--------+;
//
// If mantissa sign is negative, then the bit sequence in the remainder of
// the mantissa is only valid for the size of the exponent, and is left-
// aligned, so a bit sequence like:
//
//    00.00.90:03
//
// Is more recognizably expressed (in a 32-bit int) as 0xFFFFFFF9
// and is therefore -7
//
//

#include "Float65.h"


/****************************************************************************\
|* Constructor
\****************************************************************************/
Float65::Float65()
		:_val({0,0,0,0})
	{
	}

/****************************************************************************\
|* Constructor
\****************************************************************************/
Float65::Float65(String hex)
		:_val({0,0,0,0})
	{
	set(hex);
	}

/****************************************************************************\
|* Constructor
\****************************************************************************/
Float65::Float65(float f)
		:_val({0,0,0,0})
	{
	set(f);
	}

/****************************************************************************\
|* Convert from hex format to the internal bit-format
\****************************************************************************/
void Float65::set(String hex)
	{
	int x, h, m, l;
	
	sscanf(hex.c_str(), "%02x%02x%02x%02x", &l, &m, &h, &x);
	
	_val.b[0] = l & 0xFF;
	_val.b[1] = m & 0xFF;
	_val.b[2] = h & 0xFF;
	_val.b[3] = x & 0xFF;
	}

/****************************************************************************\
|* Convert from a float to the internal bit-format
\****************************************************************************/
void Float65::set(float f)
	{
	bool minus = false;
	if (f < 0)
		{
		minus = true;
		f = -f;
		}
	
	/************************************************************************\
	|* Integral part
	\************************************************************************/
	uint64_t ival = (uint64_t)f;
	std::vector<bool> ones;
	
	while (ival > 0)
		{
		uint64_t quotient = ival / 2;
		if (quotient * 2 == ival)
			ones.push_back(false);
		else
			ones.push_back(true);
		ival = quotient;
		}
		
	std::reverse(ones.begin(), ones.end());
	
	int mantissa = static_cast<int>(ones.size());

	/************************************************************************\
	|* Fractional part
	\************************************************************************/
	float fraction = f - (uint64_t)f;
	if (fraction != 0)
		{
		for (int i=0; i<23; i++)
			{
			float newFrac = 2 * fraction;
			ones.push_back(newFrac < 1 ? false : true);
			
			if (newFrac == 1)
				break;
			fraction = newFrac - (int)newFrac;
			}
		}

	/************************************************************************\
	|* Convert to integral representation
	\************************************************************************/
	uint32_t fval = 0; // can't be larger since we only have 23 bits
	int idx = 0;
	for (bool bval : ones)
		{
		if (idx > 0)
			fval <<= 1;
		if (bval)
			fval += 1;
		idx ++;
		if (idx == 24)
			break;
		}
	_val.l = fval;
	
	/************************************************************************\
	|* Normalise
	\************************************************************************/
	uint32_t pre 	= 			  	_val.b[2];
	pre 			= (pre << 8)  | _val.b[1];
	pre 			= (pre << 8)  | _val.b[0];
			
	while ((pre & 0x400000) == 0)
		pre = pre << 1;
		
	/************************************************************************\
	|* Deal with minus
	\************************************************************************/
	if (minus)
		{
		pre = ~pre;
		pre ++;
		pre |= 0x800000;
		}
		
	_val.b[0] = (pre      ) & 0xff;
	_val.b[1] = (pre >> 8 ) & 0xff;
	_val.b[2] = (pre >> 16) & 0xff;
	_val.b[3] = mantissa    & 0xff;
	
	}

/****************************************************************************\
|* Return a string representation
\****************************************************************************/
String Float65::toString(void)
	{
	char buf[32];
	snprintf(buf, 32, "%02x.%02x.%02x:%02x\n",
		_val.b[0], _val.b[1], _val.b[2], _val.b[3]);
	return String(buf);
	}

/****************************************************************************\
|* Return a float representation
\****************************************************************************/
float Float65::toFloat(void)
	{
	float frac = 0.5f;
	float sum  = 0.0f;

	uint32_t pre 	= 			  	_val.b[2];
	pre 			= (pre << 8)  | _val.b[1];
	pre 			= (pre << 8)  | _val.b[0];
	bool minus		= ((pre & 0x800000) == 0) ? false : true;

	if (minus)
		{
		pre = ~pre;
		pre ++;
		}

	for (int i=23; i>=0; i--)
		{
		if ((pre & (1<<i)) != 0)
			sum += frac;
		frac /= 2;
		}
	
	for (int i=0; i<_val.b[3]+1; i++)
		sum *= 2;
	
	if (minus)
		sum = - sum;
		
	return sum;
	}
	

//
//  ASTNode.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "ASTNode.h"


typedef struct
	{
	int type;				// Type of token
	String name;			// Human-readable name
	} ASTInfo;

static ASTInfo _astNodes[] =
	{
		{ASTNode::A_ADD, 			"AST_ADD        "},
		{ASTNode::A_SUBTRACT, 		"AST_SUBTRACT   "},
		{ASTNode::A_MULTIPLY, 		"AST_MULTIPLY   "},
		{ASTNode::A_DIVIDE, 		"AST_DIVIDE     "},
		{ASTNode::A_INTLIT, 		"AST_INT_LITERAL"},
		{ASTNode::A_NONE, 			"[None]         "}		// Must be last
	};

static std::map<int, ASTInfo> _info;

/*****************************************************************************\
|* Constructor: generic
\*****************************************************************************/
ASTNode::ASTNode(int op, ASTNode *left, ASTNode *mid, ASTNode *right, int iVal)
		:_op(op)
		,_left(left)
		,_mid(mid)
		,_right(right)
	{
	_value.intValue = iVal;		// Note this sets identifier also
	}

/*****************************************************************************\
|* Constructor : leaf node
\*****************************************************************************/
ASTNode::ASTNode(int op, int intValue)
		:_op(op)
		,_left(nullptr)
		,_mid(nullptr)
		,_right(nullptr)
	{
	_value.intValue = intValue;		// Note this sets identifier also
	}

/*****************************************************************************\
|* Constructor : unary node
\*****************************************************************************/
ASTNode::ASTNode(int op, ASTNode *left, int intValue)
		:_op(op)
		,_left(left)
		,_mid(nullptr)
		,_right(nullptr)
	{
	_value.intValue = intValue;		// Note this sets identifier also
	}


/*****************************************************************************\
|* Destructor
\*****************************************************************************/
ASTNode::~ASTNode(void)
	{
	if (_left)
		delete _left;
	if (_right)
		delete _right;
	delete this;
	}


/*****************************************************************************\
|* Return a string representation of the token
\*****************************************************************************/
String ASTNode::toString(void)
	{
	/*************************************************************************\
	|* Populate the map if it isn't already holding data
	\*************************************************************************/
	if (_info.size() == 0)
		{
		int i = 0;
		forever
			{
			_info[_astNodes[i].type] = _astNodes[i];
			if (_astNodes[i].type == A_NONE)
				break;
			i++;
			}
		}
		
	/*************************************************************************\
	|* Return the human-readable string
	\*************************************************************************/
	auto it = _info.find(_op);
	if (it == _info.end())
		{
		WARN("Cannot find AST node %d in the info map", _op);
		it = _info.find(A_NONE);
		}
	return it->second.name;
	}

/*****************************************************************************\
|* Interpret the AST node using left->right traversal of the tree
\*****************************************************************************/
int ASTNode::interpret(void)
	{
	int lVal = (_left)  ? _left->interpret()  : 0;
	int rVal = (_right) ? _right->interpret() : 0;
	
	if (_op == A_INTLIT)
		DBG_DEFAULT("-> int %d", _value.intValue);
	else
		DBG_DEFAULT("%d %s %d", lVal, toString().c_str(), rVal);
	
	int result = 0;
	switch (_op)
		{
		case A_ADD:
			result = lVal + rVal;
			break;
			
		case A_SUBTRACT:
			result = lVal - rVal;
			break;
		
		case A_MULTIPLY:
			result = lVal * rVal;
			break;
		
		case A_DIVIDE:
			result = lVal / rVal;
			break;
		
		case A_INTLIT:
			result = _value.intValue;
			break;
		
		default:
			FATAL(ERR_AST_UNKNOWN_OPERATOR, "Unknown AST operator %d", _op);
		}
		
	return result;
	}

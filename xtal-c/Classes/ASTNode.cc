//
//  ASTNode.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "ASTNode.h"
#include "SymbolTable.h"


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
ASTNode::ASTNode(int op,
				 int type,
				 ASTNode *left,
				 ASTNode *mid,
				 ASTNode *right,
				 int iVal)
		:_op(op)
		,_left(left)
		,_mid(mid)
		,_right(right)
		,_type(type)
		,_isRValue(false)
	{
	_value.intValue = iVal;		// Note this sets identifier also
	}

/*****************************************************************************\
|* Constructor : leaf node
\*****************************************************************************/
ASTNode::ASTNode(int op, int type, int intValue)
		:_op(op)
		,_left(nullptr)
		,_mid(nullptr)
		,_right(nullptr)
		,_type(type)
		,_isRValue(false)
	{
	_value.intValue = intValue;		// Note this sets identifier also
	}

/*****************************************************************************\
|* Constructor : unary node
\*****************************************************************************/
ASTNode::ASTNode(int op, int type, ASTNode *left, int intValue)
		:_op(op)
		,_left(left)
		,_mid(nullptr)
		,_right(nullptr)
		,_type(type)
		,_isRValue(false)
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

static int _genDumpLabel(void)
	{
	static int label = 1;
	return (label++);
	}
	
/*****************************************************************************\
|* dump the AST tree
\*****************************************************************************/
void ASTNode::dump(ASTNode *node, int label, int level)
	{
	int lFalse, lStart, lEnd;			// Labels
	
	if (node == nullptr)
		node = this;
		
	switch (node->op())
		{
		case A_IF:
			lFalse = _genDumpLabel();
			for (int i=0; i<level; i++)
				printf(" ");
			printf("A_IF");
			
			if (node->right())
				{
				lEnd = _genDumpLabel();
				printf(", end L%d", lEnd);
				}
			printf("\n");
			dump(node->left(), lFalse, level+2);
			dump(node->mid(), 0, level+2);
			if (node->right())
				dump(node->right(), 0, level+2);
			return;
		
		case A_WHILE:
			lStart = _genDumpLabel();
			for (int i=0; i<level; i++)
				printf(" ");
			printf("A_WHILE, start L%d\n", lStart);
			lEnd = _genDumpLabel();
			dump(node->left(), lEnd, level+2);
			dump(node->right(), 0, level+2);
			return;
		}
	
	// If this is a GLUE op, bump us back a level
	if (node->op() == A_GLUE)
		level -= 2;

	
	// General AST node handling
	if (node->left())
		dump(node->left(), 0, level+2);
	if (node->right())
		dump(node->right(), 0, level+2);
	
	
	for (int i=0; i < level; i++)
		printf(" ");
  
	switch (node->op())
		{
		case A_ASSIGN:
			printf("A_ASSIGN\n");
			return;
		case A_LOGOR:
			printf("A_LOGOR\n");
			return;
		case A_LOGAND:
			printf("A_LOGAND\n");
			return;
		case A_OR:
			printf("A_OR\n");
			return;
		case A_XOR:
			printf("A_XOR\n");
			return;
		case A_AND:
			printf("A_AND\n");
			return;
		case A_EQ:
			printf("A_EQ\n");
			return;
		case A_NE:
			printf("A_NE\n");
			return;
		case A_LT:
			printf("A_LE\n");
			return;
		case A_GT:
			printf("A_GT\n");
			return;
		case A_LE:
			printf("A_LE\n");
			return;
		case A_GE:
			printf("A_GE\n");
			return;
		case A_LSHIFT:
			printf("A_LSHIFT\n");
			return;
		case A_RSHIFT:
			printf("A_RSHIFT\n");
			return;
		case A_ADD:
			printf("A_ADD\n");
			return;
		case A_SUBTRACT:
			printf("A_SUBTRACT\n");
			return;
		case A_MULTIPLY:
			printf("A_MULTIPLY\n");
			return;
		case A_DIVIDE:
			printf("A_DIVIDE\n");
			return;
		case A_INTLIT:
			printf("A_INTLIT %d\n", node->value().intValue);
			return;
		case A_STRLIT:
			printf("A_STRLIT\n");
			return;
		case A_IDENT:
			{
			Symbol s = SYMTAB->table()[node->value().identifier];
			if (node->isRValue())
				printf("A_IDENT rval %s\n", s.name().c_str());
			else
				printf("A_IDENT %s\n", s.name().c_str());
			return;
			}
		case A_PRINT:
			{
			Symbol s = SYMTAB->table()[node->value().identifier];
			printf("A_PRINT %s\n", s.name().c_str());
			return;
			}
		case A_GLUE:
			printf("\n\n");
			return;
		case A_FUNCTION:
			{
			Symbol s = SYMTAB->table()[node->value().identifier];
			printf("A_FUNCTION %s\n", s.name().c_str());
			return;
			}
		case A_WIDEN:
			printf("A_WIDEN\n");
			return;
		case A_RETURN:
			printf("A_RETURN\n");
			return;
		case A_FUNCCALL:
			{
			Symbol s = SYMTAB->table()[node->value().identifier];
			printf("A_FUNCCALL %s\n", s.name().c_str());
			return;
			}
		case A_DEREF:
			if (node->isRValue())
				printf("A_DEREF rval\n");
			else
				printf("A_DEREF\n");
			return;
		case A_ADDR:
			{
			Symbol s = SYMTAB->table()[node->value().identifier];
			printf("A_ADDR %s\n", s.name().c_str());
			return;
			}
		case A_SCALE:
			printf("A_SCALE %d\n", node->value().size);
			return;
		case A_PREINC:
			printf("A_PREINC\n");
			return;
		case A_PREDEC:
			printf("A_PREDEC\n");
			return;
		case A_POSTINC:
			printf("A_POSTINC\n");
			return;
		case A_POSTDEC:
			printf("A_POSTDEC\n");
			return;
		case A_NEGATE:
			printf("A_NEGATE\n");
			return;
		case A_INVERT:
			printf("A_INVERT\n");
			return;
		case A_LOGNOT:
			printf("A_LOGNOT\n");
			return;
		case A_TOBOOL:
			printf("A_TOBOOL\n");
			return;
			
			
		default:
			FATAL(ERR_PARSE, "Unknown dumpAST operator %d", node->op());
		}
	}

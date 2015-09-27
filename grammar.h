#ifndef __GRAMMAR__H_
#define __GRAMMAR__H_

#include "lex.h"
#include <set>
#include <map>

/*
	const				: 'const' | '__const'; \\\n\
	operator			: '+' | '-' | '*' | '/' | '=' | '<' | '>' | '+=' | '-=' | '*=' | '/=' | '<<' | '>>' | '<<=' | '>>=' | '==' | '!=' | '<=' | '>=' | '++' | '--' | '%' | '&' | '^' \\\n\
						| '!' | '|' | '~' | '&=' | '^=' | '|=' | '&&' | '||' | '%=' | '(' ')' | '[' ']' | '->' | 'new' | 'new' '[' ']' | 'delete' | 'delete' '[' ']' | ',' | extended_type;  \\\n\
	scope               : ?['::'] *[ ?['template'] token ?[ < *[(extended_type_var|func_type|expr), '>,'] > ] '::'] ; \\\n\
	hard_scope          : ?['::'] +[ ?['template'] token ?[ < *[(extended_type_var|func_type|expr), '>,'] > ] '::'] ; \\\n\
	token_with_namespace: scope token ?[ < *[(extended_type_var|func_type|expr), '>,'] > ]; \\\n\
	twn_nocheck         : scope token ?[ < *[(extended_type_var|func_type|expr), '>,'] > ]; \\\n\
	user_def_type		: scope ?['template'] token ?[ < *[(extended_type_var|func_type|expr), '>,'] > ]; \\\n\
  user_def_type_no_check: scope ?['template'] token ?[ < *[(extended_type_var|func_type|expr), '>,'] > ]; \\\n\
	basic_type			: 'void' | 'va_list' | '__builtin_va_list' | +[('signed' | 'unsigned' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double' | '__int128' | 'bool' | 'wchar_t' | '__w64' | '__int64')]; \\\n\
	class_struct_union  : 'union' | 'struct' | 'class' | 'enum' ; \\\n\
	type				: *[data_type_modifier] ( basic_type | (class_struct_union | 'typename') user_def_type_no_check | user_def_type | '__typeof' '(' twn_nocheck ')' ) *[data_type_modifier] ?[ *[?[const] '*'] ?[const] ?['&'] hard_scope '*'] ;\\\n\
	type_nocheck		: *[data_type_modifier] ( basic_type | ?[(class_struct_union | 'typename')] user_def_type_no_check ) *[data_type_modifier] ?[ *[?[const] '*'] ?[const] ?['&'] hard_scope '*'] ;\\\n\
	extended_type		: type *[?[const] '*' ?['__ptr64'] ] *[data_type_modifier] ?['&']; \\\n\
	extended_type_var   : extended_type *['[' ?[expr] ']']; \\\n\
   extended_or_func_type: extended_type | func_type; \\\n\
	flow_modifier		: 'flow_root' | 'flow'; \\\n\
	decl_var			: *[?[const] '*' ?['__ptr64']] ?[const] ?['__restrict'] ?['('] ?[('&' | '*')] ?[&V token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']']; \\\n\
	decl_var2   		: *[?[const] '*' ?['__ptr64']] ?[const] ?['__restrict'] ?['('] ?[('&' | '*')] ?[&V token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']']; \\\n\
	attribute			: '__attribute__' '(' '(' *[any_token] ')' ')'; \\\n\
	declspec			: '__declspec' '(' *[any_token] ')'; \\\n\
	common_modifier		: declspec | attribute | 'static' | 'inline' | '__inline' | '__forceinline' ; \\\n\
	ext_modifier		: '__extension__' | 'extern' | 'extern' '\"C\"' | 'extern' '\"C++\"' | common_modifier; \\\n\
	func_modifier		: ext_modifier | 'virtual' | 'explicit' | 'friend' | flow_modifier ; \\\n\
	func_modifier2		: common_modifier | '__restrict' | '_cdecl' | '__cdecl' | '__thiscall' | '__stdcall' | '__fastcall' ; \\\n\
	member_modifier		: ext_modifier | 'mutable' | 'static' ; \\\n\
    data_type_modifier  : const | 'volatile' | '__complex__' ; \\\n\
    enum_items           : *[ ?[&V token ?['=' expr ] ], ','] ; \\\n\
	enum_def			: 'enum' ?[token] ^O '{' %[enum_items, '}'] '}'; \\\n\
	union_def			: 'union' ?[token] ^O '{' %[defs, '}'] '}'; \\\n\
	class_access_modifier: 'public' | 'protected' | 'private' ; \\\n\
	class_def_body		: ^O class_access_modifier ':' \\\n\
						| 'friend' ?[class_struct_union] user_def_type ^O ';' \\\n\
						| defs \\\n\
						; \\\n\
	base_class_defs		: +[ ?['virtual'] ?[class_access_modifier] ?['virtual'] user_def_type, '>,'] ; \\\n\
	class_def			: ('class' | 'struct') *[common_modifier] scope ?[token] ?[ ':' %[base_class_defs, '{'] ] ^O '{' %[class_def_body, '}'] '}' ; \\\n\
	super_type			: enum_def | union_def | class_def ; \\\n\
    builtin_type_funcs  : '__alignof__' | '__is_abstract' | '__is_class' | '__is_empty' | '__is_enum' | '__is_pod' | '__has_nothrow_assign' | '__has_nothrow_copy' | '__has_trivial_assign' | '__has_trivial_copy' | '__has_trivial_destructor' ; \\\n\
	expr				:@1 expr '.' (?['~'] token | 'operator' operator) |@1 expr '->' (?['~'] token | 'operator' operator) \\\n\
						|@2 expr '(' *[expr, ','] ')' |@2 scope 'operator' operator '(' ?[expr] ')' |@2 expr '[' expr ']' \\\n\
						|@3 expr '++' |@3 expr '--' |@3 '++' expr |@3 '--' expr |@3 '+' expr |@3 '-' expr |@3 '!' expr |@3 '~' expr |@3 '(' extended_or_func_type ')' expr \\\n\
						|@3 '*' expr |@3 '&' expr |@3 'sizeof' '(' (extended_type_var | func_type | expr) ')' \\\n\
                        |@3 scope 'new' extended_type_var |@3 scope 'new' user_def_type '(' expr2 ')' |@3 scope 'new' '(' expr ')' ?[ user_def_type ?[ '(' expr2 ')' ] ] \\\n\
                        |@3 scope 'delete' ?['[' ']'] expr \\\n\
						|@5 expr '*' expr |@5 expr '/' expr |@5 expr '%' expr \\\n\
						|@6 expr '+' expr |@6 expr '-' expr |@7 expr '<<' expr |@7 expr '>>' expr \\\n\
						|@8 expr '<' expr |@8 expr '<=' expr |@8 expr '>' expr |@8 expr '>=' expr \\\n\
						|@9 expr '==' expr |@9 expr '!=' expr \\\n\
						|@10 expr '&' expr	|@11 expr '^' expr |@12 expr '|' expr |@13 expr '&&' expr |@14 expr '||' expr  \\\n\
						|@15 expr '?' expr ':' expr |@15 expr '=' expr |@15 expr '+=' expr |@15 expr '-=' expr |@15 expr '*=' expr |@15 expr '/=' expr |@15 expr '%=' expr |@15 expr '<<=' expr |@15 expr '>>=' expr |@15 expr '&=' expr |@15 expr '^=' expr |@15 expr '|=' expr \\\n\
						|@16 'throw' ?[expr] | const_value | ?[('__real__' | '__imag__')] token_with_namespace | type '(' expr2 ')' | '(' expr2 ')' \\\n\
						| builtin_type_funcs '(' type ')' | '__is_base_of' '(' type ',' type ')' \\\n\
						| 'const_cast' '<' extended_or_func_type '>' '(' expr ')' | 'static_cast' '<' extended_or_func_type '>' '(' expr ')' | 'dynamic_cast' '<' extended_or_func_type '>' '(' expr ')' | 'reinterpret_cast' '<' extended_or_func_type '>' '(' expr ')' | '__extension__' expr \\\n\
						; \\\n\
	expr2			    : *[expr, ',']; \\\n\
	decl_obj_var		: token '(' +[expr, ','] ')' ; \\\n\
	decl_c_var		    : ?['__restrict'] decl_var ?[ '=' expr] ; \\\n\
	decl_c_obj_var	    : decl_c_var | decl_obj_var ; \\\n\
    expr_or_decl_var    : expr2 | type +[decl_c_obj_var, ','] ; \\\n\
	func_param          : type_nocheck *[func_modifier2] ?[const] ?[ decl_var ?[ '=' %[expr, ',|)'] ] ?[attribute] ] | func_type ;  \\\n\
	func_params         : *[func_param, '>,'] ?[('...' | ',' '...')] ;  \\\n\
	func_type           : extended_type ?( ?[ ?( *[func_modifier2] scope +['*'] ?[token] ) ] '(' func_params ')' *[data_type_modifier] ) ; \\\n\
	func_header         : *[func_modifier] ?[extended_type] *[func_modifier2] ?( ?( *[func_modifier2] scope ( ?['~'] token | 'operator' operator) ?['<' *[(extended_type_var|func_type|expr), ','] '>'] ) '(' %[func_params, ')'] ')' ) *[data_type_modifier] ?['throw' '(' ?[(type | '...')] ')'] ; \\\n\
    class_base_init     : user_def_type_no_check '(' expr2 ')' ; \\\n\
    class_base_inits    : *[class_base_init, ','] ; \\\n\
	template_type_defs  : +[template_type_def, '>,'] ; \\\n\
	template_type_def   : ?['template' '<' +[template_type_def, '>,'] '>'] ('class' | 'typename') ?[&T token ?['=' (?['typename' ^N] extended_type_var | func_type )] ] | type ?[&V token] ?['=' expr]; \\\n\
    template_body       : func_header (';' | ?[ ':' %[class_base_inits, '{'] ] ^O '{' %[statement, '}'] '}' ) \\\n\
						| class_struct_union ?[attribute] scope ^O token ?[ < +[(extended_type_var | func_type | expr), '>,'] > ] ?[ ?[ ':' %[base_class_defs, '{'] ] ^O '{' %[class_def_body, '}'] '}' ] ?[attribute] ';' \\\n\
						| *[member_modifier] func_type *['[' expr ']'] ^O ';' \\\n\
                        | ^O 'friend' class_struct_union scope token ';' \\\n\
                        | *[member_modifier] ^O extended_type scope %[decl_c_obj_var, ';'] ';' \\\n\
                        ; \\\n\
	defs				: ^O ';' \\\n\
						| ^O '__pragma' '(' *[any_token] ')' ';' \\\n\
						| ^O '#' any_token \\\n\
                        | class_struct_union *[common_modifier] token ^O ';' \\\n\
						| ^O 'using' ?['namespace'] scope (token | 'operator' operator) ';' \\\n\
						| *[ext_modifier] ^O 'typedef' (type +[decl_var, ','] ?[attribute] ^O ';' | \\\n\
                                                           ^O super_type extra_block | \\\n\
                                                           extended_type '(' scope '*' token ')' ^O ';' | \\\n\
														   extended_type ?( *[func_modifier2] token ) '(' func_params ')' ^O ';' | \\\n\
                                                           func_type ^O ';' | \\\n\
                                                           ^O '__typeof__' '(' (extended_type | expr) ')' token ';' ) \\\n\
						| *[member_modifier] type def_var_tail \\\n\
						| *[member_modifier] super_type extra_block \\\n\
						| func_header ?['__asm' '(' const_value ')'] *[attribute] (?['=' '0'] ^O ';' | ?[ ':' %[class_base_inits, '{'] ] ^O '{' %[statement, '}'] '}') \\\n\
						| *[member_modifier] func_type *['[' expr ']'] ^O ';' \\\n\
						| ?[ext_modifier] +['template' < %[template_type_defs] > ] ( ^O class_struct_union extra_block2 | extra_block ) \\\n\
                        | ext_modifier 'template' (('class' | 'struct') user_def_type | func_header ) ';' \\\n\
						| ext_modifier 'template' extended_type scope token < *[(extended_type_var | func_type | expr), '>,'] > '(' %[func_params, ')'] ')' ?[const] ';' \\\n\
						; \\\n\
	def_var_tail        : *[decl_c_obj_var, ','] ?[attribute] ?['__asm' '(' const_value ')'] ';' ; \\\n\
	switch_body			: 'case' const_value ':' *[statement] | 'default' ':' *[statement] ; \\\n\
	else_statement		: 'else' statement ;\\\n\
	catch_statement		: 'catch' '(' func_params ')' statement ; \\\n\
   flow_catch_statement : 'flow_catch' '(' expr ',' expr ')' statement ; \\\n\
	statement			: ^O 'break' ';'						\\\n\
						| ^O 'continue' ';'						\\\n\
						| ^O 'return' ?[expr] ';'			\\\n\
						| ^O '{' %[statement, '}'] '}'			 \\\n\
						| ^O 'if' '(' expr ')' statement \\\n\
						| ^O 'while' '(' expr ')' statement \\\n\
						| ^O 'do' statement \\\n\
						| ^O 'for' '(' ?[expr_or_decl_var] ';' ?[expr] ';' expr2 ')' statement	\\\n\
						| ^O 'switch' '(' expr ')' '{' %[switch_body, '}'] '}' \\\n\
						| ^O 'try' statement \\\n\
						| ^O 'flow_wait' '(' expr ',' expr ')' ';' \\\n\
						| ^O 'flow_fork' statement \\\n\
						| ^O 'flow_try' statement \\\n\
						| ^O '__asm' '{' %[token, '}'] '}' \\\n\
                        | expr2 ';' \\\n\
						| defs \\\n\
						; \\\n\
	start				: 'extern' ('\"C\"' | '\"C++\"') ^O '{' %[start, '}'] '}' \\\n\
						| ?['inline'] ^O 'namespace' ?[token] ?[attribute] '{' %[start, '}'] '}' \\\n\
						| defs \\\n\
						; \\\n\
*/

struct SourceTreeNode {
	SourceTreeNode*	pChild;
	SourceTreeNode*	pNext;
	int 			param;
	std::string		value;
	std::string 	file_name; // only valid for start_root
	int 			line_no;
	void*			ptr;
};

typedef std::vector<SourceTreeNode*>	SourceTreeVector;

#define MODBIT___EXTENSION__    0x1
#define MODBIT_EXTERN           0x2
#define MODBIT_EXTERN_C         0x4
#define MODBIT_EXTERN_CPP       0x8
#define MODBIT_VIRTUAL          0x10
#define MODBIT_STATIC           0x20
#define MODBIT_MUTABLE          0x40
#define MODBIT_EXPLICIT         0x80
#define MODBIT_CONST            0x100
#define MODBIT_INLINE           0x200
#define MODBIT_FLOW             0x400
#define MODBIT_FLOW_ROOT        0x800
#define MODBIT___RESTRICT       0x1000
#define MODBIT_VOLATILE         0x2000
#define MODBIT_FRIEND           0x4000
#define MODBIT___COMPLEX__      0x8000
#define MODBIT___INLINE		    0x10000
#define MODBIT___THISCALL	    0x20000
#define MODBIT__CDECL		    0x40000
#define MODBIT___CDECL		    0x80000
#define MODBIT_DECLSPEC			0x100000
#define MODBIT___STDCALL	    0x200000
#define MODBIT___FASTCALL	    0x400000
#define MODBIT___FORCEINLINE    0x800000

enum BasicTypeBasicType
{
    BASICTYPE_BASICTYPE_SIGNED,
    BASICTYPE_BASICTYPE_UNSIGNED,
    BASICTYPE_BASICTYPE_CHAR,
    BASICTYPE_BASICTYPE_SHORT,
    BASICTYPE_BASICTYPE_INT,
    BASICTYPE_BASICTYPE_LONG,
    BASICTYPE_BASICTYPE_FLOAT,
    BASICTYPE_BASICTYPE_DOUBLE,
    BASICTYPE_BASICTYPE_BOOL,
    BASICTYPE_BASICTYPE_WCHAR_T
};

enum BasicTypeType
{
    BASICTYPE_TYPE_BASIC,
    BASICTYPE_TYPE_USER_DEF,
    BASICTYPE_TYPE_TYPEOF,
    BASICTYPE_TYPE_DATA_MEMBER_POINTER,
};

enum CSUType
{
    CSU_TYPE_NONE,
    CSU_TYPE_ENUM,
    CSU_TYPE_CLASS,
    CSU_TYPE_STRUCT,
    CSU_TYPE_UNION,
};

enum ClassAccessModifierType
{
    CAM_TYPE_NONE,
    CAM_TYPE_PUBLIC,
    CAM_TYPE_PROTECTED,
    CAM_TYPE_PRIVATE,
};

enum ClassDefBodyType
{
    CDB_TYPE_CAM,
    CDB_TYPE_FRIEND,
    CDB_TYPE_DEFS,
    //CDB_TYPE_FUNC_DEF,
};

enum SuperTypeType
{
	SUPERTYPE_TYPE_TYPE,
	SUPERTYPE_TYPE_ENUM_DEF,
	SUPERTYPE_TYPE_UNION_DEF,
	SUPERTYPE_TYPE_CLASS_DEF // or struct def
};

enum DeclVarModifierType
{
    DVMOD_TYPE_POINTER,
    DVMOD_TYPE_CONST_POINTER,
    DVMOD_TYPE_CONST,
    DVMOD_TYPE_REFERENCE,
    DVMOD_TYPE_INTERNAL_POINTER,
    DVMOD_TYPE_BIT,
    DVMOD_TYPE_PARENTHESIS,
    DVMOD_TYPE_BRACKET
};

enum FuncParamType
{
    FUNC_PARAM_TYPE_REGULAR,    // type: type, name: decl_var, value: expr
    FUNC_PARAM_TYPE_FUNC,       // type: func_type, name: token
    //FUNC_PARAM_TYPE_VARGS
};

enum ExprType
{
	EXPR_TYPE_REF_ELEMENT = 0,	// expr ['~'] token ['(' expr2 ')']
	EXPR_TYPE_PTR_ELEMENT,		// expr ['~'] token ['(' expr2 ')']
	EXPR_TYPE_FUNC_CALL,		// expr *[expr, ',']
    EXPR_TYPE_OPERATOR_CALL,    // scope operator ?[expr]
	EXPR_TYPE_ARRAY,			// expr expr
	EXPR_TYPE_RIGHT_INC, 		// expr
	EXPR_TYPE_RIGHT_DEC,		// expr
	EXPR_TYPE_LEFT_INC,			// expr
	EXPR_TYPE_LEFT_DEC,			// expr
	EXPR_TYPE_POSITIVE,			// expr
	EXPR_TYPE_NEGATIVE = 10,	// expr
	EXPR_TYPE_NOT,  			// expr
	EXPR_TYPE_BIT_NOT,			// expr
	EXPR_TYPE_TYPE_CAST,		// extended_or_func_type expr
	EXPR_TYPE_INDIRECTION,		// expr
	EXPR_TYPE_ADDRESS_OF,		// expr
	EXPR_TYPE_SIZEOF,			// (extended_type_var | func_type | expr)
	EXPR_TYPE_NEW_C,	    	// scope extended_type_var
    EXPR_TYPE_NEW_OBJECT,       // scope expr2
    EXPR_TYPE_NEW_ADV,          // scope expr, user_def_type, expr2
	EXPR_TYPE_DELETE = 20,		// scope expr
	EXPR_TYPE_MULTIPLE,	        // expr expr
	EXPR_TYPE_DIVIDE,   		// expr expr
	EXPR_TYPE_REMAINDER,	    // expr expr
	EXPR_TYPE_ADD,				// expr expr
	EXPR_TYPE_SUBTRACT,			// expr expr
	EXPR_TYPE_LEFT_SHIFT,		// expr expr
	EXPR_TYPE_RIGHT_SHIFT,		// expr expr
	EXPR_TYPE_LESS_THAN,		// expr expr
	EXPR_TYPE_LESS_EQUAL,		// expr expr
	EXPR_TYPE_GREATER_THAN = 30,// expr expr
	EXPR_TYPE_GREATER_EQUAL,    // expr expr
	EXPR_TYPE_EQUAL,		    // expr expr
	EXPR_TYPE_NOT_EQUAL,		// expr expr
	EXPR_TYPE_BIT_AND,			// expr expr
	EXPR_TYPE_BIT_XOR,			// expr expr
	EXPR_TYPE_BIT_OR,			// expr expr
	EXPR_TYPE_AND,				// expr expr
	EXPR_TYPE_OR,				// expr expr
	EXPR_TYPE_TERNARY,			// expr expr expr
	EXPR_TYPE_ASSIGN = 40,      // expr expr
	EXPR_TYPE_ADD_ASSIGN,	    // expr expr
	EXPR_TYPE_SUBTRACT_ASSIGN,	// expr expr
	EXPR_TYPE_MULTIPLE_ASSIGN,	// expr expr
	EXPR_TYPE_DIVIDE_ASSIGN,	// expr expr
	EXPR_TYPE_REMAINDER_ASSIGN,	// expr expr
	EXPR_TYPE_LEFT_SHIFT_ASSIGN,	// expr expr
	EXPR_TYPE_RIGHT_SHIFT_ASSIGN,	// expr expr
	EXPR_TYPE_BIT_AND_ASSIGN,		// expr expr
	EXPR_TYPE_BIT_XOR_ASSIGN,		// expr expr
	EXPR_TYPE_BIT_OR_ASSIGN = 50,	// expr expr
	EXPR_TYPE_THROW,			    // expr
	EXPR_TYPE_CONST_VALUE,			// const_value
	EXPR_TYPE_TOKEN_WITH_NAMESPACE,	// token
    EXPR_TYPE_TYPE_CONSTRUCT,       // type, expr2
	EXPR_TYPE_PARENTHESIS,		    // expr
    EXPR_TYPE_BUILTIN_TYPE_FUNC,    // type
    EXPR_TYPE_IS_BASE_OF,			// type, type
    EXPR_TYPE_CONST_CAST,           // extended_or_func_type, expr
    EXPR_TYPE_STATIC_CAST,          // extended_or_func_type, expr
    EXPR_TYPE_DYNAMIC_CAST = 60,         // extended_or_func_type, expr
    EXPR_TYPE_REINTERPRET_CAST,     // extended_or_func_type, expr
    EXPR_TYPE_EXTENSION,            // expr
	// -----------------not used in actual grammar below-------------
	EXPR_TYPE_COMMA,                // used in expr2
    EXPR_TYPE_OBJECT,               // user_def_type *[expr, ',']
};

enum DefType
{
    DEF_TYPE_EMPTY,
	DEF_TYPE_PRE_DECL,
    DEF_TYPE_USING_NAMESPACE,
	DEF_TYPE_TYPEDEF,
    DEF_TYPE_VAR_DEF, // we put var_def before func_decl because func_header in func_decl won't check type in func_params
    DEF_TYPE_SUPER_TYPE_VAR_DEF,
	DEF_TYPE_FUNC_DECL,
	DEF_TYPE_FUNC_VAR_DEF,
	DEF_TYPE_TEMPLATE,
    DEF_TYPE_EXTERN_TEMPLATE_CLASS,
    DEF_TYPE_EXTERN_TEMPLATE_FUNC,
	DEF_TYPE_CLASS_CAM, // class use only
    DEF_TYPE_CLASS_FRIEND, // class use only
	DEF_TYPE___PRAGMA,
	DEF_TYPE_POUND_LINE, // #
};

enum TypeDefType
{
	TYPEDEF_TYPE_DATA,
	TYPEDEF_TYPE_SUPER_TYPE,
    TYPEDEF_TYPE_DATA_MEMBER_PTR,
	TYPEDEF_TYPE_FUNC,
	TYPEDEF_TYPE_FUNC_PTR,
    TYPEDEF_TYPE_TYPEOF,
};

enum TemplateType
{
    TEMPLATE_TYPE_FUNC,
    TEMPLATE_TYPE_CLASS,
    TEMPLATE_TYPE_VAR,
    TEMPLATE_TYPE_FUNC_VAR,
    TEMPLATE_TYPE_FRIEND_CLASS,
};

enum TemplateParamType
{
    TEMPLATE_PARAM_TYPE_DATA,
    TEMPLATE_PARAM_TYPE_FUNC,
    TEMPLATE_PARAM_TYPE_VALUE,
};

enum StatementType
{
	STATEMENT_TYPE_DEF,
	STATEMENT_TYPE_EXPR2,
	STATEMENT_TYPE_IF,
	STATEMENT_TYPE_WHILE,
	STATEMENT_TYPE_DO,
	STATEMENT_TYPE_FOR,
	STATEMENT_TYPE_SWITCH,
	STATEMENT_TYPE_TRY,
	STATEMENT_TYPE_RETURN,
	STATEMENT_TYPE_BREAK,
    STATEMENT_TYPE_CONTINUE,
	STATEMENT_TYPE_COMPOUND,
	STATEMENT_TYPE_FLOW_WAIT,
    STATEMENT_TYPE_FLOW_TRY,
	STATEMENT_TYPE_FLOW_FORK,
	STATEMENT_TYPE_FLOW_NEW,
	STATEMENT_TYPE___ASM,
};

enum BlockType
{
	BLOCK_TYPE_EXTERN_BLOCK,
    BLOCK_TYPE_NAMESPACE,
	BLOCK_TYPE_DEF,
	//BLOCK_TYPE_FUNC_DEF,
};

struct SourceTreeNodeWithType
{
    int type;
    SourceTreeNode* pNode;
};

std::string printTabs(int depth);
void printSourceTree(const SourceTreeNode* pSourceNode);
void deleteSourceTreeNode(SourceTreeNode* pRoot, int depth = 0, int idx = 0);
SourceTreeNode* dupSourceTreeNode(const SourceTreeNode* pSourceNode);
//void dumpToFile(FILE* fp, SourceTreeNode* pRoot);
SourceTreeNode* createEmptyNode();

/*struct FuncParam {
	FuncParamType param_type;
	SourceTreeNode* paramType;	// ExtendedType, func_type, ...
	SourceTreeNode* paramName;	// DeclVar,      token_node
	SourceTreeNode* paramValue;
};
//typedef std::vector<FuncParam> FuncParamVector;*/

class TokenWithNamespace
{
protected:
    struct ScopeInfo {
        ScopeInfo() { bHasTemplate = bSpecifiedAsTemplate = false; }
        std::string token;
        bool bHasTemplate; // has '<>' pair or not
        bool bSpecifiedAsTemplate; // has 'template'
        std::vector<SourceTreeNodeWithType> template_params;
    };
public:
    TokenWithNamespace() : m_bHasCopy(false), m_bHasRootSign(false)
    {
    }

    virtual ~TokenWithNamespace()
    {
        if (m_bHasCopy)
        {
            for (unsigned i = 0; i < m_data.size(); i++)
            {
                for (unsigned j = 0; j < m_data[i].template_params.size(); j++)
                {
                    deleteSourceTreeNode(m_data[i].template_params[j].pNode);
                }
            }
        }
    }

    bool empty() { return !m_bHasRootSign && m_data.empty(); }

    bool hasRootSign() const { return m_bHasRootSign; }
    void setHasRootSign(bool b) { m_bHasRootSign = b; }

    void addScope(const std::string& token, bool bHasTemplate = false, bool bSpecifiedAsTemplate = false)
    {
        ScopeInfo info;
        info.token = token;
        info.bHasTemplate = bHasTemplate;
        info.bSpecifiedAsTemplate = bSpecifiedAsTemplate;

        m_data.push_back(info);
    }

    void addTemplateParam(int bType, SourceTreeNode* pNode)
    {
        MY_ASSERT(m_data.size() > 0);
        MY_ASSERT(m_data.back().bHasTemplate);

        SourceTreeNodeWithType nwt;
        nwt.type = bType;
        nwt.pNode = pNode;

        m_data.back().template_params.push_back(nwt);
    }

    void resize(unsigned depth)
    {
        MY_ASSERT(depth <= m_data.size());

        if (m_bHasCopy)
        {
            for (unsigned i = depth; i < m_data.size(); i++)
            {
                for (unsigned j = 0; j < m_data[i].template_params.size(); j++)
                {
                    deleteSourceTreeNode(m_data[i].template_params[j].pNode);
                }
            }
        }
        m_data.resize(depth);
    }

    int getDepth() const { return m_data.size(); }
    std::string getToken(int depth) const { return m_data[depth].token; }
    std::string getLastToken() const { return m_data.back().token; }
    void setTokenName(int depth, const std::string& s) { m_data[depth].token = s; }

    bool scopeSpecifiedAsTemplate(unsigned depth) const
    {
        MY_ASSERT(depth >= 0 && depth <= m_data.size());
        return m_data.at(depth).bSpecifiedAsTemplate;
    }
    bool scopeHasTemplate(unsigned depth) const
    {
        MY_ASSERT(depth >= 0 && depth <= m_data.size());
        return m_data.at(depth).bHasTemplate;
    }
    int getTemplateParamCount(unsigned depth) const
    {
        MY_ASSERT(depth >= 0 && depth <= m_data.size());
        return (int)m_data.at(depth).template_params.size();
    }
    int getTemplateParamAt(unsigned depth, unsigned idx, SourceTreeNode*& pChild) const // return true means it's a type node
    {
        MY_ASSERT(depth >= 0 && depth <= m_data.size());
        MY_ASSERT(idx >= 0 && idx < m_data.at(depth).template_params.size());
        const SourceTreeNodeWithType& nwt = m_data.at(depth).template_params.at(idx);
        pChild = nwt.pNode;
        return nwt.type;
    }
    void clearTemplateParams(unsigned depth)
    {
        MY_ASSERT(depth >= 0 && depth <= m_data.size());
        m_data[depth].bHasTemplate = false;
        m_data[depth].template_params.clear();
    }

    std::string toString() const;

    void copyFrom(const TokenWithNamespace& other)
    {
        m_bHasRootSign = other.m_bHasRootSign;

        for (unsigned i = 0; i < other.m_data.size(); i++)
        {
            const ScopeInfo& src = other.m_data.at(i);
            ScopeInfo dest;
            dest.token = src.token;
            dest.bHasTemplate = src.bHasTemplate;
            dest.bSpecifiedAsTemplate = src.bSpecifiedAsTemplate;

            for (unsigned j = 0; j < src.template_params.size(); j++)
            {
                SourceTreeNodeWithType nwt;
                nwt.type = src.template_params[j].type;
                nwt.pNode = dupSourceTreeNode(src.template_params[j].pNode);
                dest.template_params.push_back(nwt);
            }
            m_data.push_back(dest);
        }

        m_bHasCopy = true;
    }

protected:
    bool m_bHasCopy;
    bool m_bHasRootSign;
    std::vector<ScopeInfo>  m_data;
};

std::string modifierBit2String(int mod);
std::string extModifierGetBit(const SourceTreeNode* pRoot);
std::string funcModifierGetBit(const SourceTreeNode* pRoot);
std::string funcModifier2GetBit(const SourceTreeNode* pRoot);
std::string memberModifierGetBit(const SourceTreeNode* pRoot);
std::string dataTypeModifierGetBit(const SourceTreeNode* pRoot);

bool isInModifiers(const StringVector& mod_strings, int mod);
void addToModifiers(StringVector& mod_strings, int mod);
void removeFromModifiers(StringVector& mod_strings, int mod);

std::string combineStrings(const StringVector& strs);

CSUType csuGetType(const SourceTreeNode* pRoot);
std::string operatorGetString(const SourceTreeNode* pRoot);
std::string templateTypeToString(TemplateType t);

std::string tokenGetValue(const SourceTreeNode* pRoot);
StringVector blockDataGetTokens(void* param);

TokenWithNamespace scopeGetInfo(const SourceTreeNode* pRoot);
SourceTreeNode* scopeCreate(const TokenWithNamespace& twn);
void scopeChangeTokenName(SourceTreeNode* pRoot, int idx, const std::string& newName);

TokenWithNamespace tokenWithNamespaceGetInfo(const SourceTreeNode* pRoot);
void tokenWithNamespaceChangeTokenName(SourceTreeNode* pRoot, int idx, const std::string& newName);
SourceTreeNode* tokenWithNamespaceCreate(const std::string& token);
SourceTreeNode* tokenWithNamespaceCreate(const TokenWithNamespace& twn);

TokenWithNamespace userDefTypeGetInfo(const SourceTreeNode* pRoot);
SourceTreeNode* userDefTypeCreateByName(const TokenWithNamespace& twn);
void userDefTypeChangeTokenName(SourceTreeNode* pRoot, int idx, const std::string& newName);

std::vector<std::string> basicTypeGetInfo(const SourceTreeNode* pRoot);
SourceTreeNode* basicTypeCreate(BasicTypeBasicType basic_type);
SourceTreeNode* basicTypeCreateVoid();
// union also call this
//SourceTreeVector basicTypeStructGetDefs(const SourceTreeNode* pRoot);
//std::map<std::string, int> basicTypeEnumGetItems(const SourceTreeNode* pRoot);

void typeGetModifierBits(const SourceTreeNode* pRoot, StringVector& mod_strings, StringVector& mod2_strings);
BasicTypeType typeGetType(const SourceTreeNode* pRoot);
SourceTreeNode* typeBasicGetInfo(const SourceTreeNode* pRoot);
CSUType typeUserDefinedGetInfo(const SourceTreeNode* pRoot, bool& bHasTypename, SourceTreeNode*& pUserDefNode);
SourceTreeNode* typeTypeOfGetInfo(const SourceTreeNode* pRoot);
void typeDmpGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedTypeNode, TokenWithNamespace& twn);
SourceTreeNode* typeCreateByBasic(SourceTreeNode* pRoot);
SourceTreeNode* typeCreateByUserDefined(CSUType csu_type, SourceTreeNode* pRoot);
// the last token of twn is not considered in the scope
SourceTreeNode* typeCreateDmp(SourceTreeNode* pExtendedTypeNode, TokenWithNamespace& twn);
//SourceTreeNode* typeCreateStruct(const std::string& name);
//void typeStructAddMember(SourceTreeNode* pRoot, SourceTreeNode* pDef);

SourceTreeNode* extendedTypeGetTypeNode(const SourceTreeNode* pRoot);
int extendedTypeGetDepth(const SourceTreeNode* pRoot);
bool extendedTypePointerIsConst(const SourceTreeNode* pRoot, int depth);
bool extendedTypePointerIsPtr64(const SourceTreeNode* pRoot, int depth);
StringVector extendedTypeGetModStrings(const SourceTreeNode* pRoot);
bool extendedTypeIsReference(const SourceTreeNode* pRoot);
SourceTreeNode* extendedTypeCreateFromType(SourceTreeNode* pRoot, SourceTreeNode* pDeclVar = NULL);
void extendedTypeAddModifier(SourceTreeNode* pRoot, DeclVarModifierType mod);

void extendedTypeVarGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedTypeNode, int& depth);
SourceTreeNode* extendedTypeVarCreateFromExtendedType(SourceTreeNode* pTypeNode);

void extendedOrFuncTypeGetInfo(const SourceTreeNode* pRoot, bool& bExtendedType, SourceTreeNode*& pChild);

void enumDefGetInfo(const SourceTreeNode* pRoot, std::string& name, void*& bracket_block);
int enumItemsGetCount(const SourceTreeNode* pRoot);
void enumItemsGetAt(const SourceTreeNode* pRoot, int idx, std::string& key, SourceTreeNode*& pExpr);

void unionDefGetInfo(const SourceTreeNode* pRoot, std::string& name, void*& bracket_block);

ClassAccessModifierType camGetType(const SourceTreeNode* pRoot);
ClassDefBodyType cdbGetType(const SourceTreeNode* pRoot);
ClassAccessModifierType cdbCamGetType(const SourceTreeNode* pRoot);
void cdbFriendGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, SourceTreeNode*& pUserDefTypeNode);
SourceTreeNode* cdbDefGetNode(const SourceTreeNode* pRoot);
//void cdbFuncDefGetInfo(const SourceTreeNode* pRoot, int& func_modifier, SourceTreeNode*& pFuncHeaderNode, int& memberInitCount, void*& bracket_block);
//void cdbFuncDefGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr);

int baseClassDefsGetCount(const SourceTreeNode* pRoot); //		: ':' +[ ?[class_access_modifier] user_def_type, ','] ;
void baseClassDefsGetChildByIndex(const SourceTreeNode* pRoot, int idx, bool& bVirtual, ClassAccessModifierType& cam_type, SourceTreeNode*& pUserDefTypeNode); //		: ':' +[ ?[class_access_modifier] user_def_type, ','] ;

void classDefGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, StringVector& mod_strings, TokenWithNamespace& name, void*& pBaseClassDefsBlock, void*& bracket_block);

void superTypeGetInfo(const SourceTreeNode* pRoot, SuperTypeType& super_type, SourceTreeNode*& pChildNode);

void funcParamGetInfo(const SourceTreeNode* pRoot, FuncParamType& param_type, StringVector& mod_strings, SourceTreeNode*& pTypeNode, SourceTreeNode*& pDeclVarNode, void*& pInitBlockData);
SourceTreeVector funcParamsGetList(const SourceTreeNode* pRoot);
bool funcParamsHasVArgs(const SourceTreeNode* pRoot);
void funcTypeGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pReturnExtendedType, StringVector& mod_strings, SourceTreeNode*& pScope, int& nDepth, std::string& name, SourceTreeNode*& pFuncParamsNode, StringVector& mod2_strings);

struct GrammarFuncHeaderInfo {
	StringVector mod_strings;
	SourceTreeNode* pReturnExtendedType;
	StringVector mod2_strings;
	StringVector mod3_strings;
	TokenWithNamespace scope;
	//int nDataMemberPointerDepth;
	std::string name;
	bool bEmptyTemplate;
	void* params_block;
	StringVector mod4_strings;
	int bThrow; // 0: no throw, 1: throw empty, 2: throw type, 3: throw ...
	SourceTreeNode* pThrowTypeNode;
};
GrammarFuncHeaderInfo funcHeaderGetInfo(const SourceTreeNode* pRoot);

TokenWithNamespace declVarGetName(const SourceTreeNode* pRoot);
int declVarGetDepth(const SourceTreeNode* pRoot);
int declVarGetPointerCount(const SourceTreeNode* pRoot);
std::string declVarGetBitsValue(const SourceTreeNode* pRoot);
bool declVarIsConst(const SourceTreeNode* pRoot);
bool declVarIsReference(const SourceTreeNode* pRoot);
bool declVarHasInternalPointer(const SourceTreeNode* pRoot);
bool declVarHasParenthesis(const SourceTreeNode* pRoot);
bool declVarPointerIsConst(const SourceTreeNode* pRoot, int depth); // is the layer of the given depth a pointer or a const pointer?
bool declVarPointerIsPtr64(const SourceTreeNode* pRoot, int depth);
int declVarGetBits(const SourceTreeNode* pRoot); // return -1 if not defined
SourceTreeVector declVarGetExprs(const SourceTreeNode* pRoot);
SourceTreeNode* declVarCreateByName(const std::string& new_name);
void declVarAddModifier(SourceTreeNode* pRoot, DeclVarModifierType mode, SourceTreeNode* pExtNode = NULL);
void declVarRemoveModifier(SourceTreeNode* pRoot, DeclVarModifierType mode);
void declVarAddArrayExpr(SourceTreeNode* pRoot, SourceTreeNode* pExprNode);
void declVarChangeName(SourceTreeNode* pRoot, const std::string& new_name);
//void declVarSetReference(SourceTreeNode* pRoot);
SourceTreeNode* declVarCreateFromExtendedType(const std::string& name, const SourceTreeNode* pRoot);

void declCVarGetInfo(const SourceTreeNode* pRoot, bool& bRestrict, SourceTreeNode*& pVar, SourceTreeNode*& pInitValue); // pVar: decl_var, pInitValue: expr
void declObjVarGetInfo(const SourceTreeNode* pRoot, std::string& name, SourceTreeVector& exprList);
bool declCObjVarGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pChild); // return true if it's a c var decl

ExprType exprGetType(const SourceTreeNode* pRoot);
SourceTreeVector exprGetExprList(const SourceTreeNode* pRoot); // for func call and object, new object
std::string exprConstGetValue(const SourceTreeNode* pRoot);
SourceTreeNode* exprGetFirstNode(const SourceTreeNode* pRoot);
SourceTreeNode* exprGetSecondNode(const SourceTreeNode* pRoot);
std::string exprGetSecondToken(const SourceTreeNode* pRoot);
SourceTreeNode* exprGetThirdNode(const SourceTreeNode* pRoot);
SourceTreeNode* exprGetOptionalThirdNode(const SourceTreeNode* pRoot);
std::string exprBuiltinFuncGetName(const SourceTreeNode* pRoot);
void exprSizeOfGetInfo(const SourceTreeNode* pRoot, int& nType, SourceTreeNode*& pChild);
void exprNewAdvGetParams(const SourceTreeNode* pRoot, SourceTreeNode*& pUserDefType, SourceTreeNode*& pExpr2);
bool exprDeleteHasArray(const SourceTreeNode* pRoot);
void exprPtrRefGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr, SourceTreeNode*& pScope, std::string& token);

SourceTreeNode* exprCreateConst(ExprType expr_type, const std::string val);
SourceTreeNode* exprCreateRefOrPtr(SourceTreeNode* pLeft, ExprType expr_type, const std::string token);
SourceTreeNode* exprCreateDoubleOperator(SourceTreeNode* pLeft, ExprType expr_type, SourceTreeNode* pRight);

SourceTreeVector expr2GetExprs(const SourceTreeNode* pRoot);
void exprOrDeclVarGetInfo(const SourceTreeNode* pRoot, bool& bIsExpr, SourceTreeNode*& pChild, SourceTreeVector& var_v);

void classBaseInitGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pUserDefTypeOrMember, SourceTreeNode*& pExpr);
SourceTreeVector classBaseInitsGetList(const SourceTreeNode* pRoot);

StatementType statementGetType(const SourceTreeNode* pRoot);
SourceTreeNode* statementExpr2GetNode(const SourceTreeNode* pRoot);
SourceTreeNode* statementReturnGetExpr(const SourceTreeNode* pRoot);
void* statementCompoundGetBracketBlock(const SourceTreeNode* pRoot);
void statementIfGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pExprOrDeclVar, SourceTreeNode*& pStatement);
void statementWhileGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pExprOrDeclVar, SourceTreeNode*& pStatement);
void statementDoGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pStatement, SourceTreeNode*& pExpr);
void statementForGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr1, SourceTreeNode*& pExpr2, SourceTreeNode*& pExpr3, SourceTreeNode*& pStatement);
SourceTreeNode* statementSwitchGetVar(const SourceTreeNode* pRoot);
int statementSwitchGetNumOfCases(const SourceTreeNode* pRoot);
SourceTreeVector statementSwitchGetCaseByIndex(const SourceTreeNode* pRoot, int idx, SourceTreeNode*& pExpr);
SourceTreeVector statementSwitchGetDefult(const SourceTreeNode* pRoot);
SourceTreeNode* statementTryGetStatement(const SourceTreeNode* pRoot);
//int statementTryGetNumOfCatches(const SourceTreeNode* pRoot);
void catchStatementGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pFuncParamsNode, SourceTreeNode*& Statement);
void statementFlowWaitGetExprs(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr1, SourceTreeNode*& pExpr2);
//void statementFlowSignalGetExprs(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr1, SourceTreeNode*& pExpr2);
SourceTreeNode* statementFlowForkGetStatement(const SourceTreeNode* pRoot);
void statementFlowNewGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExprNode, SourceTreeNode*& pStatementNode);
SourceTreeNode* statementFlowTryGetTryStatement(const SourceTreeNode* pRoot);
//int statementFlowTryGetCatchCount(const SourceTreeNode* pRoot);
SourceTreeNode* flowCatchStatementGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& expr1, SourceTreeNode*& expr2); // return child statement
StringVector statementAsmGetInfo(const SourceTreeNode* pRoot);
SourceTreeNode* statementDefGetNode(const SourceTreeNode* pRoot);

DefType defGetType(const SourceTreeNode* pRoot);
std::string defPragmaGetInfo(const SourceTreeNode* pRoot);
std::string defPoundLineGetInfo(const SourceTreeNode* pRoot);
std::string defPreDeclGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, CSUType& csu_type);
TokenWithNamespace defUsingNamespaceGetInfo(const SourceTreeNode* pRoot, bool& bHasNamespace);
//std::string defStructUnionDefGetName(const SourceTreeNode* pRoot);
//SourceTreeVector defStructUnionDefGetDefs(const SourceTreeNode* pRoot);
//SourceTreeVector defStructUnionDefGetDeclVars(const SourceTreeNode* pRoot);
// if typedefType==DATA, pType: type, pVar: decl_var, if typedefType==FUNC, pType: extended_type, pVar: func_params, if typedefType==FUNC_PTR, pType: func_type
TypeDefType defTypedefGetBasicInfo(const SourceTreeNode* pRoot, StringVector& mod_strings);
void defTypedefDataGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pSuperType, SourceTreeVector& declVarList, SourceTreeNode*& pAttribute);
void defTypedefSuperTypeGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pSuperType, void*& bracket_block);
void defTypedefDataMemberPtrGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedTypeNode, TokenWithNamespace& twn, std::string& name);
void defTypedefFuncGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedType, bool& bHasParenthesis, StringVector& mod_strings, std::string& name, SourceTreeNode*& pFuncParamsNode);
SourceTreeNode* defTypedefFuncTypeGetInfo(const SourceTreeNode* pRoot);
void defTypedefTypeOfGetInfo(const SourceTreeNode* pRoot, bool& bType, SourceTreeNode*& pExtendedTypeOrExprNode, std::string& name);
void defVarTailGetInfo(const SourceTreeNode* pRoot, SourceTreeVector& declCObjVarList, StringVector& mod_strings);

bool defFuncDeclIsFuncDef(const SourceTreeNode* pRoot);
void defFuncDeclGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pFuncHeaderNode, std::string& asm_string, SourceTreeVector& attribute_list, bool& bPureVirtual, void*& pClassBaseInitBlock, void*& bracket_block);
//void defFuncDefGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr);
void defFuncVarDefGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, SourceTreeNode*& pFuncType, int& array_count);
//void defFuncVarDefChangeVarName(SourceTreeNode* pRoot, const std::string& old_name, const std::string& new_name);
void defVarDefGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, SourceTreeNode*& pType, SourceTreeNode*& pDefVarTailNode);
void defSuperTypeVarDefGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, SourceTreeNode*& pSuperType, void*& bracket_block);
//void defVarDefChangeVarName(SourceTreeNode* pRoot, const std::string& old_name, const std::string& new_name);
//SourceTreeNode* defVarDefCreate(SourceTreeNode* pSuperType, SourceTreeNode* pDeclVar, SourceTreeNode* pInitExpr = NULL);
void defTemplateGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, std::vector<void*>& header_types, CSUType& csu_type, void*& bracket_block);

SourceTreeVector templateTypeDefsGetList(const SourceTreeNode* pRoot);
//header_type == 0: data, 1: expr, when header_type==0, pTypeNode is in templateTypeParams
void templateTypeDefGetInfo(const SourceTreeNode* pRoot, int& header_type, SourceTreeVector& templateTypeParams, bool& bClass, std::string& name, bool& bDataOrFuncType, bool& bHasTypename, SourceTreeNode*& pDefaultNode);
TemplateType templateBodyGetType(const SourceTreeNode* pRoot);
void templateBodyFuncGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pFuncHeaderNode, void*& pClassBaseInitBlock, void*& body_data);
void templateBodyClassGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, TokenWithNamespace& twn, int& specializedTypeCount, void*& pBaseClassDefsBlock, void*& body_data);
void templateBodyVarGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, SourceTreeNode*& pExtendedTypeNode, SourceTreeNode*& pScopeNode, void*& block_data);
void templateBodyFuncVarGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, SourceTreeNode*& pFuncType, SourceTreeVector& expr_list);
void templateBodyFriendClassGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, SourceTreeNode*& pScopeNode, std::string& className);
// type: 0: typename, 1: expr
// for type=0, returns name, bHasTypename, pTypeNode as template_class_def node and pDefaultNode as default extended type var node
// for type=1, returns pTypeNode, name and pDefaultNode as default expr node
void templateBodyClassGetSpecializedTypeByIndex(const SourceTreeNode* pRoot, int idx, TemplateParamType& nParamType, SourceTreeNode*& pChildNode); // 0: extendedTypeVar, 1: func_type, 2: expr
void templateBodyFuncGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr);

void defExternTemplateClassGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, bool& bClass, CSUType& csu_type, SourceTreeNode*& pUserDefTypeOrFuncHeader);
void defExternTemplateFuncGetInfo(const SourceTreeNode* pRoot, StringVector& mod_strings, SourceTreeNode*& pExtendedReturnType, SourceTreeNode*& pScope, std::string& token, int& templateParamCount, void*& pFuncParamsBlock, bool& bConst);
void defExternTemplateFuncGetParamByIndex(const SourceTreeNode* pRoot, int idx, TemplateParamType& nParamType, SourceTreeNode*& pChildNode);

BlockType blockGetType(const SourceTreeNode* pRoot);
void blockExternGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, void*& bracket_block);
void blockNamespaceGetInfo(const SourceTreeNode* pRoot, bool& bInline, std::string& name, SourceTreeNode*& pAttribute, void*& bracket_block);
SourceTreeNode* blockDefGetNode(const SourceTreeNode* pRoot);
//void blockFuncGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, SourceTreeNode*& pFuncHeaderNode, int& memberInitCount, void*& bracket_block);
//void blockFuncGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr);

SourceTreeVector bracketBlockAnalyze(void* pBlock);

//=============================================================================================

std::string displaySourceTreeConst(const SourceTreeNode* pRoot);
std::string displaySourceTreeTokenWithNamespace(const SourceTreeNode* pRoot);
std::string displaySourceTreeBasicType(const SourceTreeNode* pRoot);
std::string displayCSUType(CSUType csu_type);
std::string displayCAMType(ClassAccessModifierType cam_type);
std::string displayPrefixModifiers(int modifier, const std::string& declspec_strings);
std::string displayFuncModifiers2(int modifier);
std::string displaySourceTreeUserDefType(const SourceTreeNode* pNode);
std::string displaySourceTreeType(const SourceTreeNode* pNode, int depth = 0);
//std::string displaySourceTreeEnumDef(const SourceTreeNode* pNode, int depth = 0);
std::string displaySourceTreeUnionDef(const SourceTreeNode* pNode, int depth = 0);
std::string displaySourceTreeClassDefBody(const SourceTreeNode* pNode, int depth = 0);
std::string displaySourceTreeClassDef(const SourceTreeNode* pNode, int depth = 0);
//std::string displaySourceTreeSuperType(const SourceTreeNode* pNode, int depth = 0);
std::string displaySourceTreeExtendedType(const SourceTreeNode* pRoot, int depth = 0);
std::string displaySourceTreeExtendedTypeVar(const SourceTreeNode* pRoot);
std::string displaySourceTreeExtendedOrFuncType(const SourceTreeNode* pRoot);
std::string displaySourceTreeFlow(const SourceTreeNode* pNode, int depth = 0);
// name == "" means no replacement, name == " " means replace to ""
std::string displaySourceTreeDeclVar(const SourceTreeNode* pRoot, const std::string& name = "", std::vector<std::string> str_v = std::vector<std::string>());
std::string displaySourceTreeDeclCVar(const SourceTreeNode* pRoot);
std::string displaySourceTreeDeclObjVar(const SourceTreeNode* pRoot);
std::string displaySourceTreeAttribute(const SourceTreeNode* pNode);
std::string displaySourceTreeExtModifier(const SourceTreeNode* pRoot);
std::string displaySourceTreeFuncParam(const SourceTreeNode* pNode);
std::string displaySourceTreeFuncParams(const SourceTreeNode* pNode);
std::string displaySourceTreeFuncType(const SourceTreeNode* pRoot, bool bSkipFuncName = false);
std::string displaySourceTreeFuncModifier(const SourceTreeNode* pRoot);
std::string displaySourceTreeFuncHeader(const SourceTreeNode* pRoot);
std::string displaySourceTreeVarModifier(const SourceTreeNode* pRoot);
std::string displaySourceTreeEnumDef(const SourceTreeNode* pRoot);
std::string displaySourceTreeUnionDef(const SourceTreeNode* pRoot);
std::string displaySourceTreeClassAccessModifer(const SourceTreeNode* pRoot);
std::string displaySourceTreeClassDefBody(const SourceTreeNode* pRoot);
std::string displaySourceTreeClassDef(const SourceTreeNode* pRoot);
std::string displaySourceTreeExpr(const SourceTreeNode* pNode);
std::string displaySourceTreeExpr2(const SourceTreeNode* pNode);
std::string displaySourceTreeDefs(const SourceTreeNode* pRoot, int depth);
std::string displaySourceTreeStatement(const SourceTreeNode* pNode, int depth, bool bIfForWhile = false);
std::string displaySourceTreeStart(const SourceTreeNode* pRoot, int depth);

#define SOURCE_NODE_TYPE_CONST_VALUE	1000001
#define SOURCE_NODE_TYPE_TOKEN			1000002
#define SOURCE_NODE_TYPE_IDENTIFIER		1000004
#define SOURCE_NODE_TYPE_SMALL_BRACKET	1000005
#define SOURCE_NODE_TYPE_BIG_BRACKET	1000006

typedef std::map<std::string, int>  GrammarTempDefMap; // int ==1: type, ==2: var
typedef bool (*GrammarCheckFunc)(void* context, int mode, const SourceTreeNode* pNode, const GrammarTempDefMap& tempTypeMap);

typedef bool (*GrammarCallback)(void* context, int mode, std::string& s);

class CGrammarAnalyzer;

class CGrammarCacheMapNode
{
public:
	CGrammarCacheMapNode();
	virtual ~CGrammarCacheMapNode();

	void add(CGrammarAnalyzer* pAnalyzer, int start, int end, const std::string& rule, int ret, SourceTreeNode* pRoot, int err_n, const std::string& err_s, int real_n = 0);
	// return -1, not found(not sure), >=0 sure, pRoot = NULL, failed, != NULL succeeded
	int find(CGrammarAnalyzer* pAnalyzer, int start, int end, const std::string& rule, SourceTreeNode*& pRoot, int& ret, int& err_n, std::string& err_s, int steps = 0);

protected:
	struct ResultNode {
		SourceTreeNode* pSourceNode;
		int				ret;
		int				err_n;
		std::string		err_s;
	};
	typedef std::map<std::string, CGrammarCacheMapNode*>	CGrammarCacheMapChildrenMap;
	typedef std::map<std::string, ResultNode>	GrammarRuleResultMap;

	CGrammarCacheMapChildrenMap	children;
	GrammarRuleResultMap	result_map;
};

class CGrammarAnalyzer
{
public:
	CGrammarAnalyzer();
	virtual ~CGrammarAnalyzer();

	void initWithFile(void* context, const char* file_name, int argc, char* argv[]);
    void initWithBuffer(void* context, const char* file_name, const char* buffer);
	void initWithBlocks(void* context, void* param);
    void initWithTokens(void* context, const std::string& grammar, const StringVector& tokens, const std::string& delim = "");

	void pushTokenFront(const std::string& s);
	std::string nextToken();

	int findEndOfStatement(int n, int end_n, bool bForceSemiColon = false);
	SourceTreeNode* getBlock(StringVector* pBlockData = NULL, const std::string grammar = "", void* context = NULL);
	bool isEmpty();

	static StringVector bracketBlockGetTokens(void* param);

	bool onLexerCallback(int mode, std::string& s);

	struct GrammarTreeNode {
		std::string	name;
		std::string param;
		short priority;
		short attrib;
		GrammarTreeNode*	parent;
		GrammarTreeNode*	prev;
		GrammarTreeNode*	next;
		GrammarTreeNode*	children;
	};

	struct BracketBlock {
		GrammarTreeNode*	pGrammarNode;
		std::string         delim;
		StringVector		tokens;
		StringVector		file_stack;
		int					file_line_no;
	};

protected:
	friend CGrammarCacheMapNode;
	friend SourceTreeNode* dupSourceTreeNode(const SourceTreeNode* pSourceNode);

	typedef std::map<std::string, GrammarTreeNode*> GrammarMap;

	struct GrammarFileBufferedKeyword
	{
		std::string file_name;
		int line_no;
		std::string keyword;
	};

	struct GrammarFile {
		std::string		cur_file_name;
		std::string		next_keyword;
        std::string     next_file_name;
        int             next_line_no;
		std::vector<GrammarFileBufferedKeyword>	buffered_keywords;
	};

    struct PatternResultCache {
	    int n, err_n;
	    int flags;
	    GrammarTempDefMap tempDefMap;
	    SourceTreeNode* pSourceNode;
	    std::string err_s;
	};
    typedef std::map<std::string, PatternResultCache> PatternResultCacheMap;

	void addKeywords();
	bool isKeyword(const std::string& s);
	std::string readOneGrammarRule(CLexer& grammar_lexer, GrammarTreeNode*& root, std::string& end_s, short& priority, short& attrib, bool& bConst);
	void printGrammarRules2(GrammarTreeNode* pRoot);
	void printGrammarRules();
	void loadGrammarRules();
	std::string grammar_read_word(int& n, int end_n = -1, bool bSkipComments = true);
	// err_n returns the n which it failed exactly, err_s returns the failed reason
	// when end_n > 0, bNoMoreGrammar indicates whether there's any more grammar after pGrammarEnd
	int findPairEnd(int n, int end_n);
	void set_error(int err_n, const std::string& err_s);
	int AnalyzeGrammar(std::vector<std::string> analyze_path, GrammarTreeNode* pGrammar, GrammarTreeNode* pGrammarEnd,
			int n, int end_n, bool bNoMoreGrammar, GrammarTempDefMap& tempTypeMap, SourceTreeNode* pBlockRoot, SourceTreeNode* pSourceParent, int& flags);
	int AnalyzeGrammar2(std::vector<std::string> analyze_path, GrammarTreeNode* pGrammar, GrammarTreeNode* pGrammarEnd,
			int n, int end_n, bool bNoMoreGrammar, GrammarTempDefMap& tempTypeMap, SourceTreeNode* pBlockRoot, SourceTreeNode* pSourceParent, int& flags);
	void skipGrammarComments(int& n, int end_n);
	bool postIdentifierHandling(const std::string& grammarName, SourceTreeNode* pRoot);

	friend void deleteSourceTreeNode(SourceTreeNode* pRoot, int depth, int idx);

	static GrammarMap 				s_grammar_map;
	static std::set<std::string> 	s_keywords;
	CLexer 					m_srcfile_lexer;
	void*                   m_context;
    GrammarTreeNode*        m_grammar_root;
    std::string             m_grammar_delim;
    GrammarFile             m_gf;
    PatternResultCacheMap   m_pattern_result_cache_map;
    int                     m_err_n;
    std::string             m_err_s;
	int						m_end_n;

	void init();
};

void grammarLogDisable();
void grammarLogEnable();
void grammarSetCheckFunc(GrammarCheckFunc checkFunc, GrammarCallback callback);

#endif // __GRAMMAR__H_

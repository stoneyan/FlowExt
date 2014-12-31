#include "grammar.h"

/* struct XXX* ptr; // it works even when XXX is not defined.
 * struct AAA {
 * 	  unsigned int:8; // either name or bits should be given
 * };
 *
 * template<typename _Tp, bool = std::__is_integer<_Tp>::__value>
 *
 * normal function in a template:
 * template<typename _CharT, typename _Traits, typename _Alloc>
 *   void
 *   basic_string<_CharT, _Traits, _Alloc>::_Rep::
 *   _M_destroy(const _Alloc& __a) throw ()
 *
 * template function in a template:
 * template<bool _BoolType>
 *   template<typename _II1, typename _II2>
 *     bool
 *     __lexicographical_compare<_BoolType>::
 *     __lc(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2);
 *
 * we don't want to check type in func_param because we cannot check that in a function header of a template define outside the template
 *
 * &V add as a var
 * &T add as a type
 *
 * '(' % %), ':' % %~{: save to a block which will be analyzed later
 *
 * ^O, the one
 * ^N, don't do type check
 * ^E, search for the first const string in this grammar line and treat it as the end string.
 */
const char* g_grammar_str = "\
	const				: 'const' | '__const'; \\\n\
	operator			: '+' | '-' | '*' | '/' | '=' | '<' | '>' | '+=' | '-=' | '*=' | '/=' | '<<' | '>>' | '<<=' | '>>=' | '==' | '!=' | '<=' | '>=' | '++' | '--' | '%' | '&' | '^' \\\n\
						| '!' | '|' | '~' | '&=' | '^=' | '|=' | '&&' | '||' | '%=' | '(' ')' | '[' ']' | '->' | 'new' | 'new' '[' ']' | 'delete' | 'delete' '[' ']' | ',' | extended_type;  \\\n\
    scope               : ?['::'] *[ ?['template'] token ?[ < *[(extended_type_var|expr), ','] > ] '::'] ; \\\n\
    hard_scope          : ?['::'] +[ ?['template'] token ?[ < *[(extended_type_var|expr), ','] > ] '::'] ; \\\n\
	token_with_namespace: scope token ?[ < *[(extended_type_var|expr), ','] > ]; \\\n\
    twn_nocheck         : scope token ?[ < *[(extended_type_var|expr), ','] > ]; \\\n\
	user_def_type		: scope ?['template'] token ?[ < *[(extended_type_var|expr), ','] > ]; \\\n\
  user_def_type_no_check: scope ?['template'] token ?[ < *[(extended_type_var|expr), ','] > ]; \\\n\
	basic_type			: 'void' | 'va_list' | '__builtin_va_list' | +[('signed' | 'unsigned' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double' | '__int128' | 'bool' | 'wchar_t')]; \\\n\
	class_struct_union  : 'union' | 'struct' | 'class' | 'enum' ; \\\n\
	type				: *[data_type_modifier] ( basic_type | class_struct_union user_def_type_no_check | ?['typename' ^N] user_def_type | '__typeof' '(' twn_nocheck ')' ) *[data_type_modifier] ?[ *[?[const] '*'] ?[const] ?['&'] hard_scope '*'] ;\\\n\
	extended_type		: type *[?[const] '*'] *[data_type_modifier] ?['&']; \\\n\
	extended_type_var   : extended_type *['[' ?[expr] ']']; \\\n\
   extended_or_func_type: extended_type | func_type; \\\n\
	flow_modifier		: 'flow_root' | 'flow'; \\\n\
	decl_var			: *[?[const] '*'] ?[const] ?['__restrict'] ?['('] ?['&'] ?[&V token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']']; \\\n\
	decl_var2   		: *[?[const] '*'] ?[const] ?['__restrict'] ?['('] ?['&'] ?[&V token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']']; \\\n\
	attribute			: ^O '__attribute__' '(' '(' *[(any_token ?[any_token] | any_token '(' *[(any_token | const_value | any_token '=' const_value), ','] ')'), ','] ')' ')'; \\\n\
	ext_modifier		: '__extension__' | 'extern' | 'extern' '\"C\"' | 'extern' '\"C++\"'; \\\n\
	func_modifier		: ext_modifier | 'virtual' | 'static' | 'inline' | 'explicit' | 'friend' | flow_modifier ; \\\n\
	member_modifier		: ext_modifier | 'mutable' | 'static' ; \\\n\
    data_type_modifier  : const | 'volatile' ; \\\n\
	enum_def			: 'enum' ?[token] ^O '{' (*[&V token ?['=' expr ], ','] | *[&V token ?['=' expr] ','] ) '}'; \\\n\
	union_def			: 'union' ?[token] ^O '{' %[defs, '}'] '}'; \\\n\
	class_access_modifier: 'public' | 'protected' | 'private' ; \\\n\
	class_def_body		: ^O class_access_modifier ':' \\\n\
                        | 'friend' class_struct_union user_def_type ^O ';' \\\n\
						| defs \\\n\
						; \\\n\
	base_class_defs		: +[ ?['virtual'] ?[class_access_modifier] user_def_type, ','] ; \\\n\
	class_def			: ('class' | 'struct') ?[attribute] scope ?[token] ?[ ':' %[base_class_defs, '{'] ] ^O '{' %[class_def_body, '}'] '}' ; \\\n\
	super_type			: type | enum_def | union_def | class_def ; \\\n\
    builtin_type_funcs  : '__alignof__' | '__is_abstract' | '__is_class' | '__is_empty' | '__is_pod' | '__has_nothrow_assign' | '__has_nothrow_copy' | '__has_trivial_assign' | '__has_trivial_copy' | '__has_trivial_destructor' ; \\\n\
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
						|@16 'throw' expr | const_value | token_with_namespace | type '(' expr2 ')' | '(' expr2 ')' | builtin_type_funcs '(' type ')'  \\\n\
						| 'const_cast' '<' extended_or_func_type '>' '(' expr ')' | 'static_cast' '<' extended_or_func_type '>' '(' expr ')' | 'dynamic_cast' '<' extended_or_func_type '>' '(' expr ')' | 'reinterpret_cast' '<' extended_or_func_type '>' '(' expr ')' | '__extension__' expr \\\n\
						; \\\n\
	expr2			    : *[expr, ',']; \\\n\
	decl_obj_var		: token '(' +[expr, ','] ')' ; \\\n\
	decl_c_var		    : ?['__restrict'] decl_var ?[ '=' expr] ; \\\n\
	decl_c_obj_var	    : decl_c_var | decl_obj_var ; \\\n\
    func_param          : type ?['__restrict'] ?[const] ?[ decl_var2 ?[ '=' expr ] ?[attribute] ] | func_type ;  \\\n\
    func_params         : *[func_param, ','] ?[('...' | ',' '...')] ;  \\\n\
    func_type           : extended_type ?[ '(' scope '*' ?[token ?[ '(' func_params ')' ] ] ')' ] '(' func_params ')' *[data_type_modifier]; \\\n\
    func_header         : *[func_modifier] ?[attribute] *[func_modifier] ?[extended_type] ?[attribute] ?( scope (*['*'] ?['~'] token | 'operator' operator) ?['<' '>'] '(' %[func_params, ')'] ')' ) *[data_type_modifier] ?['throw' '(' ?[type] ')'] ; \\\n\
    class_base_init     : user_def_type_no_check '(' ?[expr] ')' ; \\\n\
    class_base_inits    : *[class_base_init, ','] ; \\\n\
    template_type_def   : ?['template' '<' +[template_type_def, ','] '>'] ('class' | 'typename') ?[&T token ?['=' ?['typename'] extended_type_var] ] | type ?[&V token] ?['=' expr]; \\\n\
    template_header     : +['template' < *[template_type_def, ','] > ] ; \\\n\
	defs				: ^O ';' \\\n\
                        | ^E class_struct_union token ^O ';' \\\n\
						| ^O 'using' ?['namespace'] scope (token | 'operator' operator) ';' \\\n\
						| *[ext_modifier] ^O 'typedef' ^E (super_type decl_var ?[attribute] | \\\n\
                                                           extended_type '(' scope '*' token ')' | \\\n\
                                                           extended_type token '(' func_params ')' | \\\n\
                                                           func_type | \\\n\
                                                           '__typeof__' '(' (extended_type | expr) ')' token) ';' \\\n\
                        | ^E *[member_modifier] super_type *[decl_c_obj_var, ','] ?[attribute] ?['__asm' '(' const_value ')'] ^O ';' \\\n\
						| func_header ?['__asm' '(' const_value ')'] *[attribute] (?['=' '0'] ^O ';' | ?[ ':' %[class_base_inits, '{'] ] ^O '{' %[statement, '}'] '}') \\\n\
						| ^E *[member_modifier] func_type ^O ';' \\\n\
						| template_header ( ^O func_header (';' | ?[ ':' %[class_base_inits, '{'] ] ^O '{' %[statement, '}'] '}' ) \\\n\
						                  | class_struct_union ^O scope token ?[ < +[(extended_type_var | func_type | expr), ','] > ] ?[ ?[ ':' %[base_class_defs, '{'] ] ^O '{' %[class_def_body, '}'] '}' ] ?[attribute] ';' \\\n\
                                          | *[member_modifier] ^O extended_type scope %[decl_c_obj_var, ';'] ';' \\\n\
                                          | ^O 'friend' class_struct_union scope token ';' ) \\\n\
                        | 'extern' 'template' (('class' | 'struct') user_def_type | func_header ) ';' \\\n\
                        | 'extern' 'template' extended_type scope token < *[(extended_type_var | func_type | expr), ','] > '(' %[func_params, ')'] ')' ?[const] ';' \\\n\
						; \\\n\
	switch_body			: 'case' const_value ':' *[statement] | 'default' ':' *[statement] ; \\\n\
	statement			: ^O 'break' ';'						\\\n\
						| ^O 'continue' ';'						\\\n\
						| ^O 'return' ^E ?[expr] ';'			\\\n\
						| ^O '{' %[statement, '}'] '}'			 \\\n\
						| ^O 'if' '(' expr ')' statement *[ 'else' 'if' '(' expr ')' statement] ?[ 'else' statement ] \\\n\
						| ^O 'while' '(' expr ')' statement \\\n\
						| ^O 'do' statement 'while' '(' expr ')' ';'  \\\n\
						| ^O 'for' '(' ?[(expr2 | type +[decl_c_obj_var, ','])] ';' ^E ?[expr] ';' expr2 ')' statement	\\\n\
						| ^O 'switch' '(' expr ')' '{' %[switch_body, '}'] '}' \\\n\
						| ^O 'try' statement +[ 'catch' '(' func_params ')' statement ] \\\n\
						| ^O 'flow_wait' '(' expr ',' expr ')' ';' \\\n\
						| ^O 'flow_fork' statement \\\n\
						| ^O 'flow_try' statement +[ 'flow_catch' '(' expr ',' expr ')' statement ] \\\n\
                        | ^O '__asm__' '(' const_value ':' const_value '(' expr ')' ')' ';' \\\n\
                        | ^E expr2 ';' \\\n\
						| defs \\\n\
						; \\\n\
	start				: 'extern' ('\"C\"' | '\"C++\"') ^O '{' %[start, '}'] '}' \\\n\
						| ^O 'namespace' ?[token] ?[attribute] '{' %[start, '}'] '}' \\\n\
						| defs \\\n\
						; \\\n\
";

//| *[var_modifier] ^O extended_type token < +[(extended_type | expr), ','] > '::' token ';' )
#define GRAMMAR_ATTRIB_IS_TEMP_TYPE 1
#define GRAMMAR_ATTRIB_IS_TEMP_VAR  2

#define ANALYZE_FLAG_BIT_THE_ONE        1
#define ANALYZE_FLAG_BIT_NO_TYPE_CHECK  2

bool g_grammar_log = false;
bool g_source_tree_log = false;
GrammarCheckFunc g_grammarCheckFunc = NULL;
GrammarCallback g_grammarCallback = NULL;
#define TRACE   if (g_grammar_log) printf

//void onIdentifierResolved(GrammarFile& gf, const std::string& name, SourceTreeNode* pSourceNode, int n);
void printSourceTree(const SourceTreeNode* pSourceNode);

#define GRAMMAR_LOG(x)	{ if (g_grammar_log) printf("%s\n", (x).c_str()); }

void deleteSourceTreeNode(SourceTreeNode* pSourceNode, int depth, int idx)
{
	//printf("deleting depth=%d, idx=%d\n", depth, idx);
	MY_ASSERT(depth < 100);

	if (!pSourceNode)
		return;

	if (pSourceNode->pChild)
		deleteSourceTreeNode(pSourceNode->pChild, depth + 1, 0);

	if (pSourceNode->pNext)
		deleteSourceTreeNode(pSourceNode->pNext, depth, idx + 1);

	if (pSourceNode->param == SOURCE_NODE_TYPE_BIG_BRACKET)
	{
		MY_ASSERT(pSourceNode->ptr);
		delete (CGrammarAnalyzer::BracketBlock*)pSourceNode->ptr;
	}
	//printf("deleting2 depth=%d, idx=%d\n", depth, idx);
	delete pSourceNode;
}

SourceTreeNode* dupSourceTreeNode(const SourceTreeNode* pSourceNode)
{
	if (!pSourceNode)
		return NULL;

	SourceTreeNode* pNode = new SourceTreeNode;
	pNode->param = pSourceNode->param;
	pNode->value = pSourceNode->value;
    if (pNode->param == SOURCE_NODE_TYPE_BIG_BRACKET)
    {
        CGrammarAnalyzer::BracketBlock* pSrcBlock = (CGrammarAnalyzer::BracketBlock*)pSourceNode->ptr;
        CGrammarAnalyzer::BracketBlock* pDestBlock = new CGrammarAnalyzer::BracketBlock;
        *pDestBlock = *pSrcBlock;
        pNode->ptr = pDestBlock;
    }
    else
        pNode->ptr = NULL;
	pNode->pChild = dupSourceTreeNode(pSourceNode->pChild);
	pNode->pNext = dupSourceTreeNode(pSourceNode->pNext);

	return pNode;
}

SourceTreeNode* findChildTail(SourceTreeNode* pSourceParent)
{
	if (pSourceParent->pChild == NULL)
		return NULL;

	pSourceParent = pSourceParent->pChild;
	while (pSourceParent->pNext)
		pSourceParent = pSourceParent->pNext;
	return pSourceParent;
}

void appendToChildTail(SourceTreeNode* pSourceParent, SourceTreeNode* pSourceNode)
{
	if (pSourceParent->pChild == NULL)
		pSourceParent->pChild = pSourceNode;
	else
	{
		pSourceParent = pSourceParent->pChild;
		while (pSourceParent->pNext)
			pSourceParent = pSourceParent->pNext;
		pSourceParent->pNext = pSourceNode;
	}
}

void deleteChildTail(SourceTreeNode* pSourceParent, SourceTreeNode* pSourceNode)
{
	if (pSourceNode == NULL)
	{
		deleteSourceTreeNode(pSourceParent->pChild);
		pSourceParent->pChild = NULL;
	}
	else
	{
		deleteSourceTreeNode(pSourceNode->pNext);
		pSourceNode->pNext = NULL;
	}
}

/*void onIdentifierResolved(GrammarFile& gf, const std::string& name, SourceTreeNode* pRoot, int n)
{
	//printf("Resolve '%s' to ", name.c_str());
	//printSourceTree(pSourceNode);
	//printf("\n");

	if (name == "defs")
	{
		//defs			: ('union' | 'struct' | 'class') token ';' \\\n\
						  | 'namespace' token_with_namespace ';' \\\n\
						  | ?[ext_modifier] 'typedef' ^ (super_type decl_var ?[attribute] | extended_type token '(' ^ func_params ')' | func_type ) ';' \\\n\
						  | *[func_modifier] ?[extended_type] token_with_namespace '(' ^ func_params ')' ?[const] ?['throw' '(' ')'] ?['__asm' '(' const_value ')'] *[attribute] ';' \\\n\
						  | *[var_modifier] func_type ';' \\\n\
						  | ?[ext_modifier] super_type ^ ?[attribute] *[ ?['__restrict'] decl_var ?[ '=' ^ expr], ','] ';'
		std::string s;
		switch (defGetType(pRoot))
		{
		case DEF_TYPE_TYPEDEF:
		{
			// decl_var			: *['*'] ?[const] ?[token] ?[':' const_value] ?['[' ']'] *['[' expr ']'];
			// func_type		   : extended_type '(' '*' ?[token] ')' '(' func_params ')';
			switch (defTypedefGetType(pRoot))
			{
			case TYPEDEF_TYPE_DATA:
			{
				SourceTreeNode* pSuperType, * pDeclVar;
				defTypedefDataGetInfo(pRoot, pSuperType, pDeclVar);
				s = declVarGetName(pDeclVar);
				MY_ASSERT(!s.empty());
				break;
			}
			case TYPEDEF_TYPE_FUNC:
			{
				SourceTreeNode* pExtendedType, *pFuncParamNode;
				defTypedefFuncGetInfo(pRoot, s, pExtendedType, pFuncParamNode);
				MY_ASSERT(!s.empty());
				break;
			}
			case 2:
			{
				SourceTreeNode* pExtendedType, *pFuncParamNode;
				defTypedefFuncPtrGetInfo(pRoot, s, pExtendedType, pFuncParamNode);
				MY_ASSERT(!s.empty());
				break;
			}
			default:
				MY_ASSERT(false);
			}
			break;
		}
		case DEF_TYPE_VAR_DEF:
		{
			SourceTreeNode* pTypeNode = defVarDefGetSuperType(pRoot);
			BasicTypeType basicType;
			std::vector<std::string> name;
			basicTypeGetType(pTypeNode, basicType, name);
			if (basicType == BASICTYPE_TYPE_STRUCT || basicType == BASICTYPE_TYPE_UNION || basicType == BASICTYPE_TYPE_ENUM)
			{
				if (name.size() == 0)
					return;
				s = name[0];
			}
			else
				return;
			break;
		}
		default:
			return;
		}
		//printf("Type '%s' is defined in %s:%d\n", s.c_str(), gf.buffered_keywords[n].file_name.c_str(), gf.buffered_keywords[n].line_no);
		g_deftype_map[s] = 1;
	}
	else if (name == "start")
	{
		pRoot->file_name = gf.buffered_keywords[n].file_name;
		pRoot->line_no = gf.buffered_keywords[n].line_no;
	}
}*/

/*void dumpToFile(FILE* fp, SourceTreeNode* pRoot)
{
	std::string s = displaySourceTreeStructType(pRoot, 0);
	fprintf(fp, "%s\n", s.c_str());
}*/

//==============================================================================
// get and compose node
SourceTreeNode* createEmptyNode()
{
	SourceTreeNode* pNode;

	pNode = new SourceTreeNode;
	pNode->pChild = pNode->pNext = NULL;
	pNode->line_no = pNode->param = 0;

	return pNode;
}

//ext_modifier	  : '__extension__' | 'extern' | 'extern' '\"C\"' | 'extern' '\"C++\"';
int extModifierGetBit(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return MODBIT___EXTENSION__;
	case 1:
		return MODBIT_EXTERN;
	case 2:
		return MODBIT_EXTERN_C;
	case 3:
		return MODBIT_EXTERN_CPP;
	default:
		MY_ASSERT(false);
	}
	return 0;
}

//func_modifier	: ext_modifier | 'virtual' | 'static' | flow;
int funcModifierGetBit(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return extModifierGetBit(pRoot->pChild->pChild);
	case 1:
		return MODBIT_VIRTUAL;
	case 2:
		return MODBIT_STATIC;
    case 3:
        return MODBIT_INLINE;
    case 4:
        return MODBIT_EXPLICIT;
    case 5:
        return MODBIT_FRIEND;
	case 6:
		return MODBIT_FLOW;
	default:
		MY_ASSERT(false);
	}
	return 0;
}

//member_modifier	  : ext_modifier | 'mutable' | 'static';
int memberModifierGetBit(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return extModifierGetBit(pRoot->pChild->pChild);
	case 1:
		return MODBIT_MUTABLE;
	case 2:
		return MODBIT_STATIC;
	default:
		MY_ASSERT(false);
	}
	return 0;
}

//data_type_modifier    : const | 'volatile';
int dataTypeModifierGetBit(const SourceTreeNode* pRoot)
{
    switch (pRoot->param)
    {
    case 0:
        return MODBIT_CONST;
    case 1:
        return MODBIT_VOLATILE;
    default:
        MY_ASSERT(false);
    }
    return 0;
}

//operator            : '+' | '-' | '*' | '/' | '=' | '<' | '>' | '+=' | '-=' | '*=' | '/=' | '<<' | '>>' | '<<=' | '>>=' | '==' | '!=' | '<=' | '>=' | '++' | '--' | '%' | '&' | '^' \\\n\
                    | '!' | '|' | '~' | '&=' | '^=' | '|=' | '&&' | '||' | '%=' | '(' ')' | '[' ']' | '->' | 'new' | 'new' '[' ']' | 'delete' | 'delete' '[' ']' | extended_type;
std::string operatorGetString(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return "+";
	case 1:
		return "-";
	case 2:
		return "*";
	case 3:
		return "/";
	case 4:
		return "=";
	case 5:
		return "<";
	case 6:
		return ">";
	case 7:
		return "+=";
	case 8:
		return "-=";
	case 9:
		return "*=";
	case 10:
		return "/=";
	case 11:
		return "<<";
	case 12:
		return ">>";
	case 13:
		return "<<=";
	case 14:
		return ">>=";
	case 15:
		return "==";
	case 16:
		return "!=";
	case 17:
		return "<=";
	case 18:
		return ">=";
	case 19:
		return "++";
	case 20:
		return "--";
	case 21:
		return "%";
	case 22:
		return "&";
	case 23:
		return "^";
	case 24:
		return "!";
	case 25:
		return "|";
	case 26:
		return "~";
	case 27:
		return "&=";
	case 28:
		return "^=";
	case 29:
		return "|=";
	case 30:
		return "&&";
	case 31:
		return "||";
	case 32:
		return "%=";
    case 33:
        return "()";
	case 34:
		return "[]";
    case 35:
        return "->";
	case 36:
		return "new";
    case 37:
        return "new[]";
    case 38:
        return "delete";
	case 39:
		return "delete[]";
    case 40:
        return ",";
	case 41:
	    return displaySourceTreeExtendedType(pRoot->pChild->pChild);
	default:
		MY_ASSERT(false);
	}
	return "";
}

std::string templateTypeToString(TemplateType t)
{
    switch (t)
    {
    case TEMPLATE_TYPE_CLASS:
        return "class";
    case TEMPLATE_TYPE_FUNC:
        return "func";
    case TEMPLATE_TYPE_VAR:
        return "var";
    case TEMPLATE_TYPE_FRIEND_CLASS:
        return "friend class";
    }

    MY_ASSERT(false);
    return "";
}

std::string tokenGetValue(const SourceTreeNode* pRoot)
{
	return pRoot->value;
}

CSUType csuGetType(const SourceTreeNode* pRoot)
{
    switch (pRoot->param)
    {
    case 0:
        return CSU_TYPE_UNION;
    case 1:
        return CSU_TYPE_STRUCT;
    case 2:
        return CSU_TYPE_CLASS;
    case 3:
        return CSU_TYPE_ENUM;
    }
    MY_ASSERT(false);
    return CSU_TYPE_NONE;
}

//scope               : ?['::'] *[ ?['template'] token ?[ < *[(extended_type|expr), ','] > ] '::'] ;
TokenWithNamespace scopeGetInfo(const SourceTreeNode* pRoot)
{
    TokenWithNamespace twn;

    twn.setHasRootSign(pRoot->param != 0);
    pRoot = pRoot->pNext;

    for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
    {
        SourceTreeNode* pNode = pRoot->pChild;

        bool bIsSpecifiedAsTemplate = (pNode->param > 0);
        pNode = pNode->pNext;

        bool bHasTemplate = (pNode->pNext->param > 0);
        twn.addScope(pNode->value, bHasTemplate, bIsSpecifiedAsTemplate);

        if (bHasTemplate)
        {
            pNode = pNode->pNext->pChild;
            for (pNode = pNode->pChild; pNode; pNode = pNode->pNext)
                twn.addTemplateParam(pNode->pChild->pChild->param == 0, pNode->pChild->pChild->pChild->pChild);
        }
    }

    return twn;
}

//scope               : ?['::'] *[ ?['template'] token ?[ < *[(extended_type_var|expr), ','] > ] '::'] ;
SourceTreeNode* scopeCreate(const TokenWithNamespace& twn)
{
    MY_ASSERT(twn.getDepth() > 0);
    SourceTreeNode* pRoot, *pNode, *pNode2;

    pRoot = createEmptyNode();
    if (twn.hasRootSign())
        pRoot->param = 1;

    pNode = pRoot;
    pNode->pNext = createEmptyNode();
    pNode->param = twn.getDepth() - 1;

    for (int i = 0; i < pNode->param; i++)
    {
        if (i == 0)
            pNode2 = pNode->pChild = createEmptyNode();
        else
            pNode2 = pNode2->pNext = createEmptyNode();
        pNode2->param = i;
        pNode2->pChild = createEmptyNode();
        SourceTreeNode* pNode3 = pNode2->pChild;

        if (twn.scopeSpecifiedAsTemplate(i))
            pNode3->param = 1;

        pNode3->pNext = createEmptyNode();
        pNode3 = pNode3->pNext;
        pNode3->value = twn.getToken(i);

        pNode3->pNext = createEmptyNode();
        pNode3 = pNode3->pNext;

        if (twn.scopeHasTemplate(i))
        {
            pNode3->param = 1;

            pNode3->pChild = createEmptyNode();
            pNode3 = pNode3->pChild;
            pNode3->param = twn.getTemplateParamCount(i);

            for (int j = 0; j < twn.getTemplateParamCount(i); j++)
            {
                if (j == 0)
                    pNode3 = pNode3->pChild = createEmptyNode();
                else
                    pNode3 = pNode3->pNext = createEmptyNode();
                pNode3->param = j;
                pNode3->pChild = createEmptyNode();
                SourceTreeNode* pNode4 = pNode3->pChild;

                pNode4->pChild = createEmptyNode();
                pNode4 = pNode4->pChild;

                SourceTreeNode* pChild;
                if (!twn.getTemplateParamAt(i, j, pChild))
                    pNode4->param = 1;

                pNode4->pChild = dupSourceTreeNode(pChild);
            }
        }
    }

    return pRoot;
}

void scopeChangeTokenName(SourceTreeNode* pRoot, int idx, const std::string& newName)
{
    pRoot = pRoot->pNext;
    MY_ASSERT(idx >= 0 && idx < pRoot->param);

    pRoot = pRoot->pChild;
    for (int i = 0; i < idx; i++)
        pRoot = pRoot->pNext;
    pRoot = pRoot->pChild;
    pRoot->value = newName;
}

//token_with_namespace: scope token ?[ < *[(extended_type|expr), ','] > ] ;
TokenWithNamespace tokenWithNamespaceGetInfo(const SourceTreeNode* pRoot)
{
    TokenWithNamespace twn = scopeGetInfo(pRoot->pChild);
	pRoot = pRoot->pNext;

	std::string token = pRoot->value;
	pRoot = pRoot->pNext;

    bool bHasTemplate = (pRoot->param > 0);
    twn.addScope(token, bHasTemplate, false);

    if (bHasTemplate)
    {
        SourceTreeNode* pNode = pRoot->pChild;
        for (pNode = pNode->pChild; pNode; pNode = pNode->pNext)
            twn.addTemplateParam(pNode->pChild->pChild->param == 0, pNode->pChild->pChild->pChild->pChild);
    }

	return twn;
}

void tokenWithNamespaceChangeTokenName(SourceTreeNode* pRoot, int idx, const std::string& newName)
{
    TokenWithNamespace twn = scopeGetInfo(pRoot->pChild);
    MY_ASSERT(idx >= 0 && idx <= twn.getDepth());

    if (idx < twn.getDepth())
        scopeChangeTokenName(pRoot->pChild, idx, newName);
    else
        pRoot->pNext->value = newName;
}

//token_with_namespace: scope token ;
SourceTreeNode* tokenWithNamespaceCreate(const TokenWithNamespace& twn)
{
    SourceTreeNode* pRoot, *pNode;

    pRoot = createEmptyNode();
    pRoot->pChild = scopeCreate(twn);

    pRoot->pNext = createEmptyNode();
    pRoot->pNext->value = twn.getLastToken();

    return pRoot;
}

SourceTreeNode* tokenWithNamespaceCreate(const std::string& token)
{
	TokenWithNamespace twn;
	twn.addScope(token, false);
	return tokenWithNamespaceCreate(twn);
}

//user_def_type	   : scope ?['template'] token ?[ '<' +[(extended_type|expr), ','] '>' ];
TokenWithNamespace userDefTypeGetInfo(const SourceTreeNode* pRoot)
{
    TokenWithNamespace twn = scopeGetInfo(pRoot->pChild);
	pRoot = pRoot->pNext;

	bool bSpecifiedAsTemplate = (pRoot->param > 0);
    pRoot = pRoot->pNext;

    bool bHasTemplate = (pRoot->pNext->param > 0);
    twn.addScope(pRoot->value, bHasTemplate, bSpecifiedAsTemplate);

    if (bHasTemplate)
    {
        pRoot = pRoot->pNext->pChild;
        for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
            twn.addTemplateParam(pRoot->pChild->pChild->param == 0, pRoot->pChild->pChild->pChild->pChild);
    }

    return twn;
}

void userDefTypeChangeTokenName(SourceTreeNode* pRoot, int idx, const std::string& newName)
{
    TokenWithNamespace twn = scopeGetInfo(pRoot->pChild);
    MY_ASSERT(idx >= 0 && idx <= twn.getDepth());

    if (idx < twn.getDepth())
        scopeChangeTokenName(pRoot->pChild, idx, newName);
	else
		pRoot->pNext->pNext->value = newName;
}

StringVector basicTypeGetInfo(const SourceTreeNode* pRoot)
{
	StringVector name;
	switch (pRoot->param)
	{
	case 0:
		name.push_back("void");
		break;

	case 1:
		name.push_back("va_list");
		break;

	case 2:
		name.push_back("__builtin_va_list");
		break;

	case 3:
	{
		pRoot = pRoot->pChild;
		for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		{
			switch (pNode->pChild->pChild->param)
			{
			case 0:
				name.push_back("signed");
				break;
			case 1:
				name.push_back("unsigned");
				break;
			case 2:
				name.push_back("char");
				break;
			case 3:
				name.push_back("short");
				break;
			case 4:
				name.push_back("int");
				break;
			case 5:
				name.push_back("long");
				break;
			case 6:
				name.push_back("float");
				break;
			case 7:
				name.push_back("double");
				break;
            case 8:
                name.push_back("__int128");
                break;
			case 9:
				name.push_back("bool");
				break;
			case 10:
				name.push_back("wchar_t");
				break;
			default:
				MY_ASSERT(false);
			}
		}
		MY_ASSERT(name.size() > 0);
		break;
	} // of case 3
	default:
		MY_ASSERT(false);
	} // end of switch

	return name;
}

//basic_type          : 'void' | 'va_list' | '__builtin_va_list' | +[('signed' | 'unsigned' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double' | '__int128' | 'bool' | 'wchar_t')];
SourceTreeNode* basicTypeCreateVoid()
{
	SourceTreeNode* pRoot;

	pRoot = createEmptyNode();
	return pRoot;
}

SourceTreeNode* basicTypeCreate(BasicTypeBasicType basic_type)
{
	SourceTreeNode* pRoot, *pNode;

	// ()
	pRoot = createEmptyNode();

	// type
	pNode = createEmptyNode();
	pNode->param = basic_type;
	pRoot->pChild = pNode;

	// +[]
	pNode = createEmptyNode();
	pNode->param = 1;
    pNode->pChild = createEmptyNode();
	pNode->pChild->pChild = pRoot;
	pRoot = pNode;

	// |
	pNode = createEmptyNode();
	pNode->param = 3;
	pNode->pChild = pRoot;
	pRoot = pNode;

	//printf("basicTypeCreate, return %s\n", displaySourceTreeBasicType(pRoot).c_str());
	return pRoot;
}

//type                : *[data_type_modifier] ( basic_type | class_struct_union user_def_type_no_check | ?['typename' ^N] user_def_type | '__typeof' '(' twn_nocheck ')' ) *[data_type_modifier] ?[ *[?[const] '*'] ?[const] ?['&'] scope '*'] ;
int typeGetModifierBits(const SourceTreeNode* pRoot)
{
    int modifier_bits = 0;

    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= dataTypeModifierGetBit(pNode->pChild->pChild);

    pRoot = pRoot->pNext->pNext;

    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= dataTypeModifierGetBit(pNode->pChild->pChild);

    return modifier_bits;
}

BasicTypeType typeGetType(const SourceTreeNode* pRoot)
{
    if (pRoot->pNext->pNext->pNext->param)
        return BASICTYPE_TYPE_DATA_MEMBER_POINTER;

	pRoot = pRoot->pNext->pChild;
	switch (pRoot->param)
	{
	case 0:
		return BASICTYPE_TYPE_BASIC;

	case 1:
	case 2:
		return BASICTYPE_TYPE_USER_DEF;

	case 3:
        return BASICTYPE_TYPE_TYPEOF;
	}

	MY_ASSERT(false);
	return BASICTYPE_TYPE_BASIC;
}

SourceTreeNode* typeBasicGetInfo(const SourceTreeNode* pRoot)
{
	MY_ASSERT(typeGetType(pRoot) == BASICTYPE_TYPE_BASIC);

	pRoot = pRoot->pNext->pChild->pChild;

	return pRoot->pChild;
}

SourceTreeNode* typeTypeOfGetInfo(const SourceTreeNode* pRoot)
{
    MY_ASSERT(typeGetType(pRoot) == BASICTYPE_TYPE_TYPEOF);

    pRoot = pRoot->pNext->pChild->pChild;

    return pRoot->pChild;
}

void typeDmpGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedTypeNode, TokenWithNamespace& twn)
{
    MY_ASSERT(typeGetType(pRoot) == BASICTYPE_TYPE_DATA_MEMBER_POINTER);
    ((SourceTreeNode*)pRoot)->pNext->pNext->pNext->param = 0;
    pExtendedTypeNode = createEmptyNode();
    pExtendedTypeNode->pChild = dupSourceTreeNode(pRoot);
    ((SourceTreeNode*)pRoot)->pNext->pNext->pNext->param = 1;

    pRoot = pRoot->pNext->pNext->pNext->pChild;
    pExtendedTypeNode->pNext = dupSourceTreeNode(pRoot);
    deleteSourceTreeNode(pExtendedTypeNode->pNext->pNext->pNext->pNext);
    pExtendedTypeNode->pNext->pNext->pNext->pNext = NULL;

    pRoot = pRoot->pNext->pNext->pNext;
    twn = scopeGetInfo(pRoot->pChild);
}

CSUType typeUserDefinedGetInfo(const SourceTreeNode* pRoot, bool& bHasTypename, SourceTreeNode*& pChild)
{
	MY_ASSERT(typeGetType(pRoot) == BASICTYPE_TYPE_USER_DEF);

	CSUType csu_type;
	pRoot = pRoot->pNext->pChild;

	if (pRoot->param == 1)
	{
		pRoot = pRoot->pChild;
		csu_type = csuGetType(pRoot->pChild);
		pRoot = pRoot->pNext;
	}
	else
	{
        csu_type = CSU_TYPE_NONE;
		pRoot = pRoot->pChild;

		bHasTypename = (pRoot->param != 0);
		pRoot = pRoot->pNext;
	}

	pChild = pRoot->pChild;
	return csu_type;
}

//type				: *[data_type_modifier] (basic_type ^ | ?[class_struct_union] user_def_type ^) *[data_type_modifier];
SourceTreeNode* typeCreateByBasic(SourceTreeNode* pChild)
{
	SourceTreeNode* pRoot, *pNode;

	pNode = createEmptyNode();
	pNode->pChild = pChild;

	// '|'
	pNode = createEmptyNode();
	pNode->param = 0;
	pNode->pChild = pRoot;
	pRoot = pNode;

	// '('
	pNode = createEmptyNode();
	pNode->pChild = pRoot;
	pRoot = pNode;

    // the last *[data_type_modifier]
    pRoot->pNext = createEmptyNode();

	// the first *[data_type_modifier]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	return pRoot;
}

SourceTreeNode* typeCreateByUserDefined(CSUType csu_type, SourceTreeNode* pChild)
{
	SourceTreeNode* pRoot, *pNode;

	pRoot = createEmptyNode();
	pRoot->pChild = pChild;

	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// csu_type
	if (csu_type != CSU_TYPE_NONE)
	{
		pNode = createEmptyNode();
		switch (csu_type)
		{
		case CSU_TYPE_UNION:
			pNode->param = 0;
			break;
		case CSU_TYPE_STRUCT:
			pNode->param = 1;
			break;
		case CSU_TYPE_CLASS:
			pNode->param = 2;
			break;
		case CSU_TYPE_ENUM:
			pNode->param = 3;
			break;
		default:
			MY_ASSERT(false);
		}
		pRoot->pChild = pNode;
	}

	// '|'
	pNode = createEmptyNode();
	pNode->param = 1;
	pNode->pChild = pRoot;
	pRoot = pNode;

	// '('
	pNode = createEmptyNode();
	pNode->pChild = pRoot;
	pRoot = pNode;

    // the last *[data_type_modifier]
    pRoot->pNext = createEmptyNode();

	// the first *[data_type_modifier]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	return pRoot;
}

//type                : *[data_type_modifier] ( basic_type | class_struct_union user_def_type_no_check | ?['typename' ^N] user_def_type | '__typeof' '(' twn_nocheck ')' ) *[data_type_modifier] ?[ *[?[const] '*'] ?[const] ?['&'] hard_scope '*'] ;
//extended_type       : type *[?[const] '*'] ?[const] ?['&'];
SourceTreeNode* typeCreateDmp(SourceTreeNode* pExtendedTypeNode, TokenWithNamespace& twn)
{
    SourceTreeNode* pTypeNode = pExtendedTypeNode->pChild;
    MY_ASSERT(typeGetType(pTypeNode) != BASICTYPE_TYPE_DATA_MEMBER_POINTER);

    SourceTreeNode* pRoot, *pNode;

    pRoot = pTypeNode;
    pNode = pRoot->pNext->pNext;

    pNode->pNext = createEmptyNode();
    pNode = pNode->pNext;
    pNode->param = 1;

    pNode->pChild = pExtendedTypeNode->pNext;

    pExtendedTypeNode->pChild = NULL;
    pExtendedTypeNode->pNext = NULL;
    deleteSourceTreeNode(pExtendedTypeNode);

    pNode = pNode->pChild->pNext->pNext;
    MY_ASSERT(pNode->pNext == NULL);

    pNode->pNext = createEmptyNode();
    pNode = pNode->pNext;

    pNode->pChild = scopeCreate(twn);

    return pRoot;
}

//extended_type       : type *[?[const] '*'] *[data_type_modifier] ?['&'];
SourceTreeNode* extendedTypeGetTypeNode(const SourceTreeNode* pRoot)
{
	return pRoot->pChild;
}

int extendedTypeGetDepth(const SourceTreeNode* pRoot)
{
	return pRoot->pNext->param;
}

bool extendedTypeIsReference(const SourceTreeNode* pRoot)
{
	return pRoot->pNext->pNext->pNext->param > 0;
}

bool extendedTypeIsConst(const SourceTreeNode* pRoot)
{
	pRoot = pRoot->pNext->pNext;

    for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
    {
        int bit = dataTypeModifierGetBit(pRoot->pChild->pChild);
        if (bit & MODBIT_CONST)
            return true;
    }

    return false;
}

bool extendedTypeIsVolatile(const SourceTreeNode* pRoot)
{
    pRoot = pRoot->pNext->pNext;

    for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
    {
        int bit = dataTypeModifierGetBit(pRoot->pChild->pChild);
        if (bit & MODBIT_VOLATILE)
            return true;
    }

    return false;
}

bool extendedTypePointerIsConst(const SourceTreeNode* pRoot, int depth) // is the layer of the given depth a pointer or a const pointer?
{
	pRoot = pRoot->pNext;
	MY_ASSERT(depth >= 0 && depth < pRoot->param);

	for (pRoot = pRoot->pChild; depth > 0; depth--)
		pRoot = pRoot->pNext;

	return pRoot->pChild->param != 0;
}

SourceTreeNode* extendedTypeCreateFromType(SourceTreeNode* pTypeNode, SourceTreeNode* pDeclVar/* = NULL*/)
{
	SourceTreeNode* pRoot, *pNode;

	// ?[&]
	pNode = createEmptyNode();
	pRoot = pNode;

    // ?[const]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

    // *[?[const] '*']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// type
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pNode->pChild = pTypeNode;
	pRoot = pNode;

	if (pDeclVar)
	{
	    for (int i = 0; i < declVarGetDepth(pDeclVar); i++)
	        extendedTypeAddModifier(pRoot, declVarPointerIsConst(pDeclVar, i) ? DVMOD_TYPE_CONST_POINTER : DVMOD_TYPE_POINTER);
	    if (declVarIsConst(pDeclVar))
	        extendedTypeAddModifier(pRoot, DVMOD_TYPE_CONST);
	    if (declVarIsReference(pDeclVar))
            extendedTypeAddModifier(pRoot, DVMOD_TYPE_REFERENCE);
	}

	return pRoot;
}

void extendedTypeAddModifier(SourceTreeNode* pRoot, DeclVarModifierType mod)
{
	SourceTreeNode* pNode;

	switch (mod)
	{
	case DVMOD_TYPE_POINTER:
    case DVMOD_TYPE_CONST_POINTER:
		pRoot = pRoot->pNext;
		pNode = createEmptyNode();
		pNode->pChild = createEmptyNode();
		if (mod == DVMOD_TYPE_CONST_POINTER)
		    pNode->pChild->pChild->param = 1;
		pNode->param = pRoot->param;
		pRoot->param++;
		if (pRoot->pChild == NULL)
			pRoot->pChild = pNode;
		else
		{
			pRoot = pRoot->pChild;
			while (pRoot->pNext)
				pRoot = pRoot->pNext;
			pRoot->pNext = pNode;
		}
		break;

    case DVMOD_TYPE_CONST:
        pRoot = pRoot->pNext->pNext;
        MY_ASSERT(pRoot->param == 0);
        pRoot->param = 1;
        break;

	case DVMOD_TYPE_REFERENCE:
		pRoot = pRoot->pNext->pNext->pNext;
		MY_ASSERT(pRoot->param == 0);
		pRoot->param = 1;
		break;

	default:
		MY_ASSERT(false);
	}
}

SourceTreeNode* extendedTypeVarCreateFromExtendedType(SourceTreeNode* pTypeNode)
{
	SourceTreeNode* pRoot, *pNode;
	pNode = createEmptyNode();
	pRoot = pNode;

	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pNode->pChild = pTypeNode;
	pRoot = pNode;

	return pRoot;
}

//extended_or_func_type: extended_type | func_type;
void extendedOrFuncTypeGetInfo(const SourceTreeNode* pRoot, bool& bExtendedType, SourceTreeNode*& pChild)
{
    bExtendedType = (pRoot->param == 0);
    pChild = pRoot->pChild->pChild;
}

//extended_type_var   : extended_type *['[' ?[expr] ']'];
void extendedTypeVarGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedTypeNode, int& depth)
{
	pExtendedTypeNode = pRoot->pChild;
	depth = pRoot->pNext->param;
}

// func_params		: type ?['__restrict'] ?[const] ?[ decl_var ?[ '=' expr ] ] | func_type | '...' ;
void funcParamGetInfo(const SourceTreeNode* pRoot, FuncParamType& param_type, int& modifierBits, SourceTreeNode*& pTypeNode, SourceTreeNode*& pDeclVarNode, SourceTreeNode*& pInitExprNode)
{
	modifierBits = 0;
	pTypeNode = pDeclVarNode = pInitExprNode = NULL;

	switch (pRoot->param)
	{
	case 0:
		pRoot = pRoot->pChild;
		param_type = FUNC_PARAM_TYPE_REGULAR;
		pTypeNode = pRoot->pChild;
		pRoot = pRoot->pNext;
		if (pRoot->param)
			modifierBits |= MODBIT___RESTRICT;
		pRoot = pRoot->pNext;
		if (pRoot->param)
			modifierBits |= MODBIT_CONST;
		pRoot = pRoot->pNext;
		if (pRoot->param == 0)
			break;
		pRoot = pRoot->pChild;
		pDeclVarNode = pRoot->pChild;
		pRoot = pRoot->pNext;
		if (pRoot->param)
			pInitExprNode = pRoot->pChild->pChild;
		break;

	case 1:
		pRoot = pRoot->pChild;
		param_type = FUNC_PARAM_TYPE_FUNC;
		pTypeNode = pRoot->pChild;
		break;

	default:
		MY_ASSERT(false);
	}
}

//func_params         : *[func_param, ','] ?['...' | ',' '...'] ;
SourceTreeVector funcParamsGetList(const SourceTreeNode* pRoot)
{
    SourceTreeVector paramList;

    if (pRoot)
    {
        for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
          paramList.push_back(pNode->pChild->pChild);
    }
    return paramList;
}

bool funcParamsHasVArgs(const SourceTreeNode* pRoot)
{
    if (!pRoot)
        return false;

    return pRoot->pNext->param != 0;
}

//func_type           : extended_type ?[ '(' scope '*' ?[token ?[ '(' func_params ')' ] ] ')' ] '(' func_params ')' *[data_type_modifier];
void funcTypeGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pReturnExtendedType, SourceTreeNode*& pScope, std::string& name, SourceTreeNode*& pOptFuncParamsNode, SourceTreeNode*& pFuncParamsNode, int& modifier_bits)
{
	pReturnExtendedType = pRoot->pChild;
	pRoot = pRoot->pNext;

	pScope = NULL;
    name = "";
    pOptFuncParamsNode = NULL;
    if (pRoot->param)
    {
        SourceTreeNode* pNode = pRoot->pChild;

        pScope = pNode->pChild;
        pNode = pNode->pNext;

        if (pNode->param)
        {
        	pNode = pNode->pChild;

            name = pNode->value;
            pNode = pNode->pNext;

            if (pNode->param)
            	pOptFuncParamsNode = pNode->pChild->pChild;
        }
    }
	pRoot = pRoot->pNext;

	pFuncParamsNode = pRoot->pChild;
    pRoot = pRoot->pNext;

    modifier_bits = 0;
    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= dataTypeModifierGetBit(pNode->pChild->pChild);
}

//func_header         : *[func_modifier] ?[attribute] *[func_modifier] ?[extended_type] ?[attribute] ?( scope (*['*'] ?['~'] token | 'operator' operator) ?['<' '>'] '(' %[func_params, ')'] ')' ) *[data_type_modifier] ?['throw' '(' ?[type] ')'] ;
void funcHeaderGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, SourceTreeNode*& pReturnExtendedType, SourceTreeNode*& pAttribute,
    TokenWithNamespace& scope, int& nDataMemberPointerDepth, std::string& name, bool& bEmptyTemplate, void*& params_block, bool& bThrow, SourceTreeNode*& pThrowTypeNode)
{
	SourceTreeNode* pNode;

	modifier_bits = 0;
    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= funcModifierGetBit(pNode->pChild->pChild);
    pRoot = pRoot->pNext;

    pAttribute = NULL;
    if (pRoot->param)
        pAttribute = pRoot->pChild->pChild;
    pRoot = pRoot->pNext;

    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= funcModifierGetBit(pNode->pChild->pChild);
    pRoot = pRoot->pNext;

	pReturnExtendedType = NULL;
	if (pRoot->param)
		pReturnExtendedType = pRoot->pChild->pChild;
	pRoot = pRoot->pNext;

    if (pRoot->param)
    {
        MY_ASSERT(!pAttribute);
        pAttribute = pRoot->pChild->pChild;
    }
    pRoot = pRoot->pNext;

    SourceTreeNode* pRoot2 = pRoot->pChild;
	scope = scopeGetInfo(pRoot2->pChild);
	pRoot2 = pRoot2->pNext;

	nDataMemberPointerDepth = 0;
	if (pRoot2->pChild->param == 0)
	{
		pNode = pRoot2->pChild->pChild;

		nDataMemberPointerDepth = pNode->param;
		pNode = pNode->pNext;

		if (pNode->param > 0)
			name += "~";
		pNode = pNode->pNext;
		name += pNode->value;
	}
	else
	{
		name += "operator ";
		pNode = pRoot2->pChild->pChild;
		name += operatorGetString(pNode->pChild);
	}
	pRoot2 = pRoot2->pNext;

	bEmptyTemplate = (pRoot2->param > 0);
    pRoot2 = pRoot2->pNext;

    MY_ASSERT(pRoot2->param == SOURCE_NODE_TYPE_BIG_BRACKET);
    params_block = pRoot2->ptr;
    pRoot2 = pRoot2->pNext;

    pRoot = pRoot->pNext;

    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= dataTypeModifierGetBit(pNode->pChild->pChild);
    pRoot = pRoot->pNext;

    bThrow = false;
    pThrowTypeNode = NULL;
    if (pRoot->param)
    {
        bThrow = true;
        if (pRoot->pChild->param)
            pThrowTypeNode = pRoot->pChild->pChild->pChild;
    }
}

//enum_def			: 'enum' ?[token] '{' (*[token ?['=' expr ], ','] | *[token ?['=' expr] ','] ) '}';
void enumDefGetInfo(const SourceTreeNode* pRoot, std::string& name, int& children_count)
{
	name = "";
	if (pRoot->param)
		name = pRoot->pChild->value;
	pRoot = pRoot->pNext;

	children_count = pRoot->pChild->pChild->param;
}

void enumDefGetChildByIndex(const SourceTreeNode* pRoot, int idx, std::string& key, SourceTreeNode*& pExpr)
{
	pRoot = pRoot->pNext->pChild->pChild;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);

	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;

	key = pRoot->value;
	pRoot = pRoot->pNext;
	pExpr = NULL;
	if (pRoot->param)
		pExpr = pRoot->pChild->pChild;
}

//union_def		   : 'union' ^ ?[token] '{' *[defs] '}';
void unionDefGetInfo(const SourceTreeNode* pRoot, std::string& name, void*& bracket_block)
{
	name = "";
	if (pRoot->param)
		name = pRoot->pChild->value;
	pRoot = pRoot->pNext;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	bracket_block = pRoot->ptr;
}

//class_access_modifier: 'public' | 'protected' | 'private' ;
ClassAccessModifierType camGetType(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return CAM_TYPE_PUBLIC;
	case 1:
		return CAM_TYPE_PROTECTED;
	case 2:
		return CAM_TYPE_PRIVATE;
	}
	MY_ASSERT(false);
	return CAM_TYPE_PUBLIC;
}

//class_def_body	  : class_access_modifier ':' \\\n\
					| defs \\\n\
					;
ClassDefBodyType cdbGetType(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return CDB_TYPE_CAM;
    case 1:
        return CDB_TYPE_FRIEND;
	case 2:
		return CDB_TYPE_DEFS;
	//case 2:
	//	return CDB_TYPE_FUNC_DEF;
	}
	MY_ASSERT(false);
	return CDB_TYPE_CAM;
}

ClassAccessModifierType cdbCamGetType(const SourceTreeNode* pRoot)
{
	MY_ASSERT(cdbGetType(pRoot) == CDB_TYPE_CAM);
	pRoot = pRoot->pChild;

	return camGetType(pRoot->pChild);
}

//| ^O 'friend' class_struct_union user_def_type ';'
void cdbFriendGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, SourceTreeNode*& pUserDefTypeNode)
{
    MY_ASSERT(cdbGetType(pRoot) == CDB_TYPE_FRIEND);
    pRoot = pRoot->pChild;

    csu_type = csuGetType(pRoot->pChild);
    pRoot = pRoot->pNext;

    pUserDefTypeNode = pRoot->pChild;
}

SourceTreeNode* cdbDefGetNode(const SourceTreeNode* pRoot)
{
	MY_ASSERT(cdbGetType(pRoot) == CDB_TYPE_DEFS);
	pRoot = pRoot->pChild;

	return pRoot->pChild;
}

/*// *[func_modifier] func_header ?[const] ?[':' +[token '(' expr ')', ','] ] '{' ^ *[statement] '}'
void cdbFuncDefGetInfo(const SourceTreeNode* pRoot, int& func_modifier, SourceTreeNode*& pFuncHeaderNode, int& memberInitCount, void*& bracket_block)
{
	MY_ASSERT(cdbGetType(pRoot) == CDB_TYPE_FUNC_DEF);
	pRoot = pRoot->pChild;

	func_modifier = 0;
	for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
	  func_modifier |= funcModifierGetBit(pNode->pChild->pChild);
	pRoot = pRoot->pNext;

	pFuncHeaderNode = pRoot->pChild;
	pRoot = pRoot->pNext;

	if (pRoot->param)
		func_modifier |= MODBIT_CONST;
	pRoot = pRoot->pNext;

	memberInitCount = 0;
	if (pRoot->param)
		memberInitCount = pRoot->pChild->param;
	pRoot = pRoot->pNext;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	bracket_block = pRoot->ptr;
}

void cdbFuncDefGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr)
{
	MY_ASSERT(cdbGetType(pRoot) == CDB_TYPE_FUNC_DEF);
	pRoot = pRoot->pChild;

	pRoot = pRoot->pNext->pNext->pNext;
	MY_ASSERT(pRoot->param != 0);
	pRoot = pRoot->pChild;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);
	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;
	name = pRoot->value;
	pRoot = pRoot->pNext;
	pExpr = pRoot->pChild;
}*/

//base_class_defs		: ':' +[ ?['virtual'] ?[class_access_modifier] user_def_type, ',']
int baseClassDefsGetCount(const SourceTreeNode* pRoot)
{
	return pRoot->param;
}

void baseClassDefsGetChildByIndex(const SourceTreeNode* pRoot, int idx, bool& bVirtual, ClassAccessModifierType& cam_type, SourceTreeNode*& pUserDefTypeNode)
{
	MY_ASSERT(idx >= 0 && idx < pRoot->param);
	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;

	bVirtual = (pRoot->param > 0);
    pRoot = pRoot->pNext;

	cam_type = CAM_TYPE_NONE;
	if (pRoot->param)
		cam_type = camGetType(pRoot->pChild->pChild);
	pRoot = pRoot->pNext;

	pUserDefTypeNode = pRoot->pChild;
}

//class_def			: ('class' | 'struct') ?[attribute] scope ?[token] ?[base_class_defs] { class_def_body }
void classDefGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, SourceTreeNode*& pAttributeNode, TokenWithNamespace& name, void*& pBaseClassDefsBlock, void*& bracket_block)
{
	csu_type = (pRoot->pChild->param == 0 ? CSU_TYPE_CLASS : CSU_TYPE_STRUCT);
	pRoot = pRoot->pNext;

	pAttributeNode = NULL;
	if (pRoot->param)
	    pAttributeNode = pRoot->pChild->pChild;
    pRoot = pRoot->pNext;

	name = scopeGetInfo(pRoot->pChild);
    pRoot = pRoot->pNext;

	if (pRoot->param)
		name.addScope(pRoot->pChild->value);
	else
	    MY_ASSERT(name.empty());
	pRoot = pRoot->pNext;

	pBaseClassDefsBlock = NULL;
	if (pRoot->param)
	{
	    MY_ASSERT(pRoot->pChild->param == SOURCE_NODE_TYPE_BIG_BRACKET);
		pBaseClassDefsBlock = pRoot->pChild->ptr;
	}
	pRoot = pRoot->pNext;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	bracket_block = pRoot->ptr;
}

//super_type		  : type | enum_def | union_def | class_def ;
void superTypeGetInfo(const SourceTreeNode* pRoot, SuperTypeType& super_type, SourceTreeNode*& pChildNode)
{
	switch (pRoot->param)
	{
	case 0:
		super_type = SUPERTYPE_TYPE_TYPE;
		break;
	case 1:
		super_type = SUPERTYPE_TYPE_ENUM_DEF;
		break;
	case 2:
		super_type = SUPERTYPE_TYPE_UNION_DEF;
		break;
	case 3:
		super_type = SUPERTYPE_TYPE_CLASS_DEF;
		break;
	}

	pChildNode = pRoot->pChild->pChild;
}

std::string declVarGetName(const SourceTreeNode* pRoot)
{
	pRoot = pRoot->pNext->pNext->pNext->pNext->pNext;
	if (pRoot->param)
		return pRoot->pChild->value;

	return "";
}

int declVarGetDepth(const SourceTreeNode* pRoot)
{
	int ret = pRoot->param;
	pRoot = pRoot->pNext->pNext->pNext->pNext->pNext->pNext->pNext->pNext;
	ret += pRoot->param;
	pRoot = pRoot->pNext;
	ret += pRoot->param;

	return ret;
}

std::string declVarGetBitsValue(const SourceTreeNode* pRoot)
{
	pRoot = pRoot->pNext->pNext->pNext->pNext->pNext->pNext->pNext;
	if (pRoot->param)
		return pRoot->pChild->value;

	return "";
}

// *[?[const] '*'] ?[const] ?['__restrict'] ?['('] ?['&'] ?[token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']'];
bool declVarIsConst(const SourceTreeNode* pRoot)
{
	return pRoot->pNext->param != 0;
}

bool declVarPointerIsConst(const SourceTreeNode* pRoot, int depth) // is the layer of the given depth a pointer or a const pointer?
{
	if (depth >= pRoot->param)
		return false;

	for (pRoot = pRoot->pChild; depth > 0; depth--)
		pRoot = pRoot->pNext;

	return pRoot->pChild->param != 0;
}

int declVarGetBits(const SourceTreeNode* pRoot)
{
    pRoot = pRoot->pNext->pNext->pNext->pNext->pNext->pNext->pNext;
    if (pRoot->param == 0)
        return -1;

    return atoi(pRoot->pChild->value.c_str());
}

SourceTreeVector declVarGetExprs(const SourceTreeNode* pRoot)
{
	SourceTreeVector ret_v;

	pRoot = pRoot->pNext->pNext->pNext->pNext->pNext->pNext->pNext->pNext->pNext;
	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
		ret_v.push_back(pRoot->pChild->pChild);

	return ret_v;
}

// decl_var		 : *[?[const] '*'] ?[const] ?['__restrict'] ?['('] ?['&'] ?[token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']'];
SourceTreeNode* declVarCreateByName(const std::string& new_name)
{
	SourceTreeNode *pRoot, *pNode;

	// *['[' expr ']']
	pNode = createEmptyNode();
	pRoot = pNode;

	// ?['[' ']']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ?[':' const_value]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ?[')']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ?[token]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;
	pRoot->param = 1;

	pNode = createEmptyNode();
	pNode->value = new_name;
	pRoot->pChild = pNode;

	// ?['&']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ?['(']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ?['__restrict']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ?[const]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// *['*']
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;
	pRoot->param = 0;

	return pRoot;
}

void declVarAddModifier(SourceTreeNode* pRoot, DeclVarModifierType mode, SourceTreeNode* pExtNode)
{
	SourceTreeNode* pNode;

	switch (mode)
	{
	case DVMOD_TYPE_POINTER:
		pNode = createEmptyNode();
		pNode->param = pRoot->param;
		pNode->pChild = createEmptyNode();
		pNode->pChild->param = 0;
		if (pRoot->pChild == NULL)
			pRoot->pChild = pNode;
		else
		{
			pRoot = pRoot->pChild;
			while (pRoot->pNext)
				pRoot = pRoot->pNext;
			pRoot->pNext = pNode;
		}
		pRoot->param++;
		break;

	case DVMOD_TYPE_CONST_POINTER:
		pNode = createEmptyNode();
		pNode->param = pRoot->param;
		pNode->pChild = createEmptyNode();
		pNode->pChild->param = 1;
		if (pRoot->pChild == NULL)
			pRoot->pChild = pNode;
		else
		{
			pRoot = pRoot->pChild;
			while (pRoot->pNext)
				pRoot = pRoot->pNext;
			pRoot->pNext = pNode;
		}
		pRoot->param++;
		break;

	case DVMOD_TYPE_CONST:
		pRoot->pNext->param = 1;
		break;

	case DVMOD_TYPE_REFERENCE:
		pRoot->pNext->pNext->pNext->pNext->param = 1;
		break;

	case DVMOD_TYPE_PARENTHESIS:
		pRoot->pNext->pNext->pNext->param = pRoot->pNext->pNext->pNext->pNext->param = 1;
		break;

	case DVMOD_TYPE_BIT:
		pRoot = pRoot->pNext->pNext->pNext->pNext->pNext->pNext->pNext;
		MY_ASSERT(pRoot->param == 0);
		pRoot->param = 1;
		pNode = createEmptyNode();
		pRoot->pChild = pNode;
		MY_ASSERT(pExtNode);
		pNode->pChild = pExtNode;
		break;

	case DVMOD_TYPE_BRACKET:
		pRoot = pRoot->pNext->pNext->pNext->pNext->pNext->pNext->pNext->pNext;
		if (pExtNode == NULL)
		{
			MY_ASSERT(pRoot->param == 0);
			pRoot->param = 1;
		}
		else
		{
			pRoot = pRoot->pNext;
			pNode = createEmptyNode();
			pNode->pChild = pExtNode;
			pNode->param = pRoot->param;
			pRoot->param++;
			if (pRoot->pChild == NULL)
				pRoot->pChild = pNode;
			else
			{
				pRoot = pRoot->pChild;
				while (pRoot->pNext)
					pRoot = pRoot->pNext;
				pRoot->pNext = pNode;
			}
		}
		break;

	default:
		MY_ASSERT(false);
	}
}

SourceTreeNode* declVarCreateFromExtendedType(const std::string& name, const SourceTreeNode* pExtendedType)
{
	SourceTreeNode* pRoot = declVarCreateByName(name);

	int depth = extendedTypeGetDepth(pExtendedType);

	for (int i = 0; i < depth; i++)
		declVarAddModifier(pRoot, (extendedTypePointerIsConst(pExtendedType, i) ? DVMOD_TYPE_CONST_POINTER : DVMOD_TYPE_POINTER));

	if (extendedTypeIsReference(pExtendedType))
		declVarAddModifier(pRoot, DVMOD_TYPE_REFERENCE);

	if (extendedTypeIsConst(pExtendedType))
		declVarAddModifier(pRoot, DVMOD_TYPE_CONST);

	return pRoot;
}

void declVarChangeName(SourceTreeNode* pRoot, const std::string& new_name)
{
	pRoot = pRoot->pNext->pNext->pNext->pNext->pNext;

	MY_ASSERT(pRoot->param > 0);

	pRoot->pChild->value = new_name;
}

bool declVarIsReference(const SourceTreeNode* pRoot)
{
	pRoot = pRoot->pNext->pNext->pNext->pNext;

	return pRoot->param != 0;
}

void declVarSetReference(SourceTreeNode* pRoot)
{
	pRoot = pRoot->pNext->pNext->pNext;

	pRoot->param = 1;
	pRoot = pRoot->pNext;

	pRoot->param = 1;
	pRoot = pRoot->pNext;

	pRoot = pRoot->pNext;
	pRoot->param = 1;
}

//decl_c_var		  : ?['__restrict'] decl_var ?[ '=' expr] ;
void declCVarGetInfo(const SourceTreeNode* pRoot, bool& bRestrict, SourceTreeNode*& pVar, SourceTreeNode*& pInitValue) // pVar: decl_var, pInitValue: expr
{
	bRestrict = (pRoot->param > 0);
	pRoot = pRoot->pNext;

	pVar = pRoot->pChild;
	pRoot = pRoot->pNext;

	pInitValue = NULL;
	if (pRoot->param)
		pInitValue = pRoot->pChild->pChild;
}

//decl_obj_var		: token '(' *[expr, ','] ')' ;
void declObjVarGetInfo(const SourceTreeNode* pRoot, std::string& name, SourceTreeVector& exprList) // pVar: decl_var, pInitValue: expr
{
	name = pRoot->value;
	pRoot = pRoot->pNext;

	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
		exprList.push_back(pRoot->pChild->pChild);
}

ExprType exprGetType(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return EXPR_TYPE_REF_ELEMENT;
	case 1:
		return EXPR_TYPE_PTR_ELEMENT;
	case 2:
		return EXPR_TYPE_FUNC_CALL;
    case 3:
        return EXPR_TYPE_OPERATOR_CALL;
	case 4:
		return EXPR_TYPE_ARRAY;
	case 5:
		return EXPR_TYPE_RIGHT_INC;
	case 6:
		return EXPR_TYPE_RIGHT_DEC;
	case 7:
		return EXPR_TYPE_LEFT_INC;
	case 8:
		return EXPR_TYPE_LEFT_DEC;
	case 9:
		return EXPR_TYPE_POSITIVE;
	case 10:
		return EXPR_TYPE_NEGATIVE;
	case 11:
		return EXPR_TYPE_NOT;
	case 12:
		return EXPR_TYPE_XOR;
	case 13:
		return EXPR_TYPE_TYPE_CAST;
	case 14:
		return EXPR_TYPE_INDIRECTION;
	case 15:
		return EXPR_TYPE_ADDRESS_OF;
	case 16:
		return EXPR_TYPE_SIZEOF;
	case 17:
		return EXPR_TYPE_NEW_C;
	case 18:
		return EXPR_TYPE_NEW_OBJECT;
    case 19:
        return EXPR_TYPE_NEW_ADV;
	case 20:
		return EXPR_TYPE_DELETE;
	case 21:
		return EXPR_TYPE_MULTIPLE;
	case 22:
		return EXPR_TYPE_DIVIDE;
	case 23:
		return EXPR_TYPE_REMAINDER;
	case 24:
		return EXPR_TYPE_ADD;
	case 25:
		return EXPR_TYPE_SUBTRACT;
	case 26:
		return EXPR_TYPE_LEFT_SHIFT;
	case 27:
		return EXPR_TYPE_RIGHT_SHIFT;
	case 28:
		return EXPR_TYPE_LESS_THAN;
	case 29:
		return EXPR_TYPE_LESS_EQUAL;
	case 30:
		return EXPR_TYPE_GREATER_THAN;
	case 31:
		return EXPR_TYPE_GREATER_EQUAL;
	case 32:
		return EXPR_TYPE_EQUAL;
	case 33:
		return EXPR_TYPE_NOT_EQUAL;
	case 34:
		return EXPR_TYPE_BIT_AND;
	case 35:
		return EXPR_TYPE_BIT_XOR;
	case 36:
		return EXPR_TYPE_BIT_OR;
	case 37:
		return EXPR_TYPE_AND;
	case 38:
		return EXPR_TYPE_OR;
	case 39:
		return EXPR_TYPE_TERNARY;
	case 40:
		return EXPR_TYPE_ASSIGN;
	case 41:
		return EXPR_TYPE_ADD_ASSIGN;
	case 42:
		return EXPR_TYPE_SUBTRACT_ASSIGN;
	case 43:
		return EXPR_TYPE_MULTIPLE_ASSIGN;
	case 44:
		return EXPR_TYPE_DIVIDE_ASSIGN;
	case 45:
		return EXPR_TYPE_REMAINDER_ASSIGN;
	case 46:
		return EXPR_TYPE_LEFT_SHIFT_ASSIGN;
	case 47:
		return EXPR_TYPE_RIGHT_SHIFT_ASSIGN;
	case 48:
		return EXPR_TYPE_BIT_AND_ASSIGN;
	case 49:
		return EXPR_TYPE_BIT_XOR_ASSIGN;
	case 50:
		return EXPR_TYPE_BIT_OR_ASSIGN;
	case 51:
		return EXPR_TYPE_THROW;
	case 52:
		return EXPR_TYPE_CONST_VALUE;
	case 53:
		return EXPR_TYPE_TOKEN_WITH_NAMESPACE;
    case 54:
        return EXPR_TYPE_TYPE_CONSTRUCT;
	case 55:
		return EXPR_TYPE_PARENTHESIS;
    case 56:
        return EXPR_TYPE_BUILTIN_TYPE_FUNC;
    case 57:
        return EXPR_TYPE_CONST_CAST;
    case 58:
        return EXPR_TYPE_STATIC_CAST;
    case 59:
        return EXPR_TYPE_DYNAMIC_CAST;
    case 60:
        return EXPR_TYPE_REINTERPRET_CAST;
    case 61:
        return EXPR_TYPE_EXTENSION;
	}
    MY_ASSERT(false);
	return EXPR_TYPE_PARENTHESIS;
}

SourceTreeVector exprGetExprList(const SourceTreeNode* pRoot)
{
	MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_FUNC_CALL || exprGetType(pRoot) == EXPR_TYPE_NEW_OBJECT || exprGetType(pRoot) == EXPR_TYPE_OBJECT);

	SourceTreeVector ret_v;

	pRoot = pRoot->pChild;
	pRoot = pRoot->pNext;
	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
		ret_v.push_back(pRoot->pChild->pChild);

	return ret_v;
}

std::string exprConstGetValue(const SourceTreeNode* pRoot)
{
	MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_CONST_VALUE);

	return pRoot->pChild->value;
}

SourceTreeNode* exprGetFirstNode(const SourceTreeNode* pRoot)
{
	return pRoot->pChild->pChild;
}

SourceTreeNode* exprGetSecondNode(const SourceTreeNode* pRoot)
{
	return pRoot->pChild->pNext->pChild;
}

std::string exprGetSecondToken(const SourceTreeNode* pRoot)
{
	return pRoot->pChild->pNext->value;
}

SourceTreeNode* exprGetThirdNode(const SourceTreeNode* pRoot)
{
	return pRoot->pChild->pNext->pNext->pChild;
}

SourceTreeNode* exprGetOptionalThirdNode(const SourceTreeNode* pRoot)
{
    pRoot = pRoot->pChild->pNext->pNext;
    if (pRoot->param == 0)
        return NULL;

    return pRoot->pChild->pChild;
}

std::string exprGetBuiltinFuncTypeName(const SourceTreeNode* pRoot)
{
    MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_BUILTIN_TYPE_FUNC);

    switch (pRoot->pChild->pChild->param)
    {
    case 0:
        return "__alignof__";
    case 1:
        return "__is_abstract";
    case 2:
        return "__is_class";
    case 3:
        return "__is_empty";
    case 4:
        return "__is_pod";
    case 5:
        return "__has_nothrow_assign";
    case 6:
        return "__has_nothrow_copy";
    case 7:
        return "__has_trivial_assign";
    case 8:
        return "__has_trivial_copy";
    case 9:
        return "__has_trivial_destructor";
    }

    MY_ASSERT(false);
    return "";
}

// 0: extended_type_var, 1: func_type, 2: expr
void exprSizeOfGetInfo(const SourceTreeNode* pRoot, int& nType, SourceTreeNode*& pChild)
{
	MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_SIZEOF);

	pRoot = pRoot->pChild->pChild;
	nType = pRoot->param;
	pChild = pRoot->pChild->pChild;
}

//|@3 scope 'new' (expr) ?[ user_def_type ?[ '(' expr2 ')' ] ]
void exprNewAdvGetParams(const SourceTreeNode* pRoot, SourceTreeNode*& pUserDefType, SourceTreeNode*& pExpr2)
{
    MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_NEW_ADV);

    pRoot = pRoot->pChild->pNext->pNext;

    pUserDefType = pExpr2 = NULL;
    if (pRoot->param)
    {
        pRoot = pRoot->pChild;

        pUserDefType = pRoot->pChild;
        pRoot = pRoot->pNext;

        if (pRoot->param)
            pExpr2 = pRoot->pChild->pChild;
    }
}

bool exprDeleteHasArray(const SourceTreeNode* pRoot)
{
	MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_DELETE);

	return pRoot->pChild->pNext->param > 0;
}

// expr '.' (?['~'] token | 'operator' operator) |@1 (?['~'] token | 'operator' operator)
void exprPtrRefGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr, std::string& token)
{
    MY_ASSERT(exprGetType(pRoot) == EXPR_TYPE_REF_ELEMENT || exprGetType(pRoot) == EXPR_TYPE_PTR_ELEMENT);
    pRoot = pRoot->pChild;

    pExpr = pRoot->pChild;
    pRoot = pRoot->pNext->pChild;

    if (pRoot->param == 0)
    {
        pRoot = pRoot->pChild;
        token = pRoot->pNext->value;
        if (pRoot->param)
            token = "~" + token;
    }
    else
    {
        pRoot = pRoot->pChild;
        token = "operator " + operatorGetString(pRoot->pChild);
    }
    //pExpr2 = NULL;
    //if (pRoot->param)
    //   pExpr2 = pRoot->pChild->pChild;
}

// expr_type must be EXPR_TYPE_CONST_VALUE or EXPR_TYPE_TOKEN
SourceTreeNode* exprCreateConst(ExprType expr_type, const std::string val)
{
	MY_ASSERT(expr_type == EXPR_TYPE_CONST_VALUE);

	SourceTreeNode* pRoot = new SourceTreeNode;
	pRoot->pChild = pRoot->pNext = NULL;
	pRoot->line_no = 0;
	pRoot->param = expr_type;

	SourceTreeNode* pNode = new SourceTreeNode;
	pRoot->pChild = pNode;
	pNode->pChild = pNode->pNext = NULL;
	pNode->line_no = pNode->param = 0;
	pNode->value = val;

	return pRoot;
}

SourceTreeNode* exprCreateRefOrPtr(SourceTreeNode* pLeft, ExprType expr_type, const std::string token)
{
	MY_ASSERT(expr_type == EXPR_TYPE_REF_ELEMENT || expr_type == EXPR_TYPE_PTR_ELEMENT);

	SourceTreeNode* pRoot = new SourceTreeNode;
	pRoot->pChild = pRoot->pNext = NULL;
	pRoot->line_no = 0;
	pRoot->param = expr_type;

	pRoot->pChild = pLeft;
	MY_ASSERT(pLeft->pNext == NULL);

	SourceTreeNode* pNode = new SourceTreeNode;
	pLeft->pNext = pNode;
	pNode->pChild = pNode->pNext = NULL;
	pNode->line_no = pNode->param = 0;
	pNode->value = token;

	return pRoot;
}

SourceTreeNode* exprCreateDoubleOperator(SourceTreeNode* pLeft, ExprType expr_type, SourceTreeNode* pRight)
{
	MY_ASSERT(expr_type == EXPR_TYPE_ARRAY || expr_type == EXPR_TYPE_MULTIPLE || expr_type == EXPR_TYPE_DIVIDE ||
			expr_type == EXPR_TYPE_REMAINDER || expr_type == EXPR_TYPE_ADD || expr_type == EXPR_TYPE_SUBTRACT ||
			expr_type == EXPR_TYPE_LEFT_SHIFT || expr_type == EXPR_TYPE_RIGHT_SHIFT || expr_type == EXPR_TYPE_LESS_THAN ||
			expr_type == EXPR_TYPE_LESS_EQUAL || expr_type == EXPR_TYPE_GREATER_THAN || expr_type == EXPR_TYPE_GREATER_EQUAL ||
			expr_type == EXPR_TYPE_EQUAL || expr_type == EXPR_TYPE_NOT_EQUAL || expr_type == EXPR_TYPE_BIT_AND ||
			expr_type == EXPR_TYPE_BIT_XOR || expr_type == EXPR_TYPE_BIT_OR || expr_type == EXPR_TYPE_AND ||
			expr_type == EXPR_TYPE_LESS_EQUAL || expr_type == EXPR_TYPE_GREATER_THAN || expr_type == EXPR_TYPE_GREATER_EQUAL ||
			expr_type == EXPR_TYPE_OR || expr_type == EXPR_TYPE_ASSIGN || expr_type == EXPR_TYPE_ADD_ASSIGN ||
			expr_type == EXPR_TYPE_SUBTRACT_ASSIGN || expr_type == EXPR_TYPE_MULTIPLE_ASSIGN || expr_type == EXPR_TYPE_DIVIDE_ASSIGN ||
			expr_type == EXPR_TYPE_REMAINDER_ASSIGN || expr_type == EXPR_TYPE_LEFT_SHIFT_ASSIGN || expr_type == EXPR_TYPE_RIGHT_SHIFT_ASSIGN ||
			expr_type == EXPR_TYPE_BIT_AND_ASSIGN || expr_type == EXPR_TYPE_BIT_XOR_ASSIGN || expr_type == EXPR_TYPE_BIT_OR_ASSIGN);

	SourceTreeNode* pRoot = new SourceTreeNode;
	pRoot->pChild = pRoot->pNext = NULL;
	pRoot->line_no = 0;
	pRoot->param = expr_type;

	pRoot->pChild = pLeft;
	MY_ASSERT(pLeft->pNext == NULL);
	pLeft->pNext = pRight;

	return pRoot;
}

//expr2			   : +[expr, ','];
SourceTreeVector expr2GetExprs(const SourceTreeNode* pRoot)
{
	SourceTreeVector ret_v;

	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
		ret_v.push_back(pRoot->pChild->pChild);

	return ret_v;
}

//simple_var_def		: type +[decl_c_obj_var, ',']
void simpleVarDefGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pType, int& var_count)
{
	pType = pRoot->pChild;
	pRoot = pRoot->pNext;

	var_count = pRoot->param;
}

void simpleVarDefGetVarByIndex(const SourceTreeNode* pRoot, int idx, bool& bObjVar, SourceTreeNode*& pChild)
{
	pRoot = pRoot->pNext;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);

	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;

	pRoot = pRoot->pChild;
	bObjVar = pRoot->param == 1;
	pChild = pRoot->pChild->pChild;
}

//class_base_init     : user_def_type_no_check '(' ?[expr] ')' ;
void classBaseInitGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pUserDefTypeOrMember, SourceTreeNode*& pExpr)
{
    pUserDefTypeOrMember = pRoot->pChild;
    pRoot = pRoot->pNext;

    pExpr = NULL;
    if (pRoot->param)
        pExpr = pRoot->pChild->pChild;
}

SourceTreeVector classBaseInitsGetList(const SourceTreeNode* pRoot)
{
    SourceTreeVector ret_v;

    for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
        ret_v.push_back(pRoot->pChild->pChild);

    return ret_v;
}

StatementType statementGetType(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return STATEMENT_TYPE_BREAK;
	case 1:
		return STATEMENT_TYPE_CONTINUE;
	case 2:
		return STATEMENT_TYPE_RETURN;
	case 3:
		return STATEMENT_TYPE_COMPOUND;
	case 4:
		return STATEMENT_TYPE_IF;
	case 5:
		return STATEMENT_TYPE_WHILE;
	case 6:
		return STATEMENT_TYPE_DO;
	case 7:
		return STATEMENT_TYPE_FOR;
	case 8:
		return STATEMENT_TYPE_SWITCH;
	case 9:
		return STATEMENT_TYPE_TRY;
	case 10:
		return STATEMENT_TYPE_FLOW_WAIT;
	case 11:
		return STATEMENT_TYPE_FLOW_FORK;
	case 12:
		return STATEMENT_TYPE_FLOW_TRY;
	case 13:
		return STATEMENT_TYPE___ASM__;
	case 14:
		return STATEMENT_TYPE_EXPR2;
    case 15:
        return STATEMENT_TYPE_DEF;
	}
	MY_ASSERT(false);
	return STATEMENT_TYPE_EXPR2;
}

SourceTreeNode* statementReturnGetExpr(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_RETURN);

	pRoot = pRoot->pChild;
	if (pRoot->param == 0)
		return NULL;
	return pRoot->pChild->pChild;
}

void* statementCompoundGetBracketBlock(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_COMPOUND);
	pRoot = pRoot->pChild;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	return pRoot->ptr;
}

void statementIfGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr, SourceTreeNode*& pStatement)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_IF);

	pRoot = pRoot->pChild;

	pExpr = pRoot->pChild;
	pRoot = pRoot->pNext;

	pStatement = pRoot->pChild;
}

int statementIfGetNumberOfElseIf(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_IF);

	pRoot = pRoot->pChild;

	return pRoot->pNext->pNext->param;
}

void statementIfGetElseIfByIndex(const SourceTreeNode* pRoot, int idx, SourceTreeNode*& pExpr, SourceTreeNode*& pStatement)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_IF);

	pRoot = pRoot->pChild->pNext->pNext;

	MY_ASSERT(idx >= 0 && idx < pRoot->param);

	int i;
	for (i = 0, pRoot = pRoot->pChild; i < idx; i++)
		pRoot = pRoot->pNext;

	pRoot = pRoot->pChild;
	pExpr = pRoot->pChild;
	pRoot = pRoot->pNext;
	pStatement = pRoot->pChild;
}

SourceTreeNode* statementIfGetElseStatement(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_IF);

	pRoot = pRoot->pChild->pNext->pNext->pNext;

	if (pRoot->param == 0)
		return NULL;

	return pRoot->pChild->pChild;
}

void statementWhileGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr, SourceTreeNode*& pStatement)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_WHILE);

	pRoot = pRoot->pChild;

	pExpr = pRoot->pChild;
	pRoot = pRoot->pNext;
	pStatement = pRoot->pChild;
}

void statementDoGetExprStatement(const SourceTreeNode* pRoot, SourceTreeNode*& pStatement, SourceTreeNode*& pExpr)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_DO);
	pRoot = pRoot->pChild;

	pStatement = pRoot->pChild;
	pRoot = pRoot->pNext;
	pExpr = pRoot->pChild;
}

//| ^O 'for' '(' ?[(expr2 | type +[decl_c_obj_var, ','])] ';' ^E ?[expr] ';' expr2 ')' statement
void statementForGetExprStatement(const SourceTreeNode* pRoot, bool& bExpr1IsDef, SourceTreeNode*& pExpr1, SourceTreeNode*& pExpr2, SourceTreeNode*& pExpr3, SourceTreeNode*& pStatement)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FOR);
	pRoot = pRoot->pChild;

	pExpr1 = NULL;
	if (pRoot->param)
	{
		SourceTreeNode* pNode = pRoot->pChild->pChild;
		if (pNode->param == 0)
		{
		    bExpr1IsDef = false;
            pExpr1 = pNode->pChild->pChild;
		}
		else
		{
            bExpr1IsDef = true;
		    pExpr1 = pNode->pChild;
		}
	}
	pRoot = pRoot->pNext;
	pExpr2 = pRoot->param ? pRoot->pChild->pChild : NULL;
	pRoot = pRoot->pNext;
	pExpr3 = pRoot->pChild;
	pRoot = pRoot->pNext;
	pStatement = pRoot->pChild;
}

SourceTreeNode* statementSwitchGetVar(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_SWITCH);
	pRoot = pRoot->pChild;

	return pRoot->pChild;
}

int statementSwitchGetNumOfCases(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_SWITCH);
	pRoot = pRoot->pChild;

	return pRoot->pNext->param;
}

SourceTreeVector statementSwitchGetCaseByIndex(const SourceTreeNode* pRoot, int idx, SourceTreeNode*& pExpr)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_SWITCH);

	pRoot = pRoot->pChild;

	SourceTreeVector ret_v;
	pRoot = pRoot->pNext->pChild;

	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;

	pRoot = pRoot->pChild;
	pExpr = pRoot->pChild;
	pRoot = pRoot->pNext;

	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
		ret_v.push_back(pRoot->pChild->pChild);

	return ret_v;
}

SourceTreeVector statementSwitchGetDefult(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_SWITCH);

	pRoot = pRoot->pChild;

	pRoot = pRoot->pNext->pNext;

	SourceTreeVector ret_v;
	if (pRoot->param)
	{
		pRoot = pRoot->pChild;
		for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
			ret_v.push_back(pRoot->pChild->pChild);
	}

	return ret_v;
}

SourceTreeNode* statementTryGetStatement(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_TRY);

	pRoot = pRoot->pChild;

	return pRoot->pChild;
}

int statementTryGetNumOfCatches(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_TRY);

	pRoot = pRoot->pChild;

	return pRoot->pNext->param;
}

void statementTryGetCatchByIndex(const SourceTreeNode* pRoot, int idx, SourceTreeNode*& pFuncParamsNode, SourceTreeNode*& pStatement)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_TRY);

	pRoot = pRoot->pChild;

	pRoot = pRoot->pNext;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);
	int i;
	for (i = 0, pRoot = pRoot->pChild; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;

	pFuncParamsNode = pRoot->pChild;
	pRoot = pRoot->pNext;

	pStatement = pRoot->pChild;
}

void statementFlowWaitGetExprs(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr1, SourceTreeNode*& pExpr2)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FLOW_WAIT);

	pRoot = pRoot->pChild;
	pExpr1 = pRoot->pChild;
	pRoot = pRoot->pNext;

	pExpr2 = pRoot->pChild;
}

/*void statementFlowSignalGetExprs(const SourceTreeNode* pRoot, SourceTreeNode*& pExpr1, SourceTreeNode*& pExpr2)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FLOW_SIGNAL);

	pRoot = pRoot->pChild;
	pExpr1 = pRoot->pChild;
	pRoot = pRoot->pNext;

	pExpr2 = pRoot->pChild;
}*/

SourceTreeNode* statementFlowForkGetStatement(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FLOW_FORK);

	pRoot = pRoot->pChild;
	return pRoot->pChild;
}

// 'flow_try' ^ statement +[ 'flow_catch' '(' expr ',' expr ')' statement ]
SourceTreeNode* statementFlowTryGetTryStatement(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FLOW_TRY);

	pRoot = pRoot->pChild;
	return pRoot->pChild;
}

int statementFlowTryGetCatchCount(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FLOW_TRY);

	pRoot = pRoot->pChild->pNext;
	return pRoot->param;
}

SourceTreeNode* statementFlowTryGetCatchAt(const SourceTreeNode* pRoot, int idx, SourceTreeNode*& expr1, SourceTreeNode*& expr2)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_FLOW_TRY);

	pRoot = pRoot->pChild->pNext;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);

	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++, pRoot = pRoot->pNext);
	pRoot = pRoot->pChild;

	expr1 = pRoot->pChild;
	pRoot = pRoot->pNext;

//printf("expr1=%s, param=%d\n", displaySourceTreeExpr(expr1).c_str(), pRoot->param);
	expr2 = pRoot->pChild;
	pRoot = pRoot->pNext;

	return pRoot->pChild;
}

//| ^O '__asm__' '(' const_value ':' const_value '(' expr ')' ')' ';'
void statementAsmGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pFirstValue, SourceTreeNode*& pSecondValue, SourceTreeNode*& pExpr)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE___ASM__);

	pRoot = pRoot->pChild;

	pFirstValue = pRoot->pChild;
	pRoot = pRoot->pNext;

	pSecondValue = pRoot->pChild;
	pRoot = pRoot->pNext;

	pExpr = pRoot->pChild;
	pRoot = pRoot->pNext;
}

SourceTreeNode* statementDefGetNode(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_DEF);

	pRoot = pRoot->pChild;
	return pRoot->pChild;
}

SourceTreeNode* statementExpr2GetNode(const SourceTreeNode* pRoot)
{
	MY_ASSERT(statementGetType(pRoot) == STATEMENT_TYPE_EXPR2);

	pRoot = pRoot->pChild;
	return pRoot->pChild;
}

DefType defGetType(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
    case 0:
        return DEF_TYPE_EMPTY;
	case 1:
		return DEF_TYPE_PRE_DECL;
	case 2:
		return DEF_TYPE_USING_NAMESPACE;
	case 3:
		return DEF_TYPE_TYPEDEF;
    case 4:
        return DEF_TYPE_VAR_DEF;
	case 5:
		return DEF_TYPE_FUNC_DECL;
	case 6:
		return DEF_TYPE_FUNC_VAR_DEF;
	case 7:
		return DEF_TYPE_TEMPLATE;
    case 8:
        return DEF_TYPE_EXTERN_TEMPLATE_CLASS;
    case 9:
        return DEF_TYPE_EXTERN_TEMPLATE_FUNC;
	}
	MY_ASSERT(false);
	return DEF_TYPE_EMPTY;
}

//defs				: class_struct_union token ';'
std::string defPreDeclGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_PRE_DECL);
	pRoot = pRoot->pChild;

	csu_type = csuGetType(pRoot->pChild);
	pRoot = pRoot->pNext;

	return pRoot->value;
}

//| ^O 'using' ?['namespace'] scope (token | 'operator' operator) ';'
TokenWithNamespace defUsingNamespaceGetInfo(const SourceTreeNode* pRoot, bool& bHasNamespace)
{
	pRoot = pRoot->pChild;

	bHasNamespace = (pRoot->param > 0);
	pRoot = pRoot->pNext;

	TokenWithNamespace twn = scopeGetInfo(pRoot->pChild);
    pRoot = pRoot->pNext->pChild;

    if (pRoot->param == 0)
        twn.addScope(pRoot->pChild->value, false);
    else
        twn.addScope("operator " + operatorGetString(pRoot->pChild->pChild), false);

    return twn;
}

// *[ext_modifier] 'typedef' ^ (super_type decl_var ?[attribute] | extended_type token '(' ^ func_params ')' | func_type ) ';'
TypeDefType defTypedefGetBasicInfo(const SourceTreeNode* pRoot, int& modifier_bits)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TYPEDEF);
	pRoot = pRoot->pChild;

	modifier_bits = 0;
	for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		modifier_bits |= extModifierGetBit(pNode->pChild->pChild);

	pRoot = pRoot->pNext;
	switch (pRoot->pChild->param)
	{
	case 0:
		return TYPEDEF_TYPE_DATA;
    case 1:
        return TYPEDEF_TYPE_DATA_MEMBER_PTR;
	case 2:
		return TYPEDEF_TYPE_FUNC;
	case 3:
		return TYPEDEF_TYPE_FUNC_PTR;
    case 4:
        return TYPEDEF_TYPE_TYPEOF;
	}
	MY_ASSERT(false);
	return TYPEDEF_TYPE_DATA;
}

void defTypedefDataGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pSuperType, SourceTreeNode*& pDeclVar, SourceTreeNode*& pAttribute)
{
	int modifier_bits;
	MY_ASSERT(defTypedefGetBasicInfo(pRoot, modifier_bits) == TYPEDEF_TYPE_DATA);
	pRoot = pRoot->pChild->pNext->pChild->pChild;

	pSuperType = pRoot->pChild;
	pRoot = pRoot->pNext;
	pDeclVar = pRoot->pChild;
	pRoot = pRoot->pNext;
	pAttribute = NULL;
	if (pRoot->param)
		pAttribute = pRoot->pChild->pChild;
}

//extended_type '(' scope '*' token ')'
void defTypedefDataMemberPtrGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedTypeNode, TokenWithNamespace& twn, std::string& name)
{
    int modifier_bits;
    MY_ASSERT(defTypedefGetBasicInfo(pRoot, modifier_bits) == TYPEDEF_TYPE_DATA_MEMBER_PTR);
    pRoot = pRoot->pChild->pNext->pChild->pChild;

    pExtendedTypeNode = pRoot->pChild;
    pRoot = pRoot->pNext;
    twn = scopeGetInfo(pRoot->pChild);
    pRoot = pRoot->pNext;
    name = pRoot->value;
}

void defTypedefFuncGetInfo(const SourceTreeNode* pRoot, std::string& name, SourceTreeNode*& pExtendedType, SourceTreeNode*& pFuncParamsNode)
{
	int modifier_bits;
	MY_ASSERT(defTypedefGetBasicInfo(pRoot, modifier_bits) == TYPEDEF_TYPE_FUNC);
	pRoot = pRoot->pChild->pNext->pChild->pChild;

	pExtendedType = pRoot->pChild;
	pRoot = pRoot->pNext;
	name = pRoot->value;
	pRoot = pRoot->pNext;
	pFuncParamsNode = pRoot->pChild;
}

SourceTreeNode* defTypedefFuncTypeGetInfo(const SourceTreeNode* pRoot)
{
	int modifier_bits;
	MY_ASSERT(defTypedefGetBasicInfo(pRoot, modifier_bits) == TYPEDEF_TYPE_FUNC_PTR);
	pRoot = pRoot->pChild->pNext->pChild->pChild;

	return pRoot->pChild;
}

void defTypedefTypeOfGetInfo(const SourceTreeNode* pRoot, bool& bType, SourceTreeNode*& pExtendedTypeOrExprNode, std::string& name)
{
    int modifier_bits;
    MY_ASSERT(defTypedefGetBasicInfo(pRoot, modifier_bits) == TYPEDEF_TYPE_TYPEOF);
    pRoot = pRoot->pChild->pNext->pChild->pChild;

    bType = (pRoot->param == 0);
    pExtendedTypeOrExprNode = pRoot->pChild->pChild->pChild;
    pRoot = pRoot->pNext;

    name = pRoot->value;
}

//| func_header ?['__asm' '(' const_value ')'] *[attribute] (?['=' '0'] ';' ^O | ?[':' +[token '(' ?[expr] ')', ','] ] { statement })
bool defFuncDeclIsFuncDef(const SourceTreeNode* pRoot)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_FUNC_DECL);

	pRoot = pRoot->pChild->pNext->pNext->pNext->pChild;

	return pRoot->param == 1;
}

void defFuncDeclGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pFuncHeaderNode, std::string& asm_string, SourceTreeVector& attribute_list, bool& bPureVirtual, void*& pClassBaseInitBlock, void*& bracket_block)
{
	SourceTreeNode* pNode;

	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_FUNC_DECL);
	pRoot = pRoot->pChild;

	pFuncHeaderNode = pRoot->pChild;
	pRoot = pRoot->pNext;

	asm_string = "";
	if (pRoot->param)
	{
		asm_string = pRoot->pChild->value;
		MY_ASSERT(!asm_string.empty());
	}
	pRoot = pRoot->pNext;

	for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		attribute_list.push_back(pNode->pChild->pChild);
	pRoot = pRoot->pNext;

	bracket_block = NULL;
	pClassBaseInitBlock = NULL;
	bPureVirtual = false;

	if (pRoot->pChild->param == 0)
	{
        pNode = pRoot->pChild->pChild;
        bPureVirtual = (pNode->param > 0);
	}
	else
	{
		pNode = pRoot->pChild->pChild;

	    if (pNode->param)
	    {
	        MY_ASSERT(pNode->pChild->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	        pClassBaseInitBlock = pNode->pChild->ptr;
	    }
	    pNode = pNode->pNext;

		MY_ASSERT(pNode->param == SOURCE_NODE_TYPE_BIG_BRACKET);
		bracket_block = pNode->ptr;
	}
}

//| *[var_modifier] func_type ';'
void defFuncVarDefGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, SourceTreeNode*& pFuncType)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_FUNC_VAR_DEF);
	pRoot = pRoot->pChild;

	modifier_bits = 0;
	for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		modifier_bits |= memberModifierGetBit(pNode->pChild->pChild);
	pRoot = pRoot->pNext;

	pFuncType = pRoot->pChild;
}

//func_type		: extended_type '(' '*' ?[token] ')' '(' func_params ')';
void defFuncVarDefChangeVarName(SourceTreeNode* pRoot, const std::string& old_name, const std::string& new_name)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_FUNC_VAR_DEF);

	pRoot = pRoot->pNext->pChild->pNext;
	MY_ASSERT(pRoot->param > 0);
	MY_ASSERT(pRoot->pChild->value == old_name);
	pRoot->pChild->value = new_name;
}

//| *[ext_modifier] super_type *[decl_c_obj_var, ','] ?[attribute] ';'
void defVarDefGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, SourceTreeNode*& pSuperType, SourceTreeNode*& pAttribute, int& numOfVars)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_VAR_DEF);
	pRoot = pRoot->pChild;

	modifier_bits = 0;
	for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		modifier_bits |= memberModifierGetBit(pNode->pChild->pChild);
	pRoot = pRoot->pNext;

	pSuperType = pRoot->pChild;
	pRoot = pRoot->pNext;

    numOfVars = pRoot->param;
    pRoot = pRoot->pNext;

	pAttribute = NULL;
	if (pRoot->param)
		pAttribute = pRoot->pChild->pChild;
	pRoot = pRoot->pNext;

}

void defVarDefGetVarByIndex(const SourceTreeNode* pRoot, int idx, bool& bObjVar, SourceTreeNode*& pChild)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_VAR_DEF);

	pRoot = pRoot->pChild;
	pRoot = pRoot->pNext->pNext;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);

	int i;
	for (i = 0, pRoot = pRoot->pChild; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;

	pRoot = pRoot->pChild;
	bObjVar = pRoot->param == 1;
	pChild = pRoot->pChild->pChild;
}

void defVarDefChangeVarName(SourceTreeNode* pRoot, const std::string& old_name, const std::string& new_name)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_VAR_DEF);

	pRoot = pRoot->pChild->pNext->pNext;
	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
	{
		if (declVarGetName(pRoot->pChild->pNext->pChild) == old_name)
		{
			declVarChangeName(pRoot->pChild->pNext->pChild, new_name);
			return;
		}
	}

	MY_ASSERT(false);
}

//| *[ext_modifier] super_type ^ ?[attribute] *[ ?['__restrict'] decl_var ?[ '=' ^ expr], ','] ';'
SourceTreeNode* defVarDefCreate(SourceTreeNode* pSuperType, SourceTreeNode* pDeclVar, SourceTreeNode* pInitExpr)
{
	SourceTreeNode *pRoot, *pNode;

	if (pDeclVar)
	{
		pNode = createEmptyNode();
		if (pInitExpr)
		{
			// expr
			pNode->param = 1;
			pNode->pChild = createEmptyNode();
			pNode->pChild->pChild = pInitExpr;
		}
		pRoot = pNode;

		// decl_var
		pNode = createEmptyNode();
		pNode->pChild = pDeclVar;
		pNode->pNext = pRoot;
		pRoot = pNode;

		// ?['_restrict']
		pNode = createEmptyNode();
		pNode->pNext = pRoot;
		pRoot = pNode;

		pNode = createEmptyNode();
		pNode->pChild = pRoot;

		// *
		pRoot = createEmptyNode();
		pRoot->param = 1;
		pRoot->pChild = pNode;
	}
	else
	{
		pRoot = createEmptyNode();
	}

	// ?[attribute]
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// type
	pNode = createEmptyNode();
	pNode->pChild = pSuperType;
	pNode->pNext = pRoot;
	pRoot = pNode;

	// ext_modifier
	pNode = createEmptyNode();
	pNode->pNext = pRoot;
	pRoot = pNode;

	// def
	pNode = createEmptyNode();
	pNode->param = DEF_TYPE_VAR_DEF;
	pNode->pChild = pRoot;
	pRoot = pNode;

	return pRoot;
}

//template_type_def   : ?['template' '<' +[template_type_def, ','] '>'] ('class' | 'typename') ?[&T token ?['=' ?['typename'] extended_type_var] ] | type ?[&V token] ?['=' expr];
void templateTypeDefGetInfo(const SourceTreeNode* pRoot, int& header_type, SourceTreeVector& templateTypeParams, bool& bClass, std::string& name, bool& bHasTypename, SourceTreeNode*& pDefaultNode)
{
    templateTypeParams.clear();
    pDefaultNode = NULL;
    bHasTypename = false;
    name = "";

    header_type = pRoot->param;
    pRoot = pRoot->pChild;
    if (header_type == 0)
    {
        if (pRoot->param)
        {
            SourceTreeNode* pNode = pRoot->pChild;
            for (pNode = pNode->pChild; pNode; pNode = pNode->pNext)
                templateTypeParams.push_back(pNode->pChild->pChild);
        }
        pRoot = pRoot->pNext;

        bClass = (pRoot->pChild->param == 0);
        pRoot = pRoot->pNext;

        if (pRoot->param)
        {
            pRoot = pRoot->pChild;
            name = pRoot->value;
            pRoot = pRoot->pNext;
            if (pRoot->param)
            {
                pRoot = pRoot->pChild;
                if (pRoot->param)
                  bHasTypename = true;
                pRoot = pRoot->pNext;
                pDefaultNode = pRoot->pChild;
            }
        }
    }
    else
    {
        MY_ASSERT(header_type == 1);
        templateTypeParams.push_back(pRoot->pChild);
        pRoot = pRoot->pNext;
        if (pRoot->param)
            name = pRoot->pChild->value;
        pRoot = pRoot->pNext;
        if (pRoot->param)
            pDefaultNode = pRoot->pChild->pChild;
    }
}

//template_header     : +['template' < *[template_type_def, ','] > ] ;
void defTemplateHeaderGetTypeByIndex(const SourceTreeNode* pRoot, int templateIdx, int typeIdx, SourceTreeNode*& pChild)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
    pRoot = pRoot->pChild->pChild;

    MY_ASSERT(templateIdx >= 0 && templateIdx < pRoot->param);
    pRoot = pRoot->pChild;
    for (int i = 0; i < templateIdx; i++)
        pRoot = pRoot->pNext;
    pRoot = pRoot->pChild;

    MY_ASSERT(typeIdx >= 0 && typeIdx < pRoot->param);
    pRoot = pRoot->pChild;
    for (int i = 0; i < typeIdx; i++)
        pRoot = pRoot->pNext;
    pRoot = pRoot->pChild;

    pChild = pRoot->pChild;
}

void defTemplateGetInfo(const SourceTreeNode* pRoot, TemplateType& template_type, std::vector<int>& typeCounts)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
    pRoot = pRoot->pChild;

    typeCounts.clear();
    for (SourceTreeNode* pNode = pRoot->pChild->pChild; pNode; pNode = pNode->pNext)
        typeCounts.push_back(pNode->pChild->param);
    pRoot = pRoot->pNext;

    switch (pRoot->pChild->param)
    {
    case 0:
        template_type = TEMPLATE_TYPE_FUNC;
        break;
    case 1:
        template_type = TEMPLATE_TYPE_CLASS;
        break;
    case 2:
        template_type = TEMPLATE_TYPE_VAR;
        break;
    case 3:
        template_type = TEMPLATE_TYPE_FRIEND_CLASS;
        break;
    default:
        MY_ASSERT(false);
    }
}

//| template (func_header ^O (';' | ?[ class_base_init ] %{ statement %}) || )
void defTemplateFuncGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pFuncHeaderNode, void*& pClassBaseInitBlock, void*& body_data)
{
	MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
	pRoot = pRoot->pChild->pNext->pChild;
	MY_ASSERT(pRoot->param == 0);
    pRoot = pRoot->pChild;

	pFuncHeaderNode = pRoot->pChild;
	pRoot = pRoot->pNext->pChild;

	body_data = NULL;
	pClassBaseInitBlock = NULL;
	if (pRoot->param == 1)
	{
		pRoot = pRoot->pChild;

		if (pRoot->param)
		{
	        MY_ASSERT(pRoot->pChild->param == SOURCE_NODE_TYPE_BIG_BRACKET);
		    pClassBaseInitBlock = pRoot->pChild->ptr;
		}
		pRoot = pRoot->pNext;

		MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
		body_data = pRoot->ptr;
	}
}

//| class_struct_union ^O scope token ?[ < +[(extended_type_var | func_type | expr), ','] > ] ?[ ?[ ':' %[base_class_defs, '{'] ] ^O '{' %[class_def_body, '}'] '}' ] ?[attribute] ';'
void defTemplateClassGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, TokenWithNamespace& twn, int& specializedTypeCount, void*& pBaseClassDefsBlock, void*& body_data)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
    pRoot = pRoot->pChild->pNext->pChild;
    MY_ASSERT(pRoot->param == 1);
    pRoot = pRoot->pChild;

    pBaseClassDefsBlock = NULL;
	body_data = NULL;

	csu_type = csuGetType(pRoot->pChild);
	pRoot = pRoot->pNext;

	twn = scopeGetInfo(pRoot->pChild);
    pRoot = pRoot->pNext;

	twn.addScope(pRoot->value);
	pRoot = pRoot->pNext;

	specializedTypeCount = 0;
	if (pRoot->param)
		specializedTypeCount = pRoot->pChild->param;
	pRoot = pRoot->pNext;

	if (pRoot->param)
	{
	    SourceTreeNode* pNode = pRoot->pChild;

		if (pNode->param)
		{
	        MY_ASSERT(pNode->pChild->param == SOURCE_NODE_TYPE_BIG_BRACKET);
			pBaseClassDefsBlock = pNode->pChild->ptr;
		}
		pNode = pNode->pNext;

		MY_ASSERT(pNode->param == SOURCE_NODE_TYPE_BIG_BRACKET);
		body_data = pNode->ptr;
	}
    pRoot = pRoot->pNext;

    // attribute?
}

//| *[var_modifier] ^O extended_type scope '::' %[decl_c_obj_var, ';'] ';' )
void defTemplateVarGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, SourceTreeNode*& pExtendedTypeNode, SourceTreeNode*& pScopeNode, void*& block_data)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
    pRoot = pRoot->pChild->pNext->pChild;
    MY_ASSERT(pRoot->param == 2);
    pRoot = pRoot->pChild;

    modifier_bits = 0;
    for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
        modifier_bits |= memberModifierGetBit(pNode->pChild->pChild);
    pRoot = pRoot->pNext;

    pExtendedTypeNode = pRoot->pChild;
    pRoot = pRoot->pNext;

    pScopeNode = pRoot->pChild;
    pRoot = pRoot->pNext;

    MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
    block_data = pRoot->ptr;
}

//| ^O 'friend' class_struct_union scope token ';' )
void defTemplateFriendClassGetInfo(const SourceTreeNode* pRoot, CSUType& csu_type, SourceTreeNode*& pScopeNode, std::string& className)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
    pRoot = pRoot->pChild->pNext->pChild;
    MY_ASSERT(pRoot->param == 3);
    pRoot = pRoot->pChild;

    csu_type = csuGetType(pRoot->pChild);
    pRoot = pRoot->pNext;

    pScopeNode = pRoot->pChild;
    pRoot = pRoot->pNext;

    className = pRoot->value;
    pRoot = pRoot->pNext;
}

//| ('class' | 'struct') ^O scope token ?[ < +[(extended_type | func_type | expr), ','] > ] (';' | ?[base_class_defs] ^O '{' %[class_def_body, '}'] '}' ';')
void defTemplateClassGetSpecializedTypeByIndex(const SourceTreeNode* pRoot, int idx, TemplateParamType& nParamType, SourceTreeNode*& pChildNode)
{
    DefType defType = defGetType(pRoot);
	MY_ASSERT(defType == DEF_TYPE_TEMPLATE);
	pRoot = pRoot->pChild->pNext->pChild;
	MY_ASSERT(pRoot->param == 1);
    pRoot = pRoot->pChild->pNext->pNext->pNext;
    MY_ASSERT(pRoot->param);
    pRoot = pRoot->pChild;

	MY_ASSERT(idx >= 0 && idx < pRoot->param);
	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;

	pRoot = pRoot->pChild;
	switch (pRoot->param)
	{
	case 0:
	    nParamType = TEMPLATE_PARAM_TYPE_DATA;
	    break;
    case 1:
        nParamType = TEMPLATE_PARAM_TYPE_FUNC;
        break;
    case 2:
        nParamType = TEMPLATE_PARAM_TYPE_VALUE;
        break;
    default:
        MY_ASSERT(false);
	}
	pChildNode = pRoot->pChild->pChild;
}

void defTemplateFuncGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_TEMPLATE);
    pRoot = pRoot->pChild->pNext->pChild;
    MY_ASSERT(pRoot->param == 0);
    pRoot = pRoot->pChild->pNext->pChild;
    MY_ASSERT(pRoot->param == 1);
    pRoot = pRoot->pChild;
    MY_ASSERT(pRoot->param > 0);
    pRoot = pRoot->pChild;

    MY_ASSERT(idx >= 0 && idx < pRoot->param);
    pRoot = pRoot->pChild;
    for (int i = 0; i < idx; i++)
        pRoot = pRoot->pNext;
    pRoot = pRoot->pChild;

    name = pRoot->value;
    pRoot = pRoot->pNext;

    pExpr = NULL;
    if (pRoot->param)
        pExpr = pRoot->pChild->pChild;
}

//| 'extern' 'template' (('class' | 'struct') user_def_type | func_header ) ';'
void defExternTemplateClassGetInfo(const SourceTreeNode* pRoot, bool& bClass, CSUType& csu_type, SourceTreeNode*& pUserDefTypeOrFuncHeader)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_EXTERN_TEMPLATE_CLASS);
    pRoot = pRoot->pChild;

    pRoot = pRoot->pChild;
    if (pRoot->param == 0)
    {
        bClass = true;
        pRoot = pRoot->pChild;
        csu_type = (pRoot->param == 0 ? CSU_TYPE_CLASS : CSU_TYPE_STRUCT);
        pRoot = pRoot->pNext;

        pUserDefTypeOrFuncHeader = pRoot->pChild;
    }
    else
    {
        bClass = false;
        pRoot = pRoot->pChild;
        pUserDefTypeOrFuncHeader = pRoot->pChild;
    }
}

//| 'extern' 'template' extended_type scope token < *[(extended_type_var | func_type | expr), ','] > '(' %[func_params, ')'] ')' ?[const] ';'
void defExternTemplateFuncGetInfo(const SourceTreeNode* pRoot, SourceTreeNode*& pExtendedReturnType, SourceTreeNode*& pScope, std::string& token, int& templateParamCount, void*& pFuncParamsBlock, bool& bConst)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_EXTERN_TEMPLATE_FUNC);
    pRoot = pRoot->pChild;

    pExtendedReturnType = pRoot->pChild;
    pRoot = pRoot->pNext;

    pScope = pRoot->pChild;
    pRoot = pRoot->pNext;

    token = pRoot->value;
    pRoot = pRoot->pNext;

    templateParamCount = pRoot->param;
    pRoot = pRoot->pNext;

    MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
    pFuncParamsBlock = pRoot->ptr;
    pRoot = pRoot->pNext;

    bConst = (pRoot->param > 0);
}

void defExternTemplateFuncGetParamByIndex(const SourceTreeNode* pRoot, int idx, TemplateParamType& nParamType, SourceTreeNode*& pChildNode)
{
    MY_ASSERT(defGetType(pRoot) == DEF_TYPE_EXTERN_TEMPLATE_FUNC);
    pRoot = pRoot->pChild->pNext->pNext->pNext;
    MY_ASSERT(idx >= 0 && idx < pRoot->param);

    pRoot = pRoot->pChild;
    for (int i = 0; i < idx; i++)
        pRoot = pRoot->pNext;
    pRoot = pRoot->pChild;

    pRoot = pRoot->pChild;
    switch (pRoot->param)
    {
    case 0:
        nParamType = TEMPLATE_PARAM_TYPE_DATA;
        break;
    case 1:
        nParamType = TEMPLATE_PARAM_TYPE_FUNC;
        break;
    case 2:
        nParamType = TEMPLATE_PARAM_TYPE_VALUE;
        break;
    default:
        MY_ASSERT(false);
    }
    pChildNode = pRoot->pChild->pChild;
}

BlockType blockGetType(const SourceTreeNode* pRoot)
{
	switch (pRoot->param)
	{
	case 0:
		return BLOCK_TYPE_EXTERN_BLOCK;
	case 1:
		return BLOCK_TYPE_NAMESPACE;
	case 2:
		return BLOCK_TYPE_DEF;
	}
	MY_ASSERT(false);
	return BLOCK_TYPE_DEF;
}

//start			  : 'extern' ('\"C\"' | '\"C++\"') { *[start] }
void blockExternGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, void*& bracket_block)
{
	MY_ASSERT(blockGetType(pRoot) == BLOCK_TYPE_EXTERN_BLOCK);
	pRoot = pRoot->pChild;

	modifier_bits = 0;
	if (pRoot->pChild->param == 0)
		modifier_bits |= MODBIT_EXTERN_C;
	else
		modifier_bits |= MODBIT_EXTERN_CPP;
	pRoot = pRoot->pNext;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	bracket_block = pRoot->ptr;
}

//| 'namespace' ?[token] ?[attribute] '{' ^ *[start] '}'
void blockNamespaceGetInfo(const SourceTreeNode* pRoot, std::string& name, SourceTreeNode* pAttribute, void*& bracket_block)
{
	MY_ASSERT(blockGetType(pRoot) == BLOCK_TYPE_NAMESPACE);
	pRoot = pRoot->pChild;

	name = "";
	if (pRoot->param)
		name = pRoot->pChild->value;
	pRoot = pRoot->pNext;

	pAttribute = NULL;
	if (pRoot->param)
		pAttribute = pRoot->pChild->pChild;
	pRoot = pRoot->pNext;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	bracket_block = pRoot->ptr;
}

SourceTreeNode* blockDefGetNode(const SourceTreeNode* pRoot)
{
	MY_ASSERT(blockGetType(pRoot) == BLOCK_TYPE_DEF);

	return pRoot->pChild->pChild;
}

/*//| *[func_modifier] func_header ?[const] ?[':' *[token '(' expr ')', ','] ] '{' ^ *[statement] '}'
void blockFuncGetInfo(const SourceTreeNode* pRoot, int& modifier_bits, SourceTreeNode*& pFuncHeaderNode, int& memberInitCount, void*& bracket_block)
{
	SourceTreeNode* pNode;

	MY_ASSERT(blockGetType(pRoot) == BLOCK_TYPE_FUNC_DEF);
	pRoot = pRoot->pChild;

	modifier_bits = 0;
	for (SourceTreeNode* pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		modifier_bits |= funcModifierGetBit(pNode->pChild->pChild);
	pRoot = pRoot->pNext;

	pFuncHeaderNode = pRoot->pChild;
	pRoot = pRoot->pNext;

	if (pRoot->param)
		modifier_bits |= MODBIT_CONST;
	pRoot = pRoot->pNext;

	if (pRoot->param)
		memberInitCount = pRoot->pChild->param;
	else
		memberInitCount = 0;
	pRoot = pRoot->pNext;

	MY_ASSERT(pRoot->param == SOURCE_NODE_TYPE_BIG_BRACKET);
	bracket_block = pRoot->ptr;
}

void blockFuncGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr)
{
	MY_ASSERT(blockGetType(pRoot) == BLOCK_TYPE_FUNC_DEF);
	pRoot = pRoot->pChild;

	pRoot = pRoot->pNext->pNext->pNext->pNext->pNext;
	MY_ASSERT(pRoot->param != 0);
	pRoot = pRoot->pChild;
	MY_ASSERT(idx >= 0 && idx < pRoot->param);
	pRoot = pRoot->pChild;
	for (int i = 0; i < idx; i++)
		pRoot = pRoot->pNext;
	pRoot = pRoot->pChild;
	name = pRoot->value;
	pRoot = pRoot->pNext;
	pExpr = pRoot->pChild;
}*/

//==========================================================
// print nodes
void printSourceTree(const SourceTreeNode* pSourceNode)
{
	printf("%s", pSourceNode->value.c_str());
	printf(":%d", pSourceNode->param);

	if (pSourceNode->pChild)
	{
		printf("(");
		printSourceTree(pSourceNode->pChild);
		printf(")");
	}

	if (pSourceNode->pNext)
	{
		printf(", ");
		printSourceTree(pSourceNode->pNext);
	}
}

std::string printTabs(int depth)
{
	std::string ret_s;

	for (int i = 0; i < depth; i++)
		ret_s += "\t";

	return ret_s;
}
// 'const' | '__const'
std::string displaySourceTreeConst(const SourceTreeNode* pRoot)
{
	if (pRoot->param == 0)
	  	return "const ";
	return "__const ";
}

//token_with_namespace: ?['::'] *[ token '::'] token ;
std::string displaySourceTreeTokenWithNamespace(const SourceTreeNode* pRoot)
{
	TokenWithNamespace twn = tokenWithNamespaceGetInfo(pRoot);

	return twn.toString();
}

//basic_type		  : 'void' | 'va_list' | '__builtin_va_list' | *[('signed' | 'unsigned' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double' | 'bool' | 'wchar_t')];
std::string displaySourceTreeBasicType(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nBasicType: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	SourceTreeNode* pNode;

	switch (pRoot->param)
	{
	case 0:
		ret_s += "void ";
		break;

	case 1:
		ret_s += "va_list ";
		break;

	case 2:
		ret_s += "__builtin_va_list ";
		break;

	case 3:
		pRoot = pRoot->pChild;
		for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		{
			switch (pNode->pChild->pChild->param)
			{
			case 0:
				ret_s += "signed ";
				break;
			case 1:
				ret_s += "unsigned ";
				break;
			case 2:
				ret_s += "char ";
				break;
			case 3:
				ret_s += "short ";
				break;
			case 4:
				ret_s += "int ";
				break;
			case 5:
				ret_s += "long ";
				break;
			case 6:
				ret_s += "float ";
				break;
			case 7:
				ret_s += "double ";
				break;
            case 8:
                ret_s += "__int128 ";
                break;
			case 9:
				ret_s += "bool ";
				break;
			case 10:
				ret_s += "wchar_t ";
				break;
			default:
				MY_ASSERT(false);
			}
		}
	}

	return ret_s.substr(0, ret_s.size() - 1);
}

std::string displayCSUType(CSUType csu_type)
{
	std::string ret_s;

	switch (csu_type)
	{
	case CSU_TYPE_NONE:
		ret_s = "";
		break;
	case CSU_TYPE_UNION:
		ret_s = "union ";
		break;
	case CSU_TYPE_STRUCT:
		ret_s = "struct ";
		break;
	case CSU_TYPE_CLASS:
		ret_s = "class ";
		break;
	case CSU_TYPE_ENUM:
		ret_s = "enum ";
		break;
	default:
		MY_ASSERT(false);
	}
	return ret_s;
}

std::string displayCAMType(ClassAccessModifierType cam_type)
{
	std::string ret_s;

	switch (cam_type)
	{
	case CAM_TYPE_NONE:
		ret_s = "";
		break;
	case CAM_TYPE_PUBLIC:
		ret_s = "public";
		break;
	case CAM_TYPE_PROTECTED:
		ret_s = "protected";
		break;
	case CAM_TYPE_PRIVATE:
		ret_s = "private";
		break;
	default:
		MY_ASSERT(false);
	}
	return ret_s;
}

//type				: *[data_type_modifier] (basic_type ^ | ?[class_struct_union] user_def_type ^) ?[const];
std::string displaySourceTreeType(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nType: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	ret_s += displayPrefixModifiers(typeGetModifierBits(pRoot));

	switch (typeGetType(pRoot))
	{
	case BASICTYPE_TYPE_BASIC:
		ret_s += displaySourceTreeBasicType(typeBasicGetInfo(pRoot));
		break;

	case BASICTYPE_TYPE_USER_DEF:
	{
		SourceTreeNode* pUserDef;
		bool bHasTypename;
		CSUType csu_type = typeUserDefinedGetInfo(pRoot, bHasTypename, pUserDef);
		ret_s += displayCSUType(csu_type);
		if (bHasTypename)
		    ret_s += "typename ";
		ret_s += displaySourceTreeUserDefType(pUserDef);
		break;
	}
    case BASICTYPE_TYPE_TYPEOF:
        ret_s += displaySourceTreeTokenWithNamespace(typeTypeOfGetInfo(pRoot));
        break;

    case BASICTYPE_TYPE_DATA_MEMBER_POINTER:
    {
        SourceTreeNode* pExtendedTypeNode;
        TokenWithNamespace twn;
        typeDmpGetInfo(pRoot, pExtendedTypeNode, twn);
        ret_s += displaySourceTreeExtendedType(pExtendedTypeNode) + " " + twn.toString() + "::*" ;
        break;
    }
	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

std::string displaySourceTreeUserDefType(const SourceTreeNode* pRoot)
{
	TokenWithNamespace twn = userDefTypeGetInfo(pRoot);

	return twn.toString();
}

//enum_def			: 'enum' ^ ?[token] '{' *[token ?['=' ^ expr ], ','] '}';
std::string displaySourceTreeEnumDef(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	std::string name;
	int children_count;
	enumDefGetInfo(pRoot, name, children_count);

	ret_s += "enum ";
	if (!name.empty())
		ret_s += name + " ";
	ret_s += "{\n";

	for (int i = 0; i < children_count; i++)
	{
		std::string key;
		SourceTreeNode* pExpr;
		enumDefGetChildByIndex(pRoot, i, key, pExpr);

		ret_s += printTabs(depth + 1) + key;
		if (pExpr)
			ret_s += " = " + displaySourceTreeExpr(pExpr);
		if (i < children_count - 1)
			ret_s += ",";
		ret_s += "\n";
	}
	ret_s += printTabs(depth) + "}";

	return ret_s;
}

//union_def		   : 'union' ^ ?[token] '{' *[defs] '}';
std::string displaySourceTreeUnionDef(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	MY_ASSERT(false);
	std::string name;
	SourceTreeVector def_list;
	//unionDefGetInfo(pRoot, name, def_list);

	ret_s += "unon ";
	if (!name.empty())
		ret_s += name + " ";
	ret_s += "{\n";

	for (int i = 0; i < def_list.size(); i++)
	{
		ret_s += printTabs(depth + 1) + displaySourceTreeDefs(def_list[i], depth + 1);
		ret_s += "\n";
	}
	ret_s += printTabs(depth) + "}";

	return ret_s;
}

std::string displayPrefixModifiers(int modifier)
{
	std::string ret_s;

	if (modifier & MODBIT___EXTENSION__)
		ret_s += "__extension__ ";
	if (modifier & MODBIT_EXTERN)
		ret_s += "extern ";
	if (modifier & MODBIT_EXTERN_C)
		ret_s += "extern \"C\" ";
	if (modifier & MODBIT_EXTERN_CPP)
		ret_s += "extern \"C++\" ";
	if (modifier & MODBIT_VIRTUAL)
		ret_s += "virtual ";
	if (modifier & MODBIT_STATIC)
		ret_s += "static ";
    if (modifier & MODBIT_CONST)
        ret_s += "const ";
    if (modifier & MODBIT_VOLATILE)
        ret_s += "volatile ";
    if (modifier & MODBIT_FRIEND)
        ret_s += "friend ";
    if (modifier & MODBIT_MUTABLE)
        ret_s += "mutable ";
    if (modifier & MODBIT_EXPLICIT)
        ret_s += "explicit ";
    if (modifier & MODBIT_INLINE)
        ret_s += "inline ";
	if (modifier & MODBIT_FLOW)
		ret_s += "flow ";
	if (modifier & MODBIT_FLOW_ROOT)
		ret_s += "flow_root ";

	return ret_s;
}

//class_def_body	  : class_access_modifier ':' \\\n\
					| defs \\\n\
					| *[func_modifier] ?[extended_type] ?['~'] token '(' func_params ')' ?[const] ?[':' +[token '(' expr ')', ','] ] '{' ^ *[statement] '}' \\\n\
					;
std::string displaySourceTreeClassDefBody(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	switch (cdbGetType(pRoot))
	{
	case CDB_TYPE_CAM:
	{
		ret_s += displayCAMType(cdbCamGetType(pRoot)) + ":\n";
		break;
	}
	case CDB_TYPE_FRIEND:
	    MY_ASSERT(false);
	    break;

	case CDB_TYPE_DEFS:
	{
		ret_s += displaySourceTreeDefs(cdbDefGetNode(pRoot), depth + 1) + "\n";
		break;
	}
	/*case CDB_TYPE_FUNC_DEF:
	{
		MY_ASSERT(false);
		int func_modifier;
		SourceTreeNode* pFuncHeaderNode, *pReturnExtendedType;
		SourceTreeVector paramVector;
		int memberInitCount;
		void* bracket_block;
		cdbFuncDefGetInfo(pRoot, func_modifier, pFuncHeaderNode, memberInitCount, bracket_block);
		ret_s += displayPrefixModifiers(func_modifier);
		if (pReturnExtendedType)
			ret_s += displaySourceTreeExtendedType(pReturnExtendedType) + " ";
		ret_s += func_name + "(" + displaySourceTreeFuncParams(pFuncParams) + ")";
		if (memberInitCount > 0)
		{
			ret_s += " : ";
			for (int i = 0; i < memberInitCount; i++)
			{
				std::string name;
				SourceTreeNode* pExpr;
				cdbFuncDefGetMemberInitByIndex(pRoot, i, name, pExpr);
				ret_s += name + "(" + displaySourceTreeExpr(pExpr) + ")";
				if (i < memberInitCount - 1)
					ret_s += ", ";
			}
		}
		ret_s += "\n" + printTabs(depth) + "{\n";
		for (int i = 0; i < statements.size(); i++)
			ret_s += displaySourceTreeStatement(statements[i], depth + 1);
		ret_s += printTabs(depth) + "}\n";
		break;
	}*/
	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

//class_def		   : class_struct_union ?[token] ?[':' +[ ?[class_access_modifier] user_def_type, ','] ] '{' *[class_def_body] '}' ;
std::string displaySourceTreeClassDef(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	CSUType csu_type;
	std::string name;
	int parentDefCount;
	SourceTreeVector classDefBodyList;
	MY_ASSERT(false);
	//classDefGetInfo(pRoot, csu_type, name, parentDefCount, classDefBodyList);

	ret_s += displayCSUType(csu_type);
	if (!name.empty())
		ret_s += name + " ";
	if (parentDefCount > 0)
	{
		ret_s += " : ";
		for (int i = 0; i < parentDefCount; i++)
		{
			ClassAccessModifierType cam_type;
			SourceTreeNode* pUserDefTypeNode;
			//classDefGetParentDefByIndex(pRoot, i, cam_type, pUserDefTypeNode);
			if (cam_type != CAM_TYPE_NONE)
				ret_s += displayCAMType(cam_type) + " ";
			ret_s + displaySourceTreeUserDefType(pUserDefTypeNode);
			if (i < parentDefCount - 1)
				ret_s += ", ";
		}
	}
	ret_s += "\n" + printTabs(depth) + "{\n";
	for (int i = 0; i < classDefBodyList.size(); i++)
		ret_s += displaySourceTreeClassDefBody(classDefBodyList[i], depth + 1);
	ret_s += printTabs(depth) + "}";

	return ret_s;
}

//super_type		  : type | enum_def | union_def | class_def ;
std::string displaySourceTreeSuperType(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	SuperTypeType super_type;
	SourceTreeNode* pChildNode;
	superTypeGetInfo(pRoot, super_type, pChildNode);
	switch (super_type)
	{
	case SUPERTYPE_TYPE_TYPE:
		ret_s = displaySourceTreeType(pChildNode, depth);
		break;
	case SUPERTYPE_TYPE_ENUM_DEF:
		ret_s = displaySourceTreeEnumDef(pChildNode, depth);
		break;
	case SUPERTYPE_TYPE_UNION_DEF:
		ret_s = displaySourceTreeUnionDef(pChildNode, depth);
		break;
	case SUPERTYPE_TYPE_CLASS_DEF:
		ret_s = displaySourceTreeClassDef(pChildNode, depth);
		break;
	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

// extended_type	   : type *[?[const] '*'] ?[const] ?['&'];
std::string displaySourceTreeExtendedType(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nType: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	ret_s += displaySourceTreeType(extendedTypeGetTypeNode(pRoot), depth);

	for (int i = 0; i < extendedTypeGetDepth(pRoot); i++)
	{
		if (extendedTypePointerIsConst(pRoot, i))
			ret_s += " const";
		ret_s += "*";
	}

	if (extendedTypeIsConst(pRoot))
	    ret_s += " const";
    if (extendedTypeIsVolatile(pRoot))
        ret_s += " volatile";
    if (extendedTypeIsReference(pRoot))
        ret_s += "&";

	return ret_s;
}

// extended_type_var   : extended_type *['[' ?[expr] ']'];
std::string displaySourceTreeExtendedTypeVar(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nType: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	ret_s += displaySourceTreeExtendedType(pRoot->pChild);
	pRoot = pRoot->pNext;

	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
	{
		ret_s += "[";
		if (pRoot->pChild->param)
			ret_s += displaySourceTreeExpr(pRoot->pChild->pChild->pChild);
		ret_s += "]";
	}

	return ret_s;
}

std::string displaySourceTreeExtendedOrFuncType(const SourceTreeNode* pRoot)
{
    bool bExtendedType;
    SourceTreeNode* pChild;
    extendedOrFuncTypeGetInfo(pRoot, bExtendedType, pChild);
    if (bExtendedType)
        return displaySourceTreeExtendedType(pChild);

    return displaySourceTreeFuncType(pChild);
}

//decl_var		  : *[?[const] '*'] ?[const] ?['__restrict'] ?['('] ?['&'] ?[token] ?[')'] ?[':' const_value] ?['[' ']'] *['[' expr ']'];
std::string displaySourceTreeDeclVar(const SourceTreeNode* pRoot, const std::string& name, std::vector<std::string> str_v)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nDeclVar: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	if (!pRoot)
		return name == " " ? "" : name;

	SourceTreeNode* pNode;

	for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
	{
		if (pNode->pChild->param)
			ret_s += "const";
		ret_s += "*";
	}
	pRoot = pRoot->pNext;

	if (pRoot->param)
		ret_s += displaySourceTreeConst(pRoot->pChild->pChild);
	pRoot = pRoot->pNext;

	if (pRoot->param)
		ret_s += " __restrict";
	pRoot = pRoot->pNext;

	if (pRoot->param)
		ret_s += "(";
	pRoot = pRoot->pNext;

	if (pRoot->param)
		ret_s += "&";
	pRoot = pRoot->pNext;

	if (name.empty())
	{
		if (pRoot->param)
			ret_s += pRoot->pChild->value;
	}
	else
		ret_s += name == " " ? "" : name;
	pRoot = pRoot->pNext;

	if (pRoot->param)
		ret_s += ")";
	pRoot = pRoot->pNext;

	if (pRoot->param)
	{
		ret_s += ":" + pRoot->pChild->value;
	}
	pRoot = pRoot->pNext;

	if (pRoot->param)
		ret_s += "[]";
	pRoot = pRoot->pNext;

	int i = 0;
	for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext, i++)
	{
		ret_s += "[";
		ret_s += (str_v.empty() ? displaySourceTreeExpr(pNode->pChild->pChild) : str_v[i]);
		ret_s += "]";
	}

	return ret_s;
}

std::string displaySourceTreeDeclCVar(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	bool bRestrict;
	SourceTreeNode* pDeclVar, *pInitValue;
	declCVarGetInfo(pRoot, bRestrict, pDeclVar, pInitValue);
	if (bRestrict)
		ret_s += "__restrict ";
	ret_s += displaySourceTreeDeclVar(pDeclVar);
	if (pInitValue)
		ret_s += " = " + displaySourceTreeExpr(pInitValue);

	return ret_s;
}

std::string displaySourceTreeDeclObjVar(const SourceTreeNode* pRoot)
{
	std::string name;
	SourceTreeVector exprList;
	declObjVarGetInfo(pRoot, name, exprList);

	std::string ret_s = name + "(";
	for (int i = 0; i < exprList.size(); i++)
	{
		if (i > 0)
			ret_s += ", ";
		ret_s += displaySourceTreeExpr(exprList[i]);
	}

	return ret_s += ")";
}

//attribute		   : '__attribute__' '(' '(' *[(token | token '(' *[(token | const_value | token '=' const_value), ','] ')'), ','] ')' ')';
std::string displaySourceTreeAttribute(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nAttribute: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	ret_s += "__attribute__ ((";

	SourceTreeNode* pNode;
	for (pRoot = pRoot->pChild; pRoot; pRoot = pRoot->pNext)
	{
		pNode = pRoot->pChild->pChild;
		if (pNode->param == 0)
		{
			ret_s += pNode->pChild->value;
		}
		else
		{
			pNode = pNode->pChild;
			ret_s += pNode->value + "(";
			pNode = pNode->pNext;
			for (pNode = pNode->pChild; pNode; pNode = pNode->pNext)
			{
				if (pNode->pChild->pChild->param == 0)
					ret_s += pNode->pChild->pChild->pChild->value;
				else if (pNode->pChild->pChild->param == 1)
					ret_s += pNode->pChild->pChild->pChild->value;
				else
					ret_s += pNode->pChild->pChild->pChild->value + "=" + pNode->pChild->pChild->pChild->pNext->value;

				if (pNode->pNext)
					ret_s += ", ";
			}
			ret_s += ")";
		}

		if (pRoot->pNext)
			ret_s += ", ";
	}

	ret_s += "))";
	return ret_s;
}

// type ?['__restrict'] ?[const] ?[ decl_var ?[ '=' expr ] ] | func_type
std::string displaySourceTreeFuncParam(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nFuncParams: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	FuncParamType param_type;
	int modifierBits;
	SourceTreeNode* pTypeNode, *pDeclVarNode, *pInitExprNode;
	funcParamGetInfo(pRoot, param_type, modifierBits, pTypeNode, pDeclVarNode, pInitExprNode);

	switch (param_type)
	{
	case FUNC_PARAM_TYPE_REGULAR:
		ret_s += displaySourceTreeType(pTypeNode);
		if (modifierBits & MODBIT___RESTRICT)
			ret_s += " __restrict";

		if (modifierBits & MODBIT_CONST)
			ret_s += " const";

		if (pDeclVarNode)
		{
			ret_s += " " + displaySourceTreeDeclVar(pDeclVarNode);
			if (pInitExprNode)
				ret_s += " = " + displaySourceTreeExpr(pInitExprNode);
		}
		break;

	case FUNC_PARAM_TYPE_FUNC:
		ret_s += displaySourceTreeFuncType(pTypeNode);
		break;

	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

std::string displaySourceTreeFuncParams(const SourceTreeNode* pRoot)
{
    std::string ret_s;

    SourceTreeVector func_v = funcParamsGetList(pRoot);
    for (unsigned i = 0; i < func_v.size(); i++)
    {
        if (i > 0)
            ret_s += ", ";

        ret_s += displaySourceTreeFuncParam(func_v[i]);
    }

    if (funcParamsHasVArgs(pRoot))
        ret_s += "...";

    return ret_s;
}

// extended_type '(' '*' ?[token] ')' '(' *[func_param] ')'
std::string displaySourceTreeFuncType(const SourceTreeNode* pRoot, bool bSkipFuncName)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nfunc_type: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	SourceTreeNode* pReturnExtendedType, *pScope, *pOptFuncParamsNode, *pFuncParamsNode;
	std::string name;
	int modifier_bits;
	funcTypeGetInfo(pRoot, pReturnExtendedType, pScope, name, pOptFuncParamsNode, pFuncParamsNode, modifier_bits);

	ret_s += displaySourceTreeExtendedType(pReturnExtendedType) + " ";

	if (pScope)
	{
        TokenWithNamespace twn = scopeGetInfo(pScope);
        twn.addScope("*" + (bSkipFuncName ? "" : name));
        ret_s += "(" + twn.toString();
        if (pOptFuncParamsNode)
        	ret_s += "(" + displaySourceTreeFuncParams(pOptFuncParamsNode) + ")";
        ret_s += ")";
	}
	ret_s += "(" + displaySourceTreeFuncParams(pFuncParamsNode) + ") ";

    ret_s += displayPrefixModifiers(modifier_bits);

	return ret_s;
}

std::string displaySourceTreeFuncHeader(const SourceTreeNode* pRoot)
{
	MY_ASSERT(false);
	return "";
}

//expr			 	:@1 expr '.' token |@1 expr '->' token  \\\n\
					|@2 expr '(' *[expr, ','] ')' |@2 expr '[' expr ']' \\\n\
					|@3 expr '++' |#3 expr '--' |@3 '++' expr |@3 '--' expr |@3 '+' expr |@3 '-' expr |#3 '!' expr |@3 '~' expr |@3 '(' extended_type ')' expr \\\n\
					|@3 '*' expr |@3 '&' expr |@3 'sizeof' '(' (extended_type_var | expr) ')' |@3 'new' extended_type_var |#3 'delete' expr ?['[' ']'] \\\n\
					|@5 expr '*' expr |@5 expr '/' expr |@5 expr '%' expr \\\n\
					|@6 expr '+' expr |#6 expr '-' expr \\\n\
					|@7 expr '<<' expr |@7 expr '>>' expr \\\n\
					|@8 expr '<' expr |@8 expr '<=' expr |@8 expr '>' expr |@8 expr '>=' expr \\\n\
					|@9 expr '==' expr |@9 expr '!=' expr \\\n\
					|@10 expr '&' expr	\\\n\
					|@11 expr '^' expr	\\\n\
					|@12 expr '|' expr	\\\n\
					|@13 expr '&&' expr   \\\n\
					|#14 expr '||' expr   \\\n\
					|@15 expr '?' expr ':' expr |@15 expr '=' expr |@15 expr '+=' expr |@15 expr '-=' expr |#15 expr '*=' expr |@15 expr '/=' expr |@15 expr '%=' expr |@15 expr '<<=' expr |@15 expr '>>=' expr |@15 expr '&=' expr |@15 expr '^=' expr |@15 expr '|=' expr \\\n\
					|@16 'throw' expr	 \\\n\
					|@17 expr ',' expr	\\\n\
					| const_value | token_with_namespace | '(' expr ')' \\\n\
					;
std::string displaySourceTreeExpr(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	if (g_source_tree_log)
	{
		printf("\nExpr: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	switch (pRoot->param)
	{
	case EXPR_TYPE_REF_ELEMENT: // expr ['~'] token ['(' expr2 ')']
	{
	    SourceTreeNode* pExpr, *pExpr2;
	    std::string token;
	    exprPtrRefGetInfo(pRoot, pExpr, token);
		ret_s += displaySourceTreeExpr(pExpr) + ".";
		ret_s += token;
		/*if (pExpr2)
		{
	        SourceTreeVector exprList = exprGetExprList(pRoot);
	        ret_s += "(";
	        for (int i = 0; i < exprList.size(); i++)
	        {
	            if (i > 0)
	                ret_s += ", ";
	            ret_s += displaySourceTreeExpr(exprList[i]);
	        }
	        ret_s += ")";
		}*/
		break;
	}
	case EXPR_TYPE_PTR_ELEMENT: // expr ['~'] token ['(' expr2 ')']
	{
        SourceTreeNode* pExpr, *pExpr2;
        std::string token;
        exprPtrRefGetInfo(pRoot, pExpr, token);
        ret_s += displaySourceTreeExpr(pExpr) + "->";
        ret_s += token;
        /*if (pExpr2)
        {
            SourceTreeVector exprList = exprGetExprList(pExpr2);
            ret_s += "(";
            for (int i = 0; i < exprList.size(); i++)
            {
                if (i > 0)
                    ret_s += ", ";
                ret_s += displaySourceTreeExpr(exprList[i]);
            }
            ret_s += ")";
        }*/
		break;
	}
	case EXPR_TYPE_FUNC_CALL:
	{
		ret_s += displaySourceTreeExpr(exprGetFirstNode(pRoot));
		SourceTreeVector exprList = exprGetExprList(pRoot);
		ret_s += "(";
		for (int i = 0; i < exprList.size(); i++)
		{
			if (i > 0)
				ret_s += ", ";
			ret_s += displaySourceTreeExpr(exprList[i]);
		}
		ret_s += ")";
		break;
	}
    case EXPR_TYPE_OPERATOR_CALL:
    {
        TokenWithNamespace twn = scopeGetInfo(exprGetFirstNode(pRoot));
        twn.addScope("operator " + operatorGetString(exprGetSecondNode(pRoot)), false);
        SourceTreeNode* pParamNode = exprGetOptionalThirdNode(pRoot);
        ret_s += twn.toString() + "(" + (pParamNode ? displaySourceTreeExpr(pParamNode) : "") + ")";
        break;
    }
	case EXPR_TYPE_ARRAY:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "[";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		ret_s += "]";
		break;
	case EXPR_TYPE_RIGHT_INC:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "++";
		break;
	case EXPR_TYPE_RIGHT_DEC:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "--";
		break;
	case EXPR_TYPE_LEFT_INC:
		ret_s += "++";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_LEFT_DEC:
		ret_s += "--";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_POSITIVE:
		ret_s += "+";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_NEGATIVE:
		ret_s += "-";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_NOT:
		ret_s += "!";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_XOR:
		ret_s += "~";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_TYPE_CAST:
		ret_s += "(";
		ret_s += displaySourceTreeExtendedOrFuncType(exprGetFirstNode(pRoot));
		ret_s += ")";
		ret_s += displaySourceTreeExpr(exprGetSecondNode(pRoot));
		break;
	case EXPR_TYPE_INDIRECTION:
		ret_s += "*";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_ADDRESS_OF:
		ret_s += "&";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_SIZEOF:
	{
		ret_s += "sizeof(";
		int nType;
		SourceTreeNode* pChild;
		exprSizeOfGetInfo(pRoot, nType, pChild);
		if (nType == 0)
			ret_s += displaySourceTreeExtendedTypeVar(pChild);
		else if (nType == 1)
            ret_s += displaySourceTreeFuncType(pChild);
		else
			ret_s += displaySourceTreeExpr(pChild);
		ret_s += ")";
		break;
	}
	case EXPR_TYPE_NEW_C: // scope 'new' extended_type_var
	{
	    TokenWithNamespace twn = scopeGetInfo(exprGetFirstNode(pRoot));
	    twn.addScope("new", false);
	    ret_s += twn.toString() + " " + displaySourceTreeExtendedTypeVar(exprGetSecondNode(pRoot));
		break;
	}
	case EXPR_TYPE_NEW_OBJECT: // scope 'new' user_def_type '(' expr2 ')'
	{
        TokenWithNamespace twn = scopeGetInfo(exprGetFirstNode(pRoot));
        twn.addScope("new", false);
        ret_s += twn.toString() + " " + displaySourceTreeUserDefType(exprGetSecondNode(pRoot));

		SourceTreeVector exprList = expr2GetExprs(exprGetThirdNode(pRoot));
		ret_s += "(";
		for (int i = 0; i < exprList.size(); i++)
		{
			if (i > 0)
				ret_s += ", ";
			ret_s += displaySourceTreeExpr(exprList[i]);
		}
		ret_s += ")";
		break;
	}
    case EXPR_TYPE_NEW_ADV: // scope 'new' (expr) ?[ user_def_type ?[ '(' expr2 ')' ] ]
    {
        TokenWithNamespace twn = scopeGetInfo(exprGetFirstNode(pRoot));
        twn.addScope("new", false);
        ret_s += twn.toString() + "(" + displaySourceTreeExpr(exprGetSecondNode(pRoot)) + ")";

        SourceTreeNode* pUserDefType, *pExpr2;
        exprNewAdvGetParams(pRoot, pUserDefType, pExpr2);
        if (pUserDefType)
        {
            ret_s += " " + displaySourceTreeUserDefType(pUserDefType);
            if (pExpr2)
            {
                SourceTreeVector exprList = expr2GetExprs(pExpr2);
                ret_s += "(";
                for (int i = 0; i < exprList.size(); i++)
                {
                    if (i > 0)
                        ret_s += ", ";
                    ret_s += displaySourceTreeExpr(exprList[i]);
                }
                ret_s += ")";
            }
        }
        break;
    }
	case EXPR_TYPE_DELETE: // scope 'delete' ?['[' ']'] expr
	{
        TokenWithNamespace twn = scopeGetInfo(exprGetFirstNode(pRoot));
        twn.addScope("delete", false);
        ret_s += twn.toString();
        if (exprDeleteHasArray(pRoot))
			ret_s += "[]";
        ret_s += " " + displaySourceTreeExpr(exprGetThirdNode(pRoot));
		break;
	}
	case EXPR_TYPE_MULTIPLE:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "*";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_DIVIDE:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "/";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_REMAINDER:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "%%";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_ADD:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "+";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_SUBTRACT:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "-";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_LEFT_SHIFT:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "<<";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_RIGHT_SHIFT:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += ">>";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_LESS_THAN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "<";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_LESS_EQUAL:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "<=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_GREATER_THAN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += ">";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_GREATER_EQUAL:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += ">=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_EQUAL:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "==";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_NOT_EQUAL:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "!=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_BIT_AND:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "&";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_BIT_XOR:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "^";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_BIT_OR:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "|";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_AND:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "&&";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_OR:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "||";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_TERNARY:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "?";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		ret_s += ":";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pNext->pChild);
		break;
	case EXPR_TYPE_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_ADD_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "+=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_SUBTRACT_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "-=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_MULTIPLE_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "*=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_DIVIDE_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "/=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_REMAINDER_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "%%=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_LEFT_SHIFT_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "<<=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_RIGHT_SHIFT_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += ">>=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_BIT_AND_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "&=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_BIT_XOR_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "^=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_BIT_OR_ASSIGN:
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "|=";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pNext->pChild);
		break;
	case EXPR_TYPE_THROW:
		ret_s += "throw ";
		ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		break;
	case EXPR_TYPE_CONST_VALUE:
		ret_s += pRoot->pChild->value;
		break;
	case EXPR_TYPE_TOKEN_WITH_NAMESPACE:
		ret_s += displaySourceTreeTokenWithNamespace(pRoot->pChild->pChild);
		break;
    case EXPR_TYPE_TYPE_CONSTRUCT:
        ret_s += displaySourceTreeType(pRoot->pChild->pChild);
        ret_s += std::string("(") + displaySourceTreeExpr2(pRoot->pChild->pNext->pChild) + ")";
        break;
	case EXPR_TYPE_PARENTHESIS:
		ret_s += "(" + displaySourceTreeExpr2(pRoot->pChild->pChild) + ")";
		break;
	case EXPR_TYPE_BUILTIN_TYPE_FUNC:    // type
        ret_s += exprGetBuiltinFuncTypeName(pRoot) + "(" + displaySourceTreeType(exprGetSecondNode(pRoot)) + ")";
        break;
	case EXPR_TYPE_CONST_CAST:      // extended_type, expr
        ret_s += "const_cast<" + displaySourceTreeExtendedOrFuncType(exprGetFirstNode(pRoot)) + ">";
        ret_s += "(" + displaySourceTreeExpr(exprGetSecondNode(pRoot)) + ")";
        break;
	case EXPR_TYPE_STATIC_CAST:      // extended_type, expr
        ret_s += "static_cast<" + displaySourceTreeExtendedOrFuncType(exprGetFirstNode(pRoot)) + ">";
        ret_s += "(" + displaySourceTreeExpr(exprGetSecondNode(pRoot)) + ")";
        break;
    case EXPR_TYPE_DYNAMIC_CAST:      // extended_type, expr
        ret_s += "dynamic_cast<" + displaySourceTreeExtendedOrFuncType(exprGetFirstNode(pRoot)) + ">";
        ret_s += "(" + displaySourceTreeExpr(exprGetSecondNode(pRoot)) + ")";
        break;
    case EXPR_TYPE_REINTERPRET_CAST:      // extended_type, expr
        ret_s += "reinterpret_cast<" + displaySourceTreeExtendedOrFuncType(exprGetFirstNode(pRoot)) + ">";
        ret_s += "(" + displaySourceTreeExpr(exprGetSecondNode(pRoot)) + ")";
        break;
	case EXPR_TYPE_EXTENSION:        // expr
        ret_s += "__extension " + displaySourceTreeExpr(pRoot->pChild->pChild);
        break;
	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

std::string displaySourceTreeExpr2(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	SourceTreeVector expr_list = expr2GetExprs(pRoot);

	for (int i = 0; i < expr_list.size(); i++)
	{
		ret_s += displaySourceTreeExpr(expr_list[i]);
		if (i < expr_list.size() - 1)
			ret_s += ", ";
	}

	return ret_s;
}
//defs				: class_struct_union token ';' \\\n\
					| 'using' 'namespace' token_with_namespace ';' \\\n\
					| *[ext_modifier] 'typedef' ^ (super_type decl_var ?[attribute] | extended_type token '(' ^ func_params ')' | func_type ) ';' \\\n\
					| *[func_modifier] ?[extended_type] token_with_namespace '(' ^ func_params ')' ?[const] ?['throw' '(' ')'] ?['__asm' '(' const_value ')'] *[attribute] ';' \\\n\
					| *[var_modifier] func_type ';' \\\n\
					| *[ext_modifier] super_type ^ ?[attribute] *[decl_c_obj_var, ','] ';'
std::string displaySourceTreeDefs(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;
	SourceTreeNode* pNode, *pNode2;

	MY_ASSERT(false);
	/*if (g_source_tree_log)
	{
		printf("\nDefs: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	ret_s += printTabs(depth);

	switch (defGetType(pRoot))
	{
	case DEF_TYPE_PRE_DECL: // 'struct' token ';'
	{
		CSUType csu_type;
		std::string name = defPreDeclGetInfo(pRoot, csu_type);
		ret_s += displayCSUType(csu_type) + name + ";\n";
		break;
	}
	case DEF_TYPE_USING_NAMESPACE: // 'union' token ';'
	{
	    bool bHasNamespace;
	    SourceTreeNode* pScope = defUsingNamespaceGetInfo(pRoot, bHasNamespace);
	    ret_s += "using ";
	    if (bHasNamespace)
	        ret_s += "namespace ";
		ret_s += displaySourceTreeTokenWithNamespace(pScope) + ";\n";
		break;
	}
	case DEF_TYPE_TYPEDEF: // | *[ext_modifier] 'typedef' ^ (super_type decl_var ?[attribute] | extended_type token '(' ^ *[func_param] ')' | func_type ) ';'
	{
		int modifier_bits;
		TypeDefType type = defTypedefGetBasicInfo(pRoot, modifier_bits);
		ret_s += displayPrefixModifiers(modifier_bits) + "typedef ";
		switch (type)
		{
		case TYPEDEF_TYPE_DATA:
		{
			SourceTreeNode* pSuperType, *pDeclVar, *pAttribute;
			defTypedefDataGetInfo(pRoot, pSuperType, pDeclVar, pAttribute);
			ret_s += displaySourceTreeSuperType(pSuperType, depth + 1) + " " + displaySourceTreeDeclVar(pDeclVar);
			if (pAttribute)
				ret_s += " " + displaySourceTreeAttribute(pAttribute);
			break;
		}
		case TYPEDEF_TYPE_FUNC:
		{
			std::string name;
			SourceTreeNode* pExtendedType;
			SourceTreeVector funcParamVector;
			defTypedefFuncGetInfo(pRoot, name, pExtendedType, funcParamVector);
			ret_s += displaySourceTreeExtendedType(pExtendedType) + " " + name + "(";
			for (int i = 0; i < funcParamVector.size(); i++)
			{
				if (i > 0)
					ret_s += ", ";
				ret_s += displaySourceTreeFuncParam(funcParamVector[i]);
			}
			ret_s += ")";
			break;
		}
		case TYPEDEF_TYPE_FUNC_PTR:
		{
			ret_s += displaySourceTreeFuncType(defTypedefFuncTypeGetInfo(pRoot));
			break;
		}
		default:
			MY_ASSERT(false);
		}
		ret_s += ";\n";
		break;
	}
	case DEF_TYPE_FUNC_DECL: //| *[func_modifier] func_header ?[const] ?['throw' '(' ')'] ?['__asm' '(' const_value ')'] *[attribute] ';'
	{
		int modifier_bits;
		SourceTreeNode* pFuncHeaderNode, *pBaseClassInit;
		std::string asm_string;
		SourceTreeVector attribute_list;
		void* bracket_block;
		//defFuncDeclGetInfo(pRoot, pFuncHeaderNode, asm_string, attribute_list, pBaseClassInit, bracket_block);
		//MY_ASSERT(pBaseClassInit == NULL);
		MY_ASSERT(bracket_block == NULL);
		ret_s += displayPrefixModifiers(modifier_bits);
		ret_s += displaySourceTreeFuncHeader(pFuncHeaderNode);
		if (modifier_bits & MODBIT_CONST)
			ret_s += " const";
		if (!asm_string.empty())
			ret_s += " __asm(" + asm_string + ")";
		for (int i = 0; i < attribute_list.size(); i++)
			ret_s += " " + displaySourceTreeAttribute(attribute_list[i]);
		break;
	}
	case DEF_TYPE_FUNC_VAR_DEF: // | *[var_modifier] func_type ';'
	{
		int modifier_bits;
		SourceTreeNode* pFuncType;
		defFuncVarDefGetInfo(pRoot, modifier_bits, pFuncType);
		ret_s += displayPrefixModifiers(modifier_bits) + displaySourceTreeFuncType(pFuncType);
		break;
	}
	case DEF_TYPE_VAR_DEF: // | *[ext_modifier] super_type ^ ?[attribute] *[ ?['__restrict'] decl_var ?[ '=' ^ expr], ','] ';'
	{
		int modifier_bits, numOfVars;
		SourceTreeNode* pSuperType, *pAttribute;
		defVarDefGetInfo(pRoot, modifier_bits, pSuperType, pAttribute, numOfVars);
		ret_s += displayPrefixModifiers(modifier_bits) + displaySourceTreeSuperType(pSuperType, depth + 1);
		if (pAttribute)
			ret_s += displaySourceTreeAttribute(pAttribute);
		for (int i = 0; i < numOfVars; i++)
		{
			if (i == 0)
				ret_s += " ";
			else
				ret_s += ", ";
			bool bObjVar;
			SourceTreeNode* pChild;
			defVarDefGetVarByIndex(pRoot, i, bObjVar, pChild);
			if (bObjVar)
				ret_s += displaySourceTreeDeclObjVar(pChild);
			else
				ret_s += displaySourceTreeDeclCVar(pChild);
		}
		break;
	}
	default:
		MY_ASSERT(false);
	}*/
	return ret_s;
}

//statement		   : 'break' ^ ';'							  \\\n\
					| 'continue' ^ ';'						   \\\n\
					| 'return' ^ ?[expr] ';'					 \\\n\
					| '{' ^ *[statement] '}'					 \\\n\
					| 'if' ^ '(' expr ')' statement *[ 'else' 'if' '(' expr ')' statement] ?[ 'else' statement ] \\\n\
					| 'while' ^ '(' expr ')' statement \\\n\
					| 'do' ^ statement 'while' '(' expr ')' ';'  \\\n\
					| 'for' ^ '(' ?[expr] ';' ?[expr] ';' ?[expr] ')' statement	\\\n\
					| 'switch' ^ '(' expr ')' '{' *[ 'case' const_value ':' *[statement] ] ?[ 'default' ':' *[statement] ] '}' \\\n\
					| 'try' ^ statement +[ 'catch' '(' func_params ')' statement ] \\\n\
					| 'flow_wait' ^ '(' expr ?[',' expr] ')' ';' \\\n\
					| 'flow_signal' ^ '(' expr ?[',' expr] ')' ';' \\\n\
					| 'flow_fork' ^ statement \\\n\
					| 'flow_try' ^ statement +[ 'flow_catch' '(' expr ?[',' expr] ')' statement ] \\\n\
					| defs \\\n\
					| expr ';' \\\n\
					;
std::string displaySourceTreeStatementAsm(const SourceTreeNode* pRoot)
{
	std::string ret_s;

	SourceTreeNode* pFirstValue, *pSecondValue, *pExpr;
	statementAsmGetInfo(pRoot, pFirstValue, pSecondValue, pExpr);

	ret_s += "__asm__(" + tokenGetValue(pFirstValue) + " : " + tokenGetValue(pSecondValue) + " (" + displaySourceTreeExpr(pExpr) + "))";
	return ret_s;
}

std::string displaySourceTreeStatement(const SourceTreeNode* pRoot, int depth, bool bIfForWhile)
{
	std::string ret_s;

	SourceTreeNode* pNode, *pNode2;

	if (g_source_tree_log)
	{
		printf("\nStatement: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	ret_s += printTabs(depth);
	switch (statementGetType(pRoot))
	{
	case STATEMENT_TYPE_BREAK:
		ret_s += "break;\n";
		break;

	case STATEMENT_TYPE_CONTINUE:
		ret_s += "continue;\n";
		break;

	case STATEMENT_TYPE_RETURN:
		pRoot = pRoot->pChild;
		ret_s += "return ";
		if (pRoot->param)
			ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += ";\n";
		break;

	case STATEMENT_TYPE_COMPOUND:
		if (bIfForWhile)
		{
			depth--;
			ret_s = printTabs(depth);
		}
		ret_s += "{\n";
		for (pRoot = pRoot->pChild->pChild; pRoot; pRoot = pRoot->pNext)
			ret_s += displaySourceTreeStatement(pRoot->pChild->pChild, depth + 1);
		ret_s += printTabs(depth) + "}\n";
		break;

	case STATEMENT_TYPE_IF: // 'if' '(' expr ')' statement *[ 'else' 'if' '(' expr ')' statement] ?[ 'else' statement ]
		pRoot = pRoot->pChild;
		ret_s += "if (";
		ret_s += displaySourceTreeExpr(pRoot->pChild);
		ret_s += ")\n";
		pRoot = pRoot->pNext;
		ret_s += displaySourceTreeStatement(pRoot->pChild, depth + 1, true);
		pRoot = pRoot->pNext;
		for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		{
			pNode2 = pNode->pChild;
			ret_s += printTabs(depth) + "else if (";
			ret_s += displaySourceTreeExpr(pNode2->pChild);
			ret_s += ")\n";
			pNode2 = pNode2->pNext;
			ret_s += displaySourceTreeStatement(pNode2->pChild, depth + 1);
		}
		pRoot = pRoot->pNext;

		if (pRoot->param)
		{
			pNode = pRoot->pChild;
			ret_s += printTabs(depth) + "else\n";
			ret_s += displaySourceTreeStatement(pNode->pChild, depth + 1);
		}
		break;

	case STATEMENT_TYPE_WHILE: // 'while' '(' expr ')' statement
		pRoot = pRoot->pChild;
		ret_s += "while (";
		ret_s += displaySourceTreeExpr(pRoot->pChild);
		ret_s += ")\n";
		pRoot = pRoot->pNext;
		ret_s += displaySourceTreeStatement(pRoot->pChild, depth + 1, true);
		break;

	case STATEMENT_TYPE_DO: // 'do' statement 'while' '(' expr ')' ';'
		pRoot = pRoot->pChild;
		ret_s += "do\n";
		ret_s += displaySourceTreeStatement(pRoot->pChild, depth + 1, true);
		ret_s += "while (\n";
		pRoot = pRoot->pNext;
		ret_s += displaySourceTreeExpr(pRoot->pChild);
		ret_s += ";\n";
		break;

	case STATEMENT_TYPE_FOR: // 'for' ^ '(' ?[(expr2 | type +[decl_var ?['=' expr], ','])] ';' ?[expr] ';' ?[expr2] ')' statement
		pRoot = pRoot->pChild;
		ret_s += "for (";
		if (pRoot->param)
		{
			pNode = pRoot->pChild->pChild;
			if (pNode->param == 0)
				ret_s += displaySourceTreeExpr2(pNode->pChild->pChild);
			else
			{
				pNode = pNode->pChild;
				ret_s += displaySourceTreeType(pNode->pChild);
				pNode = pNode->pNext;
				int n = 0;
				for (pNode = pNode->pChild; pNode; pNode = pNode->pNext, n++)
				{
					ret_s += (n == 0 ? " " : ", ");
					pNode2 = pNode->pChild;
					ret_s += displaySourceTreeDeclVar(pNode2->pChild);
					pNode2 = pNode2->pNext;
					if (pNode2->param)
						ret_s += " = " + displaySourceTreeExpr(pNode2->pChild->pChild);
				}
			}
		}
		ret_s += "; ";
		pRoot = pRoot->pNext;
		if (pRoot->param)
			ret_s += displaySourceTreeExpr(pRoot->pChild->pChild);
		ret_s += "; ";
		pRoot = pRoot->pNext;
		if (pRoot->param)
			ret_s += displaySourceTreeExpr2(pRoot->pChild->pChild);
		pRoot = pRoot->pNext;
		ret_s += ")\n";
		ret_s += displaySourceTreeStatement(pRoot->pChild, depth + 1, true);
		break;

	case STATEMENT_TYPE_SWITCH: // 'switch' '(' expr ')' '{' *[ 'case' token ':' *[statement] ] ?[ 'default' ':' *[statement] ] '}'
		pRoot = pRoot->pChild;
		ret_s += "switch (";
		ret_s += displaySourceTreeExpr(pRoot->pChild);
		ret_s += ")\n{\n";
		pRoot = pRoot->pNext;

		for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		{
			pNode2 = pNode->pChild;
			ret_s += printTabs(depth) + "case " + pNode2->value + ":\n";
			pNode2 = pNode->pNext;
			for (pNode2 = pNode2->pChild; pNode2; pNode2 = pNode2->pNext)
				ret_s += displaySourceTreeStatement(pNode2->pChild->pChild, depth + 1);
		}
		pRoot = pRoot->pNext;

		if (pRoot->param)
		{
			ret_s += printTabs(depth) + "default:\n";
			pNode = pRoot->pChild;
			for (pNode2 = pNode->pChild; pNode2; pNode2 = pNode2->pNext)
				ret_s += displaySourceTreeStatement(pNode2->pChild->pChild, depth + 1);
		}
		ret_s += printTabs(depth) + "}\n";
		break;

	case STATEMENT_TYPE_TRY: // 'try' statement +[ 'catch' '(' ?[func_param] ')' statement ]
		pRoot = pRoot->pChild;
		ret_s += "try\n";
		ret_s += displaySourceTreeStatement(pRoot->pChild, depth + 1, true);
		pRoot = pRoot->pNext;

		for (pNode = pRoot->pChild; pNode; pNode = pNode->pNext)
		{
			pNode2 = pNode->pChild;
			ret_s += printTabs(depth) + "catch (";
			if (pNode2->param)
				ret_s += displaySourceTreeFuncParam(pNode2->pChild->pChild);
			pNode2 = pNode2->pNext;
			ret_s += ")\n";
			ret_s += displaySourceTreeStatement(pNode2->pChild, depth + 1, true);
		}
		break;

	case STATEMENT_TYPE_FLOW_WAIT: // 'flow_wait' '(' expr ?[',' expr] ')' ';'
		ret_s += "flow_wait(";
		pRoot = pRoot->pChild;

		ret_s += displaySourceTreeExpr(pRoot->pChild);
		pRoot = pRoot->pNext;

		ret_s += ", " + displaySourceTreeExpr(pRoot->pChild);

		ret_s += ");\n";
		break;

	case STATEMENT_TYPE_FLOW_TRY: // 'flow_try' ^ statement +[ 'flow_catch' '(' expr ',' expr ')' statement ]
		ret_s += "flow_try\n";
		pRoot = pRoot->pChild;

		ret_s += displaySourceTreeStatement(pRoot->pChild, depth + 1, true);

		for (pRoot = pRoot->pNext->pChild; pRoot; pRoot = pRoot->pNext)
		{
			ret_s += "flow_catch (";
			pNode = pRoot->pChild;

			ret_s += displaySourceTreeExpr(pNode->pChild);
			pNode = pNode->pNext;

			ret_s += ", " + displaySourceTreeExpr(pNode->pChild);
			pNode = pNode->pNext;

			ret_s += ")\n";
			ret_s += displaySourceTreeStatement(pNode->pChild, depth + 1, true);
		}
		break;

	case STATEMENT_TYPE_FLOW_FORK: // 'flow_fork' statement
		ret_s += "flow_fork\n";
		ret_s += displaySourceTreeStatement(pRoot->pChild->pChild, depth + 1, true);
		break;

	case STATEMENT_TYPE_DEF: // def
		pRoot = pRoot->pChild;
		ret_s += displaySourceTreeDefs(pRoot->pChild, depth);
		break;

	case STATEMENT_TYPE_EXPR2: // expr ';'
		pRoot = pRoot->pChild;
		ret_s += displaySourceTreeExpr2(pRoot->pChild);
		ret_s += ";\n";
		break;

	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

//start				: 'extern' ('\"C\"' | '\"C++\"') ^ '{' *[start] '}' \\\n\
					| 'namespace' token '{' ^ *[start] '}' \\\n\
					| defs \\\n\
					| *[func_modifier] ?[extended_type] token_with_namespace '(' func_params ')' ?[const] ?[':' +[token '(' expr ')', ','] ] '{' ^ *[statement] '}'
std::string displaySourceTreeStart(const SourceTreeNode* pRoot, int depth)
{
	std::string ret_s;
	SourceTreeNode* pNode, *pNode2;

	MY_ASSERT(false);
	/*if (g_source_tree_log)
	{
		printf("\nStart: ");
		printSourceTree(pRoot);
		printf("\n");
	}

	switch (blockGetType(pRoot))
	{
	case BLOCK_TYPE_EXTERN_BLOCK: // 'extern' '\"C\"' ^ '{' *[start] '}'
	{
		int modifier_bits;
		SourceTreeVector children;
		MY_ASSERT(false);
		//blockExternGetInfo(pRoot, modifier_bits, children);
		ret_s += displayPrefixModifiers(modifier_bits) + "{\n";
		for (int i = 0; i < children.size(); i++)
			ret_s += displaySourceTreeStart(children[i], depth + 1);
		ret_s += printTabs(depth) + "}\n";
		break;
	}
	case BLOCK_TYPE_NAMESPACE:
	{
		std::string name;
		SourceTreeVector children;
		MY_ASSERT(false);
		//blockNamespaceGetInfo(pRoot, name, children);
		ret_s += "namespace " + name + "{\n";
		for (int i = 0; i < children.size(); i++)
			ret_s += displaySourceTreeStart(children[i], depth + 1);
		ret_s += printTabs(depth) + "}\n";
		break;
	}
	case BLOCK_TYPE_DEF:
	{
		ret_s += displaySourceTreeDefs(blockDefGetNode(pRoot), depth);
		break;
	}
	case BLOCK_TYPE_FUNC_DEF:
	{
		int modifier_bits, memberInitCount;
		SourceTreeNode* pReturnExtendedType, *pTokenWithNamespace, *pFuncParamNode;
		SourceTreeVector statements;
		MY_ASSERT(false);
		//blockFuncGetInfo(pRoot, modifier_bits, pReturnExtendedType, pTokenWithNamespace, pFuncParamNode, memberInitCount, statements);
		ret_s += displayPrefixModifiers(modifier_bits);
		if (pReturnExtendedType)
			ret_s += displaySourceTreeExtendedType(pReturnExtendedType) + " ";
		ret_s += displaySourceTreeTokenWithNamespace(pTokenWithNamespace) + "(" + displaySourceTreeFuncParam(pFuncParamNode) + ")";
		if (memberInitCount > 0)
		{
			ret_s += " : ";
			for (int i = 0; i < memberInitCount; i++)
			{
				std::string name;
				SourceTreeNode* pExpr;
				blockFuncGetMemberInitByIndex(pRoot, i, name, pExpr);
				ret_s += name + "(" + displaySourceTreeExpr(pExpr) + ")";
				if (i < memberInitCount - 1)
					ret_s += ", ";
			}
		}
		ret_s += "\n" + printTabs(depth) + "{\n";
		for (int i = 0; i < statements.size(); i++)
			ret_s += displaySourceTreeStatement(statements[i], depth + 1);
		ret_s += printTabs(depth) + "}\n";
		break;
	}
	default:
		MY_ASSERT(false);
	}*/
	return ret_s;
}

//===============================================CGrammarAnalyzer===============================================================

const char* g_grammar_keywords[] = {
	"const", "void", "extern", "__extension__", "__attribute__", "__restrict",
	"void", "char", "short", "int", "bool", "long", "float", "unsigned", "signed", "struct", "union", "enum", "typedef", "sizeof",
	"__builtin_va_list", "wchar_t", "operator", "new", "delete",
	"for", "if", "switch", "while", "do", "break", "flow", "flow_root", "flow_fork", "flow_try", "flow_catch",
	"class", "public", "protected", "private", "virtual", "static", "mutable", "template", "namespace", "using", "typename", "friend"
	//"default",
};

void printAnalyzePath(const std::vector<std::string>& analyze_path)
{
	std::vector<std::string>::const_iterator it = analyze_path.begin();

	size_t n = 0; //analyze_path.size();
	n -= n % 20;

	if (n != 0)
		printf("%ld...", n);

	for (it = analyze_path.begin() + n; it != analyze_path.end(); it++)
		printf("%s->", it->c_str());
}

std::string getAnalyzePath(const std::vector<std::string>& analyze_path)
{
    std::vector<std::string>::const_iterator it = analyze_path.begin();
    std::string s;

    size_t n = 0; //analyze_path.size();
    n -= n % 20;

    if (n != 0)
        s += std::string(ltoa(n)) + "...";

    for (it = analyze_path.begin() + n; it != analyze_path.end(); it++)
        s += *it + "->";

    return s;
}

bool grammarLexerCallback(void* context, int mode, std::string& s)
{
    return ((CGrammarAnalyzer*)context)->onLexerCallback(mode, s);
}

CGrammarAnalyzer::CGrammarAnalyzer()
{
	m_context = NULL;
	m_grammar_root = NULL;
}

CGrammarAnalyzer::~CGrammarAnalyzer()
{

}

void CGrammarAnalyzer::init()
{
    m_srcfile_lexer.setCallback(grammarLexerCallback, this);

	addKeywords();
	loadGrammarRules();
	//printGrammarRules();
}

bool CGrammarAnalyzer::onLexerCallback(int mode, std::string& s)
{
    return g_grammarCallback(m_context, mode, s);
}

void CGrammarAnalyzer::addKeywords()
{
	for (int i = 0; i < sizeof(g_grammar_keywords) / sizeof(g_grammar_keywords[0]); i++)
		m_keywords.insert(g_grammar_keywords[i]);
}

bool CGrammarAnalyzer::isKeyword(const std::string& s)
{
	return m_keywords.find(s) != m_keywords.end();
}

std::string CGrammarAnalyzer::readOneGrammarRule(CLexer& grammar_lexer, GrammarTreeNode*& root, std::string& end_s, short& priority, short& attrib)
{
	std::string err_s;
	root = NULL;
	GrammarTreeNode* pLast = NULL;
	std::string param;
	priority = attrib = 0;
	int bFollowingIsAType = 0;

	while (true)
	{
		std::string s = grammar_lexer.read_word_without_comment();
		if (s.empty())
			return "read a grammar without ending";

		if (s == "|" || s == ";" || s == "," || s == end_s)
		{
			end_s = s;
			return "";
		}

		if (s == "@")
		{
			if (root != NULL)
				return "Priority definition must be in the very begining of a grammar item";
			s = grammar_lexer.read_word_without_comment();
			if (!CLexer::isNumber(s))
				return s + " instead of a number after '@' was found";
			priority = atoi(s.c_str());
			continue;
		}
		GrammarTreeNode* pNode = new GrammarTreeNode;
		pNode->parent = NULL;
		pNode->next = NULL;
		pNode->children = NULL;
		pNode->attrib = 0;
		if (s[0] == '\'')
			pNode->name = s;
		else if (s == "^")
		{
		    s = grammar_lexer.read_word_without_comment();
		    if (s != "O" && s != "N" && s != "E")
                return "^ must be followed by 'O', 'N' or 'E'";
			pNode->name = std::string("^") + s;
		}
		else if (s == "&")
		{
		    s = grammar_lexer.read_word_without_comment();
		    if (s != "T" && s != "V")
                return "T or V must be followed by '&'";
			bFollowingIsAType = (s == "T" ? 1 : 2);
			continue;
		}
		else if (CLexer::isIdentifier(s))
		{
			pNode->name = s;
			if (bFollowingIsAType != 0)
			{
				MY_ASSERT(s == "token");
				pNode->attrib |= (bFollowingIsAType == 1 ? GRAMMAR_ATTRIB_IS_TEMP_TYPE : GRAMMAR_ATTRIB_IS_TEMP_VAR);
				bFollowingIsAType = 0;
			}
		}
		else if (s == "(")
		{
			pNode->name = s;
			GrammarTreeNode* pLast;
			while (true)
			{
				GrammarTreeNode* pNode2 = new GrammarTreeNode;
				pNode2->name = "|";
				pNode2->next = pNode2->children = NULL;
				std::string end_s = ")";
				err_s = readOneGrammarRule(grammar_lexer, pNode2->children, end_s, pNode2->priority, pNode2->attrib);
				if (!err_s.empty())
					return err_s;
				if (pNode2->children == NULL)
					return "An empty grammar rule is found but not allowed";

				if (end_s != "|" && end_s != ")")
					return "Ending symbol '|' or ')' is expected. But '" + end_s + "' was found";

				if (pNode->children == NULL)
					pNode->children = pLast = pNode2;
				else
				{
					pLast->next = pNode2;
					pLast = pNode2;
				}
				pNode2->parent = pNode;
				if (end_s == ")")
					break;
			}
		}
		else if (s == "<")
		{
			pNode->name = s;
			std::string end_s = ">";
			err_s = readOneGrammarRule(grammar_lexer, pNode->children, end_s, priority, attrib);
			if (!err_s.empty())
				return err_s;
            if (end_s != ">")
                return "Ending symbol '>' is expected. But '" + end_s + "' was found";
			if (pNode->children == NULL)
				return "An empty grammar rule is found but not allowed";
		}
		else if (s == "?" || s == "*" || s == "+" || s == "%")
		{
			pNode->name = s;
			std::string param = grammar_lexer.read_word_without_comment();
			if (param != "[" && param != "(")
				return "'[' or '(' is expected after " + s;
			if (param == "(")
			{
			    pNode->param = param;
			    if (s == "%")
			        return "'%' cannot be followed by '('";
			}
			std::string end_s = (param == "[" ? "]" : ")");
			short priority2, attrib2;
			s = end_s;
			err_s = readOneGrammarRule(grammar_lexer, pNode->children, s, priority2, attrib2);
			if (!err_s.empty())
				return err_s;
            if (pNode->children == NULL)
                return "An empty grammar rule is found in [] but is not allowed";
            if (pNode->name == "%")
            {
                if (pNode->children->next != NULL || !CLexer::isIdentifier(pNode->children->name))
                    return "only one identifier word is allowed in %[]";
                if (s != ",")
                    return "end char of %[] must be specified";
            }
			pNode->children->parent = pNode;
			if (param == "[" && s == ",")
			{
				s = grammar_lexer.read_word_without_comment();

				if (s[0] != '\'')
					return "a const string is expected after ',' in definition of a " + pNode->name;
				pNode->param = s;
				s = grammar_lexer.read_word_without_comment();
			}
			if (end_s != s)
				return "'" + end_s + "' is expected of definition of a " + pNode->name + ", but a '" + s + "' was found instead";
		}
		else
			return "Unrecognized token '" + s + "' was found";

		if (root == NULL)
		{
			root = pLast = pNode;
		}
		else
		{
			pLast->next = pNode;
			pLast = pNode;
		}
	}

	return "";
}

void CGrammarAnalyzer::printGrammarRules2(GrammarTreeNode* pRoot)
{
	if (pRoot == NULL)
		return;

	if (pRoot->name == "|")
	{
		if (pRoot->priority != 0)
			printf("@%d", pRoot->priority);
		printGrammarRules2(pRoot->children);

		if (pRoot->next)
			printf(" | ");
	}
	else if (pRoot->name == "(")
	{
		printf("( ");
		printGrammarRules2(pRoot->children);
		printf(" ) ");
	}
	else if (pRoot->name == "<")
	{
		printf("< ");
		printGrammarRules2(pRoot->children);
		printf(" > ");
	}
	else if (pRoot->name == "{")
	{
		printf("{ ");
		printGrammarRules2(pRoot->children);
		printf(" } ");
	}
	else if (pRoot->name == "?" || pRoot->name == "*" || pRoot->name == "+" || pRoot->name == "%")
	{
	    if (pRoot->param == "(")
	    {
            printf("%s(", pRoot->name.c_str());
            printGrammarRules2(pRoot->children);
            printf(") ");
	    }
	    else
	    {
            printf("%s[", pRoot->name.c_str());
            printGrammarRules2(pRoot->children);
            if (!pRoot->param.empty())
                printf(", %s", pRoot->param.c_str());
            printf("] ");
	    }
	}
	else
		printf("%s ", pRoot->name.c_str());

	printGrammarRules2(pRoot->next);
}

void CGrammarAnalyzer::printGrammarRules()
{
	BOOST_FOREACH (const GrammarMap::value_type& entry, m_grammar_map)
	{
		printf("\t%s : ", entry.first.c_str());
		printGrammarRules2(entry.second);
		printf(";\n");
	}
}

void CGrammarAnalyzer::loadGrammarRules()
{
	std::string err_s;
	CLexer grammar_lexer;
	grammar_lexer.startWithBuffer("grammar", g_grammar_str);

	while (true)
	{
		GrammarTreeNode *root = NULL, *pLast = NULL;

		std::string name = grammar_lexer.read_word_without_comment();
		if (name.empty())
			return;

		if (grammar_lexer.read_word_without_comment() != ":")
		{
			err_s = "':' is expected after " + name;
			fatal_error("%s at line %d", err_s.c_str(), grammar_lexer.get_cur_line_no());
		}

		while (true)
		{
			GrammarTreeNode* pNode;
			std::string end_s = ";";
			short priority, attrib;
			err_s = readOneGrammarRule(grammar_lexer, pNode, end_s, priority, attrib);
			if (!err_s.empty())
				fatal_error("Analyzing grammar failed at line %d, %s", grammar_lexer.get_cur_line_no(), err_s.c_str());
			if (pNode == NULL)
			{
				err_s = "An empty grammar rule is found but not allowed";
				fatal_error("%s at line %d", err_s.c_str(), grammar_lexer.get_cur_line_no());
			}

			if (end_s != "|" && end_s != ";")
			{
				err_s = "Ending symbol '|' or ';' is expected. But '" + end_s + "' was found";
				fatal_error("%s at line %d", err_s.c_str(), grammar_lexer.get_cur_line_no());
			}

			if (root == NULL && end_s == ";")
			{
				root = pNode;
				break;
			}
			GrammarTreeNode* pNode2 = new GrammarTreeNode;
			pNode2->name = "|";
			pNode2->parent = pNode2->next = NULL;
			pNode2->children = pNode;
			pNode2->priority = priority;
			pNode2->attrib = attrib;
			if (root == NULL)
				root = pLast = pNode2;
			else
			{
				pLast->next = pNode2;
				pLast = pNode2;
			}
			pNode->parent = pNode2;
			if (end_s == ";")
				break;
		}

		m_grammar_map[name] = root;
	}
}

// handle ##
std::string CGrammarAnalyzer::grammar_read_word(int& n, int end_n, bool bSkipComments)
{
	std::string s;
	while (true)
	{
		if (end_n > 0 && n >= end_n)
			return "";

		if (n > m_gf.buffered_keywords.size())
		{
			throw(std::string("try to read a word beyond buffered_keywords: ") + ltoa(n) + "/" + ltoa(m_gf.buffered_keywords.size()));
		}

		if (n < m_gf.buffered_keywords.size())
		{
	//printf("reading word n=%d, return %s\n", n, gf.buffered_keywords[n].c_str());
			s = m_gf.buffered_keywords[n].keyword;
			n++;
			if (bSkipComments && CLexer::isCommentWord(s))
				continue;
			break;
		}

		while (true)
		{
			std::string file_name;
			int line_no;
			if (m_gf.next_keyword.empty())
			{
				file_name = m_srcfile_lexer.get_cur_filename();
				line_no = m_srcfile_lexer.get_cur_line_no();
				s = m_srcfile_lexer.read_word();
				if (s.empty())
					return "";
			}
			else
			{
				s = m_gf.next_keyword;
				file_name = m_gf.next_file_name;
				line_no = m_gf.next_line_no;
			}
			m_gf.next_file_name = m_srcfile_lexer.get_cur_filename();
			m_gf.next_line_no = m_srcfile_lexer.get_cur_line_no();
			m_gf.next_keyword = m_srcfile_lexer.read_word();
            MY_ASSERT(m_gf.next_keyword != "##");
			if (m_gf.next_keyword == "##")
			{
				std::string s2;
				while (true)
				{
                    s2 = m_srcfile_lexer.read_word();
                    if (!CLexer::isCommentWord(s2))
                        break;
				}
                if (s2.empty())
                    throw("nothing found after ##");
				m_gf.next_keyword = s + s2;
				continue;
			}
			GrammarFileBufferedKeyword item;
			item.file_name = file_name;
			item.line_no = line_no;
			item.keyword = s;
			m_gf.buffered_keywords.push_back(item);
			break;
		}
	}

//printf("reading word file=%s:%d, n=%d, return %s\n", get_working_filename().c_str(), get_working_line_no(), n, s.c_str());
	return s;
}

int CGrammarAnalyzer::findPairEnd(int n, int end_n)
{
    std::string start_s = grammar_read_word(n, end_n);
    std::string end_s;

    if (start_s == "<")
        end_s = ">";
    else if (start_s == "(")
        end_s = ")";
    else if (start_s == "[")
        end_s = "[";
    else if (start_s == "{")
        end_s = "}";

    int depth = 0;
    while (true)
    {
        std::string s = grammar_read_word(n, end_n);
        if (s.empty())
            break;

        if (s == start_s)
            depth++;

        if (s == end_s)
        {
            if (depth == 0)
                return n - 1;

            depth--;
        }
    }

    return -1;
}

void CGrammarAnalyzer::set_error(int err_n, const std::string& err_s)
{
    if (err_n < 0)
        err_n = 0;

    if (m_err_n < err_n)
    {
        m_err_n = err_n;
        m_err_s = err_s;
    }
}

int CGrammarAnalyzer::AnalyzeGrammar2(std::vector<std::string> analyze_path, GrammarTreeNode* pGrammar, GrammarTreeNode* pGrammarEnd,
		int n, int end_n, bool bNoMoreGrammar, GrammarTempDefMap& tempDefMap, SourceTreeNode* pBlockRoot, SourceTreeNode* pSourceParent, int& flags)
{
	SourceTreeNode* pSourceNode = NULL, *pSourceNode2 = NULL, *pSourceLast = NULL;

	if (pGrammar == NULL)
		return n;

	std::string& name = pGrammar->name;
	std::string func_s;
	if (g_grammar_log)
	{
		printAnalyzePath(analyze_path); printf("%s, n=%d~%d, bNoMoreGrammar=%d\n", name.c_str(), n, end_n, bNoMoreGrammar);
	}
	else
	{
        func_s = getAnalyzePath(analyze_path) + name + ", " + ltoa(n); func_s += std::string("~") + ltoa(end_n) + ",bNoMoreGrammar=" + (bNoMoreGrammar ? "1" : "0");
	}
	//if (end_n != -1)
	//{
	for (; pGrammar != pGrammarEnd; pGrammar = pGrammar->next)
	{
		if (pGrammar->name[0] != '\'' || pGrammar->name == "'('" || pGrammar->name == "'['" || pGrammar->name == "'{'")
			break;

		std::string s = grammar_read_word(n, end_n);
		if (s.empty())
		{
			std::string err_s = "Cannot read further while expecting " + pGrammar->name;
			TRACE("%s\n", err_s.c_str());
			set_error(n - 1, err_s);
			return -1;
		}
		if ("'" + s + "'" != pGrammar->name)
		{
			//if (g_grammar_log)
			set_error(n - 1, std::string("Found '") + s + "' while expecting " + pGrammar->name);
			return -1;
		}
		TRACE("Skipping %s\n", s.c_str());
	}
	if (pGrammar == pGrammarEnd)
	{
		TRACE("empty, return %d\n", n);
		return n;
	}
	GrammarTreeNode* pGrammarP, *pGrammarTemp = NULL;
	bool bFound = false;
	for (pGrammarP = pGrammar; pGrammarP && (pGrammarEnd == NULL || pGrammarP != pGrammarEnd); pGrammarP = pGrammarP->next)
	{
		if (pGrammarP->name[0] == '\'')
		{
			bFound = true;
			break;
		}
		pGrammarTemp = pGrammarP;
	}
	if (!bFound)
		return AnalyzeGrammar(analyze_path, pGrammar, pGrammarEnd, n, end_n, bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceParent, flags);

	bool bDividerIsTheOne = pGrammarTemp && pGrammarTemp->name == "^O";
	std::string s, s_p = pGrammarP->name.substr(1, pGrammarP->name.size() - 2);
	TRACE("found divider grammar '%s', the_one=%d\n", s_p.c_str(), bDividerIsTheOne);

	SourceTreeNode* pOrigChildTail = findChildTail(pSourceParent), *pLastGoodNode = NULL;
	int orig_n = n, last_good_n = -1, flagsLastGood;
	int last_matched_np = -1;
	GrammarTempDefMap lastGoodTempDefMap, tempDefMap2;

	while (last_matched_np < 0 || end_n > 0) // in case we have multiple matches of a grammar divider, only do this in a small scope, otherwise, it will be out of control
	{
		n = orig_n;
		int n_p, n_q, flags2 = flags;
		tempDefMap2 = tempDefMap;
		bFound = false;
		int depth = 0;
		for (n_p = (last_matched_np < 0 ? n : last_matched_np + 1); end_n < 0 || n_p < end_n;)
		{
			s = grammar_read_word(n_p, end_n);
			if (s.empty())
				break;
			if (depth == 0 && s == s_p)
			{
				bFound = true;
				break;
			}
			if (s == "(" || s == "[" || s == "{")
				depth++;
			else if (s == ")" || s == "]" || s == "}")
			{
			    // when searching the second '(', it will probably passing the the first ')'
				if (depth > 0)
				    depth--;
			}
		}
		if (!bFound)
		{
			//err_n = n;
			//err_s = "not found in string, return -1";
			TRACE("%s, not found '%s' in string, last_matched_np=%d, n=%d\n", __FUNCTION__, s_p.c_str(), last_matched_np, n);
			break;
		}
		n_p--;
		last_matched_np = n_p;
		TRACE("%s, found it in string at %d\n", __FUNCTION__, n_p);
		n = AnalyzeGrammar2(analyze_path, pGrammar, pGrammarP, n, n_p, true, tempDefMap2, pBlockRoot, pSourceParent, flags2);
		if (n >= 0 && n != n_p)
		{
			TRACE("%s, First phase reaches to %d, but expecting to reach %d\n", __FUNCTION__, n, n_p);
			n = -1;
		}
		if (n < 0)
		{
			deleteChildTail(pSourceParent, pOrigChildTail);
			TRACE("%s, analyzing first phase failed, n_p=%d\n", __FUNCTION__, n_p);
			continue;
		}
		n++;

		if (bDividerIsTheOne)
		    flags2 |= ANALYZE_FLAG_BIT_THE_ONE;

		if (pGrammarP->next != NULL)
		{
            if (s_p != "(" && s_p != "[" && s_p != "{") // two parts
            {
                TRACE("analyzing last half...\n");
                n = AnalyzeGrammar2(analyze_path, pGrammarP->next, pGrammarEnd, n, end_n, bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceParent, flags2);
                if (n >= 0 && bNoMoreGrammar && n != end_n)
                {
                    TRACE("%s, last half returned %d while bNoMoreGrammar=true and end_n=%d\n", __FUNCTION__, n, end_n);
                    n = -1;
                }
                if (n < 0)
                {
                    TRACE("%s, analyzing last half failed\n", __FUNCTION__);
                    deleteChildTail(pSourceParent, pOrigChildTail);
                    continue;
                }
                TRACE("%s, combine first half and second half, and return %d\n", __FUNCTION__, n);
            }
            else // three parts
            {
                std::string s_q = (s_p == "(" ? "')'" : (s_p == "[" ? "']'" : "'}'") );

                GrammarTreeNode* pGrammarQ;
                bFound = false;
                depth = 0;
                for (pGrammarQ = pGrammarP->next; pGrammarQ && (pGrammarEnd == NULL || pGrammarQ != pGrammarEnd); pGrammarQ = pGrammarQ->next)
                {
                    if (depth == 0 && pGrammarQ->name == s_q)
                    {
                        bFound = true;
                        break;
                    }
                    if (pGrammarQ->name == "'('" || pGrammarQ->name == "'['" || pGrammarQ->name == "'{'")
                        depth++;
                    else if (pGrammarQ->name == "')'" || pGrammarQ->name == "']'" || pGrammarQ->name == "'}'")
                        depth--;
                }
                if (!bFound)
                    throw("parenthesis not paired in grammar, s_p=" + s_p);

                s_q = s_q.substr(1, 1);
                bFound = false;
                for (n_q = n; end_n < 0 || n_q < end_n;)
                {
                    s = grammar_read_word(n_q, end_n);
                    if (s.empty())
                    {
                        TRACE("%s, EOF found when searching for the next pair, return -1\n", __FUNCTION__);
                        deleteChildTail(pSourceParent, pOrigChildTail);
                        continue;
                    }
                    if (s == "(" || s == "[" || s == "{")
                        depth++;
                    if (s == ")" || s == "]" || s == "}")
                    {
                        if (depth == 0)
                        {
                            if (s == s_q)
                            {
                                bFound = true;
                                break;
                            }
                            TRACE("%s, found '%s' at %d that cannot be paired, return -1\n", __FUNCTION__, s.c_str(), n_q);
                            deleteChildTail(pSourceParent, pOrigChildTail);
                            continue;
                        }
                        depth--;
                    }
                }
                if (!bFound)
                {
                    TRACE("%s, cannot find the other match pair %s from %d~%d, return -1\n", __FUNCTION__, s_q.c_str(), n, n_q);
                    deleteChildTail(pSourceParent, pOrigChildTail);
                    continue;
                }
                TRACE("%s, Found the other match pair at %d\n", __FUNCTION__, n_q);

                n_q--;
                n = AnalyzeGrammar2(analyze_path, pGrammarP->next, pGrammarQ, n, n_q, true, tempDefMap2, pBlockRoot, pSourceParent, flags2);
                if (n < 0 || n != n_q)
                {
                    TRACE("%s, Analyze middle phase failed, n=%d, n_q=%d\n", __FUNCTION__, n, n_q);
                    deleteChildTail(pSourceParent, pOrigChildTail);
                    continue;
                }
                n++;
                pGrammarQ = pGrammarQ->next;
                n = AnalyzeGrammar2(analyze_path, pGrammarQ, pGrammarEnd, n, end_n, bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceParent, flags2);
                if (n < 0)
                {
                    TRACE("%s, Analyze last phase failed\n", __FUNCTION__);
                    deleteChildTail(pSourceParent, pOrigChildTail);
                    continue;
                }
                TRACE("%s, return combined 3 pieces and return %d\n", __FUNCTION__, n);
            }
		}
		if (last_good_n < n)
		{
		    TRACE("%s, change last_good_n to %d\n", __FUNCTION__, n);
            last_good_n = n;
            lastGoodTempDefMap = tempDefMap2;
            flagsLastGood = flags2;
            if (pLastGoodNode)
                deleteSourceTreeNode(pLastGoodNode);
            if (pOrigChildTail == NULL)
            {
                pLastGoodNode = pSourceParent->pChild;
                pSourceParent->pChild = NULL;
            }
            else
            {
                pLastGoodNode = pOrigChildTail->pNext;
                pOrigChildTail->pNext = NULL;
            }
		}
		else
		{
            deleteChildTail(pSourceParent, pOrigChildTail);
		}
	}

	if (pLastGoodNode == NULL)
	{
        TRACE("AnalyzeGrammar2 failed, return -1\n");
	    return -1;
	}
    if (pOrigChildTail == NULL)
    {
        pSourceParent->pChild = pLastGoodNode;
    }
    else
    {
        pOrigChildTail->pNext = pLastGoodNode;
    }
    tempDefMap = lastGoodTempDefMap;
    flags = flagsLastGood;
    TRACE("AnalyzeGrammar2 succeed, return %d\n", last_good_n);
    return last_good_n;
}

void CGrammarAnalyzer::skipGrammarComments(int& n, int end_n)
{
	std::string s = grammar_read_word(n, end_n, true);
	if (!s.empty())
		n--;
}

int CGrammarAnalyzer::AnalyzeGrammar(std::vector<std::string> analyze_path, GrammarTreeNode* pGrammar, GrammarTreeNode* pGrammarEnd, int n, int end_n,
	bool bNoMoreGrammar, GrammarTempDefMap& tempDefMap, SourceTreeNode* pBlockRoot, SourceTreeNode* pSourceParent, int& flags)
{
	SourceTreeNode* pSourceNode = NULL, *pSourceNode2 = NULL, *pSourceLast = NULL;

	bool bSetTheOneFlag = false;
	for (; pGrammar != pGrammarEnd; pGrammar = pGrammar->next)
	{
		std::string name = pGrammar->name;

		if (name == "^O")
		{
		    bSetTheOneFlag = true;
			continue;
		}

        if (name == "^N")
        {
            TRACE("no type check\n");
            flags |= ANALYZE_FLAG_BIT_NO_TYPE_CHECK;
            continue;
        }

        if (name == "^E")
        {
            pGrammar = pGrammar->next;
            GrammarTreeNode* pGrammarP, *pGrammarTemp = NULL;
            bool bFound = false;
            for (pGrammarP = pGrammar; pGrammarP && (pGrammarEnd == NULL || pGrammarP != pGrammarEnd); pGrammarP = pGrammarP->next)
            {
                if (pGrammarP->name[0] == '\'')
                {
                    bFound = true;
                    break;
                }
                pGrammarTemp = pGrammarP;
            }
            MY_ASSERT(bFound);
            bool bDividerIsTheOne = pGrammarTemp && pGrammarTemp->name == "^O";
            std::string s, s_p = pGrammarP->name.substr(1, pGrammarP->name.size() - 2);
            MY_ASSERT(s_p != "(" && s_p != "[" && s_p != "{");
            TRACE("found divider grammar '%s'\n", s_p.c_str());
            int depth = 0, n_p = n;
            bFound = false;
            while (end_n < 0 || n_p < end_n)
            {
                s = grammar_read_word(n_p, end_n);
                if (s.empty())
                    break;
                if (depth == 0 && s == s_p)
                {
                    bFound = true;
                    break;
                }
                if (s == "(" || s == "[" || s == "{")
                    depth++;
                else if (s == ")" || s == "]" || s == "}")
                {
                    if (depth == 0)
                        break;
                    depth--;
                }
            }
            if (!bFound)
            {
                set_error(n - 1, "expecting '" + s_p + "' but not found");
                TRACE("%s, not found '%s' in string, from %d, end_n=%d\n", __FUNCTION__, s_p.c_str(), n, end_n);
                n = -1;
                break;
            }
            n_p--;
            TRACE("%s, found '%s' at %d\n", __FUNCTION__, s_p.c_str(), n_p);
            n = AnalyzeGrammar(analyze_path, pGrammar, pGrammarP, n, n_p, true, tempDefMap, pBlockRoot, pSourceParent, flags);
            TRACE("%s, the first part returns %d\n", __FUNCTION__, n);
            if (n >= 0 && n != n_p)
            {
                TRACE("%s, First phase reaches to %d, but expecting to reach %d\n", __FUNCTION__, n, n_p);
                n = -1;
            }
            if (n < 0)
                break;
            if (bDividerIsTheOne)
                flags |= ANALYZE_FLAG_BIT_THE_ONE;
            n = AnalyzeGrammar(analyze_path, pGrammarP->next, pGrammarEnd, n_p + 1, end_n, bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceParent, flags);
            TRACE("%s, Second phase returns %d\n", __FUNCTION__, n);
            if (n >= 0 && bNoMoreGrammar && n != end_n)
            {
                TRACE("%s, last half returned %d while bNoMoreGrammar=true and end_n=%d\n", __FUNCTION__, n, end_n);
                n = -1;
            }
            break;
        }

        std::string func_s;
		std::string s = grammar_read_word(n, end_n);
		if (!s.empty())
			n--;
		if (g_grammar_log)
		{
			/*int sz = m_gf.buffered_keywords.size();
			std::string file_name;
			int line_no = 0;
			if (n < sz)
			{
				file_name = m_gf.buffered_keywords[n].file_name;
				line_no = m_gf.buffered_keywords[n].line_no;
			}*/
			//printAnalyzePath(analyze_path); printf("%s, n=%d~%d, word='%s', fn=%s, ln=%d\n", name.c_str(), n, end_n, s.c_str(), file_name.c_str(), line_no);
            printAnalyzePath(analyze_path); printf("%s%s, n=%d~%d, word='%s', flags=%d\n", name.c_str(), pGrammar->param.c_str(), n, end_n, s.c_str(), flags);
			//BOOST_FOREACH(const std::string& ss, tempDefMap)
			//  TRACE("tempDefMap has %s\n", ss.c_str());
		}
		else
		{
            func_s = getAnalyzePath(analyze_path) + name + ", " + ltoa(n); func_s += std::string("~") + ltoa(end_n) + ",word='" + name + "', flags="; func_s += ltoa(flags);
		}
        analyze_path.push_back(name);
		if (name == "const_value")
		{
			if (s.empty() || (!CLexer::isNumber(s) && s[0] != '\'' && s[0] != '"'))
			{
				set_error(n - 1, "expecting a const_value but found " + s);
				n = -1;
				break;
			}

			pSourceNode = createEmptyNode();
			pSourceNode->param = SOURCE_NODE_TYPE_CONST_VALUE;
			pSourceNode->value = s;
			n++;
			while (s[0] == '"')
			{
				s = grammar_read_word(n, end_n);
				if (s.empty())
					break;
				if (s[0] != '"')
				{
					n--;
					break;
				}
				pSourceNode->value += s;
			}
			TRACE("***matched const_value:%s\n", pSourceNode->value.c_str());
			appendToChildTail(pSourceParent, pSourceNode);
		}
		else if (name == "token" || name == "any_token")
		{
			if (s.empty() || !CLexer::isIdentifier(s) || (name == "token" && isKeyword(s)))
			{
				set_error(n - 1, "expecting a token but found " + s);
				n = -1;
				break;
			}

			TRACE("***matched token\n");
			n++;
			pSourceNode = createEmptyNode();
			pSourceNode->param = SOURCE_NODE_TYPE_TOKEN;
			pSourceNode->value = s;
			appendToChildTail(pSourceParent, pSourceNode);

			if (pGrammar->attrib & GRAMMAR_ATTRIB_IS_TEMP_TYPE)
			{
				MY_ASSERT(tempDefMap.find(s) == tempDefMap.end());
				tempDefMap[s] = 1;
				TRACE("***add temp type %s\n", s.c_str());
			}
			else if (pGrammar->attrib & GRAMMAR_ATTRIB_IS_TEMP_VAR)
            {
                TRACE("***add temp var %s\n", s.c_str());
                // name could be duplicate when in func params, the first param is a func type which has a param whose name is the same as the afterward param of the func
                MY_ASSERT(tempDefMap.find(s) == tempDefMap.end() || tempDefMap[s] == 2);
                tempDefMap[s] = 2;
            }
		}
		else if (name.at(0) == '\'')
		{
		    name = name.substr(1, name.size() - 2);
			if (s != name)
			{
				set_error(n - 1, "expecting " + name + " but found " + s);
				n = -1;
				break;
			}

			TRACE("***matched const string %s at %d\n", name.c_str(), n);
			n++;

	        if (bSetTheOneFlag)
	        {
	            TRACE("Found the one\n");
	            flags |= ANALYZE_FLAG_BIT_THE_ONE;
	            bSetTheOneFlag = false;
	        }

			if (!(name == "(" || name == "[" || name == "{"))
			    continue;

            std::string end_name = (name == "(" ? ")" : (name == "[" ? "]" : "}"));
            pGrammar = pGrammar->next;
            GrammarTreeNode* pGrammarEnd2 = pGrammar;
            if (!pGrammarEnd2 || pGrammarEnd2 == pGrammarEnd)
                break; // in case there's only one '(' in the grammar
            int depth = 0;
            while (true)
            {
                if (!pGrammarEnd2 || pGrammarEnd2 == pGrammarEnd)
                    MY_ASSERT(false);
                if (pGrammarEnd2->name == "'" + name + "'")
                    depth++;
                else if (pGrammarEnd2->name == "'" + end_name + "'")
                {
                    if (depth == 0)
                        break;
                    depth--;
                }
                pGrammarEnd2 = pGrammarEnd2->next;
            }
            int n2 = n;
            depth = 0;
            while (true)
            {
                s = grammar_read_word(n2, end_n);
                if (s.empty())
                {
                    TRACE("found end_n when searching for the closing bracket %s starting at %d\n", end_name.c_str(), n2 - 1);
                    return -1;
                }
                if (s == "(" || s == "[" || s == "{")
                    depth++;
                if (s == ")" || s == "]" || s == "}")
                {
                    if (depth == 0)
                    {
                        if (s == end_name)
                            break;
                        std::string err_s = "found unexpected closing bracket '" + s + "' at " + ltoa(n2 - 1);
                        err_s += " when searching for the closing bracket '" + end_name + "' starting at " + ltoa(n) + ":\n";
                        for (int i = 0; i <= n2;)
                            err_s += grammar_read_word(i, end_n) + " ";
                        //printf("%s ", err_s.c_str());
                        throw(err_s);
                    }
                    depth--;
                }
            }
            n2--;
            TRACE("found closing bracket %s starting at %d\n", end_name.c_str(), n2);
            n = AnalyzeGrammar(analyze_path, pGrammar, pGrammarEnd2, n, n2, true, tempDefMap, pBlockRoot, pSourceParent, flags);
            TRACE("The first half returns %d\n", n);
            if (n < 0)
                break;
            if (n != n2)
            {
                TRACE("but %d is expected\n", n2);
                n = -1;
                break;
            }
            n2++;
            n = AnalyzeGrammar(analyze_path, pGrammarEnd2->next, pGrammarEnd, n2, end_n, bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceParent, flags);
            TRACE("The second half returns %d, err_n=%d\n", n, m_err_n);
            break;
		}
		else if (CLexer::isIdentifier(name))
		{
		    std::string pattern = name + "/" + ltoa(n) + "/";
		    pattern += std::string(ltoa(end_n)) + "/";
		    pattern += std::string(ltoa(flags)) + "/";
		    for (GrammarTempDefMap::iterator it = tempDefMap.begin(); it != tempDefMap.end(); it++)
		        pattern += it->first + "=" + ltoa(it->second) + ";";

		    PatternResultCacheMap::iterator pattern_it = m_pattern_result_cache_map.find(pattern);
		    if (pattern_it != m_pattern_result_cache_map.end())
		    {
		        n = pattern_it->second.n;
		        if (n < 0)
		        {
		            TRACE("cache found! n=%d, err_n=%d, err_s=%s\n", n, pattern_it->second.err_n, pattern_it->second.err_s.c_str());
                    set_error(pattern_it->second.err_n, pattern_it->second.err_s);
		        }
		        else
		        {
                    TRACE("cache found! n=%d, flags=%d\n", n, flags);
		            appendToChildTail(pSourceParent, dupSourceTreeNode(pattern_it->second.pSourceNode));
	                flags = pattern_it->second.flags;
	                tempDefMap = pattern_it->second.tempDefMap;
		        }
		    }
		    else
		    {
                GrammarMap::iterator it = m_grammar_map.find(name);
                MY_ASSERT(it != m_grammar_map.end());

                pSourceNode = createEmptyNode();
                pSourceNode->param = SOURCE_NODE_TYPE_IDENTIFIER;
                appendToChildTail(pSourceParent, pSourceNode);

                //bool bTheOneFound2 = false;
                int start_n = n;
                int temp_err_n = m_err_n;
                std::string temp_err_s = m_err_s;
                m_err_n = -1;
                GrammarTempDefMap tempDefMap2 = tempDefMap;
                int tempFlags = flags;
                if (name == "expr") // recursive one
                    n = AnalyzeGrammar2(analyze_path, it->second, NULL, n, end_n, (pGrammar->next == pGrammarEnd) && bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceNode, tempFlags);
                else
                    n = AnalyzeGrammar(analyze_path, it->second, NULL, n, end_n, (pGrammar->next == pGrammarEnd) && bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceNode, tempFlags);
                TRACE("***%s returns n=%d, flags=%d, err_n=%d\n", name.c_str(), n, flags, m_err_n);
                if (n > 0)
                {
                    if (!postIdentifierHandling(name, pSourceNode->pChild))
                    {
                        //printf("Check failed\n");
                        set_error(n - 1, "post identifier handling failed for " + name);
                        n = -1;
                    }
                    else
                    {
                        MY_ASSERT(pSourceNode->pChild);
                        if (name == "token_with_namespace")
                        {
                            if (!g_grammarCheckFunc(m_context, 0, pSourceNode->pChild, tempDefMap2))
                            {
                                printf("Check token_with_namespace '%s', not a token\n", displaySourceTreeTokenWithNamespace(pSourceNode->pChild).c_str());
                                set_error(n - 1, displaySourceTreeTokenWithNamespace(pSourceNode->pChild) + " is not a token");
                                n = -1;
                            }
                            //TRACE("twn resolved to %s\n", displaySourceTreeTokenWithNamespace(pSourceNode->pChild).c_str());
                        }
                        else if (name == "user_def_type" && (flags & ANALYZE_FLAG_BIT_NO_TYPE_CHECK) == 0)
                        {
                            if (!g_grammarCheckFunc(m_context, 1, pSourceNode->pChild, tempDefMap2))
                            {
                                printf("Check user_def_type '%s', not a type\n", displaySourceTreeUserDefType(pSourceNode->pChild).c_str());
                                set_error(n - 1, displaySourceTreeUserDefType(pSourceNode->pChild) + " is not a user defined type");
                                n = -1;
                            }
                        }
                        if (n >= 0)
                        {
                            tempDefMap = tempDefMap2;
                        }
                    }
                }
                if (tempFlags & ANALYZE_FLAG_BIT_THE_ONE)
                    flags |= ANALYZE_FLAG_BIT_THE_ONE;

                PatternResultCache cache;
                cache.n = n;
                cache.err_n = m_err_n;
                cache.err_s = m_err_s;
                cache.tempDefMap = tempDefMap;
                cache.flags = flags;
                if (n >= 0)
                    cache.pSourceNode = dupSourceTreeNode(pSourceNode);
                else
                    cache.pSourceNode = NULL;
                m_pattern_result_cache_map[pattern] = cache;
                TRACE("Adding to cache, pattern=%s, result, n=%d, err_n=%d, flags=%d\n", pattern.c_str(), n, m_err_n, flags);

                if (m_err_n < temp_err_n)
                {
                    m_err_n = temp_err_n;
                    m_err_s = temp_err_s;
                }
		    }
		}
		else if (name == "(")
		{
			pSourceNode = createEmptyNode();
			pSourceNode->param = SOURCE_NODE_TYPE_SMALL_BRACKET;
			appendToChildTail(pSourceParent, pSourceNode);

			n = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, end_n, (pGrammar->next == pGrammarEnd) && bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceNode, flags);
			if (g_grammar_log)
			{
				printAnalyzePath(analyze_path); printf(", return %d\n", n);
			}
			if (n < 0)
				break;
		}
		else if (name == "<")
		{
			if (s != "<")
			{
				set_error(n - 1, "expecting '<' but found " + s);
				n = -1;
				break;
			}

			TRACE("***matched '<'\n");
			n++;

			int n2 = n;
			int depth = 0;
			while (true)
			{
				std::string s = grammar_read_word(n2, end_n);
				if (s.empty())
				{
					TRACE("cannot find '>'\n");
					n = -1;
					break;
				}
				if (s == "<" || s == "(" || s == "[" || s == "{")
					depth++;

				if (s == ">" || s == ")" || s == "]" || s == "}")
				{
					if (depth == 0)
					{
						if (s == ">")
						  break;
						TRACE("find unmatched parenthesis %s", s.c_str());
						n = -1;
						break;
					}
					depth--;
				}
			}
			if (n < 0)
			    break;
            TRACE("found > at %d\n", n2 - 1);
            // setting of typename doesn't mean the type params inside should also skip type checking. otherwise, an expr will be treated as a type
            int flags2 = flags & ~ANALYZE_FLAG_BIT_NO_TYPE_CHECK;
            int j = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, n2 - 1, true, tempDefMap, pBlockRoot, pSourceParent, flags2);
            TRACE("analyze <> return %d\n", j);
            if (flags2 & ANALYZE_FLAG_BIT_THE_ONE)
                flags |= ANALYZE_FLAG_BIT_THE_ONE;
			if (j < 0)
			{
			    n = -1;
			    break;
			}

			if (j != n2 - 1)
			{
				TRACE("but %d is expected", n2 - 1);
				n = -1;
				break;
			}
			n = n2;
		}
		else if (name == "%")
		{
			pSourceNode = createEmptyNode();
			pSourceNode->param = SOURCE_NODE_TYPE_BIG_BRACKET;
			appendToChildTail(pSourceParent, pSourceNode);
			BracketBlock* pBlock = new BracketBlock;
			StringVector file_stack;
			file_stack.push_back(m_gf.buffered_keywords[n - 1].file_name);
			pBlock->tokens.push_back(CLexer::file_stack_2_string(file_stack, m_gf.buffered_keywords[n - 1].line_no));
			MY_ASSERT(CLexer::isIdentifier(pGrammar->children->name));
			MY_ASSERT(pGrammar->children->next == NULL);
			MY_ASSERT(m_grammar_map.find(pGrammar->children->name) != m_grammar_map.end());
			pBlock->pGrammarNode = m_grammar_map[pGrammar->children->name];

			std::string end_str = pGrammar->param.substr(1, pGrammar->param.size() - 2);
			int depth = 0;
			while (true)
			{
				s = grammar_read_word(n, -1, false);
				if (s.empty())
					throw ("cannot find '" + end_str + "' that matches '{' starts at " + pBlock->file_stack.back() + ":" + ltoa(pBlock->file_line_no));

				if (depth == 0)
				{
                    if (s == end_str)
                    {
                        n--;
                        break;
                    }
                }

				if (s == "(" || s == "[" || s == "{")
					depth++;
				else if (s == ")" || s == "]" || s == "}")
				{
					if (depth == 0)
						throw ("unpair " + s + " found");
					depth--;
				}

				pBlock->tokens.push_back(s);
			}

			pSourceNode->ptr = pBlock;
			TRACE("***{ found at %s:%d, n=%d\n", m_srcfile_lexer.get_cur_filename().c_str(), m_srcfile_lexer.get_cur_line_no(), n);
		}
		else if (name == "?")
		{
			pSourceNode = createEmptyNode();
			appendToChildTail(pSourceParent, pSourceNode);

			if (pGrammar->param != "(")
			{
                int j = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, end_n, (pGrammar->next == pGrammarEnd) && bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceNode, flags);
                if (g_grammar_log)
                {
                    printAnalyzePath(analyze_path); printf(", ?[] returns %d\n", j);
                }
                if (j >= 0)
                {
                    pSourceNode->param = 1;
                    j = AnalyzeGrammar(analyze_path, pGrammar->next, pGrammarEnd, j, end_n, bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceParent, flags);
                    if (j >= 0)
                    {
                        if (g_grammar_log)
                        {
                            printAnalyzePath(analyze_path); printf(", ?[] passed, return %d\n", j);
                        }
                        n = j;
                        break;
                    }
                    deleteSourceTreeNode(pSourceNode->pChild);
                    deleteSourceTreeNode(pSourceNode->pNext);
                    pSourceNode->pChild = pSourceNode->pNext = NULL;
                }
                pSourceNode->param = 0;
                if (g_grammar_log)
                {
                    printAnalyzePath(analyze_path); printf(", ?[] not pass, check next, n=%d, err_n=%d\n", n, m_err_n);
                }
			}
			else
			{
			    if (s == "(")
			    {
			        pSourceNode->param = 1;
			        int n2 = findPairEnd(n, end_n);
			        if (n2 < 0)
			        {
			            std::string err_s = std::string("Cannot find the ')' started at ") + ltoa(n);
			            set_error(n - 1, err_s);
			            TRACE("%s\n", err_s.c_str());
			            n = -1;
			            break;
			        }
                    TRACE("Found ')' at %d", n2);
			        n++;
	                n = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, n2, true, tempDefMap, pBlockRoot, pSourceNode, flags);
	                if (n != n2)
	                {
	                    TRACE("The () inside analysis returns %d while ')' locates at %d, failed", n, n2);
	                    n = -1;
	                    break;
	                }
	                n = n2 + 1;
			    }
			    else
			    {
                    n = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, end_n, (pGrammar->next == pGrammarEnd) && bNoMoreGrammar, tempDefMap, pBlockRoot, pSourceNode, flags);
                    if (g_grammar_log)
                    {
                        printAnalyzePath(analyze_path); TRACE(", ?() returns %d\n", n);
                    }
			    }
                TRACE("The () inside analysis returns %d", n);
			}
		}
		else if (name == "*" || name == "+")
		{
		    MY_ASSERT(pGrammar->param != "(");
			SourceTreeNode* pSourceNodeLastGood = NULL, *pSourceNodeLastChild = NULL, *pSourceNode3;
			int nLastGood = -1, last_good_param = 0;
			int j;
			bool bTheOneFound2;

			pSourceNode = createEmptyNode();
			appendToChildTail(pSourceParent, pSourceNode);

			while (true)
			{
				//if (g_grammar_log)
					analyze_path.push_back(name + ltoa(pSourceNode->param));
				SourceTreeNode* pLastGoodTail = findChildTail(pSourceNode);
				pSourceNode2 = createEmptyNode();
				appendToChildTail(pSourceNode, pSourceNode2);
				pSourceNode2->param = pSourceNode->param;
				pSourceNode->param++;
				skipGrammarComments(n, end_n);
                int tempFlags = (flags & (~ANALYZE_FLAG_BIT_THE_ONE));
				int j = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, end_n, false/*(pGrammar->next == pGrammarEnd) && bNoMoreGrammar*/, tempDefMap, pBlockRoot, pSourceNode2, tempFlags);
				if (g_grammar_log)
				{
					printAnalyzePath(analyze_path); printf(", %s[%d] returned %d\n", name.c_str(), pSourceNode2->param, j);
				}
				//if (g_grammar_log)
					analyze_path.pop_back();
				if (j < 0)
				{
					/*if (bTheOneFound2)
					{
						deleteSourceTreeNode(pSourceRoot);
						return n;
					}*/
					if (pSourceNode2->param > 0 && !pGrammar->param.empty())
					{
						//err_s = "cannot analyze " + name + "[" + ltoa(pSourceNode2->param) + "] after delimiter " + pGrammar->param;
						//return -1;
					    n--;
					}
					deleteChildTail(pSourceNode, pLastGoodTail);
					pSourceNode->param--;
					break;
				}
				n = j;
				if (!pGrammar->param.empty())
				{
					s = grammar_read_word(n, end_n);
					if (s.empty() || s != pGrammar->param.substr(1, pGrammar->param.size() - 2))
					{
						if (!s.empty())
							n--;
						break;
					}
				}
			}
			if (name == "+" && pSourceNode->param == 0)
			{
			    TRACE("+[] is specified, but 0 match is found");
				set_error(n - 1, "+[] is specified, but 0 match is found");
				n = -1;
				break;
			}
			if (g_grammar_log)
			{
				printAnalyzePath(analyze_path); printf(", %s[] return %d, n=%d\n", name.c_str(), pSourceNode->param, n);
			}
		}
		else if (name == "|")
		{
			int last_priority = 1000;
			int last_j = -1;
			int idx = -1, last_idx = -1, j, last_good_err_n = -1;
			SourceTreeNode* pLastGood = NULL;
			bool bTheOneFound2 = false;
			std::string last_good_err_s;
			GrammarTempDefMap lastGoodTempDefMap;

			pSourceNode = createEmptyNode();
			appendToChildTail(pSourceParent, pSourceNode);
			for (idx = 0; pGrammar; pGrammar = pGrammar->next, idx++)
			{
				pSourceNode->param = idx;
				analyze_path.push_back(name + ltoa(idx));
	            GrammarTempDefMap tempDefMap2 = tempDefMap;
                int tempFlags = (flags & (~ANALYZE_FLAG_BIT_THE_ONE));
				if (pGrammar->children->name == "expr") // recursive one
					j = AnalyzeGrammar2(analyze_path, pGrammar->children, NULL, n, end_n, bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceNode, tempFlags);
				else
					j = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, end_n, bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceNode, tempFlags);
				//j = AnalyzeGrammar(analyze_path, pGrammar->children, NULL, n, end_n, bNoMoreGrammar, tempDefMap2, pBlockRoot, pSourceNode, err_n, err_s, bTheOneFound2);
				analyze_path.pop_back();
				if (g_grammar_log)
				{
					printAnalyzePath(analyze_path); printf(", |%d returned %d, flags=%d, err_n=%d\n", idx, j, tempFlags, m_err_n);
				}
				if (j > 0)
				{
					if (g_grammar_log)
					{
						printAnalyzePath(analyze_path); printf(", this choice passed, priority=%d, last_priority=%d, j=%d, last_j=%d\n", pGrammar->priority, last_priority, j, last_j);
					}
					// need to select a lower priority
					if (last_j < j || (last_j == j && last_priority < pGrammar->priority))
					{
						last_priority = pGrammar->priority;
						last_j = j;
						last_idx = idx;
						lastGoodTempDefMap = tempDefMap2;
						if (pLastGood)
							deleteSourceTreeNode(pLastGood);
						pLastGood = pSourceNode->pChild;
						pSourceNode->pChild = NULL;
					}
					else
					{
						deleteSourceTreeNode(pSourceNode->pChild);
						pSourceNode->pChild = NULL;
					}
				}
				else
				{
					deleteSourceTreeNode(pSourceNode->pChild);
					pSourceNode->pChild = NULL;
				}
                /*if ((tempFlags & ANALYZE_FLAG_BIT_THE_ONE) != 0 || last_good_err_n < err_n)
                {
                    last_good_err_n = err_n;
                    last_good_err_s = err_s;
                }*/
				if (tempFlags & ANALYZE_FLAG_BIT_THE_ONE)
				{
				    flags |= ANALYZE_FLAG_BIT_THE_ONE;
					if (j < 0)
					    n = -1;
					break;
				}
			}
			/*if (!bTheOneFound)
			{
				err_n = n;
				err_s = "don't match any choice";
			}*/
			n = last_j;
			if (n >= 0)
			{
				pSourceNode->param = last_idx;
				pSourceNode->pChild = pLastGood;
				tempDefMap = lastGoodTempDefMap;
			}
            //err_n = last_good_err_n;
            //err_s = last_good_err_s;
			if (g_grammar_log)
			{
				printAnalyzePath(analyze_path); printf(", | retrun %d, idx=%d, flags=%d, err_n=%d\n", n, last_idx, flags, m_err_n);
			}
			break;
		}

		//if (g_grammar_log)
		    analyze_path.pop_back();

        if (n < 0)
          break;

        if (bSetTheOneFlag)
        {
            TRACE("Found the one\n");
            flags |= ANALYZE_FLAG_BIT_THE_ONE;
            bSetTheOneFlag = false;
        }
	}

    if (n < 0)
      return -1;

    // skip comment words
	while (true)
	{
		std::string s = grammar_read_word(n, end_n);
		if (s.empty())
			break;
		if (!CLexer::isCommentWord(s))
		{
			n--;
			break;
		}
	}
	return n;
}

bool CGrammarAnalyzer::postIdentifierHandling(const std::string& grammarName, SourceTreeNode* pRoot)
{
    if (grammarName == "decl_var")
	{
		std::string name = declVarGetName(pRoot);
		std::string bitsValue = declVarGetBitsValue(pRoot);
		if (name.empty() && bitsValue.empty())
			return false;
	}
	else if (grammarName == "token_with_namespace")
	{
		//TokenWithNamespace twn = tokenWithNamespaceGetInfo(pRoot);
	}

	return true;
}

bool CGrammarAnalyzer::isEmpty()
{
    return m_srcfile_lexer.is_empty();
}

SourceTreeNode* CGrammarAnalyzer::getBlock(StringVector* pBlockData /* = NULL*/)
{
	//FILE* wfp = fopen("output.cpp", "wt");

	std::vector<std::string> analyze_path;
	int n = 0;
	int idx = 0;

    std::string s = grammar_read_word(n);
    if (s.empty())
        return NULL;
    /*printf("%s ", s.c_str());
    if (s == ";")
        printf("\n");*/

    SourceTreeNode* pSourceRoot = createEmptyNode();
    std::string file_name = m_gf.buffered_keywords[n - 1].file_name;
    int line_no = m_gf.buffered_keywords[n - 1].line_no;
    m_err_n = -1;
    GrammarTempDefMap tempDefMap;
    //printf("grammar analyze, %s:%d", file_name.c_str(), line_no);
    try
    {
        int flags = 0;
        n = AnalyzeGrammar(analyze_path, m_grammar_root, NULL, 0, -1, false, tempDefMap, pSourceRoot, pSourceRoot, flags);
        if (n < 0)
        {
            std::string err_string = "\n\n";
            int sz = m_gf.buffered_keywords.size();
            std::string file_name = m_gf.buffered_keywords[m_err_n].file_name;
            int line_no = m_gf.buffered_keywords[m_err_n].line_no;
            for (n = (m_err_n >= 30 ? m_err_n - 30 : 0); n < m_err_n + 30 && n < m_gf.buffered_keywords.size(); n++)
            {
                if (n == m_err_n)
                    err_string += " ^ ";
                std::string s = m_gf.buffered_keywords[n].keyword;
                if (CLexer::isCommentWord(s))
                    continue;
                if (s.empty())
                    break;
                err_string += s + " ";
            }
            //printDefineMap();

            //fprintf(wfp, "\n\n==============================\n\n");
            //for (n = 0; n < gf.buffered_keywords.size(); n++)
            //	fprintf(wfp, "%s ", gf.buffered_keywords[n].keyword.c_str());
            //fclose(wfp);
            err_string += ", " + m_err_s + " at ";
            err_string += file_name + ":" + ltoa(line_no);
            throw(err_string);
        }
        if (!m_grammar_delim.empty())
        {
            s = grammar_read_word(n);
            if (!s.empty() && s != m_grammar_delim)
                throw(std::string("delimiter '") + m_grammar_delim + "' is expected but '" + s + "' is found at " + m_gf.buffered_keywords[n - 1].file_name + ":" + ltoa(m_gf.buffered_keywords[n - 1].line_no));
        }
    }
    catch (const std::string& s)
    {
        printf("\nAnalyze failed: %s\n", s.c_str());
        MY_ASSERT(false);
        deleteSourceTreeNode(pSourceRoot);
        return NULL;
    }

    //for (int i = 0; i < n; i++)
    //	printf("%s ", gf.buffered_keywords[i].c_str());
    //printf("\n");
    //onIdentifierResolved(gf, "start", pSourceRoot, 0);

    pSourceRoot->pChild->file_name = file_name;
    pSourceRoot->pChild->line_no = line_no;
    SourceTreeNode* pRetNode = pSourceRoot->pChild;
    pSourceRoot->pChild = NULL;
    deleteSourceTreeNode(pSourceRoot);
    //s = displaySourceTreeStart(pSourceRoot, 0);
    /*printf("=========================File %s:%d=============\n", file_name.c_str(), line_no);
    for (int i = 0; i < n && i < 120; i++)
    {
      std::string s = m_gf.buffered_keywords[i].keyword;
      if (CLexer::isCommentWord(s))
          printf("%s\n", s.c_str());
      else
          printf(" %s ", s.c_str());
    }
    printf("\n");*/

    if (pBlockData)
    {
        pBlockData->clear();
        for (int i = 0; i < n; i++)
            pBlockData->push_back(m_gf.buffered_keywords[i].keyword);
    }
    m_gf.buffered_keywords.erase(m_gf.buffered_keywords.begin(), m_gf.buffered_keywords.begin() + n);

    for (PatternResultCacheMap::iterator it = m_pattern_result_cache_map.begin(); it != m_pattern_result_cache_map.end(); it++)
    {
        if (it->second.pSourceNode)
            deleteSourceTreeNode(it->second.pSourceNode);
    }
    m_pattern_result_cache_map.clear();

	return pRetNode;
}

void CGrammarAnalyzer::initWithFile(void* context, const char* file_name, int argc, char* argv[])
{
	//printf("\nAnalyzing file: %s\n", file_name);
	m_context = context;
	m_srcfile_lexer.startWithFile(file_name);

	init();
	m_grammar_root = m_grammar_map["start"];
}

void CGrammarAnalyzer::initWithBuffer(void* context, const char* file_name, const char* buffer)
{
    m_context = context;
    m_srcfile_lexer.startWithBuffer(file_name, buffer);

    init();
    m_grammar_root = m_grammar_map["start"];
}

void CGrammarAnalyzer::initWithBlocks(void* context, void* param)
{
	m_context = context;
	BracketBlock* pBlock = (BracketBlock*)param;

	m_srcfile_lexer.startWithTokens(pBlock->tokens);

	init();
	m_grammar_root = pBlock->pGrammarNode;
    m_grammar_delim = pBlock->delim;
}

void CGrammarAnalyzer::initWithTokens(void* context, const std::string& grammar, const StringVector& tokens, const std::string& delim)
{
	m_context = context;

	m_srcfile_lexer.startWithTokens(tokens);

	init();
	MY_ASSERT(m_grammar_map.find(grammar) != m_grammar_map.end());
	m_grammar_root = m_grammar_map[grammar];
	m_grammar_delim = delim;
}

StringVector CGrammarAnalyzer::bracketBlockGetTokens(void* param)
{
	BracketBlock* pBlock = (BracketBlock*)param;
	return pBlock->tokens;
}

void grammarSetCheckFunc(GrammarCheckFunc checkFunc, GrammarCallback callback)
{
	g_grammarCheckFunc = checkFunc;
	g_grammarCallback = callback;
}



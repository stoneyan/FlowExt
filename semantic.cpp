#include "semantic.h"
#include <string>
//#include <bits/stl_algobase.h>

bool g_log_semantic = false;
#define TRACE(fmt, ...)	do { if (g_log_semantic) write_log(fmt, __VA_ARGS__); } while (false)
std::string g_cur_file_name;
int g_cur_line_no;
FuncListener g_func_listener = NULL;
TypeDefPointer g_type_def_int, g_type_def_unsigned, g_type_def_bool, g_type_def_const_char_ptr, g_type_def_void, g_type_def_void_ptr, g_type_def_func_void;
SourceTreeNode* g_int_node_tree = NULL, *g_bool_node_tree = NULL;
// root is in the front
CNamespace g_global_namespace(NULL, true, true, "");
SymbolDefObject g_global_symbol_obj;
std::vector<CScope*> g_scope_stack;

std::string g_buildin_funcs =
	"typedef short __char16_t;\n"
	"typedef long __char32_t;\n"
	"void* operator new(unsigned);\n"
	"void operator delete(unsigned);\n"
	"long __builtin_expect (long exp, long c);\n"
	"void __builtin_trap (void);\n"
	"void __builtin_unreachable (void);\n"
	"void *__builtin_assume_aligned (const void *exp, long unsigned int align, ...);\n"
	"int __builtin_LINE ();\n"
	"const char * __builtin_FUNCTION ();\n"
	"const char * __builtin_FILE ();\n"
	"void __builtin___clear_cache (char *begin, char *end);\n"
	"void __builtin_prefetch (const void *addr, ...);\n"
	"double __builtin_huge_val (void);\n"
	"float __builtin_huge_valf (void);\n"
	"long double __builtin_huge_vall (void);\n"
	"int __builtin_fpclassify (int, int, int, int, int, ...);\n"
	"double __builtin_inf (void);\n"
	"float __builtin_inff (void);\n"
	"long double __builtin_infl (void);\n"
	"int		 __builtin_isinf_sign (...);\n"
	"double	  __builtin_nan (const char *str);\n"
	"float __builtin_nanf (const char *str);\n"
	"long double __builtin_nanl (const char *str);\n"
	"double __builtin_nans (const char *str);\n"
	"float __builtin_nansf (const char *str);\n"
	"long double __builtin_nansl (const char *str);\n"
	"int __builtin_ffs (int x);\n"
	"int __builtin_clz (unsigned int x);\n"
	"int __builtin_ctz (unsigned int x);\n"
	"int __builtin_clrsb (int x);\n"
	"int __builtin_popcount (unsigned int x);\n"
	"int __builtin_parity (unsigned int x);\n"
	"int __builtin_ffsl (long);\n"
	"int __builtin_clzl (unsigned long);\n"
	"int __builtin_ctzl (unsigned long);\n"
	"int __builtin_clrsbl (long);\n"
	"int __builtin_popcountl (unsigned long);\n"
	"int __builtin_parityl (unsigned long);\n"
	"int __builtin_ffsll (long long);\n"
	"int __builtin_clzll (unsigned long long);\n"
	"int __builtin_ctzll (unsigned long long);\n"
	"int __builtin_clrsbll (long long);\n"
	"int __builtin_popcountll (unsigned long long);\n"
	"int __builtin_parityll (unsigned long long);\n"
	"void* __typeof__(void*);\n"
	"void __builtin_va_start(void*, const void*);\n"
	"void __builtin_va_end(void*);\n"

	"int __sync_fetch_and_add(void* , int);\n"
	"int __sync_fetch_and_sub(void* , int);\n"
	"int __sync_fetch_and_and(void* , int);\n"
	"int __sync_fetch_and_nand(void* , int);\n"
	"int __sync_fetch_and_or(void* , int);\n"
	"int __sync_fetch_and_xor(void* , int);\n"

	"void __builtin_memcpy(void* , const void*, long unsigned);\n"
	"void __builtin_memmove(void* , const void*, long unsigned);\n"
	"int __builtin_memcmp(const void* , const void*, long unsigned);\n"
	"void __builtin_memset(void* , int, long unsigned);\n"
	"void* __builtin_memchr(const void *, int , long unsigned );\n"
	"int __builtin_strlen(const char*);\n"
	"int __builtin_strcmp(const char*, const char*);\n"
	"char* __builtin_strchr(const char*, int);\n"
	"char* __builtin_strrchr(const char*, int);\n"
	"char* __builtin_strstr(const char*, const char*);\n"
	"char* __builtin_strpbrk(const char*, const char*);\n"

	"int __builtin_vsprintf(char* __out, const char* __fmt, void* __args);\n"
	"int __builtin_vsnprintf(char* __out, int __size, const char* __fmt, void* __args);\n"
	//"uint16_t __builtin_bswap16 (uint16_t x);\n"
	//"uint32_t __builtin_bswap32 (uint32_t x);\n"
	//"uint64_t __builtin_bswap64 (uint64_t x);\n"

    "float       __builtin_cabsf(float __x);\n"
    "double      __builtin_cabs(double __x);\n"
    "double      __builtin_fabs(double __x);\n"
    "float       __builtin_fabsf(float __x);\n"
    "long double __builtin_fabsl(long double __x);\n"
    "long double __builtin_cabsl(long double __x);\n"

    "float       __builtin_cargf(float __x);\n"
    "double      __builtin_carg(double __x);\n"
    "long double __builtin_cargl(long double __x);\n"

    "float       __builtin_ceilf(float __x);\n"
    "long double __builtin_ceill(long double __x);\n"

	"float	     __builtin_acosf(float __x);\n"
	"double	     __builtin_acos(double __x);\n"
	"long double __builtin_acosl(long double __x);\n"
    "float       __builtin_cosf(float __x);\n"
    "long double __builtin_cosl(long double __x);\n"
    "double      __builtin_cos(double __x);\n"
    "float       __builtin_coshf(float __x);\n"
    "long double __builtin_coshl(long double __x);\n"
    "double      __builtin_cosh(double __x);\n"
    "float       __builtin_ccosf(float __x);\n"
    "double      __builtin_ccos(double __x);\n"
    "long double __builtin_ccosl(long double __x);\n"

    "float       __builtin_ccoshf(float __x);\n"
    "double      __builtin_ccosh(double __x);\n"
    "long double __builtin_ccoshl(long double __x);\n"

    "float       __builtin_expf(float __x);\n"
    "long double __builtin_expl(long double __x);\n"
    "double      __builtin_exp(double __x);\n"
    "float       __builtin_frexpf(float __x, int* __exp);\n"
    "long double __builtin_frexpl(long double __x, int* __exp);\n"
    "double      __builtin_frexp(double __x, int* __exp);\n"
    "float       __builtin_ldexpf(float __x, int __exp);\n"
    "long double __builtin_ldexpl(long double __x, int __exp);\n"
    "double      __builtin_ldexp(double __x, int __exp);\n"
    "float       __builtin_cexpf(float __x);\n"
    "double      __builtin_cexp(double __x);\n"
    "long double __builtin_cexpl(long double __x);\n"

    "float       __builtin_floorf(float __x);\n"
    "long double __builtin_floorl(long double __x);\n"
    "double      __builtin_floor(double __x);\n"

    "float       __builtin_logf(float __x);\n"
    "long double __builtin_logl(long double __x);\n"
    "double      __builtin_log(double __x);\n"
    "float       __builtin_clogf(float __x);\n"
    "double      __builtin_clog(double __x);\n"
    "long double __builtin_clogl(long double __x);\n"

    "float       __builtin_log10f(float __x);\n"
    "long double __builtin_log10l(long double __x);\n"
    "double      __builtin_log10(double __x);\n"

    "float       __builtin_modff(float __x, float* __iptr);\n"
    "long double __builtin_modfl(long double __x, long double* __iptr);\n"
    "float       __builtin_fmodf(float __x, float __y);\n"
    "long double __builtin_fmodl(long double __x, long double __y);\n"

    "float       __builtin_powf(float __x, float __y);\n"
    "long double __builtin_powl(long double __x, long double __y);\n"
    "double      __builtin_powi(double __x, int __i);\n"
    "float       __builtin_powif(float __x, int __n);\n"
    "long double __builtin_powil(long double __x, int __n);\n"
    "float       __builtin_cpowf(float __x, float __y);\n"
    "double      __builtin_cpow(double __x, double __y);\n"
    "long double __builtin_cpowl(long double __x, long double __y);\n"

    "float       __builtin_sinf(float __x);\n"
    "long double __builtin_sinl(long double __x);\n"
    "double      __builtin_sin(double __x);\n"
    "float       __builtin_sinhf(float __x);\n"
    "long double __builtin_sinhl(long double __x);\n"
    "double      __builtin_sinh(double __x);\n"
    "float	     __builtin_asinf(float __x);\n"
	"double	     __builtin_asin(double __x);\n"
	"long double __builtin_asinl(long double __x);\n"
    "float       __builtin_csinf(float __x);\n"
    "double      __builtin_csin(double __x);\n"
    "long double __builtin_csinl(long double __x);\n"

    "float       __builtin_csinhf(float __x);\n"
    "double      __builtin_csinh(double __x);\n"
    "long double __builtin_csinhl(long double __x);\n"

    "float       __builtin_sqrtf(float __x);\n"
    "long double __builtin_sqrtl(long double __x);\n"
    "double      __builtin_sqrt(double __x);\n"
    "float       __builtin_csqrtf(float __x);\n"
    "double      __builtin_csqrt(double __x);\n"
    "long double __builtin_csqrtl(long double __x);\n"

    "float	     __builtin_atanf(float __x);\n"
	"long double __builtin_atanl(long double __x);\n"
	"double	     __builtin_atan(double __x);\n"
	"float	     __builtin_atan2f(float __y, float __x);\n"
	"long double __builtin_atan2l(long double __y, long double __x);\n"
    "float	     __builtin_tanf(float __x);\n"
	"long double __builtin_tanl(long double__x);\n"
	"double	     __builtin_tan(double __x);\n"
    "float       __builtin_ctanf(float __x);\n"
    "double      __builtin_ctan(double __x);\n"
    "long double __builtin_ctanl(long double __x);\n"
    "float       __builtin_tanhf(float __x);\n"
    "long double __builtin_tanhl(long double__x);\n"
    "double      __builtin_tanh(double __x);\n"
    "float       __builtin_ctanhf(float __x);\n"
    "double      __builtin_ctanh(double __x);\n"
    "long double __builtin_ctanhl(long double __x);\n"

	"struct type_info;\n"
	;

bool isConstValue(const std::string& s)
{
	if (s == "true" || s == "false" || s == "null" || s == "__null")
		return true;

	if (CLexer::isNumber(s))
		return true;

	return false;
}

void enterScope(CScope* pScope)
{
	g_scope_stack.push_back(pScope);
}

void leaveScope()
{
	MY_ASSERT(!g_scope_stack.empty());
	g_scope_stack.pop_back();
}

FlowType getFlowTypeByModifierBits(const StringVector& mod_strings)
{
	if (isInModifiers(mod_strings, MODBIT_FLOW_ROOT))
		return FLOW_TYPE_FLOW_ROOT;

	if (isInModifiers(mod_strings, MODBIT_FLOW))
		return FLOW_TYPE_FLOW;

	return FLOW_TYPE_NONE;
}

std::string getSemanticTypeName(SemanticDataType type)
{
	switch (type)
	{
	case SEMANTIC_TYPE_BASIC:
		return "basic";

	case SEMANTIC_TYPE_ENUM:
		return "enum";

	case SEMANTIC_TYPE_UNION:
		return "union";

	case SEMANTIC_TYPE_STRUCT:
		return "struct";

	case SEMANTIC_TYPE_CLASS:
		return "class";

	case SEMANTIC_TYPE_TYPENAME:
		return "typename";

	case SEMANTIC_TYPE_TEMPLATE:
		return "template";

	default:
		MY_ASSERT(false);
		break;
	}

	return "";
}

SemanticDataType getSemanticTypeFromCSUType(CSUType type)
{
	switch (type)
	{
	case CSU_TYPE_ENUM:
		return SEMANTIC_TYPE_ENUM;
	case CSU_TYPE_CLASS:
		return SEMANTIC_TYPE_CLASS;
	case CSU_TYPE_STRUCT:
		return SEMANTIC_TYPE_STRUCT;
	case CSU_TYPE_UNION:
		return SEMANTIC_TYPE_UNION;
	//case CSU_TYPE_NONE:
	default:
		MY_ASSERT(false);
	}
	return SEMANTIC_TYPE_ENUM;
}

std::string getGoTypeName(GrammarObjectType t)
{
	switch (t)
	{
	case GO_TYPE_FUNC_DECL:
		return "func decl";
	case GO_TYPE_TYPEDEF:
		return "typedef";
	case GO_TYPE_VAR_DEF:
		return "var";
	case GO_TYPE_NAMESPACE:
		return "namespace";
	case GO_TYPE_ENUM:
		return "enum";
	case GO_TYPE_TEMPLATE:
		return "template";
	case GO_TYPE_CLASS:
		return "class def";
	case GO_TYPE_STATEMENT:
		return "statement";
	case GO_TYPE_EXPR2:
		return "expr2";
	case GO_TYPE_EXPR:
		return "expr";
	default:
		break;
	}
	MY_ASSERT(false);
	return "unkonwn";
}

void exprReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict);
void extendedTypeReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict);
void extendedTypeVarReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict);
void funcTypeReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict);

// this is to replace base type names in template declaration header with standard names so that it can check whether two templates are identical or not
void typeReplaceTypeNameIfAny(SourceTreeNode* pTypeNode, const std::map<std::string, std::string>& dict)
{
	int* pParamAddress = &pTypeNode->pNext->pNext->pNext->param;
	int paramOldValue = *pParamAddress;
	*pParamAddress = 0;
	if (paramOldValue)
	{
		SourceTreeNode* pScopeNode = pTypeNode->pNext->pNext->pNext->pChild->pNext->pNext->pNext->pChild;
		TokenWithNamespace twn = scopeGetInfo(pScopeNode);
		if (twn.getDepth() == 1)
		{
			std::map<std::string, std::string>::const_iterator it = dict.find(twn.getToken(0));
			if (it != dict.end())
				scopeChangeTokenName(pScopeNode, 0, it->second);
		}

		for (int i = 0; i < twn.getTemplateParamCount(twn.getDepth() - 1); i++)
		{
			//bool bType;
			SourceTreeNode* pChildNode;
			switch (twn.getTemplateParamAt(twn.getDepth() - 1, i, pChildNode))
			{
			case TEMPLATE_PARAM_TYPE_DATA:
				extendedTypeVarReplaceTypeNameIfAny(pChildNode, dict);
				break;
			case TEMPLATE_PARAM_TYPE_FUNC:
				funcTypeReplaceTypeNameIfAny(pChildNode, dict);
				break;
			case TEMPLATE_PARAM_TYPE_VALUE:
				exprReplaceTypeNameIfAny(pChildNode, dict);
				break;
			default:
				MY_ASSERT(false);
			}
		}
	}
	if (typeGetType(pTypeNode) == BASICTYPE_TYPE_BASIC)
	{
		*pParamAddress = paramOldValue;
		return;
	}
	SourceTreeNode* pUserDefNode;
	bool bHasTypename;
	CSUType csu_type = typeUserDefinedGetInfo(pTypeNode, bHasTypename, pUserDefNode);
	if (csu_type != CSU_TYPE_NONE) // this function is to replace template type parameter only, type parameter shouldn't have type prefix
	{
		*pParamAddress = paramOldValue;
		return;
	}
	TokenWithNamespace twn = userDefTypeGetInfo(pUserDefNode);

	if (twn.getDepth() == 1)
	{
		std::map<std::string, std::string>::const_iterator it = dict.find(twn.getToken(0));
		if (it != dict.end())
			userDefTypeChangeTokenName(pUserDefNode, 0, it->second);
	}

	for (int i = 0; i < twn.getTemplateParamCount(twn.getDepth() - 1); i++)
	{
		//bool bType;
		SourceTreeNode* pChildNode;
		switch (twn.getTemplateParamAt(twn.getDepth() - 1, i, pChildNode))
		{
		case TEMPLATE_PARAM_TYPE_DATA:
			extendedTypeVarReplaceTypeNameIfAny(pChildNode, dict);
			break;
		case TEMPLATE_PARAM_TYPE_FUNC:
			funcTypeReplaceTypeNameIfAny(pChildNode, dict);
			break;
		case TEMPLATE_PARAM_TYPE_VALUE:
			exprReplaceTypeNameIfAny(pChildNode, dict);
			break;
		default:
			MY_ASSERT(false);
		}
	}

	*pParamAddress = paramOldValue;
}

void extendedTypeReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict)
{
	typeReplaceTypeNameIfAny(extendedTypeGetTypeNode(pRoot), dict);
}

void extendedTypeVarReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict)
{
	SourceTreeNode* pExtendedTypeNode;
	int depth;
	extendedTypeVarGetInfo(pRoot, pExtendedTypeNode, depth);
	extendedTypeReplaceTypeNameIfAny(pExtendedTypeNode, dict);
}

// to replace return type and func param types
void funcTypeReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict)
{
	SourceTreeNode* pReturnExtendedType, *pScope, *pFuncParamsNode;
	std::string name;
	StringVector mod_strings, mod2_strings;
	int nDepth;
	funcTypeGetInfo(pRoot, pReturnExtendedType, mod_strings, pScope, nDepth, name, pFuncParamsNode, mod2_strings);

	extendedTypeReplaceTypeNameIfAny(pReturnExtendedType, dict);

	SourceTreeVector funcParamVector = funcParamsGetList(pFuncParamsNode);
	for (unsigned i = 0; i < funcParamVector.size(); i++)
	{
		FuncParamType param_type;
		StringVector mod_strings;
		SourceTreeNode* pTypeNode, *pDeclVarNode;
		void *pInitExprBlock;
		funcParamGetInfo(funcParamVector[i], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);
		if (param_type == FUNC_PARAM_TYPE_REGULAR)
			typeReplaceTypeNameIfAny(pTypeNode, dict);
		else if (param_type == FUNC_PARAM_TYPE_FUNC)
			funcTypeReplaceTypeNameIfAny(pTypeNode, dict);
	}

	/*if (pOptFuncParamsNode)
	{
		funcParamVector = funcParamsGetList(pOptFuncParamsNode);
		for (unsigned i = 0; i < funcParamVector.size(); i++)
		{
			FuncParamType param_type;
			StringVector mod_strings;
			SourceTreeNode* pTypeNode, *pDeclVarNode;
			void *pInitExprBlock;
			funcParamGetInfo(funcParamVector[i], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);
			if (param_type == FUNC_PARAM_TYPE_REGULAR)
				typeReplaceTypeNameIfAny(pTypeNode, dict);
			else if (param_type == FUNC_PARAM_TYPE_FUNC)
				funcTypeReplaceTypeNameIfAny(pTypeNode, dict);
		}
	}*/
}

void exprReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict)
{
	ExprType exprType = exprGetType(pRoot);
	switch (exprType)
	{
	case EXPR_TYPE_CONST_VALUE:
		return;

	case EXPR_TYPE_TOKEN_WITH_NAMESPACE:
	{
		TokenWithNamespace twn = tokenWithNamespaceGetInfo(exprGetSecondNode(pRoot));
		if (!twn.hasRootSign() && twn.getDepth() == 1)
		{
			std::string s = twn.getToken(0);
			std::map<std::string, std::string>::const_iterator it = dict.find(s);
			if (dict.find(s) != dict.end())
				tokenWithNamespaceChangeTokenName(exprGetSecondNode(pRoot), 0, it->second);
			break;
		}

		for (int i = 0; i < twn.getDepth(); i++)
		{
			if (!twn.scopeHasTemplate(i))
				continue;
			for (int j = 0; j < twn.getTemplateParamCount(i); j++)
			{
				SourceTreeNode* pNode;
				switch (twn.getTemplateParamAt(i, j, pNode))
				{
				case TEMPLATE_PARAM_TYPE_DATA:
					extendedTypeVarReplaceTypeNameIfAny(pNode, dict);
					break;
				case TEMPLATE_PARAM_TYPE_FUNC:
					funcTypeReplaceTypeNameIfAny(pNode, dict);
					break;
				case TEMPLATE_PARAM_TYPE_VALUE:
					exprReplaceTypeNameIfAny(pNode, dict);
					break;
				default:
					MY_ASSERT(false);
				}
			}
		}
		break;
	}

	case EXPR_TYPE_NEGATIVE:
		exprReplaceTypeNameIfAny(exprGetFirstNode(pRoot), dict);
		break;

	case EXPR_TYPE_SIZEOF:
	{
		int nType;
		SourceTreeNode* pChild;
		exprSizeOfGetInfo(pRoot, nType, pChild);
		if (nType == 0)
		{
			extendedTypeVarReplaceTypeNameIfAny(pChild, dict);
		}
		else if (nType == 1)
		{
			funcTypeReplaceTypeNameIfAny(pChild, dict);
		}
		else
		{
			exprReplaceTypeNameIfAny(pChild, dict);
		}
		break;
	}
	default:
		MY_ASSERT(false);
	}
}

/*
 * pSrc AAA, pTarget: AAA::BBB::CCC, return "BBB::CCC::name";
 * pSrc AAA::BBB::CCC, pTarget: AAA::BBB::DDD, return "BBB::DDD::name";
 * pSrc AAA, pTarget: BBB, return "::BBB::name";
 * pSrc AAA, pTarget: AAA, return "name";
 */
TokenWithNamespace getRelativeTWN2(CScope* pSrc, CScope* pTarget, const std::string& name)
{
	TokenWithNamespace twn;

	std::list<CScope*> src_path, target_path;
	std::list<CScope*>::iterator src_it, target_it;

	while ((pSrc->getGoType() == GO_TYPE_NAMESPACE && (!((CNamespace*)pSrc)->isNamespace() || !pSrc->isRealScope()))/* || pSrc->getGoType() == GO_TYPE_CLASS*/)
		pSrc = pSrc->getParent();
	while ((pTarget->getGoType() == GO_TYPE_NAMESPACE && (!((CNamespace*)pTarget)->isNamespace() || !pTarget->isRealScope()))/* || pTarget->getGoType() == GO_TYPE_CLASS*/)
		pTarget = pTarget->getParent();

	if (pSrc == pTarget)
	{
		MY_ASSERT(!name.empty());
		twn.addScope(name);
		return twn;
	}

	if (pTarget->getParent() == NULL)
	{
		MY_ASSERT(!name.empty());
		twn.addScope(name);
		twn.setHasRootSign(true);
		return twn;
	}

	CScope* pObj;
	for (pObj = pTarget; pObj->getParent(); pObj = pObj->getParent())
	{
		if (pObj == pSrc)
			break;
		target_path.push_front(pObj);
	}

	target_it = target_path.begin();
	if (pObj != pSrc && pSrc->getParent() != NULL)
	{
		for (pObj = pSrc; pObj->getParent(); pObj = pObj->getParent())
		{
			if (pObj->isRealScope())
				src_path.push_front(pObj);
		}

		src_it = src_path.begin();
		if (*src_it != *target_it)
			twn.setHasRootSign(true);
		else
		{
			for (; src_it != src_path.end() && target_it != target_path.end(); src_it++, target_it++)
			{
				if (*src_it != *target_it)
					break;
			}
			if (target_it == target_path.end())
				target_it--;
		}
	}

	std::string ret_s;

	for (; target_it != target_path.end(); target_it++)
	{
		std::string s;
		CScope* pTargetScope = *target_it;
		GrammarObjectType goType = pTargetScope->getGoType();
		switch (goType)
		{
		case GO_TYPE_NAMESPACE:
			if (!((CNamespace*)pTargetScope)->isNamespace())
				continue;
			s = ((CNamespace*)pTargetScope)->getName();
			MY_ASSERT(!s.empty());
			break;
		case GO_TYPE_CLASS:
			s = ((CClassDef*)pTargetScope)->getName();
			//MY_ASSERT(!s.empty());
			break;
		case GO_TYPE_TEMPLATE:
			s = ((CTemplate*)pTargetScope)->getName();
			//MY_ASSERT(!s.empty());
			continue;
		case GO_TYPE_FUNC:
			s = ((CFunction*)pTargetScope)->getName();
			//MY_ASSERT(!s.empty());
			break;
		default:
			MY_ASSERT(false);
		}
		twn.addScope(s);
	}
	if (!name.empty())
	{
		twn.addScope(name);
	}

	return twn;
}

TokenWithNamespace getRelativeTWN(CScope* pTarget, const std::string& name)
{
	MY_ASSERT(g_scope_stack.size() > 0);
	CScope* pSrc = g_scope_stack.back();

	return getRelativeTWN2(pSrc, pTarget, name);
}

std::string getRelativePath(CScope* pTarget, const std::string& name)
{
	TokenWithNamespace twn = getRelativeTWN(pTarget, name);
	return twn.toString();
	/*bool bHasRootSign = false;
	std::list<CScope*> src_path, target_path;
	std::list<CScope*>::iterator src_it, target_it;

	MY_ASSERT(g_scope_stack.size() > 0);
	CScope* pSrc = g_scope_stack.back();
	//std::string temp_s = pSrc->getDebugPath();

	while ((pSrc->getGoType() == GO_TYPE_NAMESPACE && (!((CNamespace*)pSrc)->isNamespace() || !pSrc->isRealScope())))
		pSrc = pSrc->getParent();
	while ((pTarget->getGoType() == GO_TYPE_NAMESPACE && (!((CNamespace*)pTarget)->isNamespace() || !pTarget->isRealScope())))
		pTarget = pTarget->getParent();

	if (pSrc == pTarget)
	{
		MY_ASSERT(!name.empty());
		return name;
	}

	if (pTarget->getParent() == NULL)
	{
		MY_ASSERT(!name.empty());
		return "::" + name;
	}

	CScope* pObj;
	for (pObj = pTarget; pObj->getParent(); pObj = pObj->getParent())
	{
		if (pObj == pSrc)
			break;
		target_path.push_front(pObj);
	}

	target_it = target_path.begin();
	if (pObj != pSrc && pSrc->getParent() != NULL)
	{
		for (pObj = pSrc; pObj->getParent(); pObj = pObj->getParent())
		{
			if (pObj->isRealScope())
				src_path.push_front(pObj);
		}

		src_it = src_path.begin();
		if (*src_it != *target_it)
			bHasRootSign = true;
		else
		{
			for (; src_it != src_path.end() && target_it != target_path.end(); src_it++, target_it++)
			{
				if (*src_it != *target_it)
					break;
			}
			if (target_it == target_path.end())
				target_it--;
		}
	}

	std::string ret_s;

	for (; target_it != target_path.end(); target_it++)
	{
		std::string s;
		CScope* pTargetScope = *target_it;
		GrammarObjectType goType = pTargetScope->getGoType();
		switch (goType)
		{
		case GO_TYPE_NAMESPACE:
			if (!((CNamespace*)pTargetScope)->isNamespace())
				continue;
			s = ((CNamespace*)pTargetScope)->getName();
			MY_ASSERT(!s.empty());
			break;
		case GO_TYPE_CLASS:
			s = ((CClassDef*)pTargetScope)->getName();
			//MY_ASSERT(!s.empty());
			break;
		case GO_TYPE_TEMPLATE:
			s = ((CTemplate*)pTargetScope)->getName();
			//MY_ASSERT(!s.empty());
			continue;
		case GO_TYPE_FUNC:
			s = ((CFunction*)pTargetScope)->getName();
			//MY_ASSERT(!s.empty());
			break;
		default:
			MY_ASSERT(false);
		}
		if (!ret_s.empty())
			ret_s += "::";
		ret_s += s;
	}
	if (!name.empty())
	{
		if (!ret_s.empty())
			ret_s += "::";
		ret_s += name;
	}

	if (bHasRootSign)
		ret_s = std::string("::") + ret_s;

	return ret_s;*/
}

int comparableWith(TypeDefPointer pDefTypeDef, TypeDefPointer pRealTypeDef)
{
	int score = 0;

	int def_depth = 0, real_depth = 0;
	bool bDefConst = false, bRealConst = false, bDefRef = false;

	bDefConst |= pDefTypeDef->isConst();
	bDefRef |= pDefTypeDef->isReference();
	while (!pDefTypeDef->isBaseType())
	{
		def_depth += pDefTypeDef->getDepth();
		pDefTypeDef = pDefTypeDef->getBaseType();
		bDefConst |= pDefTypeDef->isConst();
		bDefRef |= pDefTypeDef->isReference();
	}

	if (def_depth == 0 && (!bDefRef || bDefConst) && (pDefTypeDef->getType() == SEMANTIC_TYPE_CLASS || pDefTypeDef->getType() == SEMANTIC_TYPE_STRUCT))
	{
		if (pDefTypeDef->getClassDef()->hasConstructorOrCanBeAssignedWith(pRealTypeDef))
			return 10;
	}

	bRealConst |= pRealTypeDef->isConst();
	while (!pRealTypeDef->isBaseType())
	{
		real_depth += pRealTypeDef->getDepth();
		pRealTypeDef = pRealTypeDef->getBaseType();
		bRealConst |= pRealTypeDef->isConst();
	}
	score += (bDefConst == bRealConst ? 10 : 0);
	TRACE("defConst=%d, realConst=%d, ", bDefConst, bRealConst);

	if (pDefTypeDef->isVoid() && (real_depth >= def_depth || pRealTypeDef->toString() == "__builtin_va_list"))
		return score + def_depth;

	TRACE("dep_depth=%d, real_depth=%d, isZero=%d, ", def_depth, real_depth, pRealTypeDef->isZero());
	if (def_depth != real_depth && !pRealTypeDef->isZero())
		return -1;

	if (def_depth > 0 && real_depth == 0 && pRealTypeDef->isZero())
		return score + 10;

	score += def_depth;
	if (bDefConst == bRealConst)
		score += 10;

	SemanticDataType type = pDefTypeDef->getType();
	switch (pDefTypeDef->getType())
	{
	case SEMANTIC_TYPE_BASIC:
	case SEMANTIC_TYPE_ENUM:
	{
		if (pRealTypeDef->getType() != SEMANTIC_TYPE_BASIC && pRealTypeDef->getType() != SEMANTIC_TYPE_ENUM)
			return -1;

		std::string def_type, real_type;
		if (pDefTypeDef->getType() == SEMANTIC_TYPE_ENUM)
			def_type = "int";
		else
			def_type = trim(pDefTypeDef->toString());
		if (pRealTypeDef->getType() == SEMANTIC_TYPE_ENUM)
			real_type = "int";
		else
			real_type = trim(pRealTypeDef->toString());

		//TRACE("def_type=%s:%d, real_type=%s:%d, ", def_type.c_str(), pDefTypeDef->getType(), real_type.c_str(), pRealTypeDef->getType());
		return score += (def_type == real_type ? 10 : 0);
	}

	case SEMANTIC_TYPE_CLASS:
	case SEMANTIC_TYPE_STRUCT:
	case SEMANTIC_TYPE_UNION:
	{
		MY_ASSERT(pDefTypeDef->getClassDef());

		if (pRealTypeDef->getType() == SEMANTIC_TYPE_CLASS)
		{
			MY_ASSERT(pRealTypeDef->getClassDef());
			if (pRealTypeDef->getClassDef()->hasBaseClass(pDefTypeDef->getClassDef()))
				return score + 10;
		}

		if (pDefTypeDef->getClassDef()->hasConstructorOrCanBeAssignedWith(pRealTypeDef))
			return score + 10;
		break;
	}
	case SEMANTIC_TYPE_FUNC:
	{
		if (def_depth > 0 || real_depth > 0)
			return -1;
		if (pRealTypeDef->getType() != SEMANTIC_TYPE_FUNC)
			return -1;
		std::vector<TypeDefPointer> realTypeList;
		for (int i = 0; i < pRealTypeDef->getFuncParamCount(); i++)
			realTypeList.push_back(pRealTypeDef->getFuncParamAt(i)->getType());
		score = pDefTypeDef->checkCallParams(realTypeList, false);
		if (score > 0)
			return score + 10;
		break;
	}
	default:
		MY_ASSERT(false);
	}

	return -1;
}

TypeDefPointer createTypeByDepth(TypeDefPointer pTypeDef, int depth)
{
	if (pTypeDef->getDepth() == depth)
		return pTypeDef;

	if (depth == 0)
		return pTypeDef->getBaseType();

	//SourceTreeNode* pDeclVar = declVarCreateByName("");
	//for (int i = 0; i < depth; i++)
	//	declVarAddModifier(pDeclVar, DVMOD_TYPE_POINTER);

	return TypeDefPointer(new CTypeDef(NULL, "", pTypeDef->getBaseType(), depth));
}

// compare func param types
bool isSameFuncType(TypeDefPointer pType1, TypeDefPointer pType2)
{
	MY_ASSERT(pType1->getType() == SEMANTIC_TYPE_FUNC);
	MY_ASSERT(pType2->getType() == SEMANTIC_TYPE_FUNC);

	if (pType1->getFuncParamCount() != pType2->getFuncParamCount())
		return false;

	for (int i = 0; i < pType1->getFuncParamCount(); i++)
	{
		if (pType1->getFuncParamAt(i)->getType()->toFullString() != pType2->getFuncParamAt(i)->getType()->toFullString())
			return false;
	}

	return true;
}

std::string CGrammarObject::getPath()
{
    std::string s;
    if (m_pParent)
        s = m_pParent->getDebugPath();

    if (!getName().empty())
        s += "::" + getName();
    return s;
}

std::string CGrammarObject::getDebugPath()
{
	std::string s;
	if (m_pParent)
		s = m_pParent->getDebugPath();

	char buf[20];
	sprintf(buf, "(0x%lx)", (long)this);
	s += std::string("/") + ltoa(getGoType()) + ":" + getDebugName() + buf;
	return s;
}

CScope* CGrammarObject::getParentScope(GrammarObjectType go_type)
{
	CGrammarObject* pGO = this;
	while (pGO)
	{
		if (pGO->getGoType() == go_type)
			return (CScope*)pGO;
		pGO = pGO->getParent();
	}

	return NULL;
}

CTemplate* CGrammarObject::getParentTemplate()
{
	switch (getGoType())
	{
	case GO_TYPE_NAMESPACE:
	  return NULL;
	case GO_TYPE_TEMPLATE:
	{
		CTemplate* pTemplate = (CTemplate*)this;
		if (pTemplate->getTemplateType() == TEMPLATE_TYPE_FUNC)
			return pTemplate;
		MY_ASSERT(pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS);
		return pTemplate->isInstancedTemplate() ? NULL : (CTemplate*)this;
	}
	case GO_TYPE_CLASS:
	{
		SemanticDataType type = ((CClassDef*)this)->getType();
		if (type == SEMANTIC_TYPE_STRUCT || type == SEMANTIC_TYPE_CLASS)
		{
			if (((CClassDef*)this)->isWithinTemplate())
				return getParent()->getParentTemplate();
			if (getParent()->getGoType() == GO_TYPE_TEMPLATE && ((CTemplate*)getParent())->getTemplateName() == getParent()->getName())
				return (CTemplate*)getParent();
			return NULL;
		}
		break;
	}
	default:
		break;
	}

	if (!getParent())
		return NULL;

	return getParent()->getParentTemplate();
}

bool CGrammarObject::isParentOf(CGrammarObject* pGO)
{
	while (pGO != NULL)
	{
		pGO = pGO->getParent();
		if (pGO == this)
			return true;
	}

	return false;
}

void CTypeDef::init()
{
	m_pSpecialType = NULL;
	m_depth = 0;
	m_bReference = false;
	m_bHasVArgs = false;
	m_pFuncReturnTypeNode = NULL;
	m_nFuncThrow = 0;
	m_pThrowTypeNode = NULL;
	m_bPureVirtual = false;
	m_bZero = false;
	m_display_flag = false;
}

CTypeDef::CTypeDef(CScope* pScope, const std::string& name, const StandardType& basic_tokens) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();

	if (!name.empty())
		MY_ASSERT(pScope);
	//m_pScope = pScope;
	MY_ASSERT(basic_tokens.size() > 0);
	m_name = name;
	m_type = SEMANTIC_TYPE_BASIC;
	m_basic_tokens = basic_tokens;
	TRACE("CREATING CTypeDef basic, %s\n", getDebugPath().c_str());
}
// struct or union or enum
CTypeDef::CTypeDef(CScope* pScope, const std::string& name, CClassDef* pClassDef) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();

	if (!name.empty())
		MY_ASSERT(pScope);
	//m_pScope = pScope;
	m_name = name;
	m_pSpecialType = pClassDef;
	m_type = pClassDef->getType();
	setPrefix(getSemanticTypeName(pClassDef->getType()) + " ");
    TRACE("CREATING CTypeDef struct, %s\n", getDebugPath().c_str());
}
// func or func ptr, depth can be 0 or 1
CTypeDef::CTypeDef(CScope* pScope, const std::string& name, SemanticDataType type, TypeDefPointer pReturnType, int depth) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();
	MY_ASSERT(type == SEMANTIC_TYPE_FUNC);

	if (!name.empty())
		MY_ASSERT(pScope);
	m_name = name;
	m_type = SEMANTIC_TYPE_FUNC;
	m_depth = depth;
	m_pFuncReturnType = pReturnType;
    TRACE("CREATING CTypeDef func, %s\n", getDebugPath().c_str());
}

CTypeDef::CTypeDef(CScope* pScope, const std::string& name, SemanticDataType type, TypeDefPointer pTypeDef, TypeDefPointer pDataScope) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();

	MY_ASSERT(type == SEMANTIC_TYPE_DATA_MEMBER_POINTER);
	MY_ASSERT(pScope);
	SemanticDataType t = pDataScope->getType();
	MY_ASSERT(pDataScope->getType() == SEMANTIC_TYPE_CLASS || pDataScope->getType() == SEMANTIC_TYPE_STRUCT || pDataScope->getType() == SEMANTIC_TYPE_TEMPLATE);
	m_name = name;
	m_type = type;
	m_pBaseTypeDef = pTypeDef;
	m_pFuncReturnType = pDataScope;
	m_depth = 1;
    TRACE("CREATING CTypeDef data member pointer, %s\n", getDebugPath().c_str());
}

CTypeDef::CTypeDef(CTemplate* pTemplate) : CGrammarObject(pTemplate), m_shared_from_this(this)
{
	init();

	m_name = pTemplate->getName();
	m_type = SEMANTIC_TYPE_TEMPLATE;
	m_pSpecialType = pTemplate;
    TRACE("CREATING CTypeDef template, %s\n", getDebugPath().c_str());
}

CTypeDef::CTypeDef(CScope* pScope, const std::string& name) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();

	m_name = name;
	m_type = SEMANTIC_TYPE_TYPENAME;
    TRACE("CREATING CTypeDef typename, %s\n", getDebugPath().c_str());
}

CTypeDef::CTypeDef(CScope* pScope, const std::string& name, const TokenWithNamespace& twn) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();

	m_name = name;
	m_type = SEMANTIC_TYPE_TYPEOF;
	m_typeof_twn = twn;
    TRACE("CREATING CTypeDef typeof, %s\n", getDebugPath().c_str());
}

// complicate one
CTypeDef::CTypeDef(CScope* pScope, const std::string& name, TypeDefPointer pBaseTypeDef, int extra_depth/*, bool bReference, bool bConst*/) : CGrammarObject(pScope), m_shared_from_this(this)
{
	init();

	if (!name.empty())
		MY_ASSERT(pScope);
	//MY_ASSERT(!pBaseTypeDef->getName().empty() || pBaseTypeDef->isBaseType());

	//m_pScope = pScope;
	m_name = name;
	m_pBaseTypeDef = pBaseTypeDef;
	m_type = m_pBaseTypeDef->getType();
	//m_pDeclVarNode = pDeclVar;
	//if (m_pDeclVarNode)
	//{
		m_depth = extra_depth; //declVarGetDepth(m_pDeclVarNode);
		//m_bReference = bReference; //declVarIsReference(m_pDeclVarNode);
		//setConst(bConst);
	//}
    TRACE("CREATING CTypeDef complex, %s, depth=%d, ref=%d\n", getDebugPath().c_str(), m_depth, m_bReference);
}

CTypeDef::~CTypeDef()
{
	for (size_t i = 0; i < m_func_params.size(); i++)
	{
		CVarDef* pVarDef = m_func_params[i];
		delete pVarDef;
	}

	//deleteSourceTreeNode(m_pDeclVarNode);
	deleteSourceTreeNode(m_pThrowTypeNode);
}

std::string CTypeDef::getDebugName()
{
	return getName() + ":" + ltoa(getType());
}

/*SourceTreeNode* CTypeDef::generateTypeNode()
{
	if (isBaseType())
	{
		MY_ASSERT(m_pBasicNode);
		return dupSourceTreeNode(m_pBasicNode);
	}

	SourceTreeNode* pRoot;
	if (m_pBaseTypeDef->getName().empty())
		pRoot = m_pBaseTypeDef->generateTypeNode();
	else
		pRoot = typeCreateUserDefined(m_pBaseTypeDef->getName());

	return pRoot;
}

SourceTreeNode* CTypeDef::generateExtendedTypeNode()
{
	if (isBaseType())
		return extendedTypeCreateFromType(m_pBasicNode);

	SourceTreeNode* pRoot;
	if (m_pBaseTypeDef->getName().empty())
		pRoot = m_pBaseTypeDef->generateTypeNode();
	else
		pRoot = typeCreateUserDefined(m_pBaseTypeDef->getName());
	pRoot = extendedTypeCreateFromType(pRoot);

	for (int i = 0; i < declVarGetDepth(m_pBasicNode); i++)
	{
		if (declVarPointerIsConst(m_pBasicNode, i))
			extendedTypeAddModifier(pRoot, DVMOD_TYPE_CONST_POINTER);
		else
			extendedTypeAddModifier(pRoot, DVMOD_TYPE_POINTER);
	}

	return pRoot;
}*/

bool CTypeDef::isConst()
{
	if (m_type == SEMANTIC_TYPE_FUNC) 
		return isInModifiers(m_mod4_strings, MODBIT_CONST);

	return isInModifiers(m_mod_strings, MODBIT_CONST);
}

void CTypeDef::setConst(bool bConst)
{
	if (m_type == SEMANTIC_TYPE_FUNC)
	{
		if (bConst) 
			addToModifiers(m_mod4_strings, MODBIT_CONST);
		else
			removeFromModifiers(m_mod4_strings, MODBIT_CONST);
	}
	else
	{
		if (bConst) 
			addToModifiers(m_mod_strings, MODBIT_CONST);
		else
			removeFromModifiers(m_mod_strings, MODBIT_CONST);
	}
}

bool CTypeDef::isVolatile()
{
	//MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC); 
	return isInModifiers(m_mod4_strings, MODBIT_VOLATILE);
}

bool CTypeDef::isVirtual()
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC); 
	return isInModifiers(m_mod_strings, MODBIT_VIRTUAL);
}

void CTypeDef::setModStrings(const StringVector& mod_strings)
{
	m_mod_strings = mod_strings;
}

void CTypeDef::setMod2Strings(const StringVector& mod_strings)
{
	m_mod2_strings = mod_strings;
}

void CTypeDef::setMod3Strings(const StringVector& mod_strings)
{
	m_mod3_strings = mod_strings;
}

void CTypeDef::setMod4Strings(const StringVector& mod_strings)
{
	m_mod4_strings = mod_strings;
}

void CTypeDef::addFuncParam(CVarDef* pVarDef)
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	m_func_params.push_back(pVarDef);
}

CVarDef* CTypeDef::getFuncParamAt(int i)
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	return m_func_params[i];
}

void CTypeDef::setThrow(int nThrow, SourceTreeNode* pThrowTypeNode)
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	m_nFuncThrow = nThrow;
	m_pThrowTypeNode = pThrowTypeNode;
}

// return -1 means failed. return matching score
int CTypeDef::checkCallParams(const std::vector<TypeDefPointer>& typeList, bool bCallerIsConst)
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	TRACE("CTypeDef::%s, func %s defined in %s, param_size=%lu, const=%d, ", __FUNCTION__, getName().c_str(), definedIn().c_str(), m_func_params.size(), isConst());

	if (typeList.size() == 0 && m_func_params.size() == 1)
	{
		//TRACE("***checkCallParams, type='%s'\n", m_func_params[0]->getType()->toString().c_str());
		if (m_func_params[0]->getType()->toString() == "void ")
		{
			if (isConst())
			{
				TRACE("void return %d\n", (bCallerIsConst ? 0 : 1));
				return bCallerIsConst ? 0 : 1;
			}
			TRACE("void return %d\n", (bCallerIsConst ? -1 : 0));
			return (bCallerIsConst ? -1 : 0);
		}
	}

	if (typeList.size() > m_func_params.size() && !m_bHasVArgs)
	{
		TRACE("vargs return -1\n");
		return -1;
	}
	int score = 0;
	for (unsigned i = 0; i < m_func_params.size(); i++)
	{
		CVarDef* pVarDef = m_func_params[i];

		if (i < typeList.size())
		{
			int n = comparableWith(pVarDef->getType(), typeList[i]);
			TRACE("checkCallParams, i=%u, def=%s, real=%s, n=%d; ", i, pVarDef->getType()->toFullString().c_str(), typeList[i]->toFullString().c_str(), n);
			if (n < 0)
				return -1;
			score += n;
		}
		else if (pVarDef->getInitExpr() == NULL)
		{
			TRACE("insufficient params at %u, return -1\n", i);
			return -1;
		}
		//CExpr* pExpr = param_v[i];
	}

	if (isConst())
	{
		TRACE("score=%d, return %d\n", score, score + (bCallerIsConst ? 1 : 0));
		return score + (bCallerIsConst ? 1 : 0);
	}
	TRACE("score=%d, return %d\n", score, (bCallerIsConst ? -1 : score + 1));
	return (bCallerIsConst ? -1 : score + 1);
}

/*std::string CTypeDef::funcTypeToString(const std::string& name)
{
	std::string ret_s;

	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);

	if (m_pFuncReturnType)
		ret_s += m_pFuncReturnType->toString() + " ";
	if (isBaseType())
		ret_s += name;
	else
		ret_s += "(*" + name + ")";
	ret_s += "(";
	for (int i = 0; i < m_func_params.size(); i++)
	{
		CVarDef* pVarDef = m_func_params[i];
		ret_s += pVarDef->getType()->getBaseType()->toString() + " " + pVarDef->toString();
		if (i < m_func_params.size() - 1)
			ret_s += ", ";
	}
	if (m_bHasVArgs)
		ret_s += ", ...";
	ret_s += ")";

	return ret_s;
}*/

FlowType CTypeDef::getFuncFlowType()
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	return getFlowTypeByModifierBits(m_mod_strings);
}

void CTypeDef::setFuncFlowType(FlowType flow)
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	if (getFuncFlowType() == flow)
		return;

	if (flow == FLOW_TYPE_NONE)
	{
		removeFromModifiers(m_mod_strings, MODBIT_FLOW);
		removeFromModifiers(m_mod_strings, MODBIT_FLOW_ROOT);
	}
	else if (flow == FLOW_TYPE_NONE)
	{
		m_mod_strings.insert(m_mod_strings.begin(), modifierBit2String(MODBIT_FLOW));
	}
	else
	{
		m_mod_strings.insert(m_mod_strings.begin(), modifierBit2String(MODBIT_FLOW_ROOT));
	}
}

std::string CTypeDef::toString(int depth)
{
	std::string ret_s;

	if (!m_display_str.empty())
		return m_display_str;

	if (isConst())
		ret_s += "const ";

	ret_s += getPrefix();

	if (isBaseType())
	{
		switch (m_type)
		{
		case SEMANTIC_TYPE_BASIC:
			MY_ASSERT(m_basic_tokens.size() > 0);
			for (size_t i = 0; i < m_basic_tokens.size(); i++)
			{
				const std::string& s = m_basic_tokens[i];
				ret_s += s + " ";
			}
			ret_s.resize(ret_s.size() - 1);
			break;
		case SEMANTIC_TYPE_FUNC:
		{
			ret_s += toFuncString(m_name);
			break;
		}
		case SEMANTIC_TYPE_DATA_MEMBER_POINTER:
			ret_s += m_pBaseTypeDef->toString() + " " + m_pFuncReturnType->toString() + "::*";
			break;

		case SEMANTIC_TYPE_TYPENAME:
		{
			ret_s += m_name;
			break;
		}
		case SEMANTIC_TYPE_TYPEOF:
		{
			ret_s += "__typeof(" + m_typeof_twn.toString() + ")";
			break;
		}
		case SEMANTIC_TYPE_TEMPLATE:
		{
			ret_s += "template " + m_name;
			break;
		}
		default:
			MY_ASSERT(m_pSpecialType);
			ret_s += ((CClassDef*)m_pSpecialType)->toString(depth);
			break;
		}
		return ret_s;
	}

	if (!m_name.empty())
	{
		MY_ASSERT(getParent());
		ret_s += getRelativePath(getParent(), m_name);
	}
	else
		ret_s += m_pBaseTypeDef->toString();
	//if (m_pDeclVarNode)
	//	ret_s += displaySourceTreeDeclVar(m_pDeclVarNode, " ");
	for (int i = 0; i < m_depth; i++)
		ret_s += "*";
	if (isReference())
		ret_s += "&";

	return ret_s;
}

std::string CTypeDef::toFuncString(const std::string& name)
{
	MY_ASSERT(m_type == SEMANTIC_TYPE_FUNC);
	std::string ret_s;

	if (!isBaseType())
	{
		ret_s = getBaseType()->toFuncString(name);
		for (int i = 0; i < m_depth; i++)
			ret_s += "*";
		return ret_s;
	}

	ret_s = combineStrings(m_mod_strings);
    if (m_pFuncReturnType)
	{
		if (m_pFuncReturnTypeNode)
			ret_s += displaySourceTreeExtendedType(m_pFuncReturnTypeNode);
		else
			ret_s += m_pFuncReturnType->toString();
		ret_s += " ";
	}
	ret_s += combineStrings(m_mod2_strings);

	std::string s2 = combineStrings(m_mod3_strings);
	for (int i = 0; i < m_depth; i++)
		s2 += "*";
	s2 += name;
	if (m_depth > 0 || m_display_flag)
		s2 = "(" + s2 + ")";
	ret_s += s2;

	ret_s += "(";
	for (unsigned i = 0; i < m_func_params.size(); i++)
	{
		if (i > 0)
			ret_s += ", ";
		CVarDef* pVarDef = m_func_params[i];
		TypeDefPointer pTypeDef = pVarDef->getType();
		if (pTypeDef->getType() == SEMANTIC_TYPE_FUNC)
		{
			ret_s += combineStrings(pTypeDef->getModStrings());
			pTypeDef = pTypeDef->getBaseType();
			while (!pTypeDef->isBaseType() && pTypeDef->getName().empty() && pTypeDef->getDepth() == 0)
				pTypeDef = pTypeDef->getBaseType();
			if (pTypeDef->getName().empty())
				ret_s += pTypeDef->toFuncString(pVarDef->toString());
			else
				ret_s += pTypeDef->getName() + " " + pVarDef->toString();
		}
		else
			ret_s += pTypeDef->getBaseType()->toString() + " " + pVarDef->toString();
	}
	if (m_bHasVArgs)
		ret_s += ", ...";
	ret_s += ") ";

	ret_s += combineStrings(m_mod4_strings);

	if (m_nFuncThrow > 0)
	{
		ret_s += "throw(";
		if (m_nFuncThrow == 2)
			ret_s += displaySourceTreeType(m_pThrowTypeNode, 0);
		else if (m_nFuncThrow == 3)
			ret_s += "...";
		ret_s += ")";
	}

	if (m_bPureVirtual)
		ret_s += " = 0";
	return ret_s;
}

int CTypeDef::getFullDepth()
{
	if (isBaseType())
		return 0;

	int depth = getBaseType()->getFullDepth();

	//if (m_pDeclVarNode)
		depth += m_depth; //declVarGetDepth(m_pDeclVarNode);

	return depth;
}

std::string CTypeDef::toFullString()
{
	std::string ret_s;

	bool bConst = isConst();

    if (isBaseType())
    {
        switch (m_type)
        {
        case SEMANTIC_TYPE_BASIC:
            MY_ASSERT(m_basic_tokens.size() > 0);
            for (size_t i = 0; i < m_basic_tokens.size(); i++)
			{
				const std::string& s = m_basic_tokens[i];
                ret_s += s + " ";
			}
            ret_s.resize(ret_s.size() - 1);
            break;
        case SEMANTIC_TYPE_FUNC:
        {
			ret_s += toFuncString(m_name);
            break;
        }
        case SEMANTIC_TYPE_DATA_MEMBER_POINTER:
            ret_s += m_pBaseTypeDef->toString() + " " + m_pFuncReturnType->toString() + "::*";
            break;

        case SEMANTIC_TYPE_TYPENAME:
        {
            ret_s += m_name;
            break;
        }
        case SEMANTIC_TYPE_TYPEOF:
        {
            ret_s += "__typeof(" + m_typeof_twn.toString() + ")";
            break;
        }
        case SEMANTIC_TYPE_TEMPLATE:
        {
            ret_s += "template " + m_name;
            break;
        }
        default:
            MY_ASSERT(m_pSpecialType);
            ret_s += ((CClassDef*)m_pSpecialType)->getPath();
            break;
        }
    }
    else
    {
        ret_s += getBaseType()->toFullString();

		for (int i = 0; i < m_depth; i++)
			ret_s += "*";
	}

	if (bConst && (ret_s.size() < 6 || ret_s.substr(ret_s.size() - 6, 6) != " const"))
		ret_s += " const";
	if (isReference())
	    ret_s += "&";

	return ret_s;
}

bool CTypeDef::isVoid()
{
	if (m_depth != 0 || m_type != SEMANTIC_TYPE_BASIC)
		return false;
	
	if (isBaseType())
		return m_basic_tokens.size() == 1 && m_basic_tokens[0] == "void";

	return getBaseType()->isVoid();
}

bool CTypeDef::is_abstract()
{
	if (!isBaseType())
		return false;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->is_abstract();
}

bool CTypeDef::is_class()
{
	if (!isBaseType())
		return false;

	return (m_type == SEMANTIC_TYPE_CLASS || m_type == SEMANTIC_TYPE_STRUCT);
}

bool CTypeDef::is_empty()
{
	if (!isBaseType())
		return false;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->is_empty();
}

bool CTypeDef::is_enum()
{
    if (!isBaseType())
        return false;

    if (m_type != SEMANTIC_TYPE_ENUM)
        return false;

    return true;
}

// Returns true if the type is a class or union with no constructor or private or protected non-static members,
// no base classes, and no virtual functions. See the C++ standard, sections 8.5.1/1, 9/4, and 3.9/10 for more information on PODs.
bool CTypeDef::is_pod()
{
	if (!isBaseType())
	{
		if (getDepth() > 0)
			return true;
		return getBaseType()->is_pod();
	}

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT && m_type != SEMANTIC_TYPE_UNION)
		return true; // seems GNU return true while MSVC returns false

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->is_pod();
}

bool CTypeDef::has_nothrow_assign()
{
	if (getDepth() > 0)
		return true;

	if (!isBaseType())
		return getBaseType()->has_nothrow_assign();

	if (m_type == SEMANTIC_TYPE_BASIC || m_type == SEMANTIC_TYPE_ENUM || m_type == SEMANTIC_TYPE_UNION)
		return true;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->has_nothrow_assign();
}

bool CTypeDef::has_nothrow_copy()
{
	if (getDepth() > 0)
		return true;

	if (!isBaseType())
		return getBaseType()->has_nothrow_copy();

	if (m_type == SEMANTIC_TYPE_BASIC || m_type == SEMANTIC_TYPE_ENUM || m_type == SEMANTIC_TYPE_UNION)
		return true;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->has_nothrow_copy();
}

bool CTypeDef::has_trivial_assign()
{
	if (getDepth() > 0)
		return true;

	if (!isBaseType())
		return getBaseType()->has_trivial_assign();

	if (m_type == SEMANTIC_TYPE_BASIC || m_type == SEMANTIC_TYPE_ENUM || m_type == SEMANTIC_TYPE_UNION)
		return true;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->has_trivial_assign();
}

bool CTypeDef::has_trivial_copy()
{
	if (getDepth() > 0)
		return true;

	if (!isBaseType())
		return getBaseType()->has_trivial_copy();

	if (m_type == SEMANTIC_TYPE_BASIC || m_type == SEMANTIC_TYPE_ENUM || m_type == SEMANTIC_TYPE_UNION)
		return true;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->has_trivial_copy();
}

bool CTypeDef::has_trivial_destructor()
{
	if (getDepth() > 0)
		return true;

	if (!isBaseType())
		return getBaseType()->has_trivial_destructor();

	if (m_type == SEMANTIC_TYPE_BASIC || m_type == SEMANTIC_TYPE_ENUM)
		return true;

	if (m_type != SEMANTIC_TYPE_CLASS && m_type != SEMANTIC_TYPE_STRUCT)
		return false;

	MY_ASSERT(m_pSpecialType);
	return ((CClassDef*)m_pSpecialType)->has_trivial_destructor();
}

std::string CTypeDef::type2String(SemanticDataType basic_type)
{
	std::string s;
	switch (basic_type)
	{
	case SEMANTIC_TYPE_BASIC:
		s = "basic";
		break;
	case SEMANTIC_TYPE_ENUM:
		s = "enum;";
		break;
	case SEMANTIC_TYPE_UNION:
		s = "union";
		break;
	case SEMANTIC_TYPE_STRUCT:
		s = "struct";
		break;
	case SEMANTIC_TYPE_CLASS:
		s = "class";
		break;
	case SEMANTIC_TYPE_FUNC:
		s = "func;";
		break;
	default:
		MY_ASSERT(false);
	}

	return s;
}

CVarDef::CVarDef(CScope* pParent, const TokenWithNamespace& twn, TypeDefPointer pTypeDef, SourceTreeNode* pDeclVar, CExpr* pInitExpr) : CGrammarObject(pParent)
{
	//m_pParent = pParent;
	m_twn = twn;
	m_name = m_twn.toString();
	m_type = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, declVarGetDepth(pDeclVar)));
	m_bHasConstructor = false;
	m_pInitExpr = pInitExpr;
	m_pDeclVar = pDeclVar;
	m_bRestrict = false;
	m_bReference = m_type->isReference();
	m_bExtern = false;
	m_bHasValue = false;
	m_nValue = 0;
	m_seq_no = -1;
	m_app_flag = false;
}

CVarDef::CVarDef(CScope* pParent, const std::string& name, TypeDefPointer pTypeDef, SourceTreeNode* pDeclVar, CExpr* pInitExpr) : CGrammarObject(pParent)
{
	m_twn.addScope(name);
	m_name = name;
	m_type = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, declVarGetDepth(pDeclVar)));
	m_bHasConstructor = false;
	m_pInitExpr = pInitExpr;
	m_pDeclVar = pDeclVar;
	m_bRestrict = false;
	m_bReference = m_type->isReference();
	m_bExtern = false;
	m_bHasValue = false;
	m_nValue = 0;
	m_seq_no = -1;
	m_app_flag = false;
}

CVarDef::CVarDef(CScope* pParent, const std::string& name, TypeDefPointer pTypeDef, const ExprVector& constructorParamList) : CGrammarObject(pParent)
{
	m_twn.addScope(name);
	m_name = name;
	m_type = pTypeDef;
	m_bHasConstructor = true;
	m_exprList = constructorParamList;
	m_pInitExpr = NULL;
	m_pDeclVar = NULL;
	m_bRestrict = false;
	m_bReference = false;
	m_bExtern = false;
	m_bHasValue = false;
	m_nValue = 0;
	m_seq_no = -1;
	m_app_flag = false;
}

CVarDef::CVarDef(CScope* pParent, const TokenWithNamespace& twn, TypeDefPointer pTypeDef, const ExprVector& constructorParamList) : CGrammarObject(pParent)
{
	m_twn = twn;
	m_name = m_twn.toString();
	m_type = pTypeDef;
	m_bHasConstructor = true;
	m_exprList = constructorParamList;
	m_pInitExpr = NULL;
	m_pDeclVar = NULL;
	m_bRestrict = false;
	m_bReference = false;
	m_bExtern = false;
	m_bHasValue = false;
	m_nValue = 0;
	m_seq_no = -1;
	m_app_flag = false;
}

CVarDef::~CVarDef()
{
	deleteSourceTreeNode(m_pDeclVar);
}

std::string CVarDef::getDebugName()
{
	return getName() + ":" + ltoa(m_bHasValue);
}

bool CVarDef::isFlow()
{
	return m_pInitExpr && m_pInitExpr->isFlow();
}

void CVarDef::setValue(int nValue)
{
	TRACE("CVarDef::%s, path=%s, set value to %d\n", __FUNCTION__, getDebugPath().c_str(), nValue);

	MY_ASSERT(!m_bHasValue);
	m_bHasValue = true;
	m_nValue = nValue;
}

void CVarDef::changeName(const std::string& new_name, CExpr* pNewExpr)
{
	if (getParent()->getGoType() == GO_TYPE_FUNC)
	{
		// a param var, change param name first
		CFunction* pFunc = (CFunction*)getParent();

		// first, check the index number it resides in the function's param list
		int n = -1;
		for (int i = 0; i < pFunc->getFuncType()->getFuncParamCount(); i++)
		{
			CVarDef* pVarDef = pFunc->getFuncType()->getFuncParamAt(i);
			if (pVarDef == this)
			{
				n = i;
				pVarDef->setName(new_name);
				break;
			}
		}
		MY_ASSERT(n >= 0);

		for (int i = 0; i < pFunc->getChildrenCount(); i++)
			((CStatement*)pFunc->getChildAt(i))->changeRefVarName(m_name, pNewExpr);
	}
	else
	{
		MY_ASSERT(m_pParent->getGoType() == GO_TYPE_STATEMENT);
		StatementType type = ((CStatement*)m_pParent)->getStatementType();
		MY_ASSERT(((CStatement*)m_pParent)->getStatementType() == STATEMENT_TYPE_DEF);
		if (m_pParent->getParent()->getGoType() == GO_TYPE_NAMESPACE)
		{
			MY_ASSERT(false);
			return;
		}

		CScope* pScope = m_pParent->getParent();
		int n = -1;
		for (int i = 0; i < pScope->getChildrenCount(); i++)
		{
			if (pScope->getChildAt(i) == m_pParent)
			{
				n = i;
				break;
			}
		}
		MY_ASSERT(n >= 0);

		if (((CStatement*)m_pParent)->getDefType() == DEF_TYPE_VAR_DEF)
		{
			setName(new_name);
			if (m_pDeclVar)
				declVarChangeName(m_pDeclVar, new_name);
		}
		else
		{
			MY_ASSERT(((CStatement*)m_pParent)->getDefType() == DEF_TYPE_FUNC_VAR_DEF);
			((CStatement*)m_pParent)->changeDefVarName(m_name, new_name);
		}
		for (n++; n < pScope->getChildrenCount(); n++)
			((CStatement*)pScope->getChildAt(n))->changeRefVarName(m_name, pNewExpr);
	}

	MY_ASSERT(m_twn.getDepth() == 1);
	m_name = new_name;
	m_twn.resize(0);
	m_twn.addScope(m_name);
}

std::string CVarDef::toString(bool bDumpInitExpr)
{
	std::string ret_s;

	if (m_bHasConstructor)
	{
		ret_s = m_name + "(";
		for (unsigned i = 0; i < m_exprList.size(); i++)
		{
			if (i > 0)
				ret_s += ", ";
			ret_s += m_exprList[i]->toString(0);
		}
		ret_s += ")";
	}
	else
	{
		if (m_pDeclVar)
		{
			std::vector<std::string> str_v;
			for (size_t i = 0; i < m_exprList.size(); i++)
			{
				CExpr* pExpr2 = m_exprList[i];
				str_v.push_back(pExpr2->toString());
			}
			ret_s = displaySourceTreeDeclVar(m_pDeclVar, m_name, str_v);
		}
		else
		{
			ret_s = m_name;
			//TypeDefPointer pTypeDef = getType();
			//BasicTypeType basicType = pTypeDef->getBasicType();
			//ret_s = displaySourceTreeDeclVar(m_type->getDeclVarNode(), m_name);
		}

		if (m_pInitExpr && bDumpInitExpr)
			ret_s += " = " + m_pInitExpr->toString();
	}

	return ret_s;
}

CUsingObject::CUsingObject(const std::string& name, SymbolDefObject* pSymbolObj) : CGrammarObject(pSymbolObj->children[0]->getParent())
{
	m_name = name;
	m_pSymbolObj = pSymbolObj;
}

void checkBestMatchedFunc(const std::vector<CGrammarObject*>& children, const std::vector<TypeDefPointer>& typeList, bool bCallerIsConst, int& maxMatchScore, std::vector<CGrammarObject*>& matched_v)
{
	TRACE("SymbolDefObject::%s at %s:%d, callerIsConst=%d, typeList=(", __FUNCTION__, g_cur_file_name.c_str(), g_cur_line_no, bCallerIsConst);
	for (unsigned i = 0; i < typeList.size(); i++)
		TRACE("%s, ", typeList[i]->toFullString().c_str());
	TRACE(")\n");
	//MY_ASSERT(type == GO_TYPE_FUNC_DECL);

	for (unsigned i = 0; i < children.size(); i++)
	{
		CGrammarObject* pGrammarObj = children[i];
		int n;
		switch (pGrammarObj->getGoType())
		{
		case GO_TYPE_FUNC_DECL:
		{
			CFuncDeclare* pFuncDeclare = (CFuncDeclare*)pGrammarObj;
			TRACE("SymbolDefObject::%s, i=%u, func decl %s defined in %s\n", __FUNCTION__, i, pFuncDeclare->getName().c_str(), pFuncDeclare->definedIn().c_str());
			n = pFuncDeclare->getType()->checkCallParams(typeList, bCallerIsConst);
			TRACE("SymbolDefObject::%s, func decl return %d\n", __FUNCTION__, n);
			break;
		}
		case GO_TYPE_TEMPLATE:
		{
			CTemplate* pTemplate = (CTemplate*)pGrammarObj;
			MY_ASSERT(pTemplate->getTemplateType() == TEMPLATE_TYPE_FUNC);

			TemplateResolvedDefParamVector typeDefMap;
			TRACE("SymbolDefObject::%s, i=%u, template %s defined in %s\n", __FUNCTION__, i, pTemplate->getName().c_str(), pTemplate->definedIn().c_str());
			n = pTemplate->funcCheckFitForTypeList(typeList, typeDefMap);
			TRACE("SymbolDefObject::%s, template returned %d\n", __FUNCTION__, n);
			break;
		}
		case GO_TYPE_USING_OBJECTS:
		{
			CUsingObject* pObj = (CUsingObject*)pGrammarObj;
			TRACE("SymbolDefObject::%s, i=%u, using objects\n", __FUNCTION__, i);
			checkBestMatchedFunc(pObj->getSymbolObj()->children, typeList, bCallerIsConst, maxMatchScore, matched_v);
			TRACE("SymbolDefObject::%s, using objects, return score=%d, matched_count=%d\n", __FUNCTION__, maxMatchScore, matched_v.size());
			continue;
		}
		default:
			MY_ASSERT(false);
		}
		if (n < 0 || n < maxMatchScore)
			continue;
		if (n == maxMatchScore)
			matched_v.push_back(pGrammarObj);
		else
		{
			maxMatchScore = n;
			matched_v.clear();
			matched_v.push_back(pGrammarObj);
		}
	}
}

CFunction* CScope::getFunctionScope()
{
	CScope* pParent = this;

	while (pParent)
	{
		if (pParent->getGoType() == GO_TYPE_FUNC)
			return (CFunction*)pParent;
		pParent = pParent->getParent()->getRealScope();
	}

	return NULL;
}

void CScope::addFuncDeclare(CFuncDeclare* pFuncDecl)
{
	MY_ASSERT(m_bRealScope);
	MY_ASSERT(pFuncDecl->getType());
	TRACE("\n%s, ADDING FUNC DECLARE %s:%d in %s, FLOWTYPE=%d\n", pFuncDecl->definedIn().c_str(), pFuncDecl->getName().c_str(), pFuncDecl, getDebugPath().c_str(), pFuncDecl->getType()->isFuncFlow());
	SymbolDefMap::iterator it = m_symbol_map.find(pFuncDecl->getName());
	if (it != m_symbol_map.end())
	{
		if (getGoType() == GO_TYPE_CLASS && it->second.children.size() == 1 && 
			(pFuncDecl->getName() == ((CClassDef*)this)->getClassName() || pFuncDecl->getName() == "~" + ((CClassDef*)this)->getClassName()))
		{
			CGrammarObject* pObj = it->second.children[0];
			if (pObj->getGoType() == GO_TYPE_FUNC_DECL && ((CFuncDeclare*)pObj)->getDefLineNo() == 0) // getDefLineNo == 0 means it's a default function that doesn't exist in code
			{
				it->second.children.clear();
				delete pObj;
			}
		}
		for (std::vector<CGrammarObject*>::iterator it2 = it->second.children.begin(); it2 != it->second.children.end(); it2++)
		{
			CGrammarObject* pObj = *it2;
			if (pObj->getGoType() != GO_TYPE_FUNC_DECL)
				continue;
			if (isSameFuncType(((CFuncDeclare*)pObj)->getType(), pFuncDecl->getType()))
			{
				TRACE("CScope::%s, duplication found in the queue. do nothing\n", __FUNCTION__);
				return;
			}
		}
		if (it->second.type == GO_TYPE_TYPEDEF)
			it->second.type = GO_TYPE_FUNC_DECL;
		MY_ASSERT(it->second.type == GO_TYPE_FUNC_DECL);
	}
	else
	{
		m_symbol_map[pFuncDecl->getName()].type = GO_TYPE_FUNC_DECL;
		it = m_symbol_map.find(pFuncDecl->getName());
	}
	it->second.children.push_back(pFuncDecl);
}

// a struct or union can share the same name with a func, to reference it, need to specify struct or union.
void CScope::addTypeDef(TypeDefPointer pTypeDef)
{
	MY_ASSERT(m_bRealScope);
	MY_ASSERT(!pTypeDef->getClassDef());
	MY_ASSERT(!pTypeDef->getName().empty());

	TRACE("\n%s, ADDING TYPEDEF %s:%d, TYPE=%s\n", definedIn().c_str(), pTypeDef->getDebugPath().c_str(), pTypeDef->getType(), pTypeDef->toFullString().c_str());
	SymbolDefMap::iterator it = m_symbol_map.find(pTypeDef->getName());
	if (it == m_symbol_map.end())
		m_symbol_map[pTypeDef->getName()].type = GO_TYPE_TYPEDEF;
	else
		MY_ASSERT(it->second.type == GO_TYPE_FUNC_DECL);
	m_symbol_map[pTypeDef->getName()].children.push_back(pTypeDef.get());
	m_symbol_map[pTypeDef->getName()].pTypeDef = pTypeDef;
}

void CScope::addVarDef(CVarDef* pVarDef)
{
	MY_ASSERT(m_bRealScope);
	//MY_ASSERT(!pVarDef->getName().empty());
	TRACE("%s:%d, ADDING VAR %s(%s), type=%s", g_cur_file_name.c_str(), g_cur_line_no, pVarDef->getName().c_str(), pVarDef->getDebugPath().c_str(), pVarDef->getType()->toString().c_str());
	if (pVarDef->hasValue())
		TRACE(", value=%d", pVarDef->getValue());
	TRACE("\n");
	SymbolDefMap::iterator it = m_symbol_map.find(pVarDef->getName());
	if (it != m_symbol_map.end())
	{
		MY_ASSERT(it->second.children.size() == 1);
		MY_ASSERT(it->second.children[0]->getGoType() == GO_TYPE_TYPEDEF);
	}
	else
		m_symbol_map[pVarDef->getName()].type = GO_TYPE_VAR_DEF;
	m_symbol_map[pVarDef->getName()].children.push_back(pVarDef);
}

void CScope::removeVarDef(CVarDef* pVarDef)
{
	MY_ASSERT(m_bRealScope);
	TRACE("REMOVING VAR %s\n", pVarDef->getDebugPath().c_str());
	SymbolDefMap::iterator it = m_symbol_map.find(pVarDef->getName());
	if (it == m_symbol_map.end())
		return;
	MY_ASSERT(it->second.children.size() == 1);
	MY_ASSERT(it->second.children[0]->getGoType() == GO_TYPE_VAR_DEF);
	//MY_ASSERT(it->second.getVarDef() == pVarDef);
	m_symbol_map.erase(pVarDef->getName());
}

void CScope::addEnumDef(const std::string& name, CClassDef* pClassDef, int nValue)
{
	MY_ASSERT(m_bRealScope);
	TRACE("%s, ADDING ENUM %s=%d IN %s\n", pClassDef->definedIn().c_str(), name.c_str(), nValue, getDebugPath().c_str());
	SymbolDefMap::iterator it = m_symbol_map.find(name);
	MY_ASSERT(it == m_symbol_map.end());
	m_symbol_map[name].type = GO_TYPE_ENUM;
	m_symbol_map[name].children.push_back(pClassDef);
	m_symbol_map[name].nValue = nValue;
}

void CScope::addTemplate(CTemplate* pTemplate)
{
	MY_ASSERT(m_bRealScope);
	MY_ASSERT(!pTemplate->getName().empty());
	MY_ASSERT(pTemplate->getTemplateType() != TEMPLATE_TYPE_VAR);
	TRACE("%s:%d, ADDING %s TEMPLATE %s IN %s\n", g_cur_file_name.c_str(), g_cur_line_no, templateTypeToString(pTemplate->getTemplateType()).c_str(), pTemplate->getName().c_str(), getDebugPath().c_str());
	SymbolDefMap::iterator it = m_symbol_map.find(pTemplate->getName());
	if (pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS)
	{
		if (pTemplate->isInstancedTemplate())
		{
			MY_ASSERT(getGoType() == GO_TYPE_TEMPLATE);
			CTemplate* pParentTemplate = (CTemplate*)this;
			MY_ASSERT(pParentTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS);
			MY_ASSERT(!pParentTemplate->isInstancedTemplate());
			MY_ASSERT(pParentTemplate->getTemplateName() == pTemplate->getTemplateName());
		}
		else if (pTemplate->isSpecializedTemplate())
		{
			MY_ASSERT(getGoType() == GO_TYPE_TEMPLATE);
			CTemplate* pParentTemplate = (CTemplate*)this;
			MY_ASSERT(pParentTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS);
			MY_ASSERT(pParentTemplate->isRootTemplate());
			MY_ASSERT(pParentTemplate->getTemplateName() == pTemplate->getTemplateName());
		}
		if (it != m_symbol_map.end())
		{
			MY_ASSERT(false);
			/*MY_ASSERT(it->second.type == GO_TYPE_TEMPLATE);
			if (pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS)
			{
				MY_ASSERT(false);
			}
			else
			{
				MY_ASSERT(((CTemplate*)it->second.children[0])->getTemplateType() != TEMPLATE_TYPE_CLASS);
			}*/
		}
		else
		{
			m_symbol_map[pTemplate->getName()].type = GO_TYPE_TEMPLATE;
			it = m_symbol_map.find(pTemplate->getName());
		}
	}
	else
	{
		if (it != m_symbol_map.end())
		{
			GrammarObjectType go_type = it->second.type;
			//if (it->second.type == GO_TYPE_USING_OBJECTS)
			//	it->second.type = GO_TYPE_FUNC_DECL;
			//else
				MY_ASSERT(it->second.type == GO_TYPE_FUNC_DECL);
		}
		else
		{
			m_symbol_map[pTemplate->getName()].type = GO_TYPE_FUNC_DECL;
			it = m_symbol_map.find(pTemplate->getName());
		}
	}
	it->second.children.push_back(pTemplate);
}

void CScope::addNamespace(CNamespace* pNamespace)
{
	MY_ASSERT(m_bRealScope);
	TRACE("%s, ADDING NAMESPACE %s IN %s\n", pNamespace->definedIn().c_str(), pNamespace->getName().c_str(), getDebugPath().c_str());
	SymbolDefMap::iterator it = m_symbol_map.find(pNamespace->getName());
	MY_ASSERT(it == m_symbol_map.end());
	m_symbol_map[pNamespace->getName()].type = GO_TYPE_NAMESPACE;
	m_symbol_map[pNamespace->getName()].children.push_back(pNamespace);
}

void CScope::addUsingObjects(const std::string& name, SymbolDefObject* pSymbolObj)
{
	MY_ASSERT(m_bRealScope);
	TRACE("%s:%d, ADDING USING OBJECTS %s(0x%lx):%d IN %s\n", g_cur_file_name.c_str(), g_cur_line_no, name.c_str(), (unsigned long)pSymbolObj, pSymbolObj->type, getDebugPath().c_str());
	if (pSymbolObj->type == GO_TYPE_TEMPLATE)
		TRACE("ADDING USING OBJECTS TEMPLATE, 0x%lx:%d\n", (unsigned long)pSymbolObj->getTemplateAt(0), pSymbolObj->getTemplateAt(0)->getTemplateType());
	//MY_ASSERT(m_symbol_map.find(name) == m_symbol_map.end());
	GrammarObjectType type = pSymbolObj->type;
	CUsingObject* pUsingObject = new CUsingObject(name, pSymbolObj);
	SymbolDefMap::iterator it = m_symbol_map.find(name);
	if (it != m_symbol_map.end())
	{
		if (it->second.children.size() == 1)
		{
			CGrammarObject* pObj = it->second.children[0];
			GrammarObjectType type2 = pObj->getGoType();
			if (pObj->getGoType() == GO_TYPE_USING_OBJECTS && ((CUsingObject*)pObj)->getSymbolObj() == pSymbolObj)
				return;
		}
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		MY_ASSERT(it->second.type == pSymbolObj->type);
	}
	else
	{
		m_symbol_map[name].type = pSymbolObj->type;
		it = m_symbol_map.find(name);
	}
	it->second.children.push_back(pUsingObject);
}

SymbolDefObject* CScope::findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode)
{
	TRACE("CScope::findSymbol %s in %s, bRealScope=%d, scope=%d, mode=%d, ", name.c_str(), getDebugPath().c_str(), m_bRealScope, scope, mode);

	if (!m_bRealScope)
	{
		TRACE("not real scope\n");
		return getRealScope()->findSymbol(name, scope, mode);
	}

	/*if (bCheckParents && name == getName())
	{
		TRACE("found myself as symbol %s in %s, type=%d\n", name.c_str(), getDebugPath().c_str(), getGoType());
		if (getGoType() == GO_TYPE_TEMPLATE && ((CTemplate*)this)->getTemplateType() == TEMPLATE_TYPE_CLASS && getParent()->getGoType() == GO_TYPE_TEMPLATE && getName() == getParent()->getName()) // then it's a specialized template
		  return ((CTemplate*)getParent())->findSpecializedTemplate((CTemplate*)this);

		return getParent()->getRealScope()->findSymbol(name, false);
	}*/

	SymbolDefMap::iterator it = m_symbol_map.find(name);
	if (it != m_symbol_map.end())
	{
		MY_ASSERT(it->second.children.size() > 0);
		if (mode == FIND_SYMBOL_MODE_ANY || it->second.findChildByType(GO_TYPE_TEMPLATE))
		{
			TRACE("found symbol %s(0x%lx) in %s, type=%d, type2=%d\n", name.c_str(), (long unsigned)it->second.children[0], getDebugPath().c_str(), it->second.type, it->second.children[0]->getGoType());
			return &it->second;
		}
		TRACE("found symbol %s(0x%lx) in %s, type=%d, type2=%d, not a template, continue.             ", name.c_str(), (long unsigned)it->second.children[0], getDebugPath().c_str(), it->second.type, it->second.children[0]->getGoType());
	}

	if (scope == FIND_SYMBOL_SCOPE_PARENT && m_pParent)
	{
		//TRACE("check parent\n");
		return m_pParent->findSymbol(name, scope, mode);
	}

	//TRACE("not found\n");
	return NULL;
}

CFuncDeclare* CScope::findFuncDeclare(SymbolDefObject* pDefObj, int paramCount)
{
	//printf("finding func declare %s in %s\n", funcName.c_str(), getDebugPath().c_str());

	MY_ASSERT(pDefObj->type == GO_TYPE_FUNC_DECL);
	for (size_t i = 0; i < pDefObj->children.size(); i++)
	{
		CGrammarObject* pGrammarObj = pDefObj->children[i];
		CFuncDeclare* pFunc = (CFuncDeclare*)pGrammarObj;
		if (paramCount == -1 || pFunc->getType()->getFuncParamCount() == paramCount || (pFunc->getType()->hasVArgs() && pFunc->getType()->getFuncParamCount() <= paramCount))
			return pFunc;
		//printf("func found, but param count mismatch, declared=%d, used=%d, hasVArgs=%d\n", pFunc->getParamCount(), paramCount, pFunc->hasVArgs());
	}

	return NULL;
}

CFuncDeclare* CScope::findFuncDeclare(SymbolDefObject* pDefObj, TypeDefPointer pTypeDef)
{
	MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_FUNC);

	for (size_t i = 0; i < pDefObj->children.size(); i++)
	{
		CGrammarObject* pGrammarObj = pDefObj->children[i];
		if (pGrammarObj->getGoType() != GO_TYPE_FUNC_DECL) // the other one is func template
			continue;
		CFuncDeclare* pFunc = (CFuncDeclare*)pGrammarObj;
		if (!pTypeDef ||
			pFunc->getType()->getFuncParamCount() == pTypeDef->getFuncParamCount() ||
			(pFunc->getType()->hasVArgs() && pTypeDef->hasVArgs() && pFunc->getType()->getFuncParamCount() == pTypeDef->getFuncParamCount()))
			return pFunc;
		//printf("func found, but param count mismatch, declared=%d, used=%d, hasVArgs=%d\n", pFunc->getParamCount(), paramCount, pFunc->hasVArgs());
	}

	return NULL;
}

CFuncDeclare* CScope::findFuncDeclare(const std::string& name, const ExprVector& param_list, bool bCallerIsConst)
{
	SymbolDefMap::iterator it = m_symbol_map.find(name);
	if (it == m_symbol_map.end())
		return NULL;

	if (it->second.type != GO_TYPE_FUNC_DECL)
	{
		MY_ASSERT(false);
		return NULL;
	}

	std::vector<TypeDefPointer> typeList;
	for (size_t i = 0; i < param_list.size(); i++)
	{
		CExpr* pExpr = param_list[i];
		typeList.push_back(createTypeByDepth(pExpr->getReturnType(), pExpr->getReturnDepth()));
	}

	int maxMatchScore = -1;
	std::vector<CGrammarObject*> matched_v;
	checkBestMatchedFunc(it->second.children, typeList, bCallerIsConst, maxMatchScore, matched_v);
	if (matched_v.empty())
	{
		throw("Cannot find matched func definition for " + name);
		return NULL;
	}
	if (matched_v.size() > 1)
	{
		std::string err_s = "call of " + name + " at " + g_cur_file_name + ":" + ltoa(g_cur_line_no) + " is ambiguous. Choices are:\n";
		for (unsigned j = 0; j < matched_v.size(); j++)
			err_s += "   " + matched_v[j]->definedIn() + "\n";
		throw(err_s);
		return NULL;
	}

	CGrammarObject* pMatchedGO = matched_v[0];
	MY_ASSERT(pMatchedGO->getGoType() == GO_TYPE_FUNC_DECL);
	return (CFuncDeclare*)pMatchedGO;
}

/*void CScope::addUsingNamespace(CNamespace* pNamespace, const std::string& token) // if token is not empty, it means only the token under this namespace is allowed.
{
	BOOST_FOREACH(NamespaceTokenPair& pair, m_using_namespace_list)
	{
		if (pair.pNamespace == pNamespace && pair.token == token)
			return;
	}
	NamespaceTokenPair pair;
	pair.pNamespace = pNamespace;
	pair.token = token;
	m_using_namespace_list.push_back(pair);
}*/

/* this is kind of complicated.
   If namespace is specified (without root ::),
   	   search it in the order of parent, siblings, parent's parent, parent's parent's parent ...,
	   if found, then check remaining namespaces and token, if not found, report error.
   otherwise,
   	   search it in the order of parent, parent's parent, ...,
	   if found, use it
*/
SymbolDefObject* CScope::findSymbolEx(const TokenWithNamespace& twn, bool bCheckingParents/* = true*/, bool bCreateIfNotExists/* = false*/, bool bCreateAsType/* = false*/)
{
    std::string twn_s = twn.toString();
	TRACE("::findSymbolEx, twn=%s, path=%s, bCP=%d, bCINE=%d, bCAT=%d\n", twn.toString().c_str(), getDebugPath().c_str(), bCheckingParents, bCreateIfNotExists, bCreateAsType);
	if (twn.getDepth() == 0)
	{
		MY_ASSERT(twn.hasRootSign()); // can't be an empty twn
		return &g_global_symbol_obj;
	}

	CScope* pScope = getRealScope();
	if (twn.hasRootSign())
		pScope = &g_global_namespace;

	SymbolDefObject* pDefObj = NULL;
	for (int i = 0; i < twn.getDepth(); i++)
	{
		FindSymbolScope scope;
		if (i == 0)
		{
			if (twn.getDepth() > 1)
				scope = FIND_SYMBOL_SCOPE_PARENT;
			else
				scope = bCheckingParents ? FIND_SYMBOL_SCOPE_PARENT : FIND_SYMBOL_SCOPE_LOCAL;
		}
		else
			scope = FIND_SYMBOL_SCOPE_SCOPE;

		// if twn is a template (has type params) and pScope is under such a template, then go straight to its root template
		if (twn.scopeHasTemplate(i) && scope == FIND_SYMBOL_SCOPE_PARENT)
		{
			CScope* pScope2 = pScope;
			while (pScope2)
			{
				if (pScope2->getGoType() == GO_TYPE_TEMPLATE && ((CTemplate*)pScope2)->getTemplateType() == TEMPLATE_TYPE_CLASS &&
					((CTemplate*)pScope2)->isRootTemplate() && ((CTemplate*)pScope2)->getName() == twn.getToken(i))
				{
					pScope = pScope2;
					break;
				}
				pScope2 = pScope2->getParent();
			}
		}

		pDefObj = pScope->findSymbol(twn.getToken(i), scope, twn.scopeHasTemplate(i) ? FIND_SYMBOL_MODE_TEMPLATE : FIND_SYMBOL_MODE_ANY);
		TRACE("\nfindSymbolEx, findSymbol, i=%d, token=%s, return %s\n", i, twn.getToken(i).c_str(), (pDefObj ? getGoTypeName(pDefObj->type).c_str() : "null"));
		if (!pDefObj)
		{
			if (pScope->getGoType() == GO_TYPE_CLASS && pScope->getParent()->getGoType() == GO_TYPE_TEMPLATE && pScope->getParent()->getParent()->getGoType() == GO_TYPE_TEMPLATE)
			{
				CTemplate* pTemplate = (CTemplate*)pScope->getParent()->getParent();
				pDefObj = pTemplate->findSymbol(twn.getToken(i), FIND_SYMBOL_SCOPE_SCOPE);
				if (pDefObj)
				{
					TRACE("pDefObj->type=%d\n", pDefObj->type);
				}
				else
				{
					TRACE("pDefObj not found\n");
				}
			}
			else
				TRACE("scope type=%d\n", pScope->getGoType());

			if (bCreateIfNotExists)
			{
				CTemplate* pParentTemplate = getParentTemplate();
				//MY_ASSERT(pParentTemplate == pScope || pParentTemplate->isParentOf(pScope));
				if (twn.scopeHasTemplate(i) || twn.scopeSpecifiedAsTemplate(i))
				{
					CTemplate* pTemplate = new CTemplate(pScope, TEMPLATE_TYPE_CLASS, twn.getToken(i));
					TRACE("\nCREATING root TEMPLATE %s in decl\n", pTemplate->getDebugPath().c_str());
					pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
					pScope->addTemplate(pTemplate);
					pDefObj = pScope->findSymbol(twn.getToken(i), FIND_SYMBOL_SCOPE_SCOPE);
				}
				else if (bCreateAsType)
				{
					TypeDefPointer pTypeDef = pScope->createClassAsChild(twn.getToken(i), SEMANTIC_TYPE_CLASS);
					if (pScope->getParentTemplate())
						pTypeDef->getBaseType()->getClassDef()->setWithinTemplate();
					pDefObj = pScope->findSymbol(twn.getToken(i), FIND_SYMBOL_SCOPE_SCOPE);
				}
				else
				{
					pScope->addVarDef(new CVarDef(pScope, twn.getToken(i), g_type_def_int));
					pDefObj = pScope->findSymbol(twn.getToken(i), FIND_SYMBOL_SCOPE_SCOPE);
				}
			}
			else
				return NULL;
		}
		TRACE("\nfindSymbolEx, i=%d, depth=%d, type=%d\n", i, twn.getDepth(), pDefObj->type);
		if (i == twn.getDepth() - 1)
		{
			if (twn.scopeHasTemplate(i))
			{
				if (pDefObj->type == GO_TYPE_FUNC_DECL)
				{
					pScope = pDefObj->getFuncDeclareAt(0)->getParent();
					if ((pScope->getGoType() == GO_TYPE_CLASS && ((CClassDef*)pScope)->getTemplateName() == twn.getToken(i)) ||
						(pScope->getGoType() == GO_TYPE_TEMPLATE && ((CTemplate*)pScope)->getTemplateName() == twn.getToken(i)))
					{
						while (pScope->getParent()->getGoType() == GO_TYPE_TEMPLATE && ((CTemplate*)pScope->getParent())->getTemplateName() == twn.getToken(i))
							pScope = pScope->getParent();
						//TRACE("CScope::findSymbolEx return %s\n", pScope->getDebugPath().c_str());
						return pScope->getParent()->findSymbol(twn.getToken(i), FIND_SYMBOL_SCOPE_SCOPE);
					}
				}
				else if (pDefObj->type == GO_TYPE_TYPEDEF)
				{
					TypeDefPointer pTypeDef = pDefObj->getTypeDef();
					if (!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef() &&
						pTypeDef->getBaseType()->getClassDef()->getParent()->getGoType() == GO_TYPE_TEMPLATE)
					{
						CTemplate* pTemplate = (CTemplate*)pTypeDef->getBaseType()->getClassDef()->getParent();
						while (pTemplate->getParent()->getGoType() == GO_TYPE_TEMPLATE && pTemplate->getParent()->getName() == twn.getToken(i))
							pTemplate = (CTemplate*)pTemplate->getParent();
						//TRACE("CScope::findSymbolEx return %s\n", pTemplate->getDebugPath().c_str());
						return pTemplate->getParent()->findSymbol(pTemplate->getName(), FIND_SYMBOL_SCOPE_SCOPE);
					}
				}
				if (pDefObj->type == GO_TYPE_TEMPLATE)
                {
                    CTemplate* pTemplate = pDefObj->getTemplateAt(0);
                    MY_ASSERT(pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS);

                    bool bHasTypename = checkTemplateParamHasTypename(twn, i);

                    TRACE("check bHasTypename, twn=%s, bHasTypename=%d\n", twn.toString().c_str(), bHasTypename);
                    if (bHasTypename)
                    {
                        pTemplate = pTemplate->getTemplateByParams(twn, i);
                        if (pTemplate->isSpecializedTemplate())
                            pDefObj = ((CTemplate*)pTemplate->getParent())->findSpecializedTemplate(pTemplate);
                    }
                    else
                    {
                        std::string s2 = getDebugPath();
                        TypeDefPointer pTypeDef = pTemplate->classGetInstance(twn, i, this);
                        if (!pTypeDef)
                            return NULL;
                        CClassDef* pClassDef = pTypeDef->getBaseType()->getClassDef();
                        // we don't analyze it because it may cause recursive template instancing
                        //if (!pClassDef->isDefined())
                        //    pClassDef->analyzeByTemplate(); // it's possible the call returns false when in a template definition, a typedef points to the template itself
                        pDefObj = pClassDef->getParent()->findSymbol(pClassDef->getName(), FIND_SYMBOL_SCOPE_LOCAL);
                    }
                }
			}
			TRACE("\nfindSymbolEx, return %s\n", (pDefObj ? getGoTypeName(pDefObj->type).c_str() : "null"));
			return pDefObj;
		}

		TRACE("findSymbolEx, type=%d\n", pDefObj->type);
		if (pDefObj->type == GO_TYPE_NAMESPACE)
		{
			MY_ASSERT(!twn.scopeHasTemplate(i));
			pScope = pDefObj->getNamespace();
		}
		else if (pDefObj->type == GO_TYPE_TYPEDEF)
		{
			MY_ASSERT(!twn.scopeHasTemplate(i));
			TypeDefPointer pTypeDef = pDefObj->getTypeDef();
			while (!pTypeDef->isBaseType() && pTypeDef->getDepth() == 0)
				pTypeDef = pTypeDef->getBaseType();
			CClassDef* pClassDef = pTypeDef->getClassDef();
			if (!pClassDef->isWithinTemplate() && pClassDef->getType() != SEMANTIC_TYPE_TYPENAME && !pClassDef->isDefined())
				pClassDef->analyzeByTemplate();
			pScope = pClassDef;
			if (!pScope)
			{
				TRACE("findSymbolEx, %s base type is not a class def, i=%d\n", twn.toString().c_str(), i);
				return NULL;
			}
			TRACE("pScope=%s\n", pScope->getName().c_str());
		}
		else if (pDefObj->type == GO_TYPE_TEMPLATE)
		{
			CTemplate* pTemplate = pDefObj->getTemplateAt(0);
			MY_ASSERT(pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS);
			if (twn.scopeHasTemplate(i))
			{
				bool bHasTypename = checkTemplateParamHasTypename(twn, i);

				TRACE("check bHasTypename, twn=%s, bHasTypename=%d\n", twn.toString().c_str(), bHasTypename);
				if (bHasTypename)
				{
					pScope = pTemplate->getTemplateByParams(twn, i);
					if (!pScope)
						return NULL;
				}
				else
				{
					std::string s2 = getDebugPath();
					TypeDefPointer pTypeDef = pTemplate->classGetInstance(twn, i, this);
					if (!pTypeDef)
						return NULL;
					CClassDef* pClassDef = pTypeDef->getBaseType()->getClassDef();
					//if (!pClassDef->isDefined())
					//	pClassDef->analyzeByTemplate();
					pScope = pClassDef;
				}
			}
			else
			{
			  // it's possible that a template doesn't have type parameters. Samples are:
			  /*template<typename _CharT, typename _Traits>
				basic_ostream<_CharT, _Traits>::sentry::
				sentry(basic_ostream<_CharT, _Traits>& __os)
				: _M_ok(false), _M_os(__os)
				{ */
				pScope = pTemplate;
			}
		}
		else
		{
			TRACE("findSymbolEx, invalid type=%d\n", pDefObj->type);
			return NULL;
		}
	}

	TRACE("findSymbolEx, not found\n");
	return NULL;
}

bool CScope::checkTemplateParamHasTypename(const TokenWithNamespace& twn, int idx)
{
	TRACE("CScope::%s, twn=%s, depth=%d\n", __FUNCTION__, twn.toString().c_str(), idx);
	bool bHasTypename = false;
	for (int j = 0; j < twn.getTemplateParamCount(idx); j++)
	{
		SourceTreeNode* pNode;
		std::string s;
		switch (twn.getTemplateParamAt(idx, j, pNode))
		{
		case TEMPLATE_PARAM_TYPE_DATA:
		{
			TypeDefPointer pTypeDef = getTypeDefByExtendedTypeVarNode(pNode);
			TRACE("CScope::%s, type %s is defined as %d\n", __FUNCTION__, displaySourceTreeExtendedTypeVar(pNode).c_str(), (pTypeDef ? pTypeDef->getType() : -1));
			if (!pTypeDef || pTypeDef->getType() == SEMANTIC_TYPE_TYPENAME || pTypeDef->getType() == SEMANTIC_TYPE_TEMPLATE)
				return true;
			break;
		}
		case TEMPLATE_PARAM_TYPE_FUNC:
			MY_ASSERT(false);
		case TEMPLATE_PARAM_TYPE_VALUE:
		{
			if (exprGetType(pNode) == EXPR_TYPE_TOKEN_WITH_NAMESPACE)
			{
				TokenWithNamespace twn = tokenWithNamespaceGetInfo(exprGetSecondNode(pNode));
				if (twn.getDepth() == 1 && isConstValue(twn.getLastToken()))
					continue;
				SymbolDefObject* pObj = findSymbolEx(twn);
				if (!pObj)
				{
	                TRACE("CScope::%s, twn %s not found\n", __FUNCTION__, twn.toString().c_str());
				    return false;
				}
				TRACE("CScope::%s, expr %s is defined as %d\n", __FUNCTION__, displaySourceTreeExpr(pNode).c_str(), pObj->type);
				if (pObj->type == GO_TYPE_VAR_DEF)
					return !pObj->getVarDef()->hasValue();
			}
			break;
		}
		default:
			MY_ASSERT(false);
		}
	}

	return false;
}

// it doesn't need to check recursively, because all sub nodes have been called to this func just now
bool CScope::onGrammarCheckFunc(int mode, const SourceTreeNode* pRoot, const GrammarTempDefMap& tempDefMap)
{
	if (mode == 0) // token
	{
		TokenWithNamespace twn = tokenWithNamespaceGetInfo(pRoot);
		TRACE("CScope::check token, twn=%s, ", twn.toString().c_str());
		if (!twn.hasRootSign() && twn.getDepth() == 1)
		{
			std::string s = twn.getLastToken();
			if (s == "true" || s == "false" || s == "null" || s == "__null")
				return true;
			if (tempDefMap.find(s) != tempDefMap.end())
				return tempDefMap.at(twn.getLastToken()) == 2;
		}

        // in most cases, it brings many issues when we get down to the class type, just get down to the template is enough
        if (twn.scopeHasTemplate(twn.getDepth() - 1))
            twn.clearTemplateParams(twn.getDepth() - 1);

        // it's possible that the second to last scope is a template so we have no way to figure out what the last token is.
        // so we find symbolEx until the second to last scope. If it's a template, we simply return true.
		if (twn.getDepth() == 1)
		{
			SymbolDefObject* pSymbolObj = findSymbolEx(twn);
			if (!pSymbolObj || pSymbolObj->type == GO_TYPE_NAMESPACE || pSymbolObj->type == GO_TYPE_TYPEDEF || pSymbolObj->type == GO_TYPE_TEMPLATE)
				return false;
			return true;
		}

		//MY_ASSERT(!twn.scopeHasTemplate(twn.getDepth() - 1));
		//std::string lastToken = twn.getLastToken();
		//twn.resize(twn.getDepth() - 1);

		SymbolDefObject* pSymbolObj = findSymbolEx(twn);
		TRACE("CScope::check token, get symbol type %d\n", (pSymbolObj ? pSymbolObj->type : -1));
		if (!pSymbolObj)
			return true;

		if (pSymbolObj)
		{
		    // constructor is also a token
			if (pSymbolObj->type == GO_TYPE_TEMPLATE || pSymbolObj->type == GO_TYPE_TYPEDEF || pSymbolObj->type == GO_TYPE_CLASS)
				return false;
			return true;
		}

		//if (pSymbolObj->type == GO_TYPE_TEMPLATE)
		//	return twn.scopeHasTemplate(twn.getDepth() - 1);

		return true; // tired of detail verification
	}
	else // user def type
	{
		TokenWithNamespace twn = userDefTypeGetInfo(pRoot);
		TRACE("CScope::check type, twn=%s, ", twn.toString().c_str());
		if (!twn.hasRootSign() && twn.getDepth() == 1)
		{
			std::string s = twn.getLastToken();
			if (s == "true" || s == "false" || s == "null" || s == "__null")
				return false;
			if (tempDefMap.find(twn.getLastToken()) != tempDefMap.end())
				return tempDefMap.at(twn.getLastToken()) == 1;
		}
		// in most cases, it brings many issues when we get down to the class type, just get down to the template is enough
		if (twn.scopeHasTemplate(twn.getDepth() - 1))
		    twn.clearTemplateParams(twn.getDepth() - 1);
		if (twn.getDepth() == 1)
		{
			if (twn.getLastToken() == "__is_pod")
				return false;

			SymbolDefObject* pSymbolObj = findSymbolEx(twn);
			if (!pSymbolObj)
			{
				TRACE("findSymbolEx %s not found\n", twn.toString().c_str());
				return twn.scopeHasTemplate(0) ? true : false;
			}
			if (pSymbolObj->type == GO_TYPE_TYPEDEF)
				return true; //!twn.scopeHasTemplate(twn.getDepth() - 1);
			if (pSymbolObj->type == GO_TYPE_TEMPLATE)
			{
				//CTemplate* pTemplate = pSymbolObj->getTemplateAt(0);
				//if (pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS)
				//	return twn.scopeHasTemplate(twn.getDepth() - 1);
				return true;
			}
			TRACE("findSymbolEx %s bad type %d\n", twn.toString().c_str(), pSymbolObj->type);
			return false;
		}

		//if (twn.scopeHasTemplate(twn.getDepth() - 1))
		//	return true;

		SymbolDefObject* pSymbolObj = findSymbolEx(twn);
		if (pSymbolObj)
		{
			if (pSymbolObj->type == GO_TYPE_TEMPLATE || pSymbolObj->type == GO_TYPE_TYPEDEF || pSymbolObj->type == GO_TYPE_CLASS)
				return true;
			return false;
		}

		/*std::string lastToken = twn.getLastToken();
		twn.resize(twn.getDepth() - 1);

		SymbolDefObject* pSymbolObj = findSymbolEx(twn);
		if (!pSymbolObj)
			return false;

		if (pSymbolObj->type == GO_TYPE_TEMPLATE)
			return true; //twn.scopeHasTemplate(twn.getDepth() - 1);

		return true; // tired of detail verification*/
		TRACE("::check type not found, return true\n");
		return true;
	}

	MY_ASSERT(false);
	return false;
}

bool CScope::onGrammarCallback(int mode, std::string& s)
{
	if (mode == LEXER_CALLBACK_MODE_GET_FUNCTION)
	{
		CScope* pScope = getParentScope(GO_TYPE_FUNC);
		if (!pScope)
		{
			MY_ASSERT(false);
			return false;
		}

		s = ((CFunction*)pScope)->getName();
		return true;
	}

	MY_ASSERT(false);
	return false;
}

TypeDefPointer CScope::createClassAsChild(const std::string& name, SemanticDataType data_type)
{
	CClassDef* pClassDef = new CClassDef(this, data_type, name);
	pClassDef->setDefLocation(g_cur_file_name, g_cur_line_no);

	TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(this, name, pClassDef));
	pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);

	pTypeDef = TypeDefPointer(new CTypeDef(this, name, pTypeDef, 0));
	pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
	pClassDef->setTypeDef(pTypeDef);

	if (!name.empty())
		addTypeDef(pTypeDef);

	return pTypeDef;
}

TypeDefPointer CScope::getTypeDefByUserDefTypeNode(const SourceTreeNode* pRoot, bool bDefining, bool bHasTypename)
{
	//TRACE("\ngetTypeDefByUserDefTypeNode:%s\n", displaySourceTreeUserDefType(pRoot).c_str());

	TokenWithNamespace twn = userDefTypeGetInfo(pRoot);

	SymbolDefObject* pSymbolObj = NULL;
	pSymbolObj = findSymbolEx(twn, true, bDefining, bHasTypename);

	if (!pSymbolObj)
	{
		TRACE("\ngetTypeDefByUserDefTypeNode %s return null\n", displaySourceTreeUserDefType(pRoot).c_str());
		return TypeDefPointer();
	}

	TRACE("\ngetTypeDefByUserDefTypeNode return type=%d\n", pSymbolObj->type);
	if (pSymbolObj->type == GO_TYPE_TYPEDEF)
		return pSymbolObj->getTypeDef();

	if (pSymbolObj->type == GO_TYPE_TEMPLATE)
	{
		int depth = twn.getDepth() - 1;
		if (!twn.scopeHasTemplate(depth))
		{
			// now we assume it want to refer to the template instance class in a template implementation
			//std::string s = twn.toString();
			//throw("<> needs to be specified for template " + twn.toString());
			return pSymbolObj->getTemplateAt(0)->getRootInstancedClassDef()->getTypeDef();
		}
		MY_ASSERT(pSymbolObj->children.size() > 0);
		CTemplate* pTemplate = pSymbolObj->getTemplateAt(0);
		if (pTemplate->getTemplateType() != TEMPLATE_TYPE_CLASS)
			throw("A func template is used in where a class template " + twn.toString() + " is expected");
		MY_ASSERT(pSymbolObj->children.size() == 1); // as far as I know, only one template declaration of the same name is allowed for class template

		CTemplate* pParentTemplate = getParentTemplate();
		if (pParentTemplate)
		{
			//MY_ASSERT(pTemplate->getTypeParams().size() >= twn.getTemplateParamCount(depth)); doesn't apply for typename template
			return TypeDefPointer(new CTypeDef(pTemplate));
		}

		bool bHasTypename = checkTemplateParamHasTypename(twn, depth);
		TRACE("CSsope::%s, check bHasTypename, twn=%s, depth=%d, bHasTypename=%d\n", __FUNCTION__, twn.toString().c_str(), depth, bHasTypename);
		if (bHasTypename)
			return TypeDefPointer(new CTypeDef(pTemplate->getTemplateByParams(twn, depth)));

		TypeDefPointer pTypeDef = pTemplate->classGetInstance(twn, depth, this);
		return pTypeDef;
	}

	MY_ASSERT(false);
	throw("cannot find definition of type " + displaySourceTreeUserDefType(pRoot) + ", I'm at " + getDebugPath());
}

TypeDefPointer CScope::getTypeDefByTypeNode(const SourceTreeNode* pRoot, bool bDefining, bool bAllowUndefinedStruct)
{
	TypeDefPointer pTypeDef;

	switch (typeGetType(pRoot))
	{
	case BASICTYPE_TYPE_BASIC:
		pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", basicTypeGetInfo(typeBasicGetInfo(pRoot))));
		break;

	case BASICTYPE_TYPE_USER_DEF:
	{
		SourceTreeNode* pNode;
		bool bHasTypename;
		CSUType csu_type = typeUserDefinedGetInfo(pRoot, bHasTypename, pNode);
		pTypeDef = getTypeDefByUserDefTypeNode(pNode, bDefining, bHasTypename | bAllowUndefinedStruct);
		if (csu_type == CSU_TYPE_NONE)
		{
			if (pTypeDef)
				pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", pTypeDef, NULL));
			break;
		}

		//std::string prefix = displayCSUType(csu_type);

		if (!pTypeDef)
		{
			SemanticDataType sType = getSemanticTypeFromCSUType(csu_type);
			TokenWithNamespace twn = userDefTypeGetInfo(pNode);
			//if (!bAllowUndefinedStruct)
			//	throw(getSemanticTypeName(sType) + " " + twn.toString() + " is not defined.");
			if (twn.scopeHasTemplate(twn.getDepth() - 1) || twn.getDepth() != 1 || (sType != SEMANTIC_TYPE_STRUCT && sType != SEMANTIC_TYPE_CLASS))
				throw("in " + g_cur_file_name + ":" + ltoa(g_cur_line_no) + ", " + getSemanticTypeName(sType) + " " + twn.toString() + ", is template or namespace allowed?");

			pTypeDef = getRealScope()->createClassAsChild(twn.getLastToken(), sType);
			//pTypeDef->setPrefix(prefix);
			break;
		}

		MY_ASSERT(!pTypeDef->getClassDef());
		switch (csu_type)
		{
		case CSU_TYPE_CLASS:
		case CSU_TYPE_STRUCT:
			MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_STRUCT || pTypeDef->getType() == SEMANTIC_TYPE_CLASS);
			break;
		case CSU_TYPE_UNION:
			MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_UNION);
			break;
		case CSU_TYPE_ENUM:
			MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_ENUM);
			break;
		default:
			MY_ASSERT(false);
		}
		/*if (pTypeDef->getPrefix() != prefix)
		{
			pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), pTypeDef->getName(), pTypeDef, NULL));
			pTypeDef->setPrefix(prefix);
		}*/
		pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", pTypeDef, NULL));
		break;
	}
	case BASICTYPE_TYPE_TYPEOF:
		pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", tokenWithNamespaceGetInfo(typeTypeOfGetInfo(pRoot))));
		break;

	case BASICTYPE_TYPE_DATA_MEMBER_POINTER:
	{
		SourceTreeNode* pExtendedTypeNode;
		TokenWithNamespace twn;
		typeDmpGetInfo(pRoot, pExtendedTypeNode, twn);

		TypeDefPointer pExtendedTypeDef = getTypeDefByExtendedTypeNode(pExtendedTypeNode);
		MY_ASSERT(pExtendedTypeDef);

		TypeDefPointer pDataScope;
		SymbolDefObject* pSymbolObj = findSymbolEx(twn);
		if (!pSymbolObj)
			throw("Cannot find " + twn.toString());
		if (pSymbolObj->type == GO_TYPE_TYPEDEF)
		{
			pDataScope = pSymbolObj->getTypeDef();
			MY_ASSERT(pDataScope->getType() == SEMANTIC_TYPE_CLASS || pDataScope->getType() == SEMANTIC_TYPE_STRUCT);
		}
		else
			throw(twn.toString() + " is not a class type");

		pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", SEMANTIC_TYPE_DATA_MEMBER_POINTER, pExtendedTypeDef, pDataScope));
		break;
	}
	default:
		MY_ASSERT(false);
	}
	if (pTypeDef)
	{
		StringVector mod_strings, mod2_strings;
		typeGetModifierBits(pRoot, mod_strings, mod2_strings);
		if (isInModifiers(mod_strings, MODBIT_CONST) || isInModifiers(mod2_strings, MODBIT_CONST))
		{
			//if (!pTypeDef->getName().empty())
			//	pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, 0));
			pTypeDef->setConst(true);
		}
		pTypeDef->setDisplayString(displaySourceTreeType(pRoot));
	}
	return pTypeDef;
}

TypeDefPointer CScope::getTypeDefBySuperTypeNode(const SourceTreeNode* pRoot, bool bDefining)
{
	SuperTypeType super_type;
	SourceTreeNode* pChildNode;
	superTypeGetInfo(pRoot, super_type, pChildNode);
	TypeDefPointer pTypeDef;

	switch (super_type)
	{
	case SUPERTYPE_TYPE_TYPE:
		pTypeDef = getTypeDefByTypeNode(pChildNode, bDefining);
		break;

	default:
		MY_ASSERT(false);
	}

	//SourceTreeNode* pExtendedType = extendedTypeCreateFromType(dupSourceTreeNode(pRoot));
	//pTypeDef->setDisplaySourceTree(pExtendedType);
	//deleteSourceTreeNode(pExtendedType);
	return pTypeDef;
}

TypeDefPointer CScope::getTypeDefByExtendedTypeNode(const SourceTreeNode* pRoot, bool bDefining)
{
	if (!pRoot)
		return TypeDefPointer();

	SourceTreeNode* pTypeNode = extendedTypeGetTypeNode(pRoot);
	int depth = extendedTypeGetDepth(pRoot);

	TypeDefPointer pTypeDef = getTypeDefByTypeNode(pTypeNode, bDefining);
	if (!pTypeDef)
		return pTypeDef;
	//if (depth == 0)
	//	return pTypeDef;

	TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, extendedTypeGetDepth(pRoot)));
	pTypeDef2->setReference(extendedTypeIsReference(pRoot));
	pTypeDef2->setModStrings(extendedTypeGetModStrings(pRoot));
	return pTypeDef2;
}

TypeDefPointer CScope::getTypeDefByExtendedTypeVarNode(const SourceTreeNode* pRoot, bool bDefining)
{
	SourceTreeNode* pExtendedTypeNode;
	int depth;
	extendedTypeVarGetInfo(pRoot, pExtendedTypeNode, depth);
	TypeDefPointer pTypeDef = getTypeDefByExtendedTypeNode(pExtendedTypeNode, bDefining);
	if (depth == 0)
		return pTypeDef;

	return TypeDefPointer(new CTypeDef(NULL, "", pTypeDef->getBaseType(), depth));
}

TypeDefPointer CScope::getTypeDefByExtendedOrFuncTypeNode(const SourceTreeNode* pRoot, bool bDefining)
{
	bool bExtendedType;
	SourceTreeNode* pChild;
	extendedOrFuncTypeGetInfo(pRoot, bExtendedType, pChild);
	if (bExtendedType)
		return getTypeDefByExtendedTypeNode(pChild);

	return getTypeDefByFuncTypeNode(pChild);
}

TypeDefPointer CScope::getTypeDefByDeclVarNode(TypeDefPointer pTypeDef, const SourceTreeNode* pDeclVar)
{
	/*int depth = declVarGetDepth(pDeclVar);

	if (depth == 0)
		return pTypeDef;*/

	MY_ASSERT(pTypeDef);
	TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, declVarGetDepth(pDeclVar)));

	if (declVarIsReference(pDeclVar))
		pTypeDef2->setReference(true);
	if (declVarIsConst(pDeclVar))
		pTypeDef2->setConst(true);

	return pTypeDef2;
}

TypeDefPointer CScope::getTypeDefByFuncTypeNode(const SourceTreeNode* pRoot)
{
	SourceTreeNode* pReturnExtendedType, *pScope, *pFuncParamsNode;
	std::string name;
	StringVector mod_strings, mod2_strings;
	int nDepth;
	funcTypeGetInfo(pRoot, pReturnExtendedType, mod_strings, pScope, nDepth, name, pFuncParamsNode, mod2_strings);

	TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(pReturnExtendedType), nDepth));
	pTypeDef->setMod3Strings(mod_strings);
	pTypeDef->setMod4Strings(mod2_strings);
	pTypeDef->setFuncReturnTypeNode(dupSourceTreeNode(pReturnExtendedType));
	addFuncParamsToFuncType(pTypeDef, pFuncParamsNode);

	/*if (pOptFuncParamsNode)
	{
		pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, pTypeDef, 1));
		addFuncParamsToFuncType(pTypeDef, pOptFuncParamsNode);
	}*/
	return pTypeDef;
}

void CScope::addFuncParamsToFuncType(TypeDefPointer pTypeDef, const SourceTreeNode* pFuncParamsNode)
{
	MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_FUNC);

	if (!pFuncParamsNode)
		return;

	SourceTreeVector v = funcParamsGetList(pFuncParamsNode);
	for (unsigned i = 0; i < v.size(); i++)
	{
		FuncParamType param_type;
		StringVector mod_strings;
		SourceTreeNode* pTypeNode, *pDeclVarNode;
		void *pInitExprBlock;
		funcParamGetInfo(v[i], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);

		switch (param_type)
		{
		case FUNC_PARAM_TYPE_REGULAR:
		{
			TypeDefPointer pTypeDef2 = getTypeDefByTypeNode(pTypeNode, true, true);
			if (!pTypeDef2)
			{
				std::string s = displaySourceTreeType(pTypeNode);
				throw(std::string("cannot recognize type ") + s);
			}
			TRACE("CScope::%s, i=%u, type '%s', const=%d\n", __FUNCTION__, i, displaySourceTreeType(pTypeNode).c_str(), pTypeDef2->isConst());
			//pTypeDef2 = getTypeDefByDeclVarNode(pTypeDef2, pDeclVarNode);
			SemanticDataType basicType = pTypeDef->getType();
			CExpr* pInitExpr = NULL;
			if (pInitExprBlock && getParentTemplate() == NULL)
			{
				CGrammarAnalyzer ga;
				ga.initWithBlocks(getRealScope(), pInitExprBlock);
				SourceTreeNode* pInitExprNode = ga.getBlock();
				MY_ASSERT(ga.isEmpty());
				pInitExpr = new CExpr(this, pInitExprNode);
				deleteSourceTreeNode(pInitExprNode);
			}
			pTypeDef->addFuncParam(new CVarDef(NULL, declVarGetName(pDeclVarNode), pTypeDef2, dupSourceTreeNode(pDeclVarNode), pInitExpr));
			break;
		}
		case FUNC_PARAM_TYPE_FUNC:
		{
			TypeDefPointer pTypeDef2 = getTypeDefByFuncTypeNode(pTypeNode);

			SourceTreeNode* pReturnExtendedType, *pScope, *pFuncParamsNode;
			std::string name;
			StringVector mod_strings, mod2_strings;
			int nDepth;
			funcTypeGetInfo(pTypeNode, pReturnExtendedType, mod_strings, pScope, nDepth, name, pFuncParamsNode, mod2_strings);
			//if (!name.empty())
			pTypeDef->addFuncParam(new CVarDef(NULL, name, pTypeDef2));
			break;
		}
		default:
			MY_ASSERT(false);
		}
	}

	if (funcParamsHasVArgs(pFuncParamsNode))
		pTypeDef->setHasVArgs(true);
}

void CExpr::init()
{
	setRealScope(false);
	m_bFlow = false;
	//m_pVarDef = NULL;
	m_bFlag = false;
	m_return_type = TypeDefPointer();
	m_return_depth = 0;
	m_pFuncDeclare = NULL;
	m_return_symbol_obj = NULL;
	m_caller_is_const = false;
	m_pSourceTreeNode = NULL;
}

// returns return_type and whether includes a sync call
CExpr::CExpr(CScope* pParent) : CScope(pParent)
{
	init();
}

CExpr::CExpr(CScope* pParent, const SourceTreeNode* pRoot) : CScope(pParent)
{
	init();

	analyze(pRoot);
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, const std::string& val) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_CONST_VALUE || expr_type == EXPR_TYPE_TOKEN_WITH_NAMESPACE);

	init();
	m_expr_type = expr_type;
	m_value = val;

	if (expr_type == EXPR_TYPE_TOKEN_WITH_NAMESPACE)
		m_token_with_namespace.addScope(val, false);
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, const TokenWithNamespace& twn) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_TOKEN_WITH_NAMESPACE);

	init();
	m_expr_type = expr_type;
	m_token_with_namespace = twn;
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, TypeDefPointer pTypeDef) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_NEW_C || expr_type == EXPR_TYPE_SIZEOF);

	init();
	m_expr_type = expr_type;
	m_pTypeDef = pTypeDef;
	if (expr_type == EXPR_TYPE_NEW_C)
	{
		m_return_type = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, 1));
		m_return_depth = 1;
	}
	else
	{
		m_return_type = g_type_def_int;
		m_return_depth = 0;
	}
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, const std::string& token) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_REF_ELEMENT || expr_type == EXPR_TYPE_PTR_ELEMENT);

	init();
	m_expr_type = expr_type;
	m_value = token;
	m_token_with_namespace.addScope(token);
	addChild(pLeft);
	pLeft->setParent(this);
	m_bFlow = pLeft->isFlow();
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, const TokenWithNamespace& twn) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_REF_ELEMENT || expr_type == EXPR_TYPE_PTR_ELEMENT);

	init();
	m_expr_type = expr_type;
	m_token_with_namespace = twn;
	m_value = m_token_with_namespace.toString();
	addChild(pLeft);
	pLeft->setParent(this);
	m_bFlow = pLeft->isFlow();
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, CExpr* pRight) : CScope(pParent)
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

	init();
	m_expr_type = expr_type;
	pLeft->setParent(this);
	addChild(pLeft);
	pRight->setParent(this);
	addChild(pRight);
	m_bFlow = pLeft->isFlow() || pRight->isFlow();
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_RIGHT_INC || expr_type == EXPR_TYPE_RIGHT_DEC || expr_type == EXPR_TYPE_LEFT_INC ||
			expr_type == EXPR_TYPE_LEFT_DEC || expr_type == EXPR_TYPE_POSITIVE || expr_type == EXPR_TYPE_NEGATIVE ||
			expr_type == EXPR_TYPE_NOT || expr_type == EXPR_TYPE_BIT_NOT || expr_type == EXPR_TYPE_INDIRECTION ||
			expr_type == EXPR_TYPE_ADDRESS_OF || expr_type == EXPR_TYPE_DELETE || expr_type == EXPR_TYPE_PARENTHESIS);

	init();
	m_expr_type = expr_type;
	addChild(pExpr);
	pExpr->setParent(this);
	m_bFlow = pExpr->isFlow();
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, TypeDefPointer pTypeDef, CExpr* pLeft) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_TYPE_CAST);

	init();
	m_expr_type = expr_type;
	addChild(pLeft);
	pLeft->setParent(this);
	m_return_type = pTypeDef;
	m_return_depth = pTypeDef->getDepth();
	m_pTypeDef = pTypeDef;//->generateExtendedTypeNode();
	m_bFlow = pLeft->isFlow();
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CFuncDeclare* pFuncDeclare, const std::vector<CExpr*>& param_list) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_FUNC_CALL);

	init();
	m_expr_type = expr_type;
	m_pFuncCallType = pFuncDeclare->getType();
	CExpr* pExpr = new CExpr(pParent, EXPR_TYPE_CONST_VALUE, pFuncDeclare->getName());
	pExpr->setFuncDeclare(pFuncDeclare);
	addChild(pExpr);
	m_value = pFuncDeclare->getName();
	m_return_type = m_pFuncCallType->getFuncReturnType();
	m_return_depth = 0;
	m_bFlow = m_pFuncCallType->isFuncFlow();
	MY_ASSERT(!m_bFlow);

	for (size_t i = 0; i < param_list.size(); i++)
		addChild(param_list[i]);
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr, const std::vector<CExpr*>& param_list) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_FUNC_CALL);
	MY_ASSERT(pExpr->getFuncDeclare());

	init();
	m_expr_type = expr_type;
	addChild(pExpr);
	for (size_t i = 0; i < param_list.size(); i++)
	{
		CExpr* pObj = param_list[i];
		MY_ASSERT(pObj->getGoType() == GO_TYPE_EXPR);
		addChild((CExpr*)pObj);
	}
}

CExpr::CExpr(CScope* pParent, ExprType expr_type, CExpr* pCondExpr, CExpr* pExpr1, CExpr* pExpr2) : CScope(pParent)
{
	MY_ASSERT(expr_type == EXPR_TYPE_TERNARY);

	init();
	m_expr_type = expr_type;
	addChild(pCondExpr);
	pCondExpr->setParent(this);
	m_bFlow |= pCondExpr->isFlow();
	addChild(pExpr1);
	pExpr1->setParent(this);
	m_bFlow |= pExpr1->isFlow();
	addChild(pExpr2);
	pExpr2->setParent(this);
	m_bFlow |= pExpr2->isFlow();
}

CExpr::~CExpr()
{
}

std::string CExpr::getDebugName()
{
	return getName() + ":" + ltoa(m_expr_type);
}

void CExpr::analyze(const SourceTreeNode* pRoot)
{
	m_expr_type = exprGetType(pRoot);
	switch (m_expr_type)
	{
	case EXPR_TYPE_CONST_VALUE:			// const_value
		m_value = exprConstGetValue(pRoot);
		if (m_value.c_str()[0] == '"')
		{
			m_return_type = g_type_def_const_char_ptr;
			m_return_depth = 1;
		}
		else
		{
			m_return_type = g_type_def_int;
			m_return_depth = 0;
			if (m_value == "0")
				m_return_type->setZero();
		}
		break;

	case EXPR_TYPE_TOKEN_WITH_NAMESPACE:	// token
	{
		m_token_with_namespace.copyFrom(tokenWithNamespaceGetInfo(exprGetSecondNode(pRoot)));
		if (m_token_with_namespace.getDepth() == 1)
		{
			std::string s = m_token_with_namespace.getLastToken();
			if (s == "true" || s == "false")
			{
				m_return_type = g_type_def_int;
				break;
			}
			if (s == "null" || s == "__null")
			{
				m_return_type = g_type_def_int;
				m_return_type->setZero();
				break;
			}
		}
		SymbolDefObject* pSymbolObj = findSymbolEx(m_token_with_namespace);
		if (pSymbolObj == NULL)
		{
			if (getParentTemplate())
			{
				TRACE("CExpr::%s, cannot find %s in a template define, assume int\n", __FUNCTION__, m_token_with_namespace.toString().c_str());
				m_return_type = g_type_def_int;
				m_return_depth = 0;
				break;
			}
			std::string p = getDebugPath();
			std::string s = m_token_with_namespace.toString();
			MY_ASSERT(false);
			throw("cannot find symbol " + s);
		}
		TRACE("CExpr::%s, twn=%s, resolved type=%d\n", __FUNCTION__, m_token_with_namespace.toString().c_str(), pSymbolObj->type);
		switch (pSymbolObj->type)
		{
		case GO_TYPE_ENUM:
			m_return_type = g_type_def_int;
			m_return_depth = 0;
			break;

		case GO_TYPE_VAR_DEF:
			m_return_type = pSymbolObj->getVarDef()->getType();
			m_return_depth = m_return_type->getDepth();
			break;

		case GO_TYPE_FUNC_DECL:
		{
			CScope* pScope = getParentScope(GO_TYPE_FUNC);
			if (!pScope)
				m_caller_is_const = false;
			else
				m_caller_is_const = ((CFunction*)pScope)->getFuncType()->isConst();
			m_return_symbol_obj = pSymbolObj;
			break;
		}
		case GO_TYPE_TYPEDEF:
			m_return_type = pSymbolObj->getTypeDef();
			m_return_depth = m_return_type->getDepth();
			break;

		case GO_TYPE_TEMPLATE:
			m_return_symbol_obj = pSymbolObj;
			break;

		default:
			throw("symbol " + m_token_with_namespace.toString() + " is not allowed as a token");
		}
		break;
	}
	case EXPR_TYPE_FUNC_CALL:		// expr '(' *[expr, ','] ')'
	{
		SourceTreeNode* pFuncExpr = exprGetFirstNode(pRoot);
		SourceTreeVector param_exprs = exprGetExprList(pRoot);
		std::string func_call_str = displaySourceTreeExpr(pFuncExpr);
		CExpr* pExpr = new CExpr(this, pFuncExpr);
		addChild(pExpr);
		TRACE("func call: %s, ", displaySourceTreeExpr(pRoot).c_str());
		std::vector<TypeDefPointer> typeList;
		for (size_t i = 0; i < param_exprs.size(); i++)
		{
			SourceTreeNode*& func_param = param_exprs[i];
			//bool bFlow2;
			CExpr* pExpr = new CExpr(this, func_param);
			addChild(pExpr);
			m_bFlow |= pExpr->isFlow();

			typeList.push_back(createTypeByDepth(pExpr->getReturnType(), pExpr->getReturnDepth()));
		}

		TypeDefPointer funcType = pExpr->getReturnType();
		if (funcType)
		{
			// a call in a template
			if (funcType->getType() != SEMANTIC_TYPE_FUNC)
			{
				if (funcType->getType() != SEMANTIC_TYPE_BASIC)
				{
					m_return_type = funcType;
					m_return_depth = m_return_type->getDepth();
					MY_ASSERT(m_return_depth == 0);
					break;
				}
				//throw(func_call_str + " is not of func type, buf of a " + ltoa(funcType->getType()));
			}
			int nDepth = pExpr->getReturnDepth();
			while (!funcType->isBaseType())
			{
				nDepth += funcType->getBaseType()->getDepth();
				funcType = funcType->getBaseType();
			}
			if (nDepth != 0)
				throw(func_call_str + " is not a func ptr, but a " + pExpr->getReturnType()->toFullString() + ":" + ltoa(pExpr->getReturnDepth()));

			if (funcType->checkCallParams(typeList, funcType->isConst()) < 0)
				throw("parameters in " + func_call_str + "() don't match its definition");
		}
		else if (pExpr->m_return_symbol_obj)
		{
			if (pExpr->m_return_symbol_obj->type == GO_TYPE_FUNC_DECL)
			{
				int maxMatchScore = -1;
				std::vector<CGrammarObject*> matched_v;
				checkBestMatchedFunc(pExpr->m_return_symbol_obj->children, typeList, m_caller_is_const, maxMatchScore, matched_v);
				if (matched_v.empty())
				{
					std::string s = pExpr->toString();
					throw("Cannot find matched func definition for " + s);
				}
				else if (matched_v.size() == 1)
				{
					CGrammarObject* pMatchedGO = matched_v[0];
					if (pMatchedGO->getGoType() == GO_TYPE_FUNC_DECL)
					{
						funcType = ((CFuncDeclare*)pMatchedGO)->getType();
						pExpr->setFuncDeclare((CFuncDeclare*)pMatchedGO);
					}
					else if (pMatchedGO->getGoType() == GO_TYPE_TEMPLATE)
					{
						TemplateResolvedDefParamVector typeDefMap;
						((CTemplate*)pMatchedGO)->funcCheckFitForTypeList(typeList, typeDefMap);
						funcType = ((CTemplate*)pMatchedGO)->funcGetInstance(typeList, typeDefMap);
					}
					else
						MY_ASSERT(false);
					pExpr->setReturnType(funcType);
					pExpr->setReturnDepth(1);
				}
				else
				{
					std::string err_s = "call of " + func_call_str + " at " + g_cur_file_name + ":" + ltoa(g_cur_line_no) + " is ambiguous. Choices are:\n";
					for (unsigned j = 0; j < matched_v.size(); j++)
						err_s += "   " + matched_v[j]->definedIn() + "\n";
					throw(err_s);
				}
				TRACE("func call expr return type=%s, depth=%d\n", (funcType->getFuncReturnType() ? funcType->getFuncReturnType()->toFullString().c_str() : "void"),
				    (funcType->getFuncReturnType() ? funcType->getFuncReturnType()->getDepth() : -1));
			}
			else
				MY_ASSERT(false);
		}
		else
			MY_ASSERT(false);

		m_return_type = funcType->getFuncReturnType();
		m_return_depth = m_return_type ? m_return_type->getDepth() : 0;
		if (funcType->isFuncFlow())
		{
			m_bFlow = true;
		}
		if (m_bFlow)
		{
			TRACE(" which is a flow call\n");
		}
		else
			TRACE(" which is not a flow call\n");
		TRACE("m_bFlow=%d\n", m_bFlow);
		break;
	}
	case EXPR_TYPE_OPERATOR_CALL:
	{
		SourceTreeNode* pScopeNode = exprGetFirstNode(pRoot);
		std::string op_str = "operator " + operatorGetString(exprGetSecondNode(pRoot));
		SourceTreeNode* pParamNode = exprGetOptionalThirdNode(pRoot);
		if (pParamNode)
		{
			CExpr* pExpr = new CExpr(this, pParamNode);
			addChild(pExpr);
			m_bFlow |= pExpr->isFlow();
		}
		TypeDefPointer pTypeDef;
		m_token_with_namespace.copyFrom(scopeGetInfo(pScopeNode));
		if (!m_token_with_namespace.empty())
		{
			SymbolDefObject* pSymbolObj = findSymbolEx(m_token_with_namespace);
			if (!pSymbolObj)
				throw("cannot recognize " + m_token_with_namespace.toString());
			if (pSymbolObj->type == GO_TYPE_TEMPLATE)
			{
				MY_ASSERT(m_token_with_namespace.scopeHasTemplate(m_token_with_namespace.getDepth() - 1));
				pTypeDef = pSymbolObj->getTemplateAt(0)->classGetInstance(m_token_with_namespace, m_token_with_namespace.getDepth() - 1, getRealScope());
			}
			else if (pSymbolObj->type == GO_TYPE_TYPEDEF)
				pTypeDef = pSymbolObj->getTypeDef();
			else if (pSymbolObj->type == GO_TYPE_NAMESPACE)
				pSymbolObj = pSymbolObj->getNamespace()->findSymbol(op_str, FIND_SYMBOL_SCOPE_SCOPE);
			else
				throw(m_token_with_namespace.toString() + " is defined as a " + getGoTypeName(pSymbolObj->type) + ", not a scope");

			if (pTypeDef)
			{
				MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());
				CClassDef* pClassDef = pTypeDef->getBaseType()->getClassDef();
				pSymbolObj = pClassDef->findSymbol(op_str, FIND_SYMBOL_SCOPE_SCOPE);
			}
			if (!pSymbolObj)
				throw(m_token_with_namespace.toString() + " doesn't have " + op_str + " defined");
			if (pSymbolObj->type != GO_TYPE_FUNC_DECL)
				throw(m_token_with_namespace.toString() + "::" + op_str + " is not defined as a function");

			if (pTypeDef)
			{
				m_token_with_namespace.addScope(op_str, false);

				m_return_type = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, 1));
				m_return_depth = 0;
				break;
			}
		}

		m_token_with_namespace.addScope(op_str, false);

		m_return_type = TypeDefPointer(new CTypeDef(NULL, "", g_type_def_void, 1));
		m_return_depth = 1;
		break;
	}
	case EXPR_TYPE_SIZEOF:			// (extended_type_var | func_type | expr)
	{
		int nType;
		SourceTreeNode* pChild;
		exprSizeOfGetInfo(pRoot, nType, pChild);
		if (nType == 0)
		{
			m_pTypeDef = getTypeDefByExtendedTypeVarNode(pChild);
		}
		else if (nType == 1)
		{
			m_pTypeDef = getTypeDefByFuncTypeNode(pChild);
		}
		else
		{
			MY_ASSERT(nType == 2);
			CExpr* pExpr = new CExpr(this, pChild);
			addChild(pExpr);
			m_bFlow = pExpr->isFlow();
		}
		m_return_type = g_type_def_int;
		break;
	}
	case EXPR_TYPE_NOT:				// expr
	case EXPR_TYPE_BIT_NOT:			// expr
	{
		CExpr* pExpr = new CExpr(this, pRoot->pChild->pChild);
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = g_type_def_int;
		break;
	}
	case EXPR_TYPE_THROW:			// ?[expr]
	{
		if (pRoot->pChild->param)
		{
			CExpr* pExpr = new CExpr(this, pRoot->pChild->pChild->pChild);
			addChild(pExpr);
			m_bFlow = pExpr->isFlow();
		}
		else
			addChild(NULL);
		m_return_type = g_type_def_int;
		break;
	}
	case EXPR_TYPE_NEW_C:				// scope extended_type_var
	{
		m_token_with_namespace.copyFrom(scopeGetInfo(exprGetFirstNode(pRoot)));
		m_pTypeDef = getTypeDefByExtendedTypeVarNode(exprGetSecondNode(pRoot));
		m_return_type = TypeDefPointer(new CTypeDef(NULL, "", m_pTypeDef, 1));
		m_return_depth = 1;
		break;
	}
	case EXPR_TYPE_NEW_OBJECT:		  // scope user_def_type expr2
	{
		m_token_with_namespace.copyFrom(scopeGetInfo(exprGetFirstNode(pRoot)));
		m_pTypeDef = getTypeDefByUserDefTypeNode(exprGetSecondNode(pRoot));
		SourceTreeVector expr_v = expr2GetExprs(exprGetThirdNode(pRoot));
		for (size_t i = 0; i < expr_v.size(); i++)
		{
			SourceTreeNode* pParamNode = expr_v[i];
			//bool bFlow2;
			CExpr* pExpr = new CExpr(this, pParamNode);
			addChild(pExpr);
			m_bFlow |= pExpr->isFlow();
		}
		m_return_type = TypeDefPointer(new CTypeDef(NULL, "", m_pTypeDef, 1));
		m_return_depth = 1;
		break;
	}
	case EXPR_TYPE_NEW_ADV: // scope 'new' '(' expr ')' ?[ user_def_type ?[ '(' expr2 ')' ] ]
	{
		m_token_with_namespace.copyFrom(scopeGetInfo(exprGetFirstNode(pRoot)));
		CExpr* pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();
		SourceTreeNode* pUserDefType, *pExpr2;
		exprNewAdvGetParams(pRoot, pUserDefType, pExpr2);
		if (pUserDefType)
		{
			m_pTypeDef = getTypeDefByUserDefTypeNode(pUserDefType);
			if (pExpr2)
			{
				m_bFlag = true;
				SourceTreeVector expr_v = expr2GetExprs(pExpr2);
				for (size_t i = 0; i < expr_v.size(); i++)
				{
					SourceTreeNode* pParamNode = expr_v[i];
					//bool bFlow2;
					CExpr* pExpr = new CExpr(this, pParamNode);
					addChild(pExpr);
					m_bFlow |= pExpr->isFlow();
				}
			}
		}
		else
			m_pTypeDef = g_type_def_void;

		m_return_type = TypeDefPointer(new CTypeDef(NULL, "", m_pTypeDef, 1));
		m_return_depth = 1;
		break;
	}
	case EXPR_TYPE_DELETE:		  // scope ?[] expr
	{
		m_token_with_namespace.copyFrom(scopeGetInfo(exprGetFirstNode(pRoot)));
		CExpr* pExpr = new CExpr(this, exprGetThirdNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = g_type_def_int;
		m_bFlag = exprDeleteHasArray(pRoot);
		break;
	}
	case EXPR_TYPE_TYPE_CONSTRUCT:		// type, expr2
	{
		m_pTypeDef = getTypeDefByTypeNode(exprGetFirstNode(pRoot));
		MY_ASSERT(m_pTypeDef);

		SourceTreeVector expr_v = expr2GetExprs(exprGetSecondNode(pRoot));
		for (size_t i = 0; i < expr_v.size(); i++)
		{
			SourceTreeNode* pParamNode = expr_v[i];
			//bool bFlow2;
			CExpr* pExpr = new CExpr(this, pParamNode);
			addChild(pExpr);
			m_bFlow |= pExpr->isFlow();
		}
		m_return_type = m_pTypeDef;
		m_return_depth = m_pTypeDef->getDepth();
		break;
	}
	case EXPR_TYPE_RIGHT_INC: 		// expr
	case EXPR_TYPE_RIGHT_DEC:		// expr
	case EXPR_TYPE_LEFT_INC:			// expr
	case EXPR_TYPE_LEFT_DEC:			// expr
	case EXPR_TYPE_POSITIVE:			// expr
	case EXPR_TYPE_NEGATIVE:			// expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth();
		break;
	}
	case EXPR_TYPE_PARENTHESIS:		// expr2
	{
		CExpr2* pExpr2 = new CExpr2(this);
		pExpr2->analyze(exprGetFirstNode(pRoot));
		addChild(pExpr2);
		m_bFlow = pExpr2->isFlow();
		m_return_type = pExpr2->getReturnType();
		m_return_depth = pExpr2->getReturnDepth();
		m_return_symbol_obj = pExpr2->getReturnSymbolObj();
		break;
	}
	case EXPR_TYPE_INDIRECTION:		// expr
	{
		SourceTreeNode* pExprNode = exprGetFirstNode(pRoot);
		CExpr* pExpr = new CExpr(this, pExprNode);
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth();
		while (m_return_depth == 0 && !m_return_type->isBaseType())
		{
			m_return_type = m_return_type->getBaseType();
			m_return_depth = m_return_type->getDepth();
		}
		if (m_return_depth > 0)
		{
			m_return_depth--;
			break;
		}
		if (m_return_type->getType() != SEMANTIC_TYPE_CLASS && m_return_type->getType() != SEMANTIC_TYPE_STRUCT)
			throw(displaySourceTreeExpr(pExprNode) + " is not a class but a " + getSemanticTypeName(m_return_type->getType()));

		CClassDef* pClassDef = m_return_type->getClassDef();
		MY_ASSERT(pClassDef);
		SymbolDefObject* pSymbolObj = pClassDef->findSymbol("operator *", FIND_SYMBOL_SCOPE_PARENT);
		if (!pSymbolObj)
			throw("Class " + displaySourceTreeExpr(pExprNode) + " doesn't have operator * override method");
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		MY_ASSERT(pSymbolObj->children.size() == 1);
		MY_ASSERT(pSymbolObj->children[0]->getGoType() == GO_TYPE_FUNC_DECL);
		m_return_type = pSymbolObj->getFuncDeclareAt(0)->getType()->getFuncReturnType();
		m_return_depth = m_return_type->getDepth();
		break;
	}
	case EXPR_TYPE_ADDRESS_OF:		// expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth() + 1;
		break;
	}
	case EXPR_TYPE_ARRAY:			// expr expr
	{
		SourceTreeNode* pExprNode = exprGetFirstNode(pRoot);
		CExpr* pExpr = new CExpr(this, pExprNode);
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth();

		TRACE("Before array, type=%s, depth=%d\n", m_return_type->toString().c_str(), m_return_depth);
		while (m_return_depth == 0 && !m_return_type->isBaseType())
		{
			m_return_type = m_return_type->getBaseType();
			m_return_depth = m_return_type->getDepth();
		}
		if (m_return_depth > 0)
		{
			m_return_depth--;
		}
		else
		{
			if (m_return_type->getType() != SEMANTIC_TYPE_CLASS && m_return_type->getType() != SEMANTIC_TYPE_STRUCT)
				throw(displaySourceTreeExpr(pExprNode) + " is not a class but a " + getSemanticTypeName(m_return_type->getType()));

			CClassDef* pClassDef = m_return_type->getClassDef();
			MY_ASSERT(pClassDef);
			SymbolDefObject* pSymbolObj = pClassDef->findSymbol("operator []", FIND_SYMBOL_SCOPE_PARENT);
			if (!pSymbolObj)
				throw("Class " + displaySourceTreeExpr(pExprNode) + " doesn't have operator [] override method");
			MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
			// just choose the first one.
			//unsigned sz = pSymbolObj->children.size();
			//MY_ASSERT(pSymbolObj->children.size() == 1);
			MY_ASSERT(pSymbolObj->children[0]->getGoType() == GO_TYPE_FUNC_DECL);
			m_return_type = pSymbolObj->getFuncDeclareAt(0)->getType()->getFuncReturnType();
			m_return_depth = m_return_type->getDepth();
		}
		TRACE("After array, type=%s, depth=%d\n", m_return_type->toString().c_str(), m_return_depth);
		pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();
		break;
	}
	case EXPR_TYPE_REF_ELEMENT:	 // expr scope ['~'] token ['(' expr2 ')']
	case EXPR_TYPE_PTR_ELEMENT:
	{
		SourceTreeNode* pExprNode, *pScopeNode, *pExpr2Node = NULL;
		std::string token;
		exprPtrRefGetInfo(pRoot, pExprNode, pScopeNode, token);
		m_token_with_namespace = scopeGetInfo(pScopeNode);
		m_token_with_namespace.addScope(token);
		m_value = m_token_with_namespace.toString();

		CExpr* pExpr = new CExpr(this, pExprNode);
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();

		/*if (pExpr2Node)
		{
			m_bFlag = true;

			SourceTreeVector exprList = expr2GetExprs(pExpr2Node);
			for (int i = 0; i < exprList.size(); i++)
			{
				CExpr* pExpr = new CExpr(this, exprList[i]);
				addChild(pExpr);
				m_bFlow |= pExpr->isFlow();
			}
		}*/

		TypeDefPointer pTypeDef = pExpr->getReturnType();
		std::string s2 = pTypeDef->toFullString();
		SemanticDataType dataType = pTypeDef->getType();
		/*if (dataType != SEMANTIC_TYPE_STRUCT && dataType != SEMANTIC_TYPE_UNION && dataType != SEMANTIC_TYPE_CLASS)
		{
			std::string s = displaySourceTreeExpr(pRoot);
			throw("type " + pExpr->toString() + " is not a struct or union type, but a " + getSemanticTypeName(dataType));
		}*/

		int nDepthToRoot = pExpr->getReturnDepth();
		pTypeDef = pTypeDef->getBaseType();
		while (!pTypeDef->isBaseType())
		{
			nDepthToRoot += pTypeDef->getDepth();
			pTypeDef = pTypeDef->getBaseType();
		}
		if (nDepthToRoot == 0 && m_expr_type == EXPR_TYPE_PTR_ELEMENT && pTypeDef->getClassDef())
		{
			SymbolDefObject* pObj = pTypeDef->getClassDef()->findSymbol("operator ->", FIND_SYMBOL_SCOPE_LOCAL);
			if (pObj)
			{
				pTypeDef = pObj->getFuncDeclareAt(0)->getType()->getFuncReturnType();
				while (!pTypeDef->isBaseType())
				{
					nDepthToRoot += pTypeDef->getDepth();
					pTypeDef = pTypeDef->getBaseType();
				}
			}
		}
		if (nDepthToRoot != (m_expr_type == EXPR_TYPE_REF_ELEMENT ? 0 : 1))
			throw(pExpr->toString() + " depth is " + ltoa(nDepthToRoot));

		if (dataType == SEMANTIC_TYPE_BASIC)
		{
			//std::string s = pTypeDef->toFullString();
			//if (s != m_value && std::string("~") + s != m_value)
			//	throw("recognized method for basic types");
			m_return_type = g_type_def_func_void;
			m_return_depth = m_return_type->getDepth();
		}
		else
		{
			if (m_token_with_namespace.getLastToken().substr(0, 1) == "~") // call destructor
			{
				m_return_type = g_type_def_func_void;
				m_return_depth = m_return_type->getDepth();
				break;
			}
			SymbolDefObject* pSymbolObj = pTypeDef->getClassDef()->findSymbol(m_token_with_namespace.getLastToken(), FIND_SYMBOL_SCOPE_SCOPE);
			if (!pSymbolObj)
			{
				std::string s = pTypeDef->toFullString();
				throw("in " + pExpr->toString() + ", member " + m_value + " is not found");
			}
			if (pSymbolObj->type == GO_TYPE_VAR_DEF)
			{
				m_return_type = pSymbolObj->getVarDef()->getType();
				m_return_depth = m_return_type->getDepth();
			}
			else if (pSymbolObj->type == GO_TYPE_FUNC_DECL || pSymbolObj->type == GO_TYPE_TEMPLATE)
			{
				m_return_symbol_obj = pSymbolObj;
				m_caller_is_const = pTypeDef->isConst();
			}
			else
				throw("in " + pExpr->toString() + ", member " + m_value + " is defined as " + getGoTypeName(pSymbolObj->type).c_str());
		}
		//TRACE("resolve REF/PTR, member=%s, type=%s\n", m_value.c_str(), typeDescBlock2String(m_type_desc_block).c_str());
		break;
	}
	case EXPR_TYPE_MULTIPLE:			// expr expr
	case EXPR_TYPE_DIVIDE:			// expr expr
	case EXPR_TYPE_REMAINDER:		// expr expr
	case EXPR_TYPE_ADD:				// expr expr
	case EXPR_TYPE_LEFT_SHIFT:		// expr expr
	case EXPR_TYPE_RIGHT_SHIFT:		// expr expr
	case EXPR_TYPE_ASSIGN:			// expr expr
	case EXPR_TYPE_ADD_ASSIGN:		// expr expr
	case EXPR_TYPE_SUBTRACT_ASSIGN:	// expr expr
	case EXPR_TYPE_MULTIPLE_ASSIGN:	// expr expr
	case EXPR_TYPE_DIVIDE_ASSIGN:	// expr expr
	case EXPR_TYPE_REMAINDER_ASSIGN:	// expr expr
	case EXPR_TYPE_LEFT_SHIFT_ASSIGN:	// expr expr
	case EXPR_TYPE_RIGHT_SHIFT_ASSIGN:	// expr expr
	case EXPR_TYPE_BIT_AND_ASSIGN:		// expr expr
	case EXPR_TYPE_BIT_XOR_ASSIGN:		// expr expr
	case EXPR_TYPE_BIT_OR_ASSIGN:		// expr expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth();
		pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();
		break;
	}
	case EXPR_TYPE_SUBTRACT:			// expr expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();
		m_return_type = g_type_def_int;
		m_return_depth = 0;
		break;
	}
	case EXPR_TYPE_LESS_THAN:		// expr expr
	case EXPR_TYPE_LESS_EQUAL:		// expr expr
	case EXPR_TYPE_GREATER_THAN:		// expr expr
	case EXPR_TYPE_GREATER_EQUAL:	// expr expr
	case EXPR_TYPE_EQUAL:			// expr expr
	case EXPR_TYPE_NOT_EQUAL:		// expr expr
	case EXPR_TYPE_BIT_AND:			// expr expr
	case EXPR_TYPE_BIT_XOR:			// expr expr
	case EXPR_TYPE_BIT_OR:			// expr expr
	case EXPR_TYPE_AND:				// expr expr
	case EXPR_TYPE_OR:				// expr expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();
		m_return_type = g_type_def_int;
		break;
	}
	case EXPR_TYPE_TERNARY:			// expr expr expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_bFlow = pExpr->isFlow();
		pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth();
		m_bFlow |= pExpr->isFlow();
		pExpr = new CExpr(this, exprGetThirdNode(pRoot));
		addChild(pExpr);
		m_bFlow |= pExpr->isFlow();
		break;
	}
	case EXPR_TYPE_TYPE_CAST:		// extended_or_func_type expr
	{
		CExpr* pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_pSourceTreeNode = dupSourceTreeNode(exprGetFirstNode(pRoot));
		m_pTypeDef = getTypeDefByExtendedOrFuncTypeNode(m_pSourceTreeNode);
		m_return_type = m_pTypeDef;
		m_return_depth = m_return_type->getDepth();
		m_bFlow = pExpr->isFlow();
		break;
	}
	case EXPR_TYPE_BUILTIN_TYPE_FUNC:	   // func_type, type
	{
		m_value = exprBuiltinFuncGetName(pRoot);
		m_pTypeDef = getTypeDefByTypeNode(exprGetSecondNode(pRoot));
		m_return_type = g_type_def_bool;
		m_return_depth = 0;
		break;
	}
	case EXPR_TYPE_IS_BASE_OF:
	{
		m_pTypeDef = getTypeDefByTypeNode(exprGetFirstNode(pRoot));
		m_pTypeDef2 = getTypeDefByTypeNode(exprGetSecondNode(pRoot));
		m_return_type = g_type_def_bool;
		m_return_depth = 0;
		break;
	}
	case EXPR_TYPE_CONST_CAST:			// extended_type_or_func expr
	case EXPR_TYPE_STATIC_CAST:
	case EXPR_TYPE_DYNAMIC_CAST:
	case EXPR_TYPE_REINTERPRET_CAST:
	{
		CExpr* pExpr = new CExpr(this, exprGetSecondNode(pRoot));
		addChild(pExpr);
		m_pTypeDef = getTypeDefByExtendedOrFuncTypeNode(exprGetFirstNode(pRoot));
		m_return_type = m_pTypeDef;
		m_return_depth = m_return_type->getDepth();
		m_bFlow = pExpr->isFlow();
		break;
	}
	case EXPR_TYPE_EXTENSION:	   // expr
	{
		CExpr* pExpr = new CExpr(this, exprGetFirstNode(pRoot));
		addChild(pExpr);
		m_return_type = pExpr->getReturnType();
		m_return_depth = pExpr->getReturnDepth();
		m_bFlow = pExpr->isFlow();
		break;
	}
	default:
		MY_ASSERT(false);
	}

	//TRACE("Analyzing %s, type=%d, resolved to %s\n", displaySourceTreeExpr(pRoot).c_str(), m_expr_type, m_return_type->toString().c_str());
}

std::string CExpr::toString(int depth) // depth is not used
{
	std::string ret_s;

	switch (m_expr_type)
	{
	case EXPR_TYPE_REF_ELEMENT:	// expr ['~'] token ['(' expr2 ')']
	case EXPR_TYPE_PTR_ELEMENT:
	{
		ret_s += ((CExpr*)getChildAt(0))->toString();
		ret_s += (m_expr_type == EXPR_TYPE_REF_ELEMENT ? "." : "->");
		ret_s += m_value;
		if (m_bFlag)
		{
			ret_s += "(";
			for (int i = 1; i < getChildrenCount(); i++)
			{
				if (i > 1)
					ret_s += ", ";
				ret_s += ((CExpr*)getChildAt(i))->toString();
			}
			ret_s += ")";
		}
		break;
	}
	case EXPR_TYPE_FUNC_CALL:		// expr *[expr, ',']
	{
		ret_s = ((CExpr*)getChildAt(0))->toString() + "(";
		for (int i = 1; i < getChildrenCount(); i++)
		{
			if (i > 1)
				ret_s += ", ";
			ret_s += ((CExpr*)getChildAt(i))->toString();
		}
		ret_s += ")";
		break;
	}
	case EXPR_TYPE_OPERATOR_CALL:
	{
		ret_s = m_token_with_namespace.toString() + "(";
		if (getChildrenCount() > 0)
		{
			MY_ASSERT(getChildrenCount() == 1);
			ret_s += ((CExpr*)getChildAt(0))->toString();
		}
		ret_s += ")";
		break;
	}
	case EXPR_TYPE_ARRAY:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + "[" + ((CExpr*)getChildAt(1))->toString() + "]";
		break;
	case EXPR_TYPE_RIGHT_INC: 		// expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + "++";
		break;
	case EXPR_TYPE_RIGHT_DEC:		// expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + "--";
		break;
	case EXPR_TYPE_LEFT_INC:			// expr
		ret_s = "++" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_LEFT_DEC:			// expr
		ret_s = "--" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_POSITIVE:			// expr
		ret_s = "+" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_NEGATIVE:			// expr
		ret_s = "-" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_NOT:				// expr
		ret_s = "!" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_BIT_NOT:				// expr
		ret_s = "~" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_TYPE_CAST:		// type expr
		ret_s = "(";
		if (m_pSourceTreeNode)
		{
			bool bExtendedType;
			SourceTreeNode* pChild;
			extendedOrFuncTypeGetInfo(m_pSourceTreeNode, bExtendedType, pChild);
			if (bExtendedType)
				ret_s += displaySourceTreeExtendedType(pChild);
			else
				ret_s += displaySourceTreeFuncType(pChild);
		}
		else
			ret_s += m_pTypeDef->toString();
		ret_s += ")" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_INDIRECTION:		// expr
		ret_s = "*" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_ADDRESS_OF:		// expr
		ret_s = "&" + ((CExpr*)getChildAt(0))->toString();
		break;
	case EXPR_TYPE_SIZEOF:			// (extended_type_var | expr)
		ret_s = "sizeof(";
		if (getChildrenCount() > 0)
			ret_s += ((CExpr*)getChildAt(0))->toString();
		else
			ret_s += m_pTypeDef->toString();
		ret_s += ")";
		break;
	case EXPR_TYPE_NEW_C:		// scope extended_type_var
	{
		TokenWithNamespace twn;
		twn.copyFrom(m_token_with_namespace);
		twn.addScope("new");
		ret_s += twn.toString() + " " + m_pTypeDef->toString();
		break;
	}
	case EXPR_TYPE_NEW_OBJECT:	   // scope user_def_type expr2
	{
		TokenWithNamespace twn;
		twn.copyFrom(m_token_with_namespace);
		twn.addScope("new");
		ret_s = twn.toString() + " " + m_pTypeDef->toString() + "(";
		for (int i = 0; i < getChildrenCount(); i++)
		{
			if (i > 0)
				ret_s += ", ";
			ret_s += ((CExpr*)getChildAt(i))->toString();
		}
		ret_s += ")";
		break;
	}
	case EXPR_TYPE_NEW_ADV:	 // scope expr user_def_type expr2
	{
		TokenWithNamespace twn;
		twn.copyFrom(m_token_with_namespace);
		twn.addScope("new");
		ret_s = twn.toString() + "(" + ((CExpr*)getChildAt(0))->toString() + ")";
		if (m_pTypeDef)
		{
			ret_s += m_pTypeDef->toString();
			if (m_bFlag)
			{
				ret_s += "(";
				for (int i = 1; i < getChildrenCount(); i++)
				{
					if (i > 1)
						ret_s += ", ";
					ret_s += ((CExpr*)getChildAt(i))->toString();
				}
				ret_s += ")";
			}
		}
		break;
	}
	case EXPR_TYPE_DELETE:			// scope ?[] expr
	{
		TokenWithNamespace twn;
		twn.copyFrom(m_token_with_namespace);
		twn.addScope("delete");
		ret_s = twn.toString();
		if (m_bFlag)
			ret_s += "[]";
		ret_s += " " + ((CExpr*)getChildAt(0))->toString();
		break;
	}
	case EXPR_TYPE_MULTIPLE:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " * " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_DIVIDE:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " / " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_REMAINDER:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " % " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_ADD:				// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " + " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_SUBTRACT:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " - " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_LEFT_SHIFT:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " << " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_RIGHT_SHIFT:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " >> " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_LESS_THAN:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " < " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_LESS_EQUAL:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " <= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_GREATER_THAN:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " > " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_GREATER_EQUAL:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " >= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_EQUAL:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " == " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_NOT_EQUAL:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " != " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_BIT_AND:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " & " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_BIT_XOR:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " ^ " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_BIT_OR:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " | " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_AND:				// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " && " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_OR:				// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " || " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_TERNARY:			// expr expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " ? " + ((CExpr*)getChildAt(1))->toString() + " : " + ((CExpr*)getChildAt(2))->toString();
		break;
	case EXPR_TYPE_ASSIGN:			// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " = " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_ADD_ASSIGN:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " += " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_SUBTRACT_ASSIGN:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " -= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_MULTIPLE_ASSIGN:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " *= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_DIVIDE_ASSIGN:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " /= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_REMAINDER_ASSIGN:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " %= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_LEFT_SHIFT_ASSIGN:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " <<= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_RIGHT_SHIFT_ASSIGN:	// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " >>= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_BIT_AND_ASSIGN:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " &= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_BIT_XOR_ASSIGN:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " ^= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_BIT_OR_ASSIGN:		// expr expr
		ret_s = ((CExpr*)getChildAt(0))->toString() + " |= " + ((CExpr*)getChildAt(1))->toString();
		break;
	case EXPR_TYPE_THROW:				// expr
	{
		MY_ASSERT(getChildrenCount() == 1);
		CExpr* pExpr2 = (CExpr*)getChildAt(0);
		ret_s = "throw";
		if (pExpr2)
			ret_s += " " + pExpr2->toString();
		break;
	}
	//case EXPR_TYPE_COMMA:				// expr expr
	//	ret_s = ((CExpr*)getChildAt(0))->toString() + ", " + ((CExpr*)getChildAt(1))->toString();
	//	break;
	case EXPR_TYPE_CONST_VALUE:			// const_value
		ret_s = m_value;
		break;
	case EXPR_TYPE_TOKEN_WITH_NAMESPACE:  // token
		ret_s = m_token_with_namespace.toString();
		break;
	case EXPR_TYPE_TYPE_CONSTRUCT:	 // type expr2
	{
		ret_s = m_pTypeDef->toString() + "(";
		for (int i = 0; i < getChildrenCount(); i++)
		{
			if (i > 0)
				ret_s += ", ";
			ret_s += ((CExpr*)getChildAt(i))->toString();
		}
		ret_s += ")";
		break;
	}
	case EXPR_TYPE_PARENTHESIS:		// expr
		ret_s = "(" + ((CExpr2*)getChildAt(0))->toString() + ")";
		break;
	case EXPR_TYPE_BUILTIN_TYPE_FUNC:	 // type
		ret_s = m_value + "(" + m_pTypeDef->toString() + ")";
		break;
	case EXPR_TYPE_IS_BASE_OF:	 // type
		ret_s = "__is_base_of(" + m_pTypeDef->toString() + ", " + m_pTypeDef2->toString() + ")";
		break;
	case EXPR_TYPE_CONST_CAST:	   // type expr
		ret_s = "const_cast<" + m_pTypeDef->toString() + ">(" + ((CExpr*)getChildAt(0))->toString() + ")";
		break;
	case EXPR_TYPE_STATIC_CAST:	   // type expr
		ret_s = "static_cast<" + m_pTypeDef->toString() + ">(" + ((CExpr*)getChildAt(0))->toString() + ")";
		break;
	case EXPR_TYPE_DYNAMIC_CAST:	   // type expr
		ret_s = "dynamic_cast<" + m_pTypeDef->toString() + ">(" + ((CExpr*)getChildAt(0))->toString() + ")";
		break;
	case EXPR_TYPE_REINTERPRET_CAST:	   // type expr
		ret_s = "reinterpret_cast<" + m_pTypeDef->toString() + ">(" + ((CExpr*)getChildAt(0))->toString() + ")";
		break;
	case EXPR_TYPE_EXTENSION:	   // expr
		ret_s = "__extension__ " + ((CExpr*)getChildAt(0))->toString();
		break;
	default:
		MY_ASSERT(false);
	}
	return ret_s;
}

bool CExpr::calculateNumValue(float& f)
{
	float f2;

	switch (m_expr_type)
	{
	case EXPR_TYPE_SIZEOF:
		f = 8; // let's wait until it gets into trouble
		return true;

	case EXPR_TYPE_POSITIVE:			// expr
		return ((CExpr*)getChildAt(0))->calculateNumValue(f);

	case EXPR_TYPE_NEGATIVE:			// expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f))
			return false;
		f = -f;
		return true;

	case EXPR_TYPE_NOT:				// expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f))
			return false;
		f = !f;
		return true;

	case EXPR_TYPE_TYPE_CAST:		// type expr
	case EXPR_TYPE_STATIC_CAST:	   // type expr
	case EXPR_TYPE_DYNAMIC_CAST:	   // type expr
	case EXPR_TYPE_REINTERPRET_CAST:
		return ((CExpr*)getChildAt(0))->calculateNumValue(f);

	case EXPR_TYPE_LESS_THAN:	   // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f < f2;
		return true;

	case EXPR_TYPE_LESS_EQUAL:	  // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f <= f2;
		return true;

	case EXPR_TYPE_GREATER_THAN:		// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f > f2;
		return true;

	case EXPR_TYPE_GREATER_EQUAL:   // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f >= f2;
		return true;

	case EXPR_TYPE_EQUAL:		   // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f == f2;
		return true;

	case EXPR_TYPE_NOT_EQUAL:	   // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f != f2;
		return true;

	case EXPR_TYPE_MULTIPLE:			// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f * f2;
		return true;

	case EXPR_TYPE_DIVIDE:			// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f / f2;
		return true;

	case EXPR_TYPE_REMAINDER:		// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f % (int)f2;
		return true;

	case EXPR_TYPE_ADD:				// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f + f2;
		return true;

	case EXPR_TYPE_SUBTRACT:			// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = f - f2;
		return true;

	case EXPR_TYPE_LEFT_SHIFT:		// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f << (int)f2;
		return true;

	case EXPR_TYPE_RIGHT_SHIFT:		// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f >> (int)f2;
		return true;

	case EXPR_TYPE_BIT_AND:	  // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f & (int)f2;
		return true;

	case EXPR_TYPE_BIT_XOR:	  // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f ^ (int)f2;
		return true;

	case EXPR_TYPE_BIT_OR:		// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f | (int)f2;
		return true;

	case EXPR_TYPE_AND:		  // expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f && (int)f2;
		return true;

	case EXPR_TYPE_OR:			// expr expr
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f) || !((CExpr*)getChildAt(1))->calculateNumValue(f2))
			return false;
		f = (int)f || (int)f2;
		return true;

	case EXPR_TYPE_CONST_VALUE:			// const_value
		f = atof(m_value.c_str());
		return true;

	case EXPR_TYPE_TOKEN_WITH_NAMESPACE:  // token
	{
		if (m_token_with_namespace.getDepth() == 1 && m_token_with_namespace.getToken(0) == "false")
		{
			f = 0;
			return true;
		}

		if (m_token_with_namespace.getDepth() == 1 && m_token_with_namespace.getToken(0) == "true")
		{
			f = 1;
			return true;
		}

		SymbolDefObject* pSymbolObj = findSymbolEx(m_token_with_namespace);
		if (pSymbolObj)
		{
			if (pSymbolObj->type == GO_TYPE_ENUM)
			{
				f = pSymbolObj->nValue;
				return true;
			}
			if (pSymbolObj->type == GO_TYPE_VAR_DEF)
			{
				CVarDef* pVarDef = pSymbolObj->getVarDef();
				if (pVarDef->hasValue())
				{
					f = pVarDef->getValue();
					return true;
				}
				TRACE("CExpr::%s, %s doesn't have a value assigned\n", __FUNCTION__, pVarDef->getDebugPath().c_str());
				return false;
			}
		}

		std::string s = m_token_with_namespace.toString();
		TRACE("CExpr::%s, num calculating doesn't support %s\n", __FUNCTION__, s.c_str());
		return false;
	}
	case EXPR_TYPE_TYPE_CONSTRUCT:
	{
		if (getChildrenCount() == 0)
			return false;

		if (!((CExpr*)getChildAt(getChildrenCount() - 1))->calculateNumValue(f))
			return false;

		TRACE("CExpr::%s, type=%d\n", __FUNCTION__, m_pTypeDef->getType());
		if (m_pTypeDef->getType() == SEMANTIC_TYPE_BASIC)
			return true;
		return false;
	}
	case EXPR_TYPE_PARENTHESIS:		// expr
		return ((CExpr2*)getChildAt(0))->calculateNumValue(f);

	case EXPR_TYPE_TERNARY:		 // expr expr expr
	{
		MY_ASSERT(getChildrenCount() == 3);
		if (!((CExpr*)getChildAt(0))->calculateNumValue(f))
			return false;
		if (f)
			return ((CExpr*)getChildAt(1))->calculateNumValue(f);
		return ((CExpr*)getChildAt(2))->calculateNumValue(f);
	}
	case EXPR_TYPE_BUILTIN_TYPE_FUNC:	 // func_type, type
	{
		if (m_value == "__alignof__")
			f = 8; // TODO:
		else if (m_value == "__is_abstract")
			f = m_pTypeDef->is_abstract();
		else if (m_value == "__is_class")
			f = m_pTypeDef->is_class();
		else if (m_value == "__is_empty")
			f = m_pTypeDef->is_empty();
        else if (m_value == "__is_enum")
            f = m_pTypeDef->is_enum();
		else if (m_value == "__is_pod")
			f = m_pTypeDef->is_pod();
		else if (m_value == "__has_nothrow_assign")
			f = m_pTypeDef->has_nothrow_assign();
		else if (m_value == "__has_nothrow_copy")
			f = m_pTypeDef->has_nothrow_copy();
		else if (m_value == "__has_trivial_assign")
			f = m_pTypeDef->has_trivial_assign();
		else if (m_value == "__has_trivial_copy")
			f = m_pTypeDef->has_trivial_copy();
		else if (m_value == "__has_trivial_destructor")
			f = m_pTypeDef->has_trivial_destructor();
		else
		{
			MY_ASSERT(false);
		}
		return true;
	}
	case EXPR_TYPE_IS_BASE_OF:	 // type, type
	{
		f = 0;
		if ((m_pTypeDef->getType() == SEMANTIC_TYPE_STRUCT || m_pTypeDef->getType() == SEMANTIC_TYPE_CLASS) && m_pTypeDef->getClassDef() && 
			(m_pTypeDef2->getType() == SEMANTIC_TYPE_STRUCT || m_pTypeDef2->getType() == SEMANTIC_TYPE_CLASS) && m_pTypeDef2->getClassDef() && 
			m_pTypeDef->getClassDef()->hasBaseClass(m_pTypeDef2->getClassDef()))
				f = 1;
		return true;
	}
	default:
		TRACE("CExpr::%s, cannot calculate type %d\n", __FUNCTION__, m_expr_type);
		break;
	}
	return false;
}

CExpr* CExpr::copyExpr(CExpr* pExpr)
{
	if (!pExpr)
		return NULL;

	CExpr* pNewExpr = new CExpr(pExpr->getParent());
	*pNewExpr = *pExpr;
	if (pExpr->m_pSourceTreeNode)
		pNewExpr->m_pSourceTreeNode = dupSourceTreeNode(pExpr->m_pSourceTreeNode);

	pNewExpr->removeAllChildren();
	for (int i = 0; i < pExpr->getChildrenCount(); i++)
		pNewExpr->addChild(copyExpr((CExpr*)pExpr->getChildAt(i)));

	return pNewExpr;
}

void CExpr::changeRefVarName(const std::string& old_name, CExpr* pNewExpr)
{
	if (m_expr_type == EXPR_TYPE_TOKEN_WITH_NAMESPACE && m_token_with_namespace.getDepth() == 1 && m_token_with_namespace.getToken(0) == old_name)
	{
		CExpr* pExpr2 = copyExpr(pNewExpr);
		pExpr2->m_pParent = getParent();
		*this = *pExpr2;
		pExpr2->removeAllChildren();
		pExpr2->m_pSourceTreeNode = NULL;
		//m_token_with_namespace.setTokenName(0, new_name);
		return;
	}

	for (int i = 0; i < getChildrenCount(); i++)
		((CExpr*)getChildAt(i))->changeRefVarName(old_name, pNewExpr);
}

CExpr2::CExpr2(CScope* pParent) : CExpr(pParent)
{
}

CExpr2::~CExpr2()
{
}

void CExpr2::addExpr(CExpr* pExpr)
{
	pExpr->setParent(this);
	addChild(pExpr);

	m_bFlow |= pExpr->isFlow();
}

void CExpr2::analyze(const SourceTreeNode* pRoot)
{
	SourceTreeVector v = expr2GetExprs(pRoot);

	for (size_t i = 0; i < v.size(); i++)
	{
		SourceTreeNode* pNode = v[i];

		std::string s = displaySourceTreeExpr(pNode);
		CExpr* pExpr = new CExpr(this, pNode);
		addChild(pExpr);
		setReturnType(pExpr->getReturnType());
		setReturnDepth(pExpr->getReturnDepth());
		setReturnSymbolObj(pExpr->getReturnSymbolObj());

		m_bFlow |= pExpr->isFlow();
	}
}

bool CExpr2::calculateNumValue(float& f)
{
	CExpr* pExpr = (CExpr*)getChildAt(getChildrenCount() - 1);

	return pExpr->calculateNumValue(f);
}

std::string CExpr2::toString(int depth)
{
	std::string ret_s;

	for (int i = 0; i < getChildrenCount(); i++)
	{
		ret_s += ((CExpr*)getChildAt(i))->toString();
		if (i < getChildrenCount() - 1)
			ret_s += ", ";
	}

	return ret_s;
}

void CStatement::init()
{
	setRealScope(false);
	m_bFlow = false;
	m_bFlag = false;
	m_pExpr = NULL;
	m_cam_type = CAM_TYPE_NONE;
	m_pFuncDeclare = NULL;
	m_pGrammarObj = NULL;
	m_bAppFlag = false;
}

CStatement::CStatement(CScope* pParent) : CScope(pParent)
{
	init();
}

CStatement::CStatement(CScope* pParent, StatementType type) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_BREAK || type == STATEMENT_TYPE_RETURN);

	init();
	m_statement_type = type;
}

CStatement::CStatement(CScope* pParent, StatementType type, CExpr* pExpr) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_EXPR2 || type == STATEMENT_TYPE_RETURN || type == STATEMENT_TYPE_SWITCH);

	init();
	m_statement_type = type;
	CExpr2* pExpr2 = new CExpr2(this);
	pExpr2->addExpr(pExpr);
	addChild(pExpr2);
	pExpr->setParent(this);
	m_bFlow = pExpr->isFlow();
}

CStatement::CStatement(CScope* pParent, StatementType type, CExpr2* pExpr2) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_EXPR2);

	init();
	m_statement_type = type;
	addChild(pExpr2);
	pExpr2->setParent(this);
	m_bFlow = pExpr2->isFlow();
}

CStatement::CStatement(CScope* pParent, DefType defType, ClassAccessModifierType cam_type) : CScope(pParent)
{
	MY_ASSERT(defType == DEF_TYPE_CLASS_CAM);
	init();
	m_statement_type = STATEMENT_TYPE_DEF;
	m_def_type = defType;
	m_cam_type = cam_type;
}

CStatement::CStatement(CScope* pParent, DefType defType, CSUType csu_type, TypeDefPointer pTypeDef) : CScope(pParent)
{
	MY_ASSERT(defType == DEF_TYPE_CLASS_FRIEND);
	init();
	m_statement_type = STATEMENT_TYPE_DEF;
	m_def_type = defType;
	m_csu_type = csu_type;
	m_pTypeDef = pTypeDef;
}

CStatement::CStatement(CScope* pParent, StatementType type, TypeDefPointer pTypeDef) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_DEF);

	init();
	m_statement_type = type;
	m_def_type = DEF_TYPE_VAR_DEF;

	m_pTypeDef = pTypeDef;
}

CStatement::CStatement(CScope* pParent, StatementType type, CVarDef* pVarDef, SourceTreeNode* pTypeNode) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_DEF);

	init();
	m_statement_type = type;
	m_def_type = DEF_TYPE_VAR_DEF;

	MY_ASSERT(pVarDef);
	m_pParent->getRealScope()->addVarDef(pVarDef);
	m_var_list.push_back(pVarDef);
	pVarDef->setParent(this);
	m_bFlow = pVarDef->isFlow();
	m_pTypeDef = pVarDef->getType()->getBaseType();
}

CStatement::CStatement(CScope* pParent, StatementType type, CExpr* pExpr, CStatement* pStatement) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_IF || type == STATEMENT_TYPE_WHILE);

	init();
	m_statement_type = type;

	pExpr->setParent(this);
	addChild(pExpr);
	m_bFlow |= pExpr->isFlow();

	pStatement->setParent(this);
	addChild(pStatement);
	m_bFlow |= pStatement->isFlow();

	if (type == STATEMENT_TYPE_IF)
		addChild(NULL);
	/*addChild(pElseStatement);
	if (pElseStatement)
	{
		pElseStatement->setParent(this);
		m_bFlow |= pElseStatement->isFlow();
	}*/

	//m_int_vector.push_back(0);
}

CStatement::CStatement(CScope* pParent, StatementType type, const ScopeVector& vec) : CScope(pParent)
{
	MY_ASSERT(type == STATEMENT_TYPE_COMPOUND);

	init();
	setRealScope(true);
	m_statement_type = type;

	for (size_t i = 0; i < vec.size(); i++)
	{
		CScope* pObj = vec[i];

		MY_ASSERT(pObj->getGoType() == GO_TYPE_STATEMENT);
		pObj->setParent(this);
		addChild(pObj);
		m_bFlow |= ((CStatement*)pObj)->isFlow();
	}
}

CStatement::~CStatement()
{
	for (size_t i = 0; i < m_declVarList.size(); i++)
	{
		SourceTreeNode* pDeclVarNode = m_declVarList[i];

		deleteSourceTreeNode(pDeclVarNode);
	}
}

std::string CStatement::getDebugName()
{
	return getName() + ":" + ltoa(m_statement_type);
}

/*void CStatement::addElseIf(CExpr* pExpr, CStatement* pStatement)
{
	MY_ASSERT(m_statement_type == STATEMENT_TYPE_IF);

	pExpr->setParent(this);
	pStatement->setParent(this);

	int n = m_int_vector[0];
	MY_ASSERT(getChildrenCount() == 2 + n * 2);
	addChild(pExpr);
	m_bFlow |= pExpr->isFlow();
	addChild(pStatement);
	m_bFlow |= pStatement->isFlow();
	m_int_vector[0] = n + 1;
}*/

void CStatement::setElseStatement(CStatement* pElseStatement)
{
	MY_ASSERT(m_statement_type == STATEMENT_TYPE_IF);

	m_children[2] = pElseStatement;
	if (pElseStatement)
	{
		pElseStatement->setParent(this);
		m_bFlow |= pElseStatement->isFlow();
	}
}

void CStatement::addSwitchCase(CExpr* pExpr, const ScopeVector& statement_v)
{
	MY_ASSERT(m_statement_type == STATEMENT_TYPE_SWITCH);
	MY_ASSERT(!m_bFlag);

	pExpr->setParent(this);
	m_bFlow |= pExpr->isFlow();
	addChild(pExpr);

	for (size_t i = 0; i < statement_v.size(); i++)
	{
		CScope* pScope = statement_v[i];

		MY_ASSERT(pScope->getGoType() == GO_TYPE_STATEMENT);
		pScope->setParent(this);
		m_bFlow |= ((CStatement*)pScope)->isFlow();
		addChild(pScope);
	}

	m_int_vector.push_back(1 + statement_v.size());
}

void CStatement::addSwitchDefault(const ScopeVector& statement_v)
{
	MY_ASSERT(m_statement_type == STATEMENT_TYPE_SWITCH);
	MY_ASSERT(!m_bFlag);

	for (size_t i = 0; i < statement_v.size(); i++)
	{
		CScope* pScope = statement_v[i];

		MY_ASSERT(pScope->getGoType() == GO_TYPE_STATEMENT);
		pScope->setParent(this);
		m_bFlow |= ((CStatement*)pScope)->isFlow();
		addChild(pScope);
	}
}

// this handles simple var defs that may exists in for()
void CStatement::analyzeDeclVar(SourceTreeNode* pChildNode, SourceTreeVector& var_v)
{
	m_pTypeDef = getTypeDefByTypeNode(pChildNode);

	for (unsigned i = 0; i < var_v.size(); i++)
	{
		SourceTreeNode* pChild;
		CVarDef* pVarDef;
		if (declCObjVarGetInfo(var_v[i], pChild))
		{
			bool bRestrict;
			SourceTreeNode* pDeclVar, *pInitExpr;
			declCVarGetInfo(pChild, bRestrict, pDeclVar, pInitExpr);
			MY_ASSERT(!bRestrict);
			TokenWithNamespace twn = declVarGetName(pDeclVar);
			CExpr* pExpr = (pInitExpr ? new CExpr(this, pInitExpr) : NULL);
			if (pExpr)
				m_bFlow |= pExpr->isFlow();
			pVarDef = new CVarDef(this, twn, m_pTypeDef, dupSourceTreeNode(pDeclVar), pExpr);
			SourceTreeVector decl_expr_v = declVarGetExprs(pDeclVar);
			for (size_t i = 0; i < decl_expr_v.size(); i++)
			{
				SourceTreeNode* pNode = decl_expr_v[i];
				pVarDef->addExprInDeclVar(new CExpr(this, pNode));
			}
		}
		else
		{
			std::string name;
			SourceTreeVector exprNodeList;
			declObjVarGetInfo(pChild, name, exprNodeList);
			ExprVector exprList;
			for (size_t i = 0; i < exprNodeList.size(); i++)
			{
				SourceTreeNode* pNode = exprNodeList[i];
				exprList.push_back(new CExpr(this, pNode));
			}
			pVarDef = new CVarDef(this, name, m_pTypeDef, exprList);
		}
		pVarDef->setDefLocation(g_cur_file_name, g_cur_line_no);
		getRealScope()->addVarDef(pVarDef);
		m_var_list.push_back(pVarDef);
	}
}

void CStatement::analyze(CGrammarAnalyzer* pGrammarAnalyzer, const SourceTreeNode* pRoot)
{
	SourceTreeNode* pExprNode, *pExprNode2, *pStatementNode;
	CExpr* pExpr;
	CStatement* pStatement;
	SourceTreeVector vec;
	int i, n;

	m_statement_type = statementGetType(pRoot);
	switch (m_statement_type)
	{
	case STATEMENT_TYPE_EXPR2:
	{
		//std::string s = displaySourceTreeExpr(pRoot);
		//TRACE("____%s\n", s.c_str());
		CExpr2* pExpr2 = new CExpr2(this);
		pExpr2->analyze(statementExpr2GetNode(pRoot));
		m_bFlow = pExpr2->isFlow();
		addChild(pExpr2);
		break;
	}
	case STATEMENT_TYPE_DEF:
		analyzeDef(pGrammarAnalyzer, statementDefGetNode(pRoot));
		break;

	case STATEMENT_TYPE_IF:
	{
		SourceTreeNode* pExprNode, *pElseStatement = NULL;
		statementIfGetExprStatement(pRoot, pExprNode, pStatementNode);

		CExpr* pExpr2 = new CExpr(this);
		pExpr2->analyze(pExprNode);
		m_bFlow |= pExpr2->isFlow();
		addChild(pExpr2);

		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		m_bFlow |= pStatement->isFlow();
		addChild(pStatement);

		std::string next_token = pGrammarAnalyzer->nextToken();
		if (next_token == "else")
		{
			pElseStatement = pGrammarAnalyzer->getBlock(NULL, "else_statement", getRealScope());
			MY_ASSERT(pElseStatement);

			m_bFlag = true;
			pStatement = new CStatement(this);
			pStatement->analyze(pGrammarAnalyzer, pElseStatement->pChild);
			m_bFlow |= pStatement->isFlow();
			addChild(pStatement);
		}
		else
			addChild(NULL);

		break;
	}
	case STATEMENT_TYPE_WHILE:
	{
		//SourceTreeNode* pExprNode;
		statementWhileGetExprStatement(pRoot, pExprNode, pStatementNode);

		CExpr* pExpr2 = new CExpr(this);
		pExpr2->analyze(pExprNode);
		m_bFlow |= pExpr2->isFlow();
		addChild(pExpr2);

		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		m_bFlow |= pStatement->isFlow();
		addChild(pStatement);
		break;
	}
	case STATEMENT_TYPE_DO:
		statementDoGetExprStatement(pRoot, pStatementNode, pExprNode);
		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		m_bFlow |= pStatement->isFlow();
		addChild(pStatement);
		pExpr = new CExpr(this, pExprNode);
		m_bFlow |= pExpr->isFlow();
		addChild(pExpr);
		break;

	case STATEMENT_TYPE_FOR:
	{
		SourceTreeNode* pExprNode2, *pExprNode3;
		statementForGetExprStatement(pRoot, pExprNode, pExprNode2, pExprNode3, pStatementNode);
		if (pExprNode == NULL)
			addChild(NULL);
		else
		{
			bool bIsExpr;
			SourceTreeNode* pChildNode;
			SourceTreeVector var_v;
			exprOrDeclVarGetInfo(pExprNode, bIsExpr, pChildNode, var_v);
			if (bIsExpr)
			{
				CExpr2* pExpr2 = new CExpr2(this);
				pExpr2->analyze(pChildNode);
				addChild(pExpr2);
				m_bFlow |= pExpr2->isFlow();
			}
			else
			{
				addChild(NULL);
				analyzeDeclVar(pChildNode, var_v);
			}
		}
		if (pExprNode2)
		{
			pExpr = new CExpr(this, pExprNode2);
			m_bFlow |= pExpr->isFlow();
			addChild(pExpr);
		}
		else
			addChild(NULL);

		if (pExprNode3)
		{
			CExpr2* pExpr2 = new CExpr2(this);
			pExpr2->analyze(pExprNode3);
			m_bFlow |= pExpr2->isFlow();
			addChild(pExpr2);
		}
		else
			addChild(NULL);

		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		m_bFlow |= pStatement->isFlow();
		addChild(pStatement);
		break;
	}
	case STATEMENT_TYPE_SWITCH:
		pExprNode = statementSwitchGetVar(pRoot);
		pExpr = new CExpr(this, pExprNode);
		m_bFlow |= pExpr->isFlow();
		addChild(pExpr);
		n = statementSwitchGetNumOfCases(pRoot);
		for (i = 0; i < n; i++)
		{
			vec = statementSwitchGetCaseByIndex(pRoot, i, pExprNode);
			pExpr = new CExpr(this, pExprNode);
			m_bFlow |= pExpr->isFlow();
			addChild(pExpr);
			for (size_t j = 0; j < vec.size(); j++)
			{
				SourceTreeNode* pStatementNode = vec[j];

				pStatement = new CStatement(this);
				pStatement->analyze(pGrammarAnalyzer, pStatementNode);
				m_bFlow |= pStatement->isFlow();
				addChild(pStatement);
			}
			m_int_vector.push_back(vec.size() + 1);
		}
		vec = statementSwitchGetDefult(pRoot);
		for (size_t j = 0; j < vec.size(); j++)
		{
			SourceTreeNode* pStatementNode = vec[j];

			m_bFlag = true;
			pStatement = new CStatement(this);
			pStatement->analyze(pGrammarAnalyzer, pStatementNode);
			m_bFlow |= pStatement->isFlow();
			addChild(pStatement);
		}
		break;

	case STATEMENT_TYPE_TRY:
		pStatementNode = statementTryGetStatement(pRoot);
		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		if (pStatement->isFlow())
			throw("cannot have flow call within a try block");
		addChild(pStatement);

		for (i = 0; ; i++)
		{
			std::string next_token = pGrammarAnalyzer->nextToken();
			if (next_token != "catch")
				break;

			SourceTreeNode* pCatchStatement = pGrammarAnalyzer->getBlock(NULL, "catch_statement", getRealScope());
			MY_ASSERT(pCatchStatement);

			SourceTreeNode* pFuncParamsNode;
			catchStatementGetInfo(pCatchStatement, pFuncParamsNode, pStatementNode);
			SourceTreeVector params_v = funcParamsGetList(pFuncParamsNode);
			if (!((params_v.size() == 0 && funcParamsHasVArgs(pFuncParamsNode)) || (params_v.size() == 1 && !funcParamsHasVArgs(pFuncParamsNode))))
				throw("catch can only have 1 as function parameter");
			if (params_v.size() == 1)
			{
				FuncParamType param_type;
				StringVector mod_strings;
				SourceTreeNode* pTypeNode, *pDeclVarNode;
				void *pInitExprBlock;
				funcParamGetInfo(params_v[0], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);

				SourceTreeNode* pInitExprNode = NULL;
				if (pInitExprBlock)
				{
					CGrammarAnalyzer ga;
					ga.initWithBlocks(getRealScope(), pInitExprBlock);
					pInitExprNode = ga.getBlock();
					MY_ASSERT(ga.isEmpty());
				}

				FuncParamItem item;
				item.param_type = param_type;
				item.mod_strings = mod_strings;
				item.pTypeNode = dupSourceTreeNode(pTypeNode);
				item.pDeclVarNode = dupSourceTreeNode(pDeclVarNode);
				item.pInitExprNode = pInitExprNode;
				MY_ASSERT(item.pInitExprNode == NULL);
				m_func_param_vector.push_back(item);
			}
			else
			{
				MY_ASSERT(!m_bFlag);
				//MY_ASSERT(i == n - 1);
				m_bFlag = true;
			}
			pStatement = new CStatement(this);
			pStatement->analyze(pGrammarAnalyzer, pStatementNode);
			if (pStatement->isFlow())
				throw("cannot have flow call within a catch block");
			addChild(pStatement);
		}
		MY_ASSERT(i > 0);
		MY_ASSERT(m_int_vector.empty());
		m_int_vector.push_back(i);
		break;

	case STATEMENT_TYPE_RETURN:
		pExprNode = statementReturnGetExpr(pRoot);
		if (pExprNode)
		{
			pExpr = new CExpr(this, pExprNode);
			m_bFlow |= pExpr->isFlow();
			addChild(pExpr);
		}
		break;

	case STATEMENT_TYPE_BREAK:
	case STATEMENT_TYPE_CONTINUE:
		break;

	case STATEMENT_TYPE_COMPOUND:
	{
		setRealScope(true);
		void* bracket_block = statementCompoundGetBracketBlock(pRoot);

		CGrammarAnalyzer ga;
		ga.initWithBlocks(getRealScope(), bracket_block);
		while (SourceTreeNode* pNode = ga.getBlock())
		{
			g_cur_file_name = pNode->file_name;
			g_cur_line_no = pNode->line_no;
			TRACE("CStatement::%s, compound block=%s, ANALYZING LINE %s:%d\n", __FUNCTION__, getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
			CStatement* pStatement2 = new CStatement(this);
			//std::string s = displaySourceTreeStatement(pNode->pChild, 0);
			pStatement2->analyze(&ga, pNode);
			m_bFlow |= pStatement2->isFlow();
			addChild(pStatement2);
			deleteSourceTreeNode(pNode);
		}
		MY_ASSERT(ga.isEmpty());
		break;
	}
	/*case STATEMENT_TYPE_FLOW_SIGNAL:
		statementFlowSignalGetExprs(pRoot, pExprNode, pExprNode2);
		pExpr = new CExpr(this, pExprNode);
		m_bFlow |= pExpr->isFlow();
		addChild(pExpr);
		pExpr = new CExpr(this, pExprNode2);
		m_bFlow |= pExpr->isFlow();
		addChild(pExpr);
		break;
	*/
	case STATEMENT_TYPE_FLOW_WAIT:
		statementFlowWaitGetExprs(pRoot, pExprNode, pExprNode2);
		pExpr = new CExpr(this, pExprNode);
		addChild(pExpr);
		pExpr = new CExpr(this, pExprNode2);
		addChild(pExpr);
		//MY_ASSERT(false);
		m_bFlow = true;
		break;

	case STATEMENT_TYPE_FLOW_FORK:
		pStatementNode = statementFlowForkGetStatement(pRoot);
		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		addChild(pStatement);
		m_bFlow = true;
		break;

	case STATEMENT_TYPE_FLOW_NEW:
		statementFlowNewGetInfo(pRoot, pExprNode, pStatementNode);
		if (pExprNode)
		{
			pExpr = new CExpr(this, pExprNode);
			addChild(pExpr);
		}
		else
			addChild(NULL);
		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, pStatementNode);
		addChild(pStatement);
		m_bFlow = true;
		break;

	case STATEMENT_TYPE_FLOW_TRY:
	{
		SourceTreeNode* pExprNode2;
		pStatement = new CStatement(this);
		pStatement->analyze(pGrammarAnalyzer, statementFlowTryGetTryStatement(pRoot));
		addChild(pStatement);

		for (i = 0; ; i++)
		{
			std::string next_token = pGrammarAnalyzer->nextToken();
			if (next_token != "flow_catch")
				break;

			SourceTreeNode* pCatchStatement = pGrammarAnalyzer->getBlock(NULL, "flow_catch_statement", getRealScope());
			MY_ASSERT(pCatchStatement);

			pStatementNode = flowCatchStatementGetInfo(pCatchStatement, pExprNode, pExprNode2);
			addChild(new CExpr(this, pExprNode));
			addChild(new CExpr(this, pExprNode2));
			pStatement = new CStatement(this);
			pStatement->analyze(pGrammarAnalyzer, pStatementNode);
			addChild(pStatement);
		}
		//MY_ASSERT(false);
		m_int_vector.push_back(i);
		m_bFlow = true;
		break;
	}
	case STATEMENT_TYPE___ASM:
	{
		m_templateTokens = statementAsmGetInfo(pRoot);
		break;
	}
	default:
		MY_ASSERT(false);
	}
}

void CStatement::analyzeDef(CGrammarAnalyzer* pGrammarAnalyzer, const SourceTreeNode* pRoot, bool bTemplate)
{
	m_statement_type = STATEMENT_TYPE_DEF;
	bTemplate = (getParentTemplate() != NULL);

	//TypeDefPointer pTypeDef;
	m_bFlow = false;
	m_def_type = defGetType(pRoot);

	TRACE("CStatement::%s, defType=%d\n", __FUNCTION__, m_def_type);
	switch (m_def_type)
	{
	case DEF_TYPE_EMPTY:
		break;

	case DEF_TYPE___PRAGMA:
		m_asm_string = defPragmaGetInfo(pRoot);
		break;

	case DEF_TYPE_POUND_LINE:
		m_asm_string = defPoundLineGetInfo(pRoot);
		break;

	case DEF_TYPE_PRE_DECL:
	{
		//m_pSourceNode = dupSourceTreeNode(pRoot);
		CSUType csu_type;
		std::string name = defPreDeclGetInfo(pRoot, m_mod_strings, csu_type);
		SemanticDataType sType = getSemanticTypeFromCSUType(csu_type);

		// we comment out the block below because when the real define comes within the same template, it search for the struct name and
		// found the name is already defined as a template, then it will assert fail.
		/*if (bTemplate && (sType == SEMANTIC_TYPE_STRUCT || sType == SEMANTIC_TYPE_CLASS))
		{
			SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_SCOPE);
			MY_ASSERT(pSymbolObj == NULL);

			CTemplate* pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_CLASS, name);
			TRACE("\nCREATING root TEMPLATE %s in preDecl\n", pTemplate->getDebugPath().c_str());
			pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			getRealScope()->addTemplate(pTemplate);
			break;
		}*/
		SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_SCOPE);
		if (pSymbolObj != NULL && (pSymbolObj->type != GO_TYPE_TYPEDEF || pSymbolObj->getTypeDef()->getType() != sType))
			throw(name + " is defined as another type in " + pSymbolObj->definedIn());
		if (!pSymbolObj)
		{
			m_pTypeDef = getRealScope()->createClassAsChild(name, sType);
		}
		else
			m_pTypeDef = pSymbolObj->getTypeDef();
		MY_ASSERT(m_pTypeDef);
		break;
	}
	case DEF_TYPE_USING_NAMESPACE:
	{
		//m_pSourceNode = dupSourceTreeNode(pRoot);
		TokenWithNamespace twn = defUsingNamespaceGetInfo(pRoot, m_bFlag);
		SymbolDefObject* pSymbolObj = findSymbolEx(twn);
		if (pSymbolObj == NULL)
		{
			if (bTemplate)
				break;
			throw("cannot recognize " + twn.toString());
		}
		SymbolDefObject* pSymbolObj2 = findSymbol(twn.getLastToken(), FIND_SYMBOL_SCOPE_LOCAL);
		if (pSymbolObj->type == GO_TYPE_NAMESPACE)
		{
			MY_ASSERT(getRealScope()->getGoType() == GO_TYPE_NAMESPACE);
			((CNamespace*)getRealScope())->addUsingNamespace(pSymbolObj->getNamespace());
		}
		else if (pSymbolObj->type == GO_TYPE_TYPEDEF)
		{
			MY_ASSERT(pSymbolObj->pTypeDef);
			if (pSymbolObj2)
			{
				if (pSymbolObj->type != pSymbolObj2->type)
					throw(twn.getLastToken() + " is already defined as a " + getGoTypeName(pSymbolObj2->type) + " in this scope");
				if (pSymbolObj->pTypeDef->toFullString() != pSymbolObj2->pTypeDef->toFullString())
					throw(twn.getLastToken() + " is already defined as " + pSymbolObj2->pTypeDef->toFullString() + " in this scope");
				TRACE("already defined. skip\n");
			}
			else
			{
				SemanticDataType type = pSymbolObj->pTypeDef->getType();
				TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), twn.getLastToken(), pSymbolObj->pTypeDef, 0));
				getRealScope()->addTypeDef(pTypeDef);
			}
		}
		else
			getRealScope()->addUsingObjects(twn.getLastToken(), pSymbolObj);

		switch (pSymbolObj->type)
		{
		case GO_TYPE_NAMESPACE:
			m_pGrammarObj = pSymbolObj->getNamespace();
			m_asm_string = "";
			break;
		case GO_TYPE_FUNC_DECL:
		case GO_TYPE_TEMPLATE:
		case GO_TYPE_TYPEDEF:
		case GO_TYPE_VAR_DEF:
		{
			CGrammarObject* pGO = pSymbolObj->children[0];
			GrammarObjectType type = pGO->getGoType();
			CGrammarObject* pParent = pGO->getParent();
			GrammarObjectType type2 = pParent->getGoType();

			//MY_ASSERT(pSymbolObj->children[0]->getParent()->getGoType() == GO_TYPE_NAMESPACE);
			m_pGrammarObj = pSymbolObj->children[0]->getParent();
			m_asm_string = pSymbolObj->children[0]->getName();
			break;
		}
		default:
			throw(twn.toString() + " is not a namespace or a scope");
		}
		break;
	}
	case DEF_TYPE_TYPEDEF:
	{
		//m_pSourceNode = dupSourceTreeNode(pRoot);
		std::string name;
		m_typedefType = defTypedefGetBasicInfo(pRoot, m_mod_strings);
		switch (m_typedefType)
		{
		case TYPEDEF_TYPE_DATA:
		{
			if (!bTemplate)
			{
				SourceTreeNode* pTypeNode, *pAttribute;
				SourceTreeVector declVarList;
				defTypedefDataGetInfo(pRoot, pTypeNode, declVarList, pAttribute);

				m_pTypeDef = getTypeDefByTypeNode(pTypeNode, (getParentTemplate() != NULL), true);
				std::string display_str = displaySourceTreeType(pTypeNode);
				if (m_pTypeDef && m_pTypeDef->toString() != display_str)
				{
					m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", m_pTypeDef, 0));
					m_pTypeDef->setDisplayString(display_str);
				}
				MY_ASSERT(m_pTypeDef->getType() != SEMANTIC_TYPE_TYPENAME);

				for (size_t i = 0; i < declVarList.size(); i++)
				{
					SourceTreeNode* pDeclVar = declVarList[i];

					m_declVarList.push_back(dupSourceTreeNode(pDeclVar));

					TypeDefPointer pTypeDef2 = getTypeDefByDeclVarNode(m_pTypeDef, pDeclVar);
					pTypeDef2->setParent(getRealScope());

					TokenWithNamespace twn = declVarGetName(pDeclVar);
					MY_ASSERT(twn.getDepth() == 1);
					const std::string name = twn.getLastToken();
					if (name.empty())
						throw("name in typedef cannot be empty");
					if (m_pTypeDef->getName() != name)
					{
						SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL);
						if (pSymbolObj)
						{
							if (pSymbolObj->type == GO_TYPE_TYPEDEF && pSymbolObj->children.size() == 1 && pSymbolObj->getTypeDef()->toFullString() == pTypeDef2->toFullString())
							{
								TRACE("typedef %s duplicate from previous, ignore it\n", name.c_str());
								continue;
							}
							std::string s = "Type " + name + " is defined differently in seperate locations:\n";
							s += "   " + pSymbolObj->getTypeDef()->toFullString() + " in " + pSymbolObj->definedIn() + "\n";
							s += "   " + pTypeDef2->toFullString() + " in " + g_cur_file_name + ":" + ltoa(g_cur_line_no);
							throw(s);
						}
					}

					//printf("\nline=%s\n", displaySourceTreeDefs(pRoot, 0).c_str());
					if (m_pTypeDef->getName() != name || m_pTypeDef->getParent() != getRealScope())
					{
						pTypeDef2->setName(name);
						pTypeDef2->setDefLocation(g_cur_file_name, g_cur_line_no);
						getRealScope()->addTypeDef(pTypeDef2);
					}
				}
			}
			else
			{
				SourceTreeNode* pSuperTypeNode, *pAttribute;
				SourceTreeVector declVarList;
				defTypedefDataGetInfo(pRoot, pSuperTypeNode, declVarList, pAttribute);

				for (size_t i = 0; i < declVarList.size(); i++)
				{
					SourceTreeNode* pDeclVar = declVarList[i];

					TokenWithNamespace twn = declVarGetName(pDeclVar);
					MY_ASSERT(twn.getDepth() == 1);
					const std::string name = twn.getLastToken();
					if (name.empty())
						throw("name in typedef cannot be empty");

					TypeDefPointer pTypeDef = getRealScope()->createClassAsChild(name, SEMANTIC_TYPE_TYPENAME);
					pTypeDef->getBaseType()->getClassDef()->setWithinTemplate();
				}
			}
			//printf("******type define %s: %s---------\n", pTypeDef->getName().c_str(), pTypeDef->toString().c_str());
			break;
		}
		case TYPEDEF_TYPE_SUPER_TYPE:
		{
			if (!bTemplate)
			{
				SourceTreeNode* pSuperTypeNode;
				void* remain_block;
				defTypedefSuperTypeGetInfo(pRoot, pSuperTypeNode, remain_block);

				m_pTypeDef = analyzeSuperType(pSuperTypeNode, (getParentTemplate() != NULL));
				MY_ASSERT(m_pTypeDef->getType() != SEMANTIC_TYPE_TYPENAME);

				CGrammarAnalyzer ga2;
				ga2.initWithBlocks(getRealScope(), remain_block);
				SourceTreeNode* pTailNode = ga2.getBlock(NULL, "def_var_tail", getRealScope());
				if (pTailNode == NULL)
					throw("Cannot recognize type list");

				SourceTreeVector declCObjVarList;
				StringVector mod_strings;
				defVarTailGetInfo(pTailNode, declCObjVarList, mod_strings);

				for (size_t i = 0; i < declCObjVarList.size(); i++)
				{
					SourceTreeNode* pDeclCObjVar = declCObjVarList[i];

					SourceTreeNode* pDeclCVar, *pDeclVar, *pInitValue;
					if (!declCObjVarGetInfo(pDeclCObjVar, pDeclCVar))
						throw("a object var is not allowed here");

					bool bRestrict;
					declCVarGetInfo(pDeclCVar, bRestrict, pDeclVar, pInitValue);
					MY_ASSERT(!pInitValue);
					m_declVarList.push_back(dupSourceTreeNode(pDeclVar));

					TypeDefPointer pTypeDef2 = getTypeDefByDeclVarNode(m_pTypeDef, pDeclVar);
					pTypeDef2->setParent(getRealScope());

					TokenWithNamespace twn = declVarGetName(pDeclVar);
					MY_ASSERT(twn.getDepth() == 1);
					const std::string name = twn.getLastToken();
					if (name.empty())
						throw("name in typedef cannot be empty");
					if (m_pTypeDef->getName() != name)
					{
						SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL);
						if (pSymbolObj)
						{
							if (pSymbolObj->type == GO_TYPE_TYPEDEF && pSymbolObj->children.size() == 1 && pSymbolObj->getTypeDef()->toFullString() == pTypeDef2->toFullString())
							{
								TRACE("typedef %s duplicate from previous, ignore it\n", name.c_str());
								continue;
							}
							std::string s = "Type " + name + " is defined differently in seperate locations:\n";
							s += "   " + pSymbolObj->getTypeDef()->toFullString() + " in " + pSymbolObj->definedIn() + "\n";
							s += "   " + pTypeDef2->toFullString() + " in " + g_cur_file_name + ":" + ltoa(g_cur_line_no);
							throw(s);
						}
					}

					//printf("\nline=%s\n", displaySourceTreeDefs(pRoot, 0).c_str());
					if (m_pTypeDef->getName() != name || m_pTypeDef->getParent() != getRealScope())
					{
						pTypeDef2->setName(name);
						pTypeDef2->setDefLocation(g_cur_file_name, g_cur_line_no);
						getRealScope()->addTypeDef(pTypeDef2);
					}
				}
			}
			else
			{
				SourceTreeNode* pTailNode = pGrammarAnalyzer->getBlock(NULL, "def_var_tail", getRealScope());
				if (pTailNode == NULL)
					throw("Cannot recognize type list");

				SourceTreeVector declCObjVarList;
				StringVector mod_strings;
				defVarTailGetInfo(pTailNode, declCObjVarList, mod_strings);

				for (size_t i = 0; i < declCObjVarList.size(); i++)
				{
					SourceTreeNode* pDeclCObjVar = declCObjVarList[i];

					SourceTreeNode* pDeclCVar, *pDeclVar, *pInitValue;
					if (!declCObjVarGetInfo(pDeclCObjVar, pDeclCVar))
						throw("a object var is not allowed here");

					bool bRestrict;
					declCVarGetInfo(pDeclCVar, bRestrict, pDeclVar, pInitValue);
					MY_ASSERT(!pInitValue);
					TokenWithNamespace twn = declVarGetName(pDeclVar);
					MY_ASSERT(twn.getDepth() == 1);
					const std::string name = twn.getLastToken();
					if (name.empty())
						throw("name in typedef cannot be empty");

					TypeDefPointer pTypeDef = getRealScope()->createClassAsChild(name, SEMANTIC_TYPE_TYPENAME);
					pTypeDef->getBaseType()->getClassDef()->setWithinTemplate();
				}
			}
			//printf("******type define %s: %s---------\n", pTypeDef->getName().c_str(), pTypeDef->toString().c_str());
			break;
		}
		case TYPEDEF_TYPE_DATA_MEMBER_PTR:
		{
			SourceTreeNode *pExtendedTypeNode;
			TokenWithNamespace twn;
			std::string name;
			defTypedefDataMemberPtrGetInfo(pRoot, pExtendedTypeNode, twn, name);

			if (twn.empty())
				throw("scope in typedef cannot be empty");
			if (name.empty())
				throw("name in typedef cannot be empty");

			if (!bTemplate)
			{
				TypeDefPointer pExtendedTypeDef = getTypeDefByExtendedTypeNode(pExtendedTypeNode);
				MY_ASSERT(pExtendedTypeDef);

				TypeDefPointer pDataScope;
				SymbolDefObject* pSymbolObj = findSymbolEx(twn);
				if (!pSymbolObj)
					throw("Cannot find " + twn.toString());
				if (pSymbolObj->type == GO_TYPE_TYPEDEF)
				{
					pDataScope = pSymbolObj->getTypeDef();
					MY_ASSERT(pDataScope->getType() == SEMANTIC_TYPE_CLASS);
				}
				else
					throw(twn.toString() + " is not a class type");

				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name, SEMANTIC_TYPE_DATA_MEMBER_POINTER, pExtendedTypeDef, pDataScope));
				m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
				getRealScope()->addTypeDef(m_pTypeDef);
			}
			else
			{
				getRealScope()->createClassAsChild(name, SEMANTIC_TYPE_TYPENAME);
			}
			//printf("******type define %s: %s---------\n", pTypeDef->getName().c_str(), pTypeDef->toString().c_str());
			break;
		}
		case TYPEDEF_TYPE_FUNC:
		{
			std::string name;
			SourceTreeNode *pExtendedReturnType, *pFuncParamsNode;
			StringVector mod_strings;
			bool bHasParenthesis;
			defTypedefFuncGetInfo(pRoot, pExtendedReturnType, bHasParenthesis, mod_strings, name, pFuncParamsNode);
			if (!bTemplate)
			{
				SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL);
				if (pSymbolObj)
					throw("func " + name + " is already defined as a " + ltoa(pSymbolObj->type) + " in " + pSymbolObj->definedIn());
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name, SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(pExtendedReturnType), 0));
				m_pTypeDef->setMod3Strings(mod_strings);
				m_pTypeDef->setDisplayFlag(bHasParenthesis);
				m_pTypeDef->setFuncReturnTypeNode(dupSourceTreeNode(pExtendedReturnType));
				addFuncParamsToFuncType(m_pTypeDef, pFuncParamsNode);
			}
			else
			{
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name));
			}
			m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
			getRealScope()->addTypeDef(m_pTypeDef);
			break;
		}
		case TYPEDEF_TYPE_FUNC_PTR:
		{
			std::string name;
			SourceTreeNode *pExtendedReturnType, *pScope, *pFuncParamsNode;
			StringVector mod_strings, mod2_strings;
			int nDepth;
			funcTypeGetInfo(defTypedefFuncTypeGetInfo(pRoot), pExtendedReturnType, mod_strings, pScope, nDepth, name, pFuncParamsNode, mod2_strings);
			if (name.empty())
				throw("the name in func typedef cannot be empty");
			if (!bTemplate)
			{
				SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL);
				if (pSymbolObj)
					throw("func typedef " + name + " is already defined as a " + ltoa(pSymbolObj->type) + " in " + pSymbolObj->definedIn());
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name, SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(pExtendedReturnType), nDepth));
				m_pTypeDef->setMod3Strings(mod_strings);
				m_pTypeDef->setMod4Strings(mod2_strings);
				m_pTypeDef->setFuncReturnTypeNode(dupSourceTreeNode(pExtendedReturnType));
				addFuncParamsToFuncType(m_pTypeDef, pFuncParamsNode);
				/*if (pOptFuncParamsNode)
				{
					m_pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, m_pTypeDef, 1));
					addFuncParamsToFuncType(m_pTypeDef, pOptFuncParamsNode);
				}*/
				m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
				TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(getRealScope(), name, m_pTypeDef, 0));
				pTypeDef2->setDefLocation(g_cur_file_name, g_cur_line_no);
				getRealScope()->addTypeDef(pTypeDef2);
			}
			else
			{
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name));
				m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
				getRealScope()->addTypeDef(m_pTypeDef);
			}
			break;
		}
		case TYPEDEF_TYPE_TYPEOF:
		{
			bool bType;
			std::string name;
			SourceTreeNode *pExtendedTypeOrExprNode;
			defTypedefTypeOfGetInfo(pRoot, bType, pExtendedTypeOrExprNode, name);

			if (!bTemplate)
			{
				if (bType)
					m_pTypeDef = getTypeDefByExtendedTypeNode(pExtendedTypeOrExprNode);
				else
				{
					m_pExpr = new CExpr(this, pExtendedTypeOrExprNode);
					m_pTypeDef = m_pExpr->getReturnType();
				}
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name, SEMANTIC_TYPE_FUNC, m_pTypeDef, 0));
			}
			else
			{
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), name));
			}
			m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
			getRealScope()->addTypeDef(m_pTypeDef);
			break;
		}
		default:
			MY_ASSERT(false);
		}
		break;
	}
	case DEF_TYPE_VAR_DEF:
	case DEF_TYPE_SUPER_TYPE_VAR_DEF:
	{
		SourceTreeNode* pTypeNode, *pSuperTypeNode, *pDefVarTailNode;

		if (m_def_type == DEF_TYPE_VAR_DEF)
		{
			defVarDefGetInfo(pRoot, m_mod_strings, pTypeNode, pDefVarTailNode);
		}
		else
		{
			void* remain_block;
			defSuperTypeVarDefGetInfo(pRoot, m_mod_strings, pSuperTypeNode, remain_block);

			CGrammarAnalyzer ga2;
			ga2.initWithBlocks(getRealScope(), remain_block);
			pDefVarTailNode = ga2.getBlock(NULL, "def_var_tail", getRealScope());
			if (pDefVarTailNode == NULL)
				throw("Cannot recognize var list");
		}

		SourceTreeVector declCObjVarList;
		StringVector mod_strings;
		defVarTailGetInfo(pDefVarTailNode, declCObjVarList, mod_strings);

		bool bHasNoPointer = false;
		for (size_t i = 0; i < declCObjVarList.size(); i++)
		{
			SourceTreeNode* pDeclCObjVar = declCObjVarList[i];

			SourceTreeNode* pDeclCVar;
			if (!declCObjVarGetInfo(pDeclCObjVar, pDeclCVar))
				bHasNoPointer = true;
			else
			{
				bool bRestrict;
				SourceTreeNode* pDeclVar, *pInitValue;
				declCVarGetInfo(pDeclCVar, bRestrict, pDeclVar, pInitValue);
				if (declVarGetDepth(pDeclVar) == 0 && !declVarIsReference(pDeclVar))
					bHasNoPointer = true;
			}
		}

		if (m_def_type == DEF_TYPE_VAR_DEF)
		{
			m_pTypeDef = getTypeDefByTypeNode(pTypeNode, !bHasNoPointer, true);
			std::string display_str = displaySourceTreeType(pTypeNode);
			if (m_pTypeDef && m_pTypeDef->toString() != display_str)
			{
				m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", m_pTypeDef, 0));
				m_pTypeDef->setDisplayString(display_str);
			}
		}
		else
			m_pTypeDef = analyzeSuperType(pSuperTypeNode, (getParentTemplate() != NULL));

		if (!m_pTypeDef) // if we cannot analyze it, we just fake one
		{
			MY_ASSERT(bTemplate);
			m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), ""));
		}

		TRACE("CStatement::%s, vardef, type found, numVars=%d\n", __FUNCTION__, declCObjVarList.size());
		for (size_t i = 0; i < declCObjVarList.size(); i++)
		{
			SourceTreeNode* pDeclCObjVar = declCObjVarList[i];

			SourceTreeNode* pChild;
			std::string name;
			CVarDef* pVarDef = NULL;
			if (!declCObjVarGetInfo(pDeclCObjVar, pChild))
			{
				SourceTreeVector exprNodeList;
				declObjVarGetInfo(pChild, name, exprNodeList);
				ExprVector exprList;
				for (size_t i = 0; i < exprNodeList.size(); i++)
				{
					SourceTreeNode* pNode = exprNodeList[i];
					exprList.push_back(new CExpr(this, pNode));
				}
				pVarDef = new CVarDef(this, name, m_pTypeDef, exprList);
			}
			else
			{
				bool bRestrict;
				SourceTreeNode* pDeclVar, *pInitValue;
				declCVarGetInfo(pChild, bRestrict, pDeclVar, pInitValue);
				TokenWithNamespace twn = declVarGetName(pDeclVar);
				name = twn.toString();
				//if (name.empty())
				//	continue;

				CExpr* pExpr = NULL;
				if (pInitValue) // don't do this when analyzing a template
				{
					if (getParentTemplate() == NULL)
					{
						pExpr = new CExpr(getRealScope(), pInitValue);
						m_bFlow |= pExpr->isFlow();
					}
					else
						TRACE("var %s has init expr but is under a template\n", toString(0).c_str());
				}
				pVarDef = new CVarDef(this, twn, m_pTypeDef, dupSourceTreeNode(pDeclVar), pExpr);
				float f;
				if (pExpr)
				{
					if (pExpr->calculateNumValue(f))
						pVarDef->setValue(f);
					else
						TRACE("var %s has init expr but cannot calculate\n", pVarDef->toString().c_str());
				}

				SourceTreeVector decl_expr_v = declVarGetExprs(pDeclVar);
				for (size_t i = 0; i < decl_expr_v.size(); i++)
				{
					SourceTreeNode* pNode = decl_expr_v[i];
					pVarDef->addExprInDeclVar(new CExpr(m_pParent->getRealScope(), pNode));
				}
				pVarDef->setRestrict(bRestrict);
			}
			if (isInModifiers(m_mod_strings, MODBIT_EXTERN))
				pVarDef->setExtern();

			bool bToBeAdded = true;
			if (name.empty())
				bToBeAdded = false;
			else
			{
				SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL);
				if (pSymbolObj)
				{
					CVarDef* pVarDef2 = pSymbolObj->getVarDef();
					if (pVarDef2)
					{
						if (pVarDef->isExtern() && pVarDef2->isExtern())
							throw("Var " + name + " is already defined in " + pVarDef2->definedIn());
						if (pVarDef->getType()->toFullString() != pVarDef2->getType()->toFullString())
							throw("Var " + name + " is already defined as " + pVarDef2->getType()->toFullString());
						if (!pVarDef->isExtern())
							pSymbolObj->removeVarDef();
						else
							bToBeAdded = false;
					}
					else
					{
						if (pSymbolObj->children.size() > 1 || pSymbolObj->children[0]->getGoType() != GO_TYPE_TYPEDEF)
							throw("Var " + name + " is already defined as other than var");
					}
				}
			}

			if (bToBeAdded)
			{
				pVarDef->setDefLocation(g_cur_file_name, g_cur_line_no);
				getRealScope()->addVarDef(pVarDef);
			}
			m_var_list.push_back(pVarDef);
		}
		break;
	}
	case DEF_TYPE_FUNC_DECL:
	{
		//m_pSourceNode = dupSourceTreeNode(pRoot);
		//printf("%s\n", displaySourceTreeDefs(pRoot, 1).c_str());
		std::string asm_string;
		SourceTreeNode* pFuncHeaderNode;
		SourceTreeVector attribute_list;
		bool bPureVirtual;
		void* pBaseClassInitBlock, *bracket_block;
		defFuncDeclGetInfo(pRoot, pFuncHeaderNode, asm_string, attribute_list, bPureVirtual, pBaseClassInitBlock, bracket_block);
		MY_ASSERT(pBaseClassInitBlock == NULL);
		MY_ASSERT(bracket_block == NULL);

		GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pFuncHeaderNode);
		//m_declspec_strings = funcHeaderInfo.declspec_strings;
		//m_bThrow = funcHeaderInfo.bThrow;
		FlowType flowType = getFlowTypeByModifierBits(funcHeaderInfo.mod_strings);

		/*if (bTemplate)
		{
			MY_ASSERT(scope.empty());
			m_pTypeDef = TypeDefPointer(new CTypeDef(NULL, name, getTypeDefByExtendedTypeNode(pReturnExtendedType), flowType, 1));
			m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
			getRealScope()->addTypeDef(m_pTypeDef);
			break;
		}*/
		CGrammarAnalyzer ga2;
		ga2.initWithBlocks(getRealScope(), funcHeaderInfo.params_block);
		SourceTreeNode* pFuncParamsNode = ga2.getBlock();
		MY_ASSERT(ga2.isEmpty());

		m_pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(funcHeaderInfo.pReturnExtendedType), 0));
		m_pTypeDef->setModStrings(funcHeaderInfo.mod_strings);
		m_pTypeDef->setMod2Strings(funcHeaderInfo.mod2_strings);
		m_pTypeDef->setMod3Strings(funcHeaderInfo.mod3_strings);
		m_pTypeDef->setMod4Strings(funcHeaderInfo.mod4_strings);
		m_pTypeDef->setFuncReturnTypeNode(dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType));
		addFuncParamsToFuncType(m_pTypeDef, pFuncParamsNode);
		deleteSourceTreeNode(pFuncParamsNode);
		m_pTypeDef->setThrow(funcHeaderInfo.bThrow, dupSourceTreeNode(funcHeaderInfo.pThrowTypeNode));
		m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);
		m_pTypeDef->setPureVirtual(bPureVirtual);

		if (funcHeaderInfo.scope.empty())
		{
			SymbolDefObject* pSymbolObj = findSymbol(funcHeaderInfo.name, FIND_SYMBOL_SCOPE_LOCAL);
			if (pSymbolObj)
			{
				if (pSymbolObj->type != GO_TYPE_FUNC_DECL && pSymbolObj->type != GO_TYPE_TYPEDEF)
					throw("Func " + funcHeaderInfo.name + " already defined other than func");
				//CFuncDeclare* pFuncDeclare = findFuncDeclare(pSymbolObj, m_pTypeDef);
				//if (pFuncDeclare && flowType != pFuncDeclare->getType()->getFuncFlowType())
				//	throw("Func " + name + " is declared twice but with different flow attribute");
			}
			m_pFuncDeclare = new CFuncDeclare(getRealScope(), funcHeaderInfo.name, m_pTypeDef);
			m_pFuncDeclare->setDefLocation(g_cur_file_name, g_cur_line_no);
			getRealScope()->addFuncDeclare(m_pFuncDeclare);

			for (size_t i = 0; i < attribute_list.size(); i++)
			{
				SourceTreeNode* pNode = attribute_list[i];
				m_attribute_list.push_back(dupSourceTreeNode(pNode));
			}
		}
		else
		{
			//if (funcHeaderInfo.nDataMemberPointerDepth == 0)
			//{
				funcHeaderInfo.scope.addScope(funcHeaderInfo.name, false);
				SymbolDefObject* pSymbolObj = findSymbolEx(funcHeaderInfo.scope);
				if (pSymbolObj != NULL && pSymbolObj->type == GO_TYPE_FUNC_DECL)
					m_pFuncDeclare = findFuncDeclare(pSymbolObj, m_pTypeDef);
				if (m_pFuncDeclare == NULL)
				{
					// it's possible to define an extern func decl for a template function
					if (isInModifiers(m_mod_strings, MODBIT_EXTERN))
						break;
					throw("Func " + funcHeaderInfo.scope.toString() + " should have been declared in that namespace");
				}
				if (flowType != m_pFuncDeclare->getType()->getFuncFlowType())
					throw("Func " + funcHeaderInfo.scope.toString() + " is declared twice but with different flow attribute");
		}
		break;
	}
	case DEF_TYPE_FUNC_VAR_DEF:
	{
		if (bTemplate)
			break;
		//m_pSourceNode = dupSourceTreeNode(pRoot);
		SourceTreeNode* pFuncType;
		int array_count;
		defFuncVarDefGetInfo(pRoot, m_mod_strings, pFuncType, array_count);
		m_pTypeDef = getTypeDefByFuncTypeNode(pFuncType);

		TypeDefPointer pTypeDef = m_pTypeDef;
		if (array_count > 0)
			pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, array_count));

		// get func var name;
		SourceTreeNode* pReturnExtendedType, *pScope, *pFuncParamsNode;
		std::string name;
		StringVector mod_strings, mod2_strings;
		int nDepth;
		funcTypeGetInfo(pFuncType, pReturnExtendedType, mod_strings, pScope, nDepth, name, pFuncParamsNode, mod2_strings);

		if (nDepth == 1)
		{
			CVarDef* pVarDef = new CVarDef(this, name, pTypeDef);
			pVarDef->setDefLocation(g_cur_file_name, g_cur_line_no);
			m_pParent->getRealScope()->addVarDef(pVarDef);
			m_var_list.push_back(pVarDef);
		}
		else
		{
			MY_ASSERT(false); // dmp
			/*if (bTemplate)
				break;

			m_def_type = DEF_TYPE_FUNC_VAR_DEF;
			TypeDefPointer pFuncType = m_pTypeDef;
			SymbolDefObject* pSymbolObj = findSymbolEx(funcHeaderInfo.scope);
			if (pSymbolObj == NULL || pSymbolObj->type != GO_TYPE_TYPEDEF || pSymbolObj->getTypeDef()->getClassDef())
				throw("Scope " + funcHeaderInfo.scope.toString() + " should be a class type");

			m_pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", SEMANTIC_TYPE_DATA_MEMBER_POINTER, pFuncType, pSymbolObj->getTypeDef()));
			CVarDef* pVarDef = new CVarDef(this, funcHeaderInfo.name, m_pTypeDef);
			pVarDef->setDefLocation(g_cur_file_name, g_cur_line_no);
			m_pParent->getRealScope()->addVarDef(pVarDef);
			m_var_list.push_back(pVarDef);*/
		}
		break;
	}
	case DEF_TYPE_TEMPLATE:
	{
		std::vector<void*> header_types;
		CSUType csu_type;
		void* remain_block;
		defTemplateGetInfo(pRoot, m_mod_strings, header_types, csu_type, remain_block);

		CTemplate* pTempTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_CLASS, "temp template");
		pTempTemplate->addHeaderTypeDefs(header_types);

		CGrammarAnalyzer ga2;
		ga2.initWithBlocks(getRealScope(), remain_block);
		if (csu_type != CSU_TYPE_NONE)
			ga2.pushTokenFront(trim(displayCSUType(csu_type)));
		SourceTreeNode* pBodyNode = ga2.getBlock(NULL, "template_body", pTempTemplate);
		MY_ASSERT(ga2.getBlock() == NULL);
		MY_ASSERT(pBodyNode);

		TemplateType template_type = templateBodyGetType(pBodyNode);
		switch (template_type)
		{
		case TEMPLATE_TYPE_FUNC:
		{
			SourceTreeNode* pFuncHeaderNode;
			void* pBaseClassInitBlock, *body_data;
			templateBodyFuncGetInfo(pBodyNode, pFuncHeaderNode, pBaseClassInitBlock, body_data);
			m_bFlag = (body_data != NULL);

			GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pFuncHeaderNode);

			SymbolDefObject* pSymbolObj = NULL;
			CScope* pParentScope = getRealScope();
			/*if (funcHeaderInfo.nDataMemberPointerDepth > 0)
			{
				//std::string s = twn.toString();
				//MY_ASSERT(twn.getDepth() == 1 && !twn.hasRootSign());
				funcHeaderInfo.scope.resize(0);
			}*/
			if (!funcHeaderInfo.scope.empty())
			{
				pSymbolObj = pTempTemplate->findSymbolEx(funcHeaderInfo.scope, true);
				MY_ASSERT(pSymbolObj);

				if (!pSymbolObj)
					throw("template " + funcHeaderInfo.scope.toString() + " is not defined");
				if (pSymbolObj->type == GO_TYPE_NAMESPACE)
					pParentScope = pSymbolObj->getNamespace();
				else if (pSymbolObj->type == GO_TYPE_TEMPLATE)
					pParentScope = pSymbolObj->getTemplateAt(0);
				else if (pSymbolObj->type == GO_TYPE_TYPEDEF)
				{
					TypeDefPointer pTypeDef = pSymbolObj->getTypeDef();
					MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());
					pParentScope = pTypeDef->getBaseType()->getClassDef();
				}
				else
					MY_ASSERT(false);

				funcHeaderInfo.scope.addScope(funcHeaderInfo.name, false);

				MY_ASSERT(pParentScope);
				pSymbolObj = pParentScope->findSymbol(funcHeaderInfo.name, FIND_SYMBOL_SCOPE_LOCAL);
				MY_ASSERT(pSymbolObj);

				m_pTemplate = new CTemplate(pParentScope, TEMPLATE_TYPE_FUNC, funcHeaderInfo.name);
				TRACE("\nCREATING func TEMPLATE %s\n", m_pTemplate->getDebugPath().c_str());
				//pParentScope->addTemplate(m_pTemplate);
				m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			}
			else
			{
				funcHeaderInfo.scope.addScope(funcHeaderInfo.name, false);
				pSymbolObj = findSymbolEx(funcHeaderInfo.scope, false);

				MY_ASSERT(!funcHeaderInfo.name.empty());
				m_pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_FUNC, funcHeaderInfo.name);
				TRACE("\nCREATING func TEMPLATE %s\n", m_pTemplate->getDebugPath().c_str());
				//getRealScope()->addTemplate(m_pTemplate);
				m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			}

			m_pTemplate->analyzeFunc(header_types, pBodyNode);

			bool bFoundSame = false;
			if (pSymbolObj)
			{
				if (pSymbolObj->type != GO_TYPE_FUNC_DECL)
					throw(funcHeaderInfo.name + " is already defined other than func decl");

				for (size_t i = 0; i < pSymbolObj->children.size(); i++)
				{
					CGrammarObject* pGrammarObj = pSymbolObj->children[i];

					if (pGrammarObj->getGoType() != GO_TYPE_TEMPLATE)
						continue;

					CTemplate* pTemplate = (CTemplate*)pGrammarObj;
					MY_ASSERT(pTemplate->getTemplateType() == TEMPLATE_TYPE_FUNC);

					if (pTemplate->isSame(m_pTemplate))
					{
						bFoundSame = true;
						if (m_pTemplate->isDefined())
						{
							if (pTemplate->isDefined())
								throw("This template is already defined before");
							pSymbolObj->children[i] = m_pTemplate;
						}
						break;
					}
				}
			}
			if (!bFoundSame)
				pParentScope->addTemplate(m_pTemplate);
			break;
		}
		case TEMPLATE_TYPE_CLASS:
		{
			m_pTemplate = NULL;
			int specializedTypeCount;
			TokenWithNamespace twn;
			void* body_data, *pBaseClassDefsBlock;
			CSUType csu_type;
			templateBodyClassGetInfo(pBodyNode, csu_type, twn, specializedTypeCount, pBaseClassDefsBlock, body_data);
			m_bFlag = (body_data != NULL);

			SymbolDefObject* pSymbolObj;
			pSymbolObj = findSymbolEx(twn, false);
			CTemplate* pTemplate = NULL;
			if (pSymbolObj)
			{
				if (pSymbolObj->type != GO_TYPE_TEMPLATE)
					throw(twn.toString() + " is already defined as a " + getGoTypeName(pSymbolObj->type));
				MY_ASSERT(pSymbolObj->children.size() == 1);
				pTemplate = pSymbolObj->getTemplateAt(0);
				if (pTemplate->getTemplateType() != TEMPLATE_TYPE_CLASS)
					throw(twn.toString() + " is already defined as a class template");
				TRACE("Template %s is previously defined at %s\n", pTemplate->getDebugPath().c_str(), pTemplate->definedIn().c_str());
			}
			else
			{
				TRACE("template is not defined previously\n");
			}

			if (specializedTypeCount == 0)
			{
				// it's a root template
				//MY_ASSERT(twn.getDepth() == 1);
				m_pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_CLASS, twn.getLastToken());
				TRACE("\nCREATING root TEMPLATE %s in statement\n", m_pTemplate->getDebugPath().c_str());
				m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
				m_pTemplate->analyzeBaseClass(header_types, pBodyNode);

				if (!pTemplate)
				{
					pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_CLASS, twn.getLastToken());
					TRACE("\nCREATING root TEMPLATE %s in map\n", pTemplate->getDebugPath().c_str());
					pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
					pTemplate->analyzeBaseClass(header_types, pBodyNode);
					getRealScope()->addTemplate(pTemplate);

					if (body_data)
						pTemplate->analyzeClassBody(pBaseClassDefsBlock, body_data);
				}
				else
				{
					if (pTemplate->m_typeParams.empty())
					{
						pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
						pTemplate->analyzeBaseClass(header_types, pBodyNode);
						if (body_data)
						{
							pTemplate->saveClassBody(pBaseClassDefsBlock, body_data);
							pTemplate->analyzeClassBody(pBaseClassDefsBlock, body_data);
						}
					}
					else
					{
						if (body_data)
						{
							if (pTemplate->isDefined())
								throw("This template is already defined before");

							pTemplate->mergeWithBaseClass(m_pTemplate, true);
							pTemplate->saveClassBody(pBaseClassDefsBlock, body_data);
							pTemplate->analyzeClassBody(pBaseClassDefsBlock, body_data);
						}
						else
							pTemplate->mergeWithBaseClass(m_pTemplate, false);
					}
				}
			}
			else
			{
				if (!pTemplate)
					throw("The root template has not been defined before");

				m_pTemplate = pTemplate->analyzeSpecializedClass(header_types, pBodyNode);
				m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);

				CTemplate* pTemplate2 = pTemplate->findSpecializedTemplateByUniqueId(m_pTemplate->getUniqueId());
				if (!pTemplate2)
				{
					pTemplate2 = pTemplate->analyzeSpecializedClass(header_types, pBodyNode);
					pTemplate2->setDefLocation(g_cur_file_name, g_cur_line_no);
					pTemplate->addSpecializedTemplate(pTemplate2);

					if (body_data)
						pTemplate2->analyzeClassBody(pBaseClassDefsBlock, body_data);
				}
				else
				{
					if (body_data)
					{
						if (pTemplate2->isDefined())
							throw("This template is already defined before");

						pTemplate2->mergeWithSpecializedClass(m_pTemplate, true);
						pTemplate2->saveClassBody(pBaseClassDefsBlock, body_data);
						pTemplate2->setDefLocation(g_cur_file_name, g_cur_line_no);
						pTemplate2->analyzeClassBody(pBaseClassDefsBlock, body_data);
					}
				}
			}
			/*if (m_pTemplate)
			{
				if (body_data == NULL)
					break;

				if (m_pTemplate->isDefined())
					throw("This template is already defined before");

				m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			}
			else
			{
				MY_ASSERT(twn.getDepth() == 1);
				m_pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_CLASS, twn.getLastToken());
				getRealScope()->addTemplate(m_pTemplate);
				m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			}

			m_pTemplate = m_pTemplate->analyzeClass(pBodyNode);*/
			break;
		}
		case TEMPLATE_TYPE_VAR:
			if (bTemplate)
				break;
			m_pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_VAR, "");
			TRACE("\nCREATING var TEMPLATE %s\n", m_pTemplate->getDebugPath().c_str());
			m_pTemplate->readTemplateHeaderIntoTypeParams(header_types);
			m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			m_pTemplate->analyzeVar(pBodyNode);
			break;

		case TEMPLATE_TYPE_FUNC_VAR:
			if (bTemplate)
				break;
			m_pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_FUNC_VAR, "");
			TRACE("\nCREATING func_var TEMPLATE %s\n", m_pTemplate->getDebugPath().c_str());
			m_pTemplate->readTemplateHeaderIntoTypeParams(header_types);
			m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			m_pTemplate->analyzeFuncVar(pBodyNode);
			break;

		case TEMPLATE_TYPE_FRIEND_CLASS:
			if (bTemplate)
				break;
			m_pTemplate = new CTemplate(getRealScope(), TEMPLATE_TYPE_FRIEND_CLASS, "");
			TRACE("\nCREATING friend TEMPLATE %s in map\n", m_pTemplate->getDebugPath().c_str());
			m_pTemplate->readTemplateHeaderIntoTypeParams(header_types);
			m_pTemplate->setDefLocation(g_cur_file_name, g_cur_line_no);
			m_pTemplate->analyzeFriendClass(pBodyNode);
			break;

		default:
			MY_ASSERT(false);
		}
        delete pTempTemplate;
		break;
	}
	case DEF_TYPE_EXTERN_TEMPLATE_CLASS:
	{
		CSUType csu_type;
		SourceTreeNode* pUserDefTypeOrFuncHeader;
		defExternTemplateClassGetInfo(pRoot, m_mod_strings, m_bFlag, csu_type, pUserDefTypeOrFuncHeader);
		if (m_bFlag)
		{
			m_pTypeDef = getTypeDefByUserDefTypeNode(pUserDefTypeOrFuncHeader);
			MY_ASSERT(m_pTypeDef);
			break;

			TokenWithNamespace twn = userDefTypeGetInfo(pUserDefTypeOrFuncHeader);
			SymbolDefObject* pSymbolObj = findSymbolEx(twn);
			if (!pSymbolObj)
				throw(twn.toString() + " is not defined");
			if (pSymbolObj->type != GO_TYPE_TEMPLATE)
				throw(twn.toString() + " is not defined as a template");
			CTemplate* pTemplate = pSymbolObj->getTemplateAt(0);
			MY_ASSERT(pTemplate->getTemplateType() == TEMPLATE_TYPE_CLASS);
			MY_ASSERT(pTemplate->getClassType() == getSemanticTypeFromCSUType(csu_type));

			// make sure there's no specialized template for this
			MY_ASSERT(twn.scopeHasTemplate(twn.getDepth() - 1));
			std::string type_str;
			TemplateResolvedDefParamVector resolvedDefParams;
			for (size_t i = 0; i < twn.getTemplateParamCount(twn.getDepth() - 1); i++)
			{
				if (!type_str.empty())
					type_str += ",";

				TemplateResolvedDefParam param;
				SourceTreeNode* pChild;
				switch (twn.getTemplateParamAt(twn.getDepth() - 1, i, pChild))
				{
				case TEMPLATE_PARAM_TYPE_DATA:
				{
					type_str += displaySourceTreeExtendedTypeVar(pChild);
					param.pTypeDef = getTypeDefByExtendedTypeVarNode(pChild);
					break;
				}
				case TEMPLATE_PARAM_TYPE_FUNC:
				{
					type_str += displaySourceTreeFuncType(pChild);
					param.pTypeDef = getTypeDefByFuncTypeNode(pChild);
					break;
				}
				case TEMPLATE_PARAM_TYPE_VALUE:
				{
					type_str += displaySourceTreeExpr(pChild);
					CExpr expr(this, pChild);
					float f;
					if (!expr.calculateNumValue(f))
						throw(std::string("CStatement::") + __FUNCTION__ + ", cannot calculate " + type_str);
					param.numValue = f;
					break;
				}
				default:
					MY_ASSERT(false);
				}
				resolvedDefParams.push_back(param);
			}
			type_str = pTemplate->getName() + "<" + type_str + " >";

			pSymbolObj = pTemplate->findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
			if (pSymbolObj)
				throw("this template already has a specialized implementation for this");

			CTemplate* pInstancedTemplate = pTemplate->duplicateAsChild(type_str);
			pInstancedTemplate->setResolvedDefParams(resolvedDefParams);
			pTemplate->addTemplate(pInstancedTemplate);

			m_pTypeDef = pInstancedTemplate->createClassAsChild(type_str, SEMANTIC_TYPE_CLASS);
			CClassDef* pClassDef = m_pTypeDef->getBaseType()->getClassDef();

			if (!(pTemplate->isDefined()))
			{
				TRACE("CTemplate::%s, instance for %s\n", __FUNCTION__, pInstancedTemplate->getDebugPath().c_str());

				pClassDef->analyzeClassDef(pTemplate->m_classBaseTypeDefs, NULL, pTemplate->m_body_sv);
			}
			else
			{
				TRACE("CTemplate::%s, template hasn't been defined yet\n", __FUNCTION__);
			}
		}
		else // a func type
		{
			GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pUserDefTypeOrFuncHeader);
			//m_bThrow = funcHeaderInfo.bThrow;

			CGrammarAnalyzer ga2;
			ga2.initWithBlocks(getRealScope(), funcHeaderInfo.params_block);
			SourceTreeNode* pFuncParamsNode = ga2.getBlock();
			MY_ASSERT(ga2.getBlock() == NULL);

			m_pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(funcHeaderInfo.pReturnExtendedType), 0));
			m_pTypeDef->setModStrings(funcHeaderInfo.mod_strings);
			m_pTypeDef->setFuncReturnTypeNode(dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType));
			m_pTypeDef->setThrow(funcHeaderInfo.bThrow, dupSourceTreeNode(funcHeaderInfo.pThrowTypeNode));
			addFuncParamsToFuncType(m_pTypeDef, pFuncParamsNode);
			deleteSourceTreeNode(pFuncParamsNode);
			m_pTypeDef->setDefLocation(g_cur_file_name, g_cur_line_no);

			if (funcHeaderInfo.scope.empty())
			{
				SymbolDefObject* pSymbolObj = findSymbol(funcHeaderInfo.name, FIND_SYMBOL_SCOPE_LOCAL);
				if (pSymbolObj)
				{
					if (pSymbolObj->type != GO_TYPE_FUNC_DECL)
						throw("Func " + funcHeaderInfo.name + " already defined other than func");
					CFuncDeclare* pFuncDeclare = findFuncDeclare(pSymbolObj, m_pTypeDef);
					//if (pFuncDeclare && flowType != pFuncDeclare->getType()->getFuncFlowType())
					//	throw("Func " + name + " is declared twice but with different flow attribute");
				}
				m_pFuncDeclare = new CFuncDeclare(getRealScope(), funcHeaderInfo.name, m_pTypeDef);
				m_pFuncDeclare->setDefLocation(g_cur_file_name, g_cur_line_no);
				getRealScope()->addFuncDeclare(m_pFuncDeclare);
			}
			else
			{
				funcHeaderInfo.scope.addScope(funcHeaderInfo.name, false);
				SymbolDefObject* pSymbolObj = findSymbolEx(funcHeaderInfo.scope);
				if (pSymbolObj == NULL || pSymbolObj->type != GO_TYPE_FUNC_DECL)
					throw("Func " + funcHeaderInfo.scope.toString() + " should have been declared in that namespace");
				m_pFuncDeclare = findFuncDeclare(pSymbolObj, m_pTypeDef);
				if (m_pFuncDeclare == NULL)
					throw("Func " + funcHeaderInfo.scope.toString() + " should have been declared in that namespace");
			}
		}
		break;
	}
	case DEF_TYPE_EXTERN_TEMPLATE_FUNC:
	{
		SourceTreeNode* pDeclVarNode = dupSourceTreeNode(pRoot);
		m_declVarList.push_back(pDeclVarNode);

		SourceTreeNode* pExtendedReturnType, *pScope;
		std::string token;
		int templateParamCount;
		void* pFuncParamsBlock;
		bool bConst;
		defExternTemplateFuncGetInfo(pRoot, m_mod_strings, pExtendedReturnType, pScope, token, templateParamCount, pFuncParamsBlock, bConst);

		CGrammarAnalyzer ga2;
		ga2.initWithBlocks(getRealScope(), pFuncParamsBlock);
		SourceTreeNode* pFuncParamsNode = ga2.getBlock();
		MY_ASSERT(ga2.getBlock() == NULL);
		//defExternTemplateFuncGetParamByIndex(const SourceTreeNode* pRoot, int idx, TemplateParamType& nParamType, SourceTreeNode*& pChildNode);
		break;
	}
	default:
		MY_ASSERT(false);
	}
}

TypeDefPointer CStatement::analyzeSuperType(const SourceTreeNode* pSuperTypeNode, bool bAllowUndefinedStruct)
{
	SuperTypeType super_type;
	SourceTreeNode* pChildNode;
	superTypeGetInfo(pSuperTypeNode, super_type, pChildNode);

	TypeDefPointer pTypeDef;

	MY_ASSERT(super_type != SUPERTYPE_TYPE_TYPE);
	/*{
		pTypeDef = getTypeDefByTypeNode(pChildNode, (getParentTemplate() != NULL), bAllowUndefinedStruct);
		std::string display_str = displaySourceTreeType(pChildNode);
		if (pTypeDef && pTypeDef->toString() != display_str)
		{
			pTypeDef = TypeDefPointer(new CTypeDef(getRealScope(), "", pTypeDef, 0));
			pTypeDef->setDisplayString(display_str);
		}
		//if (pTypeDef == NULL)
		//	throw("cannot recognize type:" + displaySourceTreeType(pChildNode));

		//MY_ASSERT(!pTypeDef->getClassDef());	could be a template type
		return pTypeDef;
	}*/

	SemanticDataType dataType;
	TokenWithNamespace twn;
	std::string prefix;

	switch (super_type)
	{
	case SUPERTYPE_TYPE_ENUM_DEF:
	{
		dataType = SEMANTIC_TYPE_ENUM;
		std::string name;
		void* bracket_block;
		enumDefGetInfo(pChildNode, name, bracket_block);
		twn.addScope(name);
		prefix = "enum ";
		break;
	}
	case SUPERTYPE_TYPE_UNION_DEF:
	{
		dataType = SEMANTIC_TYPE_UNION;
		std::string name;
		void* bracket_block;
		unionDefGetInfo(pChildNode, name, bracket_block);
		twn.addScope(name);
		prefix = "union ";
		break;
	}
	case SUPERTYPE_TYPE_CLASS_DEF:
	{
		CSUType csu_type;
		void* bracket_block, *pBaseClassDefsBlock;
		StringVector mod_strings;
		classDefGetInfo(pChildNode, csu_type, mod_strings, twn, pBaseClassDefsBlock, bracket_block);
		if (csu_type == CSU_TYPE_STRUCT)
			dataType = SEMANTIC_TYPE_STRUCT;
		else if (csu_type == CSU_TYPE_CLASS)
			dataType = SEMANTIC_TYPE_CLASS;
		else
			MY_ASSERT(false);
		prefix = displayCSUType(csu_type);
		break;
	}
	default:
		MY_ASSERT(false);
	}

	std::string err_s;
	CClassDef* pClassDef = NULL;
	CScope* pParent = getRealScope();
	if (!twn.empty())
	{
		if (twn.getDepth() > 1)
		{
			TokenWithNamespace twn2;
			twn2.copyFrom(twn);
			twn2.resize(twn2.getDepth() - 1);
			SymbolDefObject* pSymbolObj = findSymbolEx(twn2, false);
			if (!pSymbolObj)
			  throw(twn2.toString() + " cannot be found");
			if (pSymbolObj->type == GO_TYPE_NAMESPACE)
				pParent = pSymbolObj->getNamespace();
			else if (pSymbolObj->type == GO_TYPE_TYPEDEF)
			{
				TypeDefPointer pTypeDef = pSymbolObj->getTypeDef();
				if (pTypeDef->getType() != SEMANTIC_TYPE_CLASS && pTypeDef->getType() != SEMANTIC_TYPE_STRUCT)
				{
					err_s = twn2.toString() + " is defined as a " + getSemanticTypeName(pTypeDef->getType()) + " in " + pSymbolObj->definedIn() + " but a class or template is expected";
					throw(err_s);
				}
				MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());
				pParent = pTypeDef->getBaseType()->getClassDef();
			}
			else
				MY_ASSERT(false);
		}
		SymbolDefObject* pSymbolObj = pParent->findSymbol(twn.getLastToken(), FIND_SYMBOL_SCOPE_LOCAL);
		if (pSymbolObj)
		{
			if (pSymbolObj->type != GO_TYPE_TYPEDEF)
			{
				err_s = twn.toString() + " is previously defined as a non data type:" + ltoa(pSymbolObj->type) + " in " + pSymbolObj->definedIn();
				throw(err_s);
			}
			pTypeDef = pSymbolObj->getTypeDef();
			if ((pTypeDef->getType() == SEMANTIC_TYPE_CLASS || pTypeDef->getType() == SEMANTIC_TYPE_STRUCT) && !pTypeDef->getBaseType()->getClassDef()->isDefined())
			{
				if (pTypeDef->getType() != dataType)
					pTypeDef->setType(dataType);
			}
			else if (pTypeDef->getType() != dataType)
			{
				err_s = twn.toString() + " is previously defined with a different data type:" + getSemanticTypeName(pTypeDef->getType()) + " in " + pSymbolObj->definedIn();
				throw(err_s);
			}
			if (pTypeDef->isBaseType() || !pTypeDef->getBaseType()->isBaseType() || pTypeDef->getBaseType()->getClassDef() == NULL || pTypeDef->getBaseType()->getClassDef()->isDefined())
			{
				err_s = twn.toString() + " is previously defined in " + pSymbolObj->definedIn() + " with conflict";
				throw(err_s);
			}
			pTypeDef = pTypeDef->getBaseType();
			pClassDef = pTypeDef->getClassDef();
		}
	}

	if (!pClassDef)
	{
		pTypeDef = pParent->createClassAsChild(twn.empty() ? "" : twn.getLastToken(), dataType);
		pTypeDef = pTypeDef->getBaseType();
		pClassDef = pTypeDef->getBaseType()->getClassDef();

		TRACE("*** check parent template, path=%s, parent template=%lx\n", getDebugPath().c_str(), (long)getParentTemplate());
		if (pParent->getParentTemplate())
			pClassDef->setWithinTemplate();
	}

	pTypeDef->setPrefix(prefix);
	pClassDef->analyze(pChildNode);
	pClassDef->setDefined(true);

	/*if (super_type == SUPERTYPE_TYPE_ENUM_DEF)
	{
		bool bUnderTemplate = (getParentTemplate() != NULL);
		int nValue = 0;
		for (int i = 0; i < pClassDef->getChildrenCount(); i++)
		{
			CScope* pGrammarObj = pClassDef->getChildAt(i);
			MY_ASSERT(pGrammarObj->getGoType() == GO_TYPE_STATEMENT);
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			MY_ASSERT(pStatement->getDefType() == DEF_TYPE_VAR_DEF);
			MY_ASSERT(pStatement->getVarCount() == 1);
			CVarDef* pVar = pStatement->getVarAt(0);
			if (!bUnderTemplate && pVar->getInitExpr())
			{
				float f = pVar->getInitExpr()->calculateNumValue();
				nValue = (int)f;
			}
			getRealScope()->addEnumDef(pVar->getName(), pClassDef, nValue++);
		}
	}*/

	return pTypeDef;
}

std::string CStatement::toString(int depth)
{
	std::string ret_s;

	bool bParentIsIfWhlieFor = depth > 10000;
	depth %= 10000;

	//ret_s += (isFlow() ? "+" : "-");
	ret_s += printTabs(depth);

	switch (m_statement_type)
	{
	case STATEMENT_TYPE_EXPR2:
	{
		ret_s += ((CExpr2*)m_children[0])->toString() + ";\n";
		break;
	}
	case STATEMENT_TYPE_DEF:
		ret_s = toDefString(depth);
		break;

	case STATEMENT_TYPE_BREAK:
		ret_s += "break;\n";
		break;

	case STATEMENT_TYPE_CONTINUE:
		ret_s += "continue;\n";
		break;

	case STATEMENT_TYPE_RETURN:
		ret_s += "return";
		if (m_children.size() > 0)
			ret_s += " " + ((CExpr*)m_children[0])->toString();
		ret_s += ";\n";
		break;

	case STATEMENT_TYPE_COMPOUND:
	{
		if (bParentIsIfWhlieFor)
			depth--;
		ret_s = /*(isFlow() ? "+" : "-") + */printTabs(depth) + "{\n";
		for (unsigned i = 0; i < m_children.size(); i++)
			ret_s += ((CStatement*)m_children[i])->toString(depth + 1);
		ret_s += printTabs(depth) + "}\n";
		break;
	}
	case STATEMENT_TYPE_IF:
	{
		MY_ASSERT(getChildrenCount() == 3);
		ret_s += "if (";

		CExpr* pExpr = (CExpr*)m_children[0];
		if (pExpr)
			ret_s += pExpr->toString();
		else if (m_pTypeDef)
		{
			ret_s += m_pTypeDef->toString() + " ";
			for (unsigned i = 0; i < m_var_list.size(); i++)
			{
				if (i > 0)
					ret_s += ", ";
				ret_s += m_var_list[i]->toString();
			}
		}
		ret_s += ")\n" + ((CStatement*)getChildAt(1))->toString(10000 + depth + 1);
		CStatement* pElseStatement = (CStatement*)getChildAt(2);
		if (pElseStatement)
		{
			ret_s += printTabs(depth) + "else\n";
			ret_s += pElseStatement->toString(10000 + depth + 1);
		}
		break;
	}
	case STATEMENT_TYPE_WHILE:
	{
		MY_ASSERT(getChildrenCount() == 2);
		ret_s += "while (";

		CExpr2* pExpr2 = (CExpr2*)m_children[0];
		if (pExpr2)
			ret_s += pExpr2->toString();
		else if (m_pTypeDef)
		{
			ret_s += m_pTypeDef->toString() + " ";
			for (unsigned i = 0; i < m_var_list.size(); i++)
			{
				if (i > 0)
					ret_s += ", ";
				ret_s += m_var_list[i]->toString();
			}
		}
		ret_s += ")\n" + ((CStatement*)m_children[1])->toString(10000 + depth + 1);
		break;
	}
	case STATEMENT_TYPE_DO:
		ret_s += "do\n" + ((CStatement*)m_children[0])->toString(10000 + depth);
		ret_s += printTabs(depth) + "while (" + ((CExpr*)m_children[1])->toString() + ");\n";
		break;

	case STATEMENT_TYPE_FOR:
	{
		MY_ASSERT(getChildrenCount() == 4);
		ret_s += "for (";
		CExpr2* pExpr2 = (CExpr2*)m_children[0];
		if (pExpr2)
			ret_s += pExpr2->toString();
		else if (m_pTypeDef)
		{
			ret_s += m_pTypeDef->toString() + " ";
			for (unsigned i = 0; i < m_var_list.size(); i++)
			{
				if (i > 0)
					ret_s += ", ";
				ret_s += m_var_list[i]->toString();
			}
		}
		ret_s += "; ";
		CExpr* pExpr = (CExpr*)m_children[1];
		if (pExpr)
			ret_s += pExpr->toString();
		ret_s += "; ";
		pExpr2 = (CExpr2*)m_children[2];
		if (pExpr2)
			ret_s += pExpr2->toString();
		ret_s += ")\n";
		ret_s += ((CStatement*)m_children[3])->toString(10000 + depth + 1);
		break;
	}
	case STATEMENT_TYPE_SWITCH:
	{
		ret_s += "switch (" + ((CExpr*)m_children[0])->toString() + ")\n";
		ret_s += printTabs(depth) + "{\n";
		int n = 1;
		for (unsigned i = 0; i < m_int_vector.size(); i++)
		{
			int m = m_int_vector[i];
			ret_s += printTabs(depth) + "case " + ((CExpr*)getChildAt(n))->toString() + ":\n";
			for (int j = 1; j < m; j++)
				ret_s += ((CStatement*)getChildAt(n + j))->toString(depth + 1);
			n += m;
		}
		if (m_bFlag)
		{
			ret_s += printTabs(depth) + "default:\n";
			for (; n < getChildrenCount(); n++)
				ret_s += ((CStatement*)getChildAt(n))->toString(depth + 1);
		}
		ret_s += printTabs(depth) + "}\n";
		break;
	}
	case STATEMENT_TYPE_TRY:
		ret_s += "try\n";
		ret_s += ((CStatement*)m_children[0])->toString(10000 + depth + 1);
		MY_ASSERT(m_int_vector[0] + 1 == m_children.size());
		MY_ASSERT(m_func_param_vector.size() + (m_bFlag ? 1 : 0) == m_int_vector[0]);
		for (unsigned i = 0; i < m_int_vector[0]; i++)
		{
			ret_s += printTabs(depth) + "catch (";
			if (i == m_int_vector[0] - 1 && m_bFlag)
				ret_s += "...";
			else
			{
				FuncParamItem& param = m_func_param_vector.at(i);
				switch (param.param_type)
				{
				case FUNC_PARAM_TYPE_REGULAR:
					ret_s += displaySourceTreeType(param.pTypeNode);
					if (param.pDeclVarNode)
					{
						ret_s += " " + displaySourceTreeDeclVar(param.pDeclVarNode);
						if (param.pInitExprNode)
							ret_s += " = " + displaySourceTreeExpr(param.pInitExprNode);
					}
					break;
				case FUNC_PARAM_TYPE_FUNC:
					ret_s += displaySourceTreeFuncType(param.pTypeNode);
					break;
				default:
					MY_ASSERT(false);
				}
			}
			ret_s += ")\n";
			ret_s += ((CStatement*)getChildAt(1 + i))->toString(depth + 1);
		}
		break;
	/*case STATEMENT_TYPE_FLOW_SIGNAL:
		ret_s += "flow_signal(" + ((CExpr*)m_children[0])->toString() + ", " + ((CExpr*)m_children[1])->toString() + ");\n";
		break;
	*/
	case STATEMENT_TYPE_FLOW_WAIT:
		ret_s += "flow_wait(" + ((CExpr*)m_children[0])->toString() + ", " + ((CExpr*)m_children[1])->toString() + ");\n";
		break;

	case STATEMENT_TYPE_FLOW_FORK:
		ret_s += "flow_fork\n";
		ret_s += ((CStatement*)m_children[0])->toString(10000 + depth + 1);
		break;

	case STATEMENT_TYPE_FLOW_NEW:
	{
		CExpr* pExpr = (CExpr*)m_children[0];
		if (pExpr)
			ret_s += pExpr->toString() + " = ";
		ret_s += "flow_new\n";
		ret_s += ((CStatement*)m_children[1])->toString(10000 + depth + 1);
		break;
	}
	case STATEMENT_TYPE_FLOW_TRY:
	{
		ret_s += "flow_try\n";
		ret_s += ((CStatement*)m_children[0])->toString(10000 + depth + 1);
		for (int i = 0; i < m_int_vector[0]; i++)
		{
			CExpr* pExpr1 = (CExpr*)m_children[1 + i * 3];
			CExpr* pExpr2 = (CExpr*)m_children[1 + i * 3 + 1];
			CStatement* pStatement = (CStatement*)m_children[1 + i * 3 + 2];
			ret_s += printTabs(depth) + "flow_catch (" + pExpr1->toString() + ", " + pExpr2->toString() + ")\n" + pStatement->toString(10000 + depth + 1);
		}
		break;
	}
	case STATEMENT_TYPE___ASM:
	{
		ret_s += "__asm {\n" + printTabs(depth + 1);
		for (size_t i = 0; i < m_templateTokens.size(); i++)
		{
			const std::string& s = m_templateTokens[i];

			if (CLexer::isCommentWord(s))
			{
				if (s.substr(0, 3) == "//*")
				{
					for (int i = 0; i < atoi(s.c_str() + 3); i++)
						ret_s += "\n" + printTabs(depth + 1);
				}
				continue;
			}
			ret_s += s + " ";
		}
		ret_s += "\n" + printTabs(depth) + "}\n";
		break;
	}
	default:
		MY_ASSERT(false);
	}

	return ret_s;
}

std::string CStatement::toDefString(int depth)
{
	std::string ret_s = printTabs(depth);

	switch (m_def_type)
	{
	case DEF_TYPE_EMPTY:
		break;

	case DEF_TYPE___PRAGMA:
		ret_s += "__pragma(" + m_asm_string + ")";
		break;

	case DEF_TYPE_POUND_LINE:
		ret_s += "#" + m_asm_string;
		return ret_s + "\n";

	case DEF_TYPE_PRE_DECL:
		ret_s += getSemanticTypeName(m_pTypeDef->getType()) + " " + combineStrings(m_mod_strings) + m_pTypeDef->getName();
		break;

	case DEF_TYPE_USING_NAMESPACE:
		ret_s += "using ";
		if (m_bFlag)
			ret_s += "namespace ";
		ret_s += getRelativePath(m_pGrammarObj, m_asm_string);
		break;

	case DEF_TYPE_TYPEDEF:
		ret_s += combineStrings(m_mod_strings) + "typedef ";
		if (m_typedefType != TYPEDEF_TYPE_TYPEOF)
		{
			if (m_pTypeDef->getType() != SEMANTIC_TYPE_FUNC)
			{
				ret_s += m_pTypeDef->toString(depth) + " ";
				for (size_t i = 0; i < m_declVarList.size(); i++)
				{
					if (i > 0)
						ret_s += ", ";
					std::string s = displaySourceTreeDeclVar(m_declVarList[i]);
					ret_s += s;
				}
			}
			else
			{
				TypeDefPointer pTypeDef = m_pTypeDef;
				while (!pTypeDef->isBaseType() && pTypeDef->getDisplayStr().empty() && pTypeDef->getName().empty() && pTypeDef->getDepth() == 0)
					pTypeDef = pTypeDef->getBaseType();
				if (!pTypeDef->getDisplayStr().empty() || !pTypeDef->getName().empty())
				{
					ret_s += m_pTypeDef->toString(depth) + " ";
					for (size_t i = 0; i < m_declVarList.size(); i++)
					{
						if (i > 0)
							ret_s += ", ";
						std::string s = displaySourceTreeDeclVar(m_declVarList[i]);
						ret_s += s;
					}
				}
				else
					ret_s += m_pTypeDef->toString(depth);
			}
		}
		else
		{
			std::string s;
			if (m_pExpr)
				s = m_pExpr->toString(0);
			else
				s = m_pTypeDef->getBaseType()->toString(0);
			ret_s += "__typeof__(" + s + ")	" + m_pTypeDef->getName();
		}
		break;

	case DEF_TYPE_FUNC_DECL:
		//| *[func_modifier] ?[extended_type] token_with_namespace '(' func_params ')' ?[const] ?['throw' '(' ')'] ?['__asm' '(' const_value ')'] *[attribute] ';'
		ret_s += m_pTypeDef->toFuncString(getRelativePath(m_pFuncDeclare->getParent(), m_pFuncDeclare->getName()));
		//if (isInModifiers(m_mod_strings, MODBIT_CONST))
		//	ret_s += " const";
		if (!m_asm_string.empty())
			ret_s += "__asm(" + m_asm_string + ")";
		for (size_t i = 0; i < m_attribute_list.size(); i++)
		{
			SourceTreeNode* pAttributeNode = m_attribute_list[i];
			ret_s += " " + displaySourceTreeAttribute(pAttributeNode);
		}
		break;

	case DEF_TYPE_FUNC_VAR_DEF:
		ret_s += combineStrings(m_mod_strings) + m_pTypeDef->toFuncString(m_var_list[0]->getName());
		break;

	case DEF_TYPE_VAR_DEF:
	case DEF_TYPE_SUPER_TYPE_VAR_DEF:
	{
		//std::string temp_s = m_pTypeDef->getDebugPath();
		ret_s += combineStrings(m_mod_strings) + m_pTypeDef->toString(depth);
		for (size_t i = 0; i < m_attribute_list.size(); i++)
		{
			SourceTreeNode* pAttributeNode = m_attribute_list[i];
			ret_s += " " + displaySourceTreeAttribute(pAttributeNode);
		}
		int n = 0;
		for (size_t i = 0; i < m_var_list.size(); i++)
		{
			CVarDef* pVarDef = m_var_list[i];

			if (n == 0)
				ret_s += " ";
			else
				ret_s += ", ";

			if (pVarDef->isRestrict())
				ret_s += "__restrict ";
			ret_s += pVarDef->toString();
			n++;
		}
		break;
	}
	case DEF_TYPE_TEMPLATE:
		MY_ASSERT(m_pTemplate);
		ret_s += combineStrings(m_mod_strings) + m_pTemplate->toString(m_bFlag, depth);
		return ret_s;

	case DEF_TYPE_EXTERN_TEMPLATE_CLASS:
	{
		ret_s += combineStrings(m_mod_strings);
		ret_s += "template ";
		if (m_bFlag) // is class
		{
			MY_ASSERT(!m_pTypeDef->isBaseType() && m_pTypeDef->getBaseType()->isBaseType() && m_pTypeDef->getBaseType()->getClassDef());
			CClassDef* pClassDef = m_pTypeDef->getBaseType()->getClassDef();
			ret_s += getSemanticTypeName(pClassDef->getType()) + " " + m_pTypeDef->toString();
		}
		else
		{
			ret_s += combineStrings(m_mod_strings) + getRelativePath(m_pFuncDeclare->getParent(), m_pFuncDeclare->getName()) + m_pTypeDef->toFuncString("");
			//if (isInModifiers(m_mod_strings, MODBIT_CONST))
			//	ret_s += " const";
			//if (m_bThrow > 0)
			//	ret_s += " throw()";
			if (!m_asm_string.empty())
				ret_s += "__asm(" + m_asm_string + ")";
			for (size_t i = 0; i < m_attribute_list.size(); i++)
			{
				SourceTreeNode* pAttributeNode = m_attribute_list[i];
				ret_s += " " + displaySourceTreeAttribute(pAttributeNode);
			}
		}
		break;
	}
	case DEF_TYPE_EXTERN_TEMPLATE_FUNC:
	{
		ret_s += combineStrings(m_mod_strings);
		ret_s += "template ";

		SourceTreeNode* pExtendedReturnType, *pScope, *pDeclVarNode;
		std::string token;
		int templateParamCount;
		void* pFuncParamsBlock;
		bool bConst;

		MY_ASSERT(m_declVarList.size() == 1);
		pDeclVarNode = m_declVarList[0];
		StringVector mod_strings;
		defExternTemplateFuncGetInfo(pDeclVarNode, mod_strings, pExtendedReturnType, pScope, token, templateParamCount, pFuncParamsBlock, bConst);
		ret_s = displaySourceTreeExtendedType(pExtendedReturnType) + " ";
		if (pScope)
			ret_s += scopeGetInfo(pScope).toString();
		ret_s += token + "<";
		for (int i = 0; i < templateParamCount; i++)
		{
			if (i > 0)
				ret_s += ", ";

			TemplateParamType nParamType;
			SourceTreeNode* pChildNode;
			defExternTemplateFuncGetParamByIndex(pDeclVarNode, i, nParamType, pChildNode);
		}
		ret_s += ">(";
		CGrammarAnalyzer ga2;
		ga2.initWithBlocks(getRealScope(), pFuncParamsBlock);
		SourceTreeNode* pFuncParamsNode = ga2.getBlock();
		MY_ASSERT(ga2.getBlock() == NULL);
		ret_s += displaySourceTreeFuncParams(pFuncParamsNode);
		ret_s += ")";
		if (bConst)
			ret_s += " const";
		break;
	}
	case DEF_TYPE_CLASS_CAM:
		ret_s = printTabs(depth - 1) + displayCAMType(m_cam_type) + ":\n";
		return ret_s;

	case DEF_TYPE_CLASS_FRIEND:
		MY_ASSERT(m_pTypeDef->getBaseType());
		MY_ASSERT(m_pTypeDef->getBaseType()->getClassDef());
		ret_s = printTabs(depth - 1) + "friend " + displayCSUType(m_csu_type) + " " + getSemanticTypeName(m_pTypeDef->getType()) + " " + getRelativePath(m_pTypeDef->getBaseType()->getClassDef()) + ";\n";
		return ret_s;

	default:
		MY_ASSERT(false);
	}

	return ret_s + ";\n";
}

// this statement is a def var type and contains this var, change the name in its source node tree.
void CStatement::changeDefVarName(const std::string& old_name, const std::string& new_name)
{
	MY_ASSERT(m_statement_type == STATEMENT_TYPE_DEF);

	switch (m_def_type)
	{
	case DEF_TYPE_FUNC_VAR_DEF:
		MY_ASSERT(m_var_list[0]->getName() == old_name);
		m_var_list[0]->setName(new_name);
		break;
	default:
		MY_ASSERT(false);
	}
}

// need to change ref of a var with old_name in this statement and its children to new_name
// return whether found a def statement that contains the def of a var with the same name. If returns true, then should not proceed in the same scope
bool CStatement::changeRefVarName(const std::string& old_name, CExpr* pNewExpr)
{
	switch (m_statement_type)
	{
	case STATEMENT_TYPE_EXPR2:
	{
		((CExpr*)m_children[0])->changeRefVarName(old_name, pNewExpr);
		break;
	}
	case STATEMENT_TYPE_DEF:
	{
		switch (m_def_type)
		{
		case DEF_TYPE_VAR_DEF:
		case DEF_TYPE_SUPER_TYPE_VAR_DEF:
		{
			for (size_t i = 0; i < m_var_list.size(); i++)
			{
				CVarDef* pVarDef = m_var_list[i];

				if (pVarDef->getName() == old_name)
					return true;
				if (pVarDef->getInitExpr())
					pVarDef->getInitExpr()->changeRefVarName(old_name, pNewExpr);
				ExprVector expr_v = pVarDef->getExprListInDeclVar();
				for (size_t j = 0; j < expr_v.size(); j++)
				{
					CExpr* pExpr = expr_v[j];
					pExpr->changeRefVarName(old_name, pNewExpr);
				}
			}
			break;
		}
		case DEF_TYPE_FUNC_VAR_DEF:
			MY_ASSERT(m_var_list.size() == 1);
			if (m_var_list[0]->getName() == old_name)
				return true;
			//m_var_list[0]->setName(new_name);
			break;
		default:
			break;
		}
		break;
	}
	case STATEMENT_TYPE_FOR:
	{
		for (size_t i = 0; i < m_var_list.size(); i++)
		{
			CVarDef* pVarDef = m_var_list[i];

			if (pVarDef->getName() == old_name)
				return true;
			if (pVarDef->getInitExpr())
				pVarDef->getInitExpr()->changeRefVarName(old_name, pNewExpr);
			ExprVector expr_v = pVarDef->getExprListInDeclVar();
			for (size_t j = 0; j < expr_v.size(); j++)
			{
				CExpr* pExpr = expr_v[j];
				pExpr->changeRefVarName(old_name, pNewExpr);
			}
		}
	}
	default:
		for (int i = 0; i < getChildrenCount(); i++)
		{
			CScope* pChild = getChildAt(i);
			if (!pChild)
				continue;
			if (pChild->getGoType() == GO_TYPE_EXPR)
				((CExpr*)pChild)->changeRefVarName(old_name, pNewExpr);
			else
				((CStatement*)pChild)->changeRefVarName(old_name, pNewExpr);
		}
		break;
	}

	return false;
}

CClassDef::CClassDef(CScope* pParent, SemanticDataType basic_type, const std::string& name) : CScope(pParent)
{
	m_bDefined = false;
	m_bWithinTemplate = false;
	setRealScope(true);
	m_name = name;
	m_type = basic_type;

	TRACE("\nCClassDef::CClassDef, path=%s\n", getDebugPath().c_str());

	if (pParent->getGoType() == GO_TYPE_TEMPLATE)
	{
		while (pParent->getParent()->getGoType() == GO_TYPE_TEMPLATE)
			pParent = pParent->getParent();
		m_template_name = pParent->getName();
	}
}

std::string CClassDef::getDebugName()
{
	return getName() + ":" + ltoa(m_bWithinTemplate);
}

void CClassDef::addBaseClass(CClassDef* pClassDef, ClassAccessModifierType cam_type, bool bVirtual)
{
	struct BaseClassCAMPair bc;
	bc.bVirtual = bVirtual;
	bc.cam_type = cam_type;
	bc.pTypeDef = pClassDef->getTypeDef();
	TokenWithNamespace twn = getRelativeTWN2(this, pClassDef, "");
	bc.pTypeNode = userDefTypeCreateByName(twn);

	m_baseTypeList.push_back(bc);
}

void CClassDef::analyze(const SourceTreeNode* pRoot)
{
	MY_ASSERT(m_bDefined == false);
	bool bUnderTemplate = (getParentTemplate() != NULL);
	//const SourceTreeNode* pNode;

	switch (m_type)
	{
	case SEMANTIC_TYPE_ENUM:
	{
		void* bracket_block;
		enumDefGetInfo(pRoot, m_name, bracket_block);

		CGrammarAnalyzer ga;
		ga.initWithBlocks(getRealScope(), bracket_block);
		SourceTreeNode* pNode = ga.getBlock();
		MY_ASSERT(ga.isEmpty());

		int children_count = enumItemsGetCount(pNode);
		int nValue = 0;
		m_enum_has_last_comma = false;
		for (int i = 0; i < children_count; i++)
		{
			std::string key;
			SourceTreeNode* pExprNode;
			enumItemsGetAt(pNode, i, key, pExprNode);
			if (key.empty())
			{
				if (i < children_count - 1)
					throw("empty item found in enum " + m_name + " definition");
				m_enum_has_last_comma = true;
				break;
			}
			// we add child here which will add varDef in this pClassDef, we'll also add enum def outside of the pClassDef. the varDef will not be searched.
			CExpr* pInitExpr = NULL;
			if (!bUnderTemplate && pExprNode)
				pInitExpr = new CExpr(getParent()->getRealScope(), pExprNode); // to prevent from when calculating expr value, the enum will be found as a var instead of a enum
			CVarDef* pVarDef = new CVarDef(this, key, g_type_def_int, NULL, pInitExpr);
			addChild(new CStatement(this, STATEMENT_TYPE_DEF, pVarDef));
			if (pInitExpr)
			{
				float f;
				if (!pInitExpr->calculateNumValue(f))
					throw(std::string("CClassDef::") + __FUNCTION__ + ", cannot calculate " + pInitExpr->toString());
				nValue = (int)f;
			}
			getParent()->getRealScope()->addEnumDef(pVarDef->getName(), this, nValue++);
		}
		break;
	}
	case SEMANTIC_TYPE_UNION:
	{
		void* bracket_block;
		unionDefGetInfo(pRoot, m_name, bracket_block);

		CGrammarAnalyzer ga;
		ga.initWithBlocks(getRealScope(), bracket_block);
		while (SourceTreeNode* pNode = ga.getBlock())
		{
			g_cur_file_name = pNode->file_name;
			g_cur_line_no = pNode->line_no;
			TRACE("CClassDef::%s, union=%s, ANALYZING LINE %s:%d\n", __FUNCTION__, getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
			CStatement* pStatement = new CStatement(this);
			pStatement->analyzeDef(&ga, pNode);
			addChild(pStatement);
			deleteSourceTreeNode(pNode);
		}
		MY_ASSERT(ga.isEmpty());
		break;
	}
	case SEMANTIC_TYPE_STRUCT:
	case SEMANTIC_TYPE_CLASS:
	{
		CSUType csu_type;
		void* bracket_block, *pBaseClassDefBlock;
		TokenWithNamespace twn;
		classDefGetInfo(pRoot, csu_type, m_mod_strings, twn, pBaseClassDefBlock, bracket_block);
		MY_ASSERT((twn.empty() ? "" : twn.getLastToken()) == getName());

		int parentDefCount = 0;
		SourceTreeNode* pBaseClassDefsNode = NULL;
		if (pBaseClassDefBlock)
		{
			CGrammarAnalyzer ga;
			ga.initWithBlocks(getRealScope(), pBaseClassDefBlock);
			pBaseClassDefsNode = ga.getBlock();
			MY_ASSERT(ga.isEmpty());
			parentDefCount = pBaseClassDefsNode ? baseClassDefsGetCount(pBaseClassDefsNode) : 0;
		}
		ClassBaseTypeDefVector classBaseTypeDefs;
		for (int i = 0; i < parentDefCount; i++)
		{
			ClassBaseTypeDef defItem;
			baseClassDefsGetChildByIndex(pBaseClassDefsNode, i, defItem.bVirtual, defItem.cam_type, defItem.pUserDefTypeNode);
			defItem.pUserDefTypeNode = dupSourceTreeNode(defItem.pUserDefTypeNode);

			classBaseTypeDefs.push_back(defItem);
		}

		analyzeClassDef(classBaseTypeDefs, bracket_block);
		break;
	}
	default:
		MY_ASSERT(false);
	}

	setDefined(true);
}

bool CClassDef::analyzeByTemplate()
{
	TRACE("CClassDef::%s, pClass=%s\n", __FUNCTION__, getDebugPath().c_str());
	MY_ASSERT(!isDefined());
	MY_ASSERT(getParent()->getGoType() == GO_TYPE_TEMPLATE);
    MY_ASSERT(m_template_name == ((CTemplate*)getParent())->getTemplateName());

	CTemplate* pParentTemplate = (CTemplate*)getParent();
	MY_ASSERT(pParentTemplate->isInstancedTemplate());
	CTemplate* pGrandParent = (CTemplate*)pParentTemplate->getParent();

	if (!pGrandParent->isDefined())
	    return false;

	analyzeClassDef(pGrandParent->m_classBaseTypeDefs, NULL, pGrandParent->m_body_sv);
	return true;
}

void CClassDef::analyzeClassDef(const ClassBaseTypeDefVector& classBaseTypeDefs, void* bracket_block, const StringVector& tokens)
{
	MY_ASSERT(!isDefined());
	setDefined(true);
	bool bTemplate = (getParentTemplate() != NULL);

	TypeDefPointer pTypeThis = TypeDefPointer(new CTypeDef(NULL, "", getTypeDef(), 1));
	addVarDef(new CVarDef(this, "this", pTypeThis, NULL));

	// add two default constructor and destructor. they will be replaced when direct function is defined.
	addFuncDeclare(new CFuncDeclare(this, getClassName(), TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, g_type_def_void, 0))));
	addFuncDeclare(new CFuncDeclare(this, "~" + getClassName(), TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, g_type_def_void, 0))));

	if (!bTemplate)
	{
		for (unsigned i = 0; i < classBaseTypeDefs.size(); i++)
		{
			ClassAccessModifierType cam_type = classBaseTypeDefs[i].cam_type;
			SourceTreeNode* pUserDefTypeNode = classBaseTypeDefs[i].pUserDefTypeNode;

			TypeDefPointer pTypeDef = getTypeDefByUserDefTypeNode(pUserDefTypeNode, true);
			CTypeDef* pType = pTypeDef.get();
			if (!pTypeDef)
				throw("cannot find definition of " + displaySourceTreeUserDefType(pUserDefTypeNode));
			if (pTypeDef->getType() != SEMANTIC_TYPE_CLASS && pTypeDef->getType() != SEMANTIC_TYPE_STRUCT)
				throw("base class " + displaySourceTreeUserDefType(pUserDefTypeNode) + " should be of struct or class type but a " + ltoa(pTypeDef->getType()));
			std::string s = pTypeDef->toFullString();
			TypeDefPointer pTypeDef2 = pTypeDef;
			while (!pTypeDef2->isBaseType() && pTypeDef2->getDepth() == 0)
				pTypeDef2 = pTypeDef2->getBaseType();
			if (!pTypeDef2->getClassDef())
				throw("base class " + displaySourceTreeUserDefType(pUserDefTypeNode) + " must be a direct struct or class");
			CClassDef* pBaseClassDef = pTypeDef2->getClassDef();
			MY_ASSERT(pBaseClassDef != this);
			if (!pBaseClassDef->isDefined())
			{
				TRACE("CClassDef::%s, analyze base class because it's not defined, path=%s\n", __FUNCTION__, pBaseClassDef->getDebugPath().c_str());
				pBaseClassDef->analyzeByTemplate();
			}
			BaseClassCAMPair pair = {classBaseTypeDefs[i].bVirtual, cam_type, pTypeDef, dupSourceTreeNode(pUserDefTypeNode)};
			TRACE("Adding base class %s to %s\n", pTypeDef->toString().c_str(), m_name.c_str());
			m_baseTypeList.push_back(pair);
		}
	}

	std::vector<TempBlock> func_def_list;
	std::vector<ClassDefBlock> class_def_list;

	SourceTreeVector ret_v;
	CGrammarAnalyzer ga;
	if (bracket_block)
		ga.initWithBlocks(getRealScope(), bracket_block);
	else
		ga.initWithTokens(getRealScope(), "class_def_body", tokens);

	TRACE("START ANALYZING CLASS %s\n", getDebugPath().c_str());
	ClassAccessModifierType cam_type = CAM_TYPE_PRIVATE;
	int n = 0;
	while (SourceTreeNode* pNode = ga.getBlock())
	{
	    n++;
		ret_v.push_back(pNode);
		g_cur_file_name = pNode->file_name;
		g_cur_line_no = pNode->line_no;
		TRACE("CClassDef::%s, class=%s, ANALYZING LINE %s:%d\n", __FUNCTION__, getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
		switch (cdbGetType(pNode))
		{
		case CDB_TYPE_CAM:
		{
			cam_type = cdbCamGetType(pNode);
			CStatement* pStatement = new CStatement(this, DEF_TYPE_CLASS_CAM, cam_type);
			pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
			addChild(pStatement);
			break;
		}
		case CDB_TYPE_FRIEND:
		{
			CSUType csu_type;
			SourceTreeNode* pUserDefTypeNode;
			cdbFriendGetInfo(pNode, csu_type, pUserDefTypeNode);
			TypeDefPointer pTypeDef = getTypeDefByUserDefTypeNode(pUserDefTypeNode);
			CStatement* pStatement = new CStatement(this, DEF_TYPE_CLASS_FRIEND, csu_type, pTypeDef);
			pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
			addChild(pStatement);
			break;
		}
		case CDB_TYPE_DEFS:
		{
			pNode = cdbDefGetNode(pNode);
			DefType defType = defGetType(pNode);
			while (true)
			{
				// a sub class might be defined whose implementation may call a method of the main class after this sub class.
				// therefore, we cannot analyze this sub class until all methods of the main class have been gone through.
				if (defType == DEF_TYPE_SUPER_TYPE_VAR_DEF)
				{
					SourceTreeNode* pSuperTypeNode;
					StringVector mod_strings;
					void* remain_block;
					defSuperTypeVarDefGetInfo(pNode, mod_strings, pSuperTypeNode, remain_block);

					SuperTypeType super_type;
					SourceTreeNode* pChildNode;
					superTypeGetInfo(pSuperTypeNode, super_type, pChildNode);

					if (super_type == SUPERTYPE_TYPE_CLASS_DEF)
					{
						CSUType csu_type;
						void* bracket_block, *pBaseClassDefsBlock;
						StringVector mod_strings;
						TokenWithNamespace twn;
						classDefGetInfo(pChildNode, csu_type, mod_strings, twn, pBaseClassDefsBlock, bracket_block);

						SemanticDataType dataType;
						if (csu_type == CSU_TYPE_STRUCT)
							dataType = SEMANTIC_TYPE_STRUCT;
						else if (csu_type == CSU_TYPE_CLASS)
							dataType = SEMANTIC_TYPE_CLASS;
						else
							MY_ASSERT(false);

						MY_ASSERT(twn.getDepth() <= 1);
						if (twn.getDepth() == 1)
						{
							std::string name = twn.getLastToken();
							SymbolDefObject* pSymbolObj = findSymbol(name, FIND_SYMBOL_SCOPE_SCOPE);
							if (pSymbolObj != NULL && (pSymbolObj->type != GO_TYPE_TYPEDEF || pSymbolObj->getTypeDef()->getType() != dataType))
								throw(name + " is defined as another type in " + pSymbolObj->definedIn());
							if (!pSymbolObj)
								getRealScope()->createClassAsChild(name, dataType);

							CStatement* pStatement = new CStatement(this);
							pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
							pStatement->setClassAccessModifierType(cam_type);
							addChild(pStatement);

							ClassDefBlock cdb(pStatement, pNode);
							class_def_list.push_back(cdb);
							break;
						}
					}
				}
				if (defType != DEF_TYPE_FUNC_DECL || !defFuncDeclIsFuncDef(pNode))
				{
					CStatement* pStatement = new CStatement(this);
					pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
					pStatement->analyzeDef(&ga, pNode);
					pStatement->setClassAccessModifierType(cam_type);
					addChild(pStatement);
					break;
				}

				//int func_modifier;
				SourceTreeNode* pFuncHeaderNode;
				void* pBaseClassInitBlock, *bracket_block2;
				bool bPureVirtual;
				std::string asm_string;
				SourceTreeVector attribute_list;
				defFuncDeclGetInfo(pNode, pFuncHeaderNode, asm_string, attribute_list, bPureVirtual, pBaseClassInitBlock, bracket_block2);
				MY_ASSERT(!bPureVirtual);
				//MY_ASSERT(!bThrow); // has throw() in <exception>
				MY_ASSERT(asm_string.empty());
				MY_ASSERT(attribute_list.empty());

				GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pFuncHeaderNode);

				if (!funcHeaderInfo.scope.empty())
					throw("cannot specify namespace to method name " + funcHeaderInfo.scope.toString());

				CGrammarAnalyzer ga2;
				ga2.initWithBlocks(getRealScope(), funcHeaderInfo.params_block);
				SourceTreeNode* pFuncParamsNode = ga2.getBlock();
				MY_ASSERT(ga2.getBlock() == NULL);

				//TRACE("CClassDef::%s, adding function %s, param_size=%d\n", __FUNCTION__, func_name.c_str(), (int)paramList.size());
				TypeDefPointer type_def_ptr = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(funcHeaderInfo.pReturnExtendedType), 0));
				type_def_ptr->setModStrings(funcHeaderInfo.mod_strings);
				type_def_ptr->setMod2Strings(funcHeaderInfo.mod2_strings);
				type_def_ptr->setMod3Strings(funcHeaderInfo.mod3_strings);
				type_def_ptr->setMod4Strings(funcHeaderInfo.mod4_strings);
				type_def_ptr->setFuncReturnTypeNode(dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType));
				type_def_ptr->setThrow(funcHeaderInfo.bThrow, dupSourceTreeNode(funcHeaderInfo.pThrowTypeNode));
				addFuncParamsToFuncType(type_def_ptr, pFuncParamsNode);
				deleteSourceTreeNode(pFuncParamsNode);
				type_def_ptr->setThrow(funcHeaderInfo.bThrow, funcHeaderInfo.pThrowTypeNode);
				type_def_ptr->setDefLocation(g_cur_file_name, g_cur_line_no);

				SymbolDefObject* pSymbolObj = findSymbol(funcHeaderInfo.name, FIND_SYMBOL_SCOPE_LOCAL);
				if (pSymbolObj && pSymbolObj->type != GO_TYPE_FUNC_DECL)
					throw(funcHeaderInfo.name + " is already defined in " + pSymbolObj->definedIn());

				CFuncDeclare* pFuncDeclare = new CFuncDeclare(this, funcHeaderInfo.name, type_def_ptr);
				pFuncDeclare->setDefLocation(g_cur_file_name, g_cur_line_no);
				addFuncDeclare(pFuncDeclare);

				TRACE("CClassDef::%s, new function %s\n", __FUNCTION__, funcHeaderInfo.name.c_str());
				CFunction* pFunc = new CFunction(this, funcHeaderInfo.name, type_def_ptr, getFlowTypeByModifierBits(funcHeaderInfo.mod_strings), pFuncDeclare);
				// otherwise, the function should be analyzed on demand, which is not implemented right now
				if (getParent()->getGoType() != GO_TYPE_TEMPLATE || ((CTemplate*)getParent())->m_template_name != m_template_name)
					func_def_list.push_back(TempBlock(pFunc, pBaseClassInitBlock, bracket_block2, pNode));
				//void cdbFuncDefGetMemberInitByIndex(const SourceTreeNode* pRoot, int idx, std::string& name, SourceTreeNode*& pExpr);
				pFunc->setClassAccessModifierType(cam_type);
				addChild(pFunc);
				break;
			}
			break;
		}
		default:
			MY_ASSERT(false);
		}
	}
	MY_ASSERT(ga.isEmpty());

	for (size_t i = 0; i < class_def_list.size(); i++)
	{
		ClassDefBlock& cdb = class_def_list[i];
		cdb.pStatement->analyzeDef(NULL, cdb.pRoot);
	}
	for (size_t i = 0; i < func_def_list.size(); i++)
	{
		TempBlock& tempBlock = func_def_list[i];
		tempBlock.pFunc->analyze(tempBlock.pBaseClassInitBlock, tempBlock.bracket_block, tempBlock.pRoot);
	}
	for (size_t i = 0; i < ret_v.size(); i++)
	{
		SourceTreeNode* pNode = ret_v[i];
		deleteSourceTreeNode(pNode);
	}

	TRACE("STOP ANALYZING CLASS %s\n", getDebugPath().c_str());
}

void CClassDef::addDef(CStatement* pStatement)
{
	MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
	addChild(pStatement);
	m_bDefined = true;
}

bool CClassDef::hasBaseClass(CClassDef* pClassDef)
{
	if (pClassDef->getDebugPath() == getDebugPath())
		return true;

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		TypeDefPointer pTypeDef2 = pair.pTypeDef;
		while (!pTypeDef2->isBaseType() && pTypeDef2->getDepth() == 0)
			pTypeDef2 = pTypeDef2->getBaseType();
		MY_ASSERT(pTypeDef2->getClassDef());
		if (pTypeDef2->getClassDef()->hasBaseClass(pClassDef))
			return true;
	}

	return false;
}

bool CClassDef::hasConstructorOrCanBeAssignedWith(TypeDefPointer pTypeDef)
{
	TRACE("\nCClassDef::hasConstructorOrCanBeAssignedWith, me=%s, className=%s, pTypeDef=%s, ", getDebugPath().c_str(), getClassName().c_str(), pTypeDef->toFullString().c_str());
	if ((pTypeDef->getType() == SEMANTIC_TYPE_CLASS || pTypeDef->getType() == SEMANTIC_TYPE_STRUCT || pTypeDef->getType() == SEMANTIC_TYPE_UNION) && pTypeDef->getClassDef())
	{
		//MY_ASSERT(pTypeDef->getClassDef());
		if (pTypeDef->getClassDef()->getDebugPath() == getDebugPath())
		{
			TRACE("same\n");
			return true;
		}
	}

	std::vector<TypeDefPointer> typeList;
	typeList.push_back(pTypeDef);

	SymbolDefObject* pSymbolObj;
	int maxMatchScore = -1;
	std::vector<CGrammarObject*> matched_v;

	if (!getName().empty())
	{
		pSymbolObj = findSymbol(getClassName(), FIND_SYMBOL_SCOPE_LOCAL);
		if (pSymbolObj)
		{
			MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
			std::vector<CGrammarObject*> func_list;
			std::string my_s = getPath();
			TRACE("checking constructor\n");
			for (size_t i = 0; i < pSymbolObj->children.size(); i++)
			{
				CGrammarObject* pGrammarObj = pSymbolObj->children[i];
				// remove self constructor to avoid recursive calling
				if (pGrammarObj->getGoType() == GO_TYPE_FUNC_DECL)
				{
					CFuncDeclare* pFuncDeclare = (CFuncDeclare*)pGrammarObj;
					if (pFuncDeclare->getType()->getFuncParamCount() > 0)
					{
						CVarDef* pVarDef = pFuncDeclare->getType()->getFuncParamAt(0);
						TypeDefPointer pTypeDef = pVarDef->getType();
						std::string s2 = pTypeDef->toFullString();
						if (pTypeDef->getFullDepth() == 0)
						{
							while (!pTypeDef->isBaseType())
								pTypeDef = pTypeDef->getBaseType();
							if (pTypeDef->getClassDef() == this)
							{
								TRACE("skip self constructor\n");
								continue;
							}
						}
					}
				}
				TRACE("add to constructor func list, \n");
				func_list.push_back(pGrammarObj);
			}
			checkBestMatchedFunc(func_list, typeList, false, maxMatchScore, matched_v);
			if (matched_v.size() == 1)
			{
				TRACE("found a match one\n");
				return true;
			}
			if (matched_v.size() > 1)
			{
				std::string err_s = "call of constructor " + getName() + "(" + pTypeDef->toFullString() + ") is ambiguous. Choices are:\n";
				for (unsigned j = 0; j < matched_v.size(); j++)
					err_s += "   " + matched_v[j]->definedIn() + "\n";
				throw(err_s);
			}
			TRACE("match not found\n");
		}
		else
			TRACE("no constructor found\n");
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		TypeDefPointer pTypeDef2 = pair.pTypeDef;
		while (!pTypeDef2->isBaseType() && pTypeDef2->getDepth() == 0)
			pTypeDef2 = pTypeDef2->getBaseType();
		MY_ASSERT(pTypeDef2->getClassDef());
		TRACE("checking base class %s\n", pTypeDef2->getClassDef()->getDebugPath().c_str());
		if (pTypeDef2->getClassDef()->hasConstructorOrCanBeAssignedWith(pTypeDef))
		{
			TRACE("bingo\n");
			return true;
		}
		TRACE("failed\n");
	}

	return false;
}

SymbolDefObject* CClassDef::findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode)
{
	TRACE("CClassDef::findSymbol %s in %s, scope=%d, mode=%d, ", name.c_str(), getDebugPath().c_str(), scope, mode);

	if (getParent()->getGoType() == GO_TYPE_TEMPLATE && ((CTemplate*)getParent())->getTemplateType() == TEMPLATE_TYPE_CLASS &&
	    !isDefined() && ((CTemplate*)getParent())->isInstancedTemplate())
        analyzeByTemplate();

	if (scope == FIND_SYMBOL_SCOPE_PARENT && (name == m_name || m_template_name == name))
		return getParent()->findSymbol(m_name, FIND_SYMBOL_SCOPE_LOCAL, mode);

	SymbolDefObject* pSymbolObj = NULL;
	if (pSymbolObj = CScope::findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL, mode))
		return pSymbolObj;

	if (scope >= FIND_SYMBOL_SCOPE_SCOPE)
	{
		for (size_t i = 0; i < m_baseTypeList.size(); i++)
		{
			BaseClassCAMPair& pair = m_baseTypeList[i];

			TypeDefPointer pTypeDef2 = pair.pTypeDef;
			while (!pTypeDef2->isBaseType() && pTypeDef2->getDepth() == 0)
				pTypeDef2 = pTypeDef2->getBaseType();

			CClassDef* pBaseClassDef = pTypeDef2->getClassDef();
			MY_ASSERT(pBaseClassDef);

			if (pSymbolObj = pBaseClassDef->findSymbol(name, scope, mode))
				return pSymbolObj;
		}
	}

	if (scope == FIND_SYMBOL_SCOPE_PARENT && m_pParent)
	{
		if (pSymbolObj = m_pParent->findSymbol(name, scope, mode))
			return pSymbolObj;
	}

	//TRACE("not found\n");
	return NULL;
}

bool CClassDef::has_constructor()
{
	if (getType() != SEMANTIC_TYPE_CLASS && getType() != SEMANTIC_TYPE_STRUCT)
		return false;

	std::string constructor_name = getClassName();
	SymbolDefObject* pSymbolObj = findSymbol(constructor_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		if (pSymbolObj->children.size() != 1 || pSymbolObj->getFuncDeclareAt(0)->getDefLineNo() != 0) // getDefLineNo == 0 means it's a default function that doesn't exist in code
			return true;
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		TypeDefPointer pTypeDef = pair.pTypeDef;
		int depth = 0;
		pTypeDef = getRootType(pTypeDef, depth);
		MY_ASSERT(depth == 0);
		MY_ASSERT(pTypeDef->getClassDef());
		if (pTypeDef->getClassDef()->has_constructor())
			return true;
	}

	for (int i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() != GO_TYPE_STATEMENT) // another type is GO_TYPE_FUNCTION
			continue;

		CStatement* pStatement = (CStatement*)pGrammarObj;
		MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
		switch (pStatement->getDefType())
		{
		case DEF_TYPE_EMPTY:
		case DEF_TYPE_PRE_DECL:
		case DEF_TYPE_CLASS_CAM:
		case DEF_TYPE_CLASS_FRIEND:
		case DEF_TYPE_TYPEDEF:
		case DEF_TYPE_EXTERN_TEMPLATE_CLASS:
		case DEF_TYPE_EXTERN_TEMPLATE_FUNC:
		case DEF_TYPE_USING_NAMESPACE:
		case DEF_TYPE_TEMPLATE:
		case DEF_TYPE_FUNC_DECL:
		case DEF_TYPE_FUNC_VAR_DEF:
			break;

		case DEF_TYPE_VAR_DEF:
		case DEF_TYPE_SUPER_TYPE_VAR_DEF:
		{
			if (isInModifiers(pStatement->m_mod_strings, MODBIT_STATIC))
				break;
			for (size_t i = 0; i < pStatement->m_var_list.size(); i++)
			{
				CVarDef* pVarDef = pStatement->m_var_list[i];

				TypeDefPointer pTypeDef = pVarDef->getType();
				int depth = 0;
				pTypeDef = getRootType(pTypeDef, depth);
				if (depth == 0 && pTypeDef->getClassDef() && pTypeDef->getClassDef()->has_constructor())
					return true;
			}
			break;
		}

		default:
			MY_ASSERT(false);
		}
	}

	return false;
}

bool CClassDef::is_abstract()
{
	MY_ASSERT(getType() == SEMANTIC_TYPE_CLASS || getType() == SEMANTIC_TYPE_STRUCT);

	for (int i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
		{
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			switch (pStatement->getDefType())
			{
			case DEF_TYPE_EMPTY:
			case DEF_TYPE_PRE_DECL:
			case DEF_TYPE_CLASS_CAM:
			case DEF_TYPE_CLASS_FRIEND:
			case DEF_TYPE_TYPEDEF:
			case DEF_TYPE_EXTERN_TEMPLATE_CLASS:
			case DEF_TYPE_EXTERN_TEMPLATE_FUNC:
			case DEF_TYPE_USING_NAMESPACE:
			case DEF_TYPE_TEMPLATE:
			case DEF_TYPE_FUNC_VAR_DEF:
			case DEF_TYPE_VAR_DEF:
				break;

			case DEF_TYPE_FUNC_DECL:
				if (pStatement->m_pTypeDef->isPureVirtual())
					return true;
				break;

			default:
				MY_ASSERT(false);
			}
		}
	}

	return true;
}

bool CClassDef::is_empty() // don't have constructor or destructor, don't have protected, private or vitural method
{
	MY_ASSERT(getType() == SEMANTIC_TYPE_CLASS || getType() == SEMANTIC_TYPE_STRUCT);

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		ClassAccessModifierType cam_type = pair.cam_type;
		if (cam_type == CAM_TYPE_NONE)
			cam_type = (getType() == SEMANTIC_TYPE_CLASS ? CAM_TYPE_PRIVATE : CAM_TYPE_PUBLIC);
		if (cam_type != CAM_TYPE_PUBLIC)
			return false;
		TypeDefPointer pTypeDef2 = pair.pTypeDef;
		if (!pTypeDef2->is_empty())
			return false;
	}

	for (int i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
		{
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			switch (pStatement->getDefType())
			{
			case DEF_TYPE_EMPTY:
			case DEF_TYPE_PRE_DECL:
			case DEF_TYPE_CLASS_CAM:
			case DEF_TYPE_CLASS_FRIEND:
			case DEF_TYPE_TYPEDEF:
			case DEF_TYPE_EXTERN_TEMPLATE_CLASS:
			case DEF_TYPE_EXTERN_TEMPLATE_FUNC:
				break;

			case DEF_TYPE_USING_NAMESPACE:
				if (pStatement->m_pGrammarObj->getGoType() == GO_TYPE_VAR_DEF)
					return false;;
				break;

			case DEF_TYPE_TEMPLATE:
				MY_ASSERT(pStatement->m_pTemplate->getTemplateType() == TEMPLATE_TYPE_FUNC);
				break;

			case DEF_TYPE_FUNC_DECL:
				if (pStatement->m_pTypeDef->isVirtual())
					return false;
				break;

			case DEF_TYPE_FUNC_VAR_DEF:
				return false;

			case DEF_TYPE_VAR_DEF:
			{
				if (isInModifiers(pStatement->m_mod_strings, MODBIT_STATIC))
					break;
				for (size_t j = 0; j < pStatement->m_var_list.size(); j++)
				{
					CVarDef* pVarDef = pStatement->m_var_list[j];

					if (pVarDef->getDeclVarNode() && declVarGetBits(pVarDef->getDeclVarNode()) == 0)
						return false;
				}
				break;
			}

			default:
				MY_ASSERT(false);
			}
		}
		else if (pGrammarObj->getGoType() == GO_TYPE_FUNC)
		{
			CFunction* pFunc = (CFunction*)pGrammarObj;
			if (pFunc->getFuncType()->isVirtual())
				return false;
		}
	}

	return true;
}

// Returns true if the type is a class or union with no constructor or private or protected non-static members,
// no base classes, and no virtual functions. See the C++ standard, sections 8.5.1/1, 9/4, and 3.9/10 for more information on PODs.
bool CClassDef::is_pod()
{
	MY_ASSERT(getType() == SEMANTIC_TYPE_CLASS || getType() == SEMANTIC_TYPE_STRUCT || getType() == SEMANTIC_TYPE_UNION);

	if (!m_baseTypeList.empty())
		return false;

	std::string constructor_name = (m_template_name.empty() ? getName() : m_template_name);
	SymbolDefObject* pSymbolObj = findSymbol(constructor_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		return false;
	}
	pSymbolObj = findSymbol("~" + constructor_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		return false;
	}
	for (int i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
		{
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			switch (pStatement->getDefType())
			{
			case DEF_TYPE_EMPTY:
			case DEF_TYPE_PRE_DECL:
			case DEF_TYPE_CLASS_CAM:
			case DEF_TYPE_CLASS_FRIEND:
			case DEF_TYPE_TYPEDEF:
			case DEF_TYPE_EXTERN_TEMPLATE_CLASS:
			case DEF_TYPE_EXTERN_TEMPLATE_FUNC:
				break;

			case DEF_TYPE_USING_NAMESPACE:
				if (pStatement->m_pGrammarObj->getGoType() == GO_TYPE_VAR_DEF && pStatement->getClassAccessModifierType() != CAM_TYPE_PUBLIC)
					return false;;
				break;

			case DEF_TYPE_TEMPLATE:
				MY_ASSERT(pStatement->m_pTemplate->getTemplateType() == TEMPLATE_TYPE_FUNC);
				break;

			case DEF_TYPE_FUNC_DECL:
				if (pStatement->m_pTypeDef->isVirtual())
					return false;
				break;

			case DEF_TYPE_FUNC_VAR_DEF:
			case DEF_TYPE_VAR_DEF:
			{
				if (isInModifiers(pStatement->m_mod_strings, MODBIT_STATIC))
					break;
				if (pStatement->getClassAccessModifierType() != CAM_TYPE_PUBLIC)
					return false;
				break;
			}

			default:
				MY_ASSERT(false);
			}
		}
		else if (pGrammarObj->getGoType() == GO_TYPE_FUNC)
		{
			CFunction* pFunc = (CFunction*)pGrammarObj;
			if (pFunc->getFuncType()->isVirtual())
				return false;
		}
	}

	return true;
}

bool CClassDef::has_nothrow_assign()
{
	SymbolDefObject* pSymbolObj = findSymbol("operator =", FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		for (unsigned i = 0; i < pSymbolObj->children.size(); i++)
		{
			CGrammarObject* pGO = pSymbolObj->children[i];
			if (pGO->getGoType() == GO_TYPE_FUNC_DECL)
			{
				CFuncDeclare* pFuncDeclare = (CFuncDeclare*)pGO;
				TypeDefPointer pFuncTypeDef = pFuncDeclare->getType();
				if (pFuncTypeDef->getFuncParamCount() == 1)
				{
					TypeDefPointer pParamTypeDef = pFuncTypeDef->getFuncParamAt(0)->getType();
					while (pParamTypeDef->getDepth() == 0 && !pParamTypeDef->isBaseType())
						pParamTypeDef = pParamTypeDef->getBaseType();
					if (pParamTypeDef->getType() == SEMANTIC_TYPE_CLASS && pParamTypeDef->getDepth() == 0 && pParamTypeDef->getClassDef() == this)
					{
						if (pFuncTypeDef->getFuncThrow() > 0 || pFuncTypeDef->getFuncThrowTypeNode())
							return false;
					}
				}
			}
		}
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		if (!pair.pTypeDef->has_nothrow_assign())
			return false;
	}

	for (size_t i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
		{
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			if (pStatement->getDefType() == DEF_TYPE_FUNC_DECL)
			{
				if (pStatement->m_pTypeDef->isVirtual())
					return false;
			}
			else if (pStatement->getDefType() == DEF_TYPE_VAR_DEF)
			{
				if (isInModifiers(pStatement->m_mod_strings, MODBIT_STATIC))
					continue;
				TypeDefPointer pTypeDef = pStatement->m_pTypeDef;
				while (pTypeDef->getDepth() == 0 && !pTypeDef->isBaseType())
					pTypeDef = pTypeDef->getBaseType();
				if (pTypeDef->getType() == SEMANTIC_TYPE_CLASS && pTypeDef->getDepth() == 0 && !pTypeDef->has_nothrow_assign())
					return false;
			}
		}
		else if (pGrammarObj->getGoType() == GO_TYPE_FUNC)
		{
			CFunction* pFunc = (CFunction*)pGrammarObj;
			if (pFunc->getFuncType()->isVirtual())
				return false;
		}
	}

	return true;
}

bool CClassDef::has_nothrow_copy()
{
	std::string constructor_name = (m_template_name.empty() ? getName() : m_template_name);
	SymbolDefObject* pSymbolObj = findSymbol(constructor_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		for (unsigned i = 0; i < pSymbolObj->children.size(); i++)
		{
			CGrammarObject* pGO = pSymbolObj->children[i];
			if (pGO->getGoType() == GO_TYPE_FUNC_DECL)
			{
				CFuncDeclare* pFuncDeclare = (CFuncDeclare*)pGO;
				TypeDefPointer pFuncTypeDef = pFuncDeclare->getType();
				if (pFuncTypeDef->getFuncParamCount() == 1)
				{
					TypeDefPointer pParamTypeDef = pFuncTypeDef->getFuncParamAt(0)->getType();
					while (pParamTypeDef->getDepth() == 0 && !pParamTypeDef->isBaseType())
						pParamTypeDef = pParamTypeDef->getBaseType();
					if (pParamTypeDef->getType() == SEMANTIC_TYPE_CLASS && pParamTypeDef->getDepth() == 0 && pParamTypeDef->getClassDef() == this)
					{
						if (pFuncTypeDef->getFuncThrow() > 0 || pFuncTypeDef->getFuncThrowTypeNode())
							return false;
					}
				}
			}
		}
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		if (!pair.pTypeDef->has_nothrow_copy())
			return false;
	}

	return true;
}

bool CClassDef::has_trivial_assign()
{
	SymbolDefObject* pSymbolObj = findSymbol("operator =", FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		for (unsigned i = 0; i < pSymbolObj->children.size(); i++)
		{
			CGrammarObject* pGO = pSymbolObj->children[i];
			if (pGO->getGoType() == GO_TYPE_FUNC_DECL)
			{
				CFuncDeclare* pFuncDeclare = (CFuncDeclare*)pGO;
				TypeDefPointer pTypeDef = pFuncDeclare->getType();
				if (pTypeDef->getFuncParamCount() == 1)
				{
					pTypeDef = pTypeDef->getFuncParamAt(0)->getType();
					while (pTypeDef->getDepth() == 0 && !pTypeDef->isBaseType())
						pTypeDef = pTypeDef->getBaseType();
					if (pTypeDef->getType() == SEMANTIC_TYPE_CLASS && pTypeDef->getDepth() == 0 && pTypeDef->getClassDef() == this)
						return false;
				}
			}
		}
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		if (!pair.pTypeDef->has_trivial_assign())
			return false;
	}

	for (int i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
		{
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			if (pStatement->getDefType() == DEF_TYPE_FUNC_DECL)
			{
				if (pStatement->m_pTypeDef->isVirtual())
					return false;
			}
			else if (pStatement->getDefType() == DEF_TYPE_VAR_DEF)
			{
				if (isInModifiers(pStatement->m_mod_strings, MODBIT_STATIC))
					continue;
				TypeDefPointer pTypeDef = pStatement->m_pTypeDef;
				while (pTypeDef->getDepth() == 0 && !pTypeDef->isBaseType())
					pTypeDef = pTypeDef->getBaseType();
				if (pTypeDef->getType() == SEMANTIC_TYPE_CLASS && pTypeDef->getDepth() == 0 && !pTypeDef->has_trivial_assign())
					return false;
			}
		}
		else if (pGrammarObj->getGoType() == GO_TYPE_FUNC)
		{
			CFunction* pFunc = (CFunction*)pGrammarObj;
			if (pFunc->getFuncType()->isVirtual())
				return false;
		}
	}

	return true;
}

bool CClassDef::has_trivial_copy()
{
	std::string constructor_name = (m_template_name.empty() ? getName() : m_template_name);
	SymbolDefObject* pSymbolObj = findSymbol(constructor_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		for (unsigned i = 0; i < pSymbolObj->children.size(); i++)
		{
			CGrammarObject* pGO = pSymbolObj->children[i];
			if (pGO->getGoType() == GO_TYPE_FUNC_DECL)
			{
				CFuncDeclare* pFuncDeclare = (CFuncDeclare*)pGO;
				TypeDefPointer pTypeDef = pFuncDeclare->getType();
				if (pTypeDef->getFuncParamCount() == 1)
				{
					pTypeDef = pTypeDef->getFuncParamAt(0)->getType();
					while (pTypeDef->getDepth() == 0 && !pTypeDef->isBaseType())
						pTypeDef = pTypeDef->getBaseType();
					if (pTypeDef->getType() == SEMANTIC_TYPE_CLASS && pTypeDef->getDepth() == 0 && pTypeDef->getClassDef() == this)
						return false;
				}
			}
		}
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		if (!pair.pTypeDef->has_trivial_copy())
			return false;
	}

	for (size_t i = 0; i < getChildrenCount(); i++)
	{
		CScope* pGrammarObj = getChildAt(i);
		if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
		{
			CStatement* pStatement = (CStatement*)pGrammarObj;
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			if (pStatement->getDefType() == DEF_TYPE_FUNC_DECL)
			{
				if (pStatement->m_pTypeDef->isVirtual())
					return false;
			}
		}
		else if (pGrammarObj->getGoType() == GO_TYPE_FUNC)
		{
			CFunction* pFunc = (CFunction*)pGrammarObj;
			if (pFunc->getFuncType()->isVirtual())
				return false;
		}
	}

	return true;
}

bool CClassDef::has_trivial_destructor()
{
	std::string constructor_name = (m_template_name.empty() ? getName() : m_template_name);
	SymbolDefObject* pSymbolObj = findSymbol("~" + constructor_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		return false;
	}

	for (size_t i = 0; i < m_baseTypeList.size(); i++)
	{
		BaseClassCAMPair& pair = m_baseTypeList[i];

		if (!pair.pTypeDef->has_trivial_destructor())
			return false;
	}

	return true;
}

std::string CClassDef::toString(int depth)
{
	std::string ret_s;

	ret_s += /*getSemanticTypeName(m_type) + " " + */combineStrings(m_mod_strings) + getName(); // the prefix will be added by typedef

	for (unsigned i = 0; i < m_baseTypeList.size(); i++)
	{
		if (i == 0)
			ret_s += " : ";
		else
			ret_s += ", ";

		BaseClassCAMPair& baseType = m_baseTypeList[i];
		if (baseType.bVirtual)
			ret_s += "virtual ";
		ret_s += displayCAMType(baseType.cam_type) + " " + displaySourceTreeUserDefType(baseType.pTypeNode);
	}
	ret_s += "\n" + printTabs(depth) + "{\n";

	switch (m_type)
	{
	case SEMANTIC_TYPE_ENUM:
	{
		for (int i = 0; i < getChildrenCount(); i++)
		{
			CStatement* pStatement = (CStatement*)getChildAt(i);
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			MY_ASSERT(pStatement->getDefType() == DEF_TYPE_VAR_DEF);
			CVarDef* pVarDef = pStatement->getVarAt(0);
			ret_s += printTabs(depth + 1) + pVarDef->toString();
			if (i < getChildrenCount() - 1 || m_enum_has_last_comma)
				ret_s += ",";
			ret_s += "\n";
		}
		break;
	}
	case SEMANTIC_TYPE_UNION:
	{
		for (int i = 0; i < getChildrenCount(); i++)
		{
			CStatement* pStatement = (CStatement*)getChildAt(i);
			MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
			ret_s += pStatement->toString(depth + 1);
		}
		break;
	}
	case SEMANTIC_TYPE_STRUCT:
	case SEMANTIC_TYPE_CLASS:
	{
		enterScope(this);
		for (int i = 0; i < getChildrenCount(); i++)
		{
			CScope* pGrammarObj = getChildAt(i);
			if (pGrammarObj->getGoType() == GO_TYPE_STATEMENT)
			{
				CStatement* pStatement = (CStatement*)pGrammarObj;
				MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);
				ret_s += pStatement->toString(depth + 1);
			}
			else
			{
				MY_ASSERT(pGrammarObj->getGoType() == GO_TYPE_FUNC);
				CFunction* pFunc = (CFunction*)pGrammarObj;
				ret_s += pFunc->toString(depth + 1) + "\n";
			}
		}
		leaveScope();
		break;
	}
	case SEMANTIC_TYPE_TYPENAME:
		ret_s = "typename " + m_name;
		break;
	default:
		MY_ASSERT(false);
	}

	ret_s += printTabs(depth) + "}";

//printf("addDef, this=%ld, size=%d, return %s\n", this, getChildrenCount(), ret_s.c_str());
	return ret_s;
}

CTemplate::CTemplate(CScope* pParent, TemplateType type, const std::string& name) : CScope(pParent)
{
	setRealScope(true);
	m_bDefined = false;
	m_nTemplateType = type;
	m_name = name;
	m_template_name = name;
	m_data_type = SEMANTIC_TYPE_CLASS; // class or struct
	m_pFuncReturnExtendedTypeNode = NULL;
	m_func_hasVArgs = false;
}

CTemplate::~CTemplate()
{
	clear();
}

std::string CTemplate::getDebugName()
{
	return getName() + ":" + ltoa(m_nTemplateType);
}

void CTemplate::clear()
{
	for (size_t i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam& param = m_typeParams[i];

		deleteSourceTreeNode(param.pDefaultNode);
	}
	m_typeParams.clear();

	deleteSourceTreeNode(m_pFuncReturnExtendedTypeNode);
	m_pFuncReturnExtendedTypeNode = NULL;

	for (size_t i = 0; i < m_funcParams.size(); i++)
	{
		FuncParamItem& param = m_funcParams[i];

		deleteSourceTreeNode(param.pTypeNode);
		deleteSourceTreeNode(param.pDeclVarNode);
		deleteSourceTreeNode(param.pInitExprNode);
	}
	m_funcParams.clear();
}

void CTemplate::clearTypeAndVarDefs()
{
	for (SymbolDefMap::iterator it = m_symbol_map.begin(); it != m_symbol_map.end();)
	{
		if (it->second.type == GO_TYPE_TYPEDEF)
		{
			TypeDefPointer pTypeDef = it->second.getTypeDef();
			if (!(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef() && pTypeDef->getBaseType()->getClassDef()->m_template_name == getName()))
			{
				m_symbol_map.erase(it++);
				continue;
			}
		}
		if (it->second.type == GO_TYPE_VAR_DEF)
		{
			m_symbol_map.erase(it++);
			continue;
		}
		it++;
	}
}

bool CTemplate::inParamTypeNames(const std::string s)
{
	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam& param = m_typeParams[i];
		if (param.type == TEMPLATE_PARAM_TYPE_DATA && param.name == s)
			return true;
	}

	return false;
}

CTemplate::TypeParam CTemplate::readTemplateTypeParam(const SourceTreeNode* pChild)
{
	std::string typeName;
	SourceTreeVector templateTypeParams;
	SourceTreeNode* pDefaultNode;
	bool bClass, bHasTypename, bDataOrFuncType;
	int header_type;
	templateTypeDefGetInfo(pChild, header_type, templateTypeParams, bClass, typeName, bDataOrFuncType, bHasTypename, pDefaultNode);

	TypeParam param;
	param.name = typeName;
	param.bHasDefault = param.bHasTypename = false;
	param.pDefaultNode = param.pTypeNode = NULL;
	if (header_type == 0)
	{
		param.type = TEMPLATE_PARAM_TYPE_DATA;
		param.pTypeNode = dupSourceTreeNode(pChild);
		param.bClass = bClass;
		param.bHasTypename = bHasTypename;
	}
	else
	{
		param.type = TEMPLATE_PARAM_TYPE_VALUE;
		MY_ASSERT(templateTypeParams.size() == 1);
		param.pNumValueType = getTypeDefByTypeNode(templateTypeParams[0]);
		MY_ASSERT(param.pNumValueType);
		TypeDefPointer pTypeDef = param.pNumValueType;
		std::string s = pTypeDef->toFullString();
		CTypeDef* pTD = pTypeDef.get();
		while (!pTypeDef->isBaseType())
		{
			MY_ASSERT(pTypeDef->getDepth() == 0);
			pTypeDef = pTypeDef->getBaseType();
		}
		//MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_BASIC); could be a typename
	}
	if (pDefaultNode)
	{
		param.bHasDefault = true;
		param.bDefaultDataOrFuncType = bDataOrFuncType;
		param.pDefaultNode = dupSourceTreeNode(pDefaultNode);
	}

	return param;
}

void CTemplate::createParamTypeFromTemplateHeader(const SourceTreeNode* pRoot)
{
	std::string typeName;
	SourceTreeVector templateTypeParams;
	SourceTreeNode* pDefaultNode;
	bool bClass, bHasTypename, bDataOrFuncType;
	int header_type;
	templateTypeDefGetInfo(pRoot, header_type, templateTypeParams, bClass, typeName, bDataOrFuncType, bHasTypename, pDefaultNode);
	TRACE("CTemplate::%s, v_size=%lu\n", __FUNCTION__, templateTypeParams.size());
	if (templateTypeParams.empty())
		createClassAsChild(typeName, SEMANTIC_TYPE_TYPENAME);
	else
	{
		CTemplate* pTemplate = new CTemplate(this, TEMPLATE_TYPE_CLASS, typeName);
		pTemplate->m_data_type = (bClass ? SEMANTIC_TYPE_CLASS : SEMANTIC_TYPE_STRUCT);

		for (unsigned i = 0; i < templateTypeParams.size(); i++)
		{
			SourceTreeNode* pNode = templateTypeParams[i];
			pTemplate->createParamTypeFromTemplateHeader(pNode);
		}

		addTemplate(pTemplate);
	}
}

void CTemplate::addHeaderTypeDefs(const std::vector<void*>& header_types)
{
	for (size_t i = 0; i < header_types.size(); i++)
	{
		CGrammarAnalyzer ga2;
		ga2.initWithBlocks(getRealScope(), header_types[i]);

		SourceTreeNode* pTypeDefsNode = ga2.getBlock();
		//MY_ASSERT(pTypeDefsNode != NULL); // template header type could be empty
		MY_ASSERT(ga2.isEmpty());

		if (pTypeDefsNode)
		{
			SourceTreeVector def_v = templateTypeDefsGetList(pTypeDefsNode);
			for (size_t j = 0; j < def_v.size(); j++)
			{
				SourceTreeNode* pHeaderTypeNode = def_v[j];

				TypeParam param = readTemplateTypeParam(pHeaderTypeNode);
				if (!param.name.empty())
				{
					if (param.type == TEMPLATE_PARAM_TYPE_DATA)
						createParamTypeFromTemplateHeader(param.pTypeNode);
					else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
						addVarDef(new CVarDef(this, param.name, param.pNumValueType, NULL));
					else
						MY_ASSERT(false);
				}
	//			if (param.type == TEMPLATE_PARAM_TYPE_DATA && param.pDefaultNode)
	//				displaySourceTreeExtendedTypeVar(param.pDefaultNode);
				m_typeParams.push_back(param);
			}
		}
	}
}

void CTemplate::readTemplateHeaderIntoTypeParams(const std::vector<void*>& header_types)
{
	MY_ASSERT(m_typeParams.empty());

	CGrammarAnalyzer ga2;
	ga2.initWithBlocks(getRealScope(), header_types.back());

	SourceTreeNode* pTypeDefsNode = ga2.getBlock();
	//MY_ASSERT(pTypeDefsNode != NULL);
	MY_ASSERT(ga2.isEmpty());
	SourceTreeVector def_v = templateTypeDefsGetList(pTypeDefsNode);
	for (size_t i = 0; i < def_v.size(); i++)
	{
		SourceTreeNode* pHeaderTypeNode = def_v[i];

		TypeParam param = readTemplateTypeParam(pHeaderTypeNode);
		if (!param.name.empty())
		{
			if (param.type == TEMPLATE_PARAM_TYPE_DATA)
				createParamTypeFromTemplateHeader(param.pTypeNode);
			else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
				addVarDef(new CVarDef(this, param.name, param.pNumValueType, NULL));
			else
				MY_ASSERT(false);
		}
		//if (param.type == TEMPLATE_PARAM_TYPE_DATA && param.pDefaultNode)
		//	displaySourceTreeExtendedTypeVar(param.pDefaultNode);
		m_typeParams.push_back(param);
	}
	MY_ASSERT(ga2.isEmpty());
}

void CTemplate::analyzeFunc(const std::vector<void*>& header_types, const SourceTreeNode* pBodyNode)
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_FUNC);

	SourceTreeNode* pFuncHeaderNode;
	void* pBaseClassInitBlock, *body_data;
	templateBodyFuncGetInfo(pBodyNode, pFuncHeaderNode, pBaseClassInitBlock, body_data);

	if (body_data == NULL)
	{
		MY_ASSERT(m_typeParams.empty());
	}
	else
		clear();

	readTemplateHeaderIntoTypeParams(header_types);

	GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pFuncHeaderNode);
	m_mod_strings = funcHeaderInfo.mod_strings;
	m_mod2_strings = funcHeaderInfo.mod2_strings;
	m_mod3_strings = funcHeaderInfo.mod3_strings;
	m_mod4_strings = funcHeaderInfo.mod4_strings;
	if (funcHeaderInfo.bThrow)
	{
		m_throw_string = " throw(";
		if (funcHeaderInfo.bThrow == 2)
			m_throw_string += displaySourceTreeType(funcHeaderInfo.pThrowTypeNode);
		else if (funcHeaderInfo.bThrow == 3)
			m_throw_string += "...";
		m_throw_string += ")";
	}

	MY_ASSERT(m_name == funcHeaderInfo.name);

	bool bNeedToDeleteReturnExtendedType = false;
	/*if (funcHeaderInfo.nDataMemberPointerDepth > 0)
	{
		//MY_ASSERT(twn.getDepth() == 1 && !twn.hasRootSign());
		funcHeaderInfo.scope.addScope(""); // typeCreateDmp() requires twn to have an extra token. will be deleted later
		SourceTreeNode* pTypeNode = typeCreateDmp(dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType), funcHeaderInfo.scope);
		funcHeaderInfo.pReturnExtendedType = extendedTypeCreateFromType(pTypeNode);
		for (int i = 1; i < funcHeaderInfo.nDataMemberPointerDepth; i++)
			extendedTypeAddModifier(funcHeaderInfo.pReturnExtendedType, DVMOD_TYPE_POINTER);
		funcHeaderInfo.scope.resize(0);
		bNeedToDeleteReturnExtendedType = true;
	}*/

	m_pFuncReturnExtendedTypeNode = dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType);

	if (bNeedToDeleteReturnExtendedType)
	{
		deleteSourceTreeNode(funcHeaderInfo.pReturnExtendedType);
		funcHeaderInfo.pReturnExtendedType = NULL;
	}
	//std::string s = displaySourceTreeExtendedType(m_pFuncReturnExtendedTypeNode);

	TRACE("START ANALYZING FUNC TEMPLATE %s\n", getDebugPath().c_str());
	CGrammarAnalyzer ga;
	ga.initWithBlocks(getRealScope(), funcHeaderInfo.params_block);
	SourceTreeNode* pFuncParamsNode = ga.getBlock();
	SourceTreeVector paramList = funcParamsGetList(pFuncParamsNode);
	int sz = paramList.size();
	MY_ASSERT(ga.isEmpty());
	for (unsigned i = 0; i < paramList.size(); i++)
	{
		FuncParamType param_type;
		StringVector mod_strings;
		SourceTreeNode* pTypeNode, *pDeclVarNode;
		void *pInitExprBlock;
		funcParamGetInfo(paramList[i], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);

		SourceTreeNode* pInitExprNode = NULL;
		if (pInitExprBlock)
		{
			CGrammarAnalyzer ga2;
			ga2.initWithBlocks(getRealScope(), pInitExprBlock);
			pInitExprNode = ga2.getBlock();
			MY_ASSERT(ga2.isEmpty());
		}

		FuncParamItem item;
		item.param_type = param_type;
		//MY_ASSERT(param_type == FUNC_PARAM_TYPE_REGULAR);
		item.mod_strings = mod_strings;

		item.pTypeNode = dupSourceTreeNode(pTypeNode);
		//std::string s = displaySourceTreeType(item.pTypeNode);

		item.pDeclVarNode = dupSourceTreeNode(pDeclVarNode);
		//s = displaySourceTreeDeclVar(item.pDeclVarNode);
		item.pInitExprNode = pInitExprNode;
		//if (item.pInitExprNode)
		//	s = displaySourceTreeExpr(item.pInitExprNode);

		m_funcParams.push_back(item);
	}
	m_func_hasVArgs = funcParamsHasVArgs(pFuncParamsNode);
	deleteSourceTreeNode(pFuncParamsNode);
	TRACE("STOP ANALYZING TEMPLATE %s\n", getDebugPath().c_str());

	if (!body_data)
		return;

	m_func_base_init_sv = CGrammarAnalyzer::bracketBlockGetTokens(pBaseClassInitBlock);
	m_body_sv = CGrammarAnalyzer::bracketBlockGetTokens(body_data);
}

// return -1 means failed. return number of params that not matching exactly
int CTemplate::funcCheckFitForTypeList(const TypeDefVector& typeList, TemplateResolvedDefParamVector& resolvedDefParams)
{
	MY_ASSERT(getTemplateType() == TEMPLATE_TYPE_FUNC);

	if (typeList.size() > m_funcParams.size())
		return -1;

	int ret_n = 0;
	resolvedDefParams.clear();

	unsigned i;
	for (i = 0; i < m_funcParams.size(); i++)
	{
		FuncParamItem& item = m_funcParams[i];
		if (i < typeList.size())
		{
			TypeDefPointer pTypeDef = typeList[i];

			if (item.pTypeDef)
			{
				MY_ASSERT(false);
				if (item.pTypeDef->toFullString() != pTypeDef->toFullString())
					return -1;
			}
			else if (item.param_type == FUNC_PARAM_TYPE_REGULAR)
			{
				SourceTreeNode* pExtendedType = extendedTypeCreateFromType(dupSourceTreeNode(item.pTypeNode), item.pDeclVarNode);
				int n = resolveParamType(extendedTypeVarCreateFromExtendedType(pExtendedType), pTypeDef, resolvedDefParams);
				if (n < 0)
					return -1;
				ret_n += n;
			}
			else
			{
				TRACE("CTemplate::%s, compare func type, paramType='%s', realType='%s'\n", __FUNCTION__, displaySourceTreeFuncType(item.pTypeNode).c_str(), pTypeDef->toFullString().c_str());
				MY_ASSERT(false);
			}
		}
		else
		{
			if (item.pInitExprNode == NULL)
				return -1;
		}
	}

	return ret_n;
}

TypeDefPointer CTemplate::funcGetInstance(const TypeDefVector& typeList, const TemplateResolvedDefParamVector& resolvedDefParams)
{
	std::string type_str;
	for (size_t i = 0; i < resolvedDefParams.size(); i++)
	{
		if (!type_str.empty())
			type_str += ",";
		MY_ASSERT(resolvedDefParams[i].pTypeDef);
		type_str += resolvedDefParams[i].pTypeDef->toFullString();
	}
	type_str += m_name + "<" + type_str + " >";

	SymbolDefObject* pSymbolObj = findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_FUNC_DECL);
		MY_ASSERT(pSymbolObj->children[0]->getGoType() == GO_TYPE_TEMPLATE);
		CTemplate* pInstancedTemplate = (CTemplate*)pSymbolObj->getFuncDeclareAt(0);
		pSymbolObj = pInstancedTemplate->findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
		MY_ASSERT(pSymbolObj);
		MY_ASSERT(pSymbolObj->type == GO_TYPE_TYPEDEF);
		TypeDefPointer pTypeDef = pSymbolObj->getTypeDef();
		MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_FUNC);
		return pTypeDef;
	}

	TRACE("instance func template for :%s\n", type_str.c_str());

	CTemplate* pInstancedTemplate = duplicateAsChild(type_str);
	for (size_t i = 0; i < resolvedDefParams.size(); i++)
	{
		if (resolvedDefParams[i].pTypeDef)
			pInstancedTemplate->addTypeDef(TypeDefPointer(new CTypeDef(this, resolvedDefParams[i].typeName, resolvedDefParams[i].pTypeDef, 0)));
		//else
		//	pInstancedTemplate->addVarDef(new CVarDef(this, resolvedDefParams[i].typeName, g_type_def_int, NULL));
	}
	addTemplate(pInstancedTemplate);
	pInstancedTemplate->setResolvedDefParams(resolvedDefParams);

	TypeDefPointer pFuncType = TypeDefPointer(new CTypeDef(pInstancedTemplate, type_str, SEMANTIC_TYPE_FUNC, pInstancedTemplate->getTypeDefByExtendedTypeNode(m_pFuncReturnExtendedTypeNode), 0));
	pFuncType->setDefLocation(g_cur_file_name, g_cur_line_no);
	for (unsigned i = 0; i < m_funcParams.size(); i++)
	{
		FuncParamItem& item = m_funcParams[i];
		switch (item.param_type)
		{
		case FUNC_PARAM_TYPE_REGULAR:
		{
			TypeDefPointer pTypeDef = item.pTypeDef;
			if (!pTypeDef)
				pTypeDef = pInstancedTemplate->getTypeDefByDeclVarNode(getTypeDefByTypeNode(item.pTypeNode), item.pDeclVarNode);

			CVarDef* pVarDef = new CVarDef(pInstancedTemplate, declVarGetName(item.pDeclVarNode), pTypeDef, NULL, (item.pInitExprNode ? new CExpr(pInstancedTemplate, item.pInitExprNode) : NULL));
			pFuncType->addFuncParam(pVarDef);
			break;
		}
		case FUNC_PARAM_TYPE_FUNC:
		{
			TypeDefPointer pTypeDef = pInstancedTemplate->getTypeDefByFuncTypeNode(item.pTypeNode);
			pTypeDef->addFuncParam(new CVarDef(pInstancedTemplate, "", pTypeDef));
			break;
		}
		default:
			MY_ASSERT(false);
		}
	}
	if (m_func_hasVArgs)
		pFuncType->setHasVArgs(true);

	//CFunction* pFunc = new CFunction(this, type_str, pFuncType, FLOW_TYPE_NONE, new CFuncDeclare(this, type_str, pFuncType));
	pInstancedTemplate->addTypeDef(pFuncType);

	//pFunc->analyze(bracket_block, pRoot, memberInitCount);

	return pFuncType;
}

void CTemplate::addSpecializedTemplate(CTemplate* pTemplate)
{
	SymbolDefObject sdo;
	sdo.type = GO_TYPE_TEMPLATE;
	sdo.children.push_back(pTemplate);
	m_specializeDefList.push_back(sdo);

	/*for (int i = 0; i < pTemplate->m_typeParams.size(); i++)
	{
		TypeParam param = pTemplate->m_typeParams[i];

		if (!param.name.empty())
		{
			if (param.type == TEMPLATE_PARAM_TYPE_DATA)
				pTemplate->createClassAsChild(param.name, SEMANTIC_TYPE_TYPENAME);
			else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
				pTemplate->addVarDef(new CVarDef(this, param.name, param.pNumValueType, NULL));
		}
	}*/
}

CTemplate* CTemplate::findSpecializedTemplateByUniqueId(const std::string& uniqueId)
{
	// check whether this specialization has been defined before
	for (size_t i = 0; i < m_specializeDefList.size(); i++)
	{
		CTemplate* pTemplate = m_specializeDefList[i].getTemplateAt(0);
		if (pTemplate->m_uniqueId == uniqueId)
			return pTemplate;
	}

	return NULL;
}

/*
 * there can be only one template for each name, whose specializedTypeCount is 0. for all others, whose specializedTypeCount is !0, are it's specialized templates
 * specialized template's specializedTypeCount must be equal to the main template's typeCount.
 */
void CTemplate::analyzeBaseClass(const std::vector<void*>& header_types, const SourceTreeNode* pBodyNode)
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_CLASS);

	int specializedTypeCount;
	CSUType csu_type;
	TokenWithNamespace twn;
	void* body_data, *pBaseClassDefsBlock;
	templateBodyClassGetInfo(pBodyNode, csu_type, twn, specializedTypeCount, pBaseClassDefsBlock, body_data);
	MY_ASSERT(twn.getLastToken() == m_name);

	MY_ASSERT(header_types.size() == 1);
	MY_ASSERT(specializedTypeCount == 0); // not a specialized def

	TRACE("CTemplate::%s, root template, path=%s\n", __FUNCTION__, getDebugPath().c_str());
	if (blockDataGetTokens(header_types.back()).empty())
		throw("Type parameters cannot be empty in a template definition");

	MY_ASSERT(csu_type != CSU_TYPE_ENUM);
	SemanticDataType data_type = getSemanticTypeFromCSUType(csu_type);
	m_data_type = data_type;

	// this API might be called more than once with different declarations. so need to clear all symbols and types
	readTemplateHeaderIntoTypeParams(header_types);
	TRACE("CTemplate::%s, typeParam count=%lu\n", __FUNCTION__, m_typeParams.size());

	if (body_data)
		saveClassBody(pBaseClassDefsBlock, body_data);
}

CTemplate* CTemplate::analyzeSpecializedClass(const std::vector<void*>& header_types, const SourceTreeNode* pBodyNode)
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_CLASS);

	int specializedTypeCount;
	CSUType csu_type;
	TokenWithNamespace twn;
	void* body_data, *pBaseClassDefsBlock;
	templateBodyClassGetInfo(pBodyNode, csu_type, twn, specializedTypeCount, pBaseClassDefsBlock, body_data);
	MY_ASSERT(twn.getLastToken() == m_name);

	MY_ASSERT(header_types.size() == 1);
	MY_ASSERT(specializedTypeCount != 0);

	// for specialized template defs
	TRACE("CTemplate::%s, specialized template, path=%s\n", __FUNCTION__, getDebugPath().c_str());
	// seems STRUCT and CLASS can be mixed
	//if (m_data_type != getSemanticTypeFromCSUType(csu_type))
	//	throw(std::string("template ") + m_name + " is defined as a " + getSemanticTypeName(m_data_type) + " that different from here.");

	if (specializedTypeCount > (int)m_typeParams.size())
		throw(std::string("It has ") + ltoa(specializedTypeCount) + " specialized type parameters but this template only accept " + ltoa(m_typeParams.size()));

	CTemplate* pSpecializedTemplate = new CTemplate(this, m_nTemplateType, m_name);
	TRACE("\nCREATING sepcialized TEMPLATE %s\n", pSpecializedTemplate->getDebugPath().c_str());
	pSpecializedTemplate->readTemplateHeaderIntoTypeParams(header_types); // sometimes typeParam defines different than the root one

	// composing uniqueId, replace typeNames with standard names.
	// check whether it's already defined by checking uniqueId.
	std::string uniqueId;
	std::map<std::string, std::string> typeNameMap;
	int nameSeq = 1;
	std::vector<TypeParam> typeParams, specializedTypeParams;

	CGrammarAnalyzer ga2;
	ga2.initWithTokens(getRealScope(), "template_type_def", blockDataGetTokens(header_types.back()), ",");
	while (true)
	{
		SourceTreeNode* pHeaderTypeNode = ga2.getBlock();
		if (pHeaderTypeNode == NULL)
			break;
		TypeParam param = readTemplateTypeParam(pHeaderTypeNode);

		std::string newName;
		switch (param.type)
		{
		case TEMPLATE_PARAM_TYPE_DATA:
			newName = std::string("_template_data_") + ltoa(nameSeq++);
			break;
		case TEMPLATE_PARAM_TYPE_FUNC:
			newName = std::string("_template_func_") + ltoa(nameSeq++);
			break;
		case TEMPLATE_PARAM_TYPE_VALUE:
			newName = std::string("_template_value_") + ltoa(nameSeq++);
			break;
		default:
			MY_ASSERT(false);
		}
		MY_ASSERT(typeNameMap.find(param.name) == typeNameMap.end()); // is there any exception?
		typeNameMap[param.name] = newName;
		typeParams.push_back(param);
	}
	MY_ASSERT(ga2.isEmpty());

	size_t i;
	for (i = 0; i < (size_t)specializedTypeCount; i++)
	{
		const TypeParam& typeParam = m_typeParams[i];

		TemplateParamType paramType;
		SourceTreeNode* pChildNode;
		templateBodyClassGetSpecializedTypeByIndex(pBodyNode, i, paramType, pChildNode);

		std::string s;
		if (typeParam.type == TEMPLATE_PARAM_TYPE_DATA)
		{
			if (paramType == TEMPLATE_PARAM_TYPE_VALUE)
				throw(std::string("The ") + ltoa(i) + " specialized type parameter's type is different from the one defined previously");
			SourceTreeNode* pDup = dupSourceTreeNode(pChildNode);
			if (paramType == TEMPLATE_PARAM_TYPE_DATA)
			{
				extendedTypeVarReplaceTypeNameIfAny(pDup, typeNameMap);
				s = displaySourceTreeExtendedTypeVar(pDup);
			}
			else
			{
				funcTypeReplaceTypeNameIfAny(pDup, typeNameMap);
				s = displaySourceTreeFuncType(pDup);
			}
			deleteSourceTreeNode(pDup);
		}
		else if (typeParam.type == TEMPLATE_PARAM_TYPE_FUNC)
		{
			if (paramType != TEMPLATE_PARAM_TYPE_FUNC)
				throw(std::string("The ") + ltoa(i) + " specialized type parameter's type is different from the one defined previously");
			SourceTreeNode* pDup = dupSourceTreeNode(pChildNode);
			funcTypeReplaceTypeNameIfAny(pDup, typeNameMap);
			s = displaySourceTreeFuncType(pDup);
			deleteSourceTreeNode(pDup);
		}
		else
		{
			if (paramType != TEMPLATE_PARAM_TYPE_VALUE)
				throw(std::string("The ") + ltoa(i) + " specialized type parameter's type is different from the one defined previously");
			SourceTreeNode* pDup = dupSourceTreeNode(pChildNode);
			exprReplaceTypeNameIfAny(pDup, typeNameMap);
			s = displaySourceTreeExpr(pDup);
			deleteSourceTreeNode(pDup);
		}

		if (!uniqueId.empty())
			uniqueId += ", ";
		uniqueId += s;

		TypeParam param;
		param.name = typeParam.name;
		param.type = paramType;
		param.bHasDefault = false;
		param.pDefaultNode = dupSourceTreeNode(pChildNode);
		specializedTypeParams.push_back(param);
	}

	for (; i < m_typeParams.size(); i++)
	{
		const TypeParam& typeParam = m_typeParams[i];

		if (!typeParam.bHasDefault)
			throw(std::string("This specialized template has less parameter than needed, cnt=") + ltoa(specializedTypeCount) + std::string(", i=") + ltoa(i));

		std::string s;
		if (typeParam.type == TEMPLATE_PARAM_TYPE_DATA)
		{
			SourceTreeNode* pDup = dupSourceTreeNode(typeParam.pDefaultNode);
			extendedTypeVarReplaceTypeNameIfAny(pDup, typeNameMap);
			s = displaySourceTreeExtendedTypeVar(pDup);
			deleteSourceTreeNode(pDup);
		}
		else if (typeParam.type == TEMPLATE_PARAM_TYPE_FUNC)
		{
			SourceTreeNode* pDup = dupSourceTreeNode(typeParam.pDefaultNode);
			funcTypeReplaceTypeNameIfAny(pDup, typeNameMap);
			s = displaySourceTreeFuncType(pDup);
			deleteSourceTreeNode(pDup);
		}
		else
		{
			MY_ASSERT(typeParam.type == TEMPLATE_PARAM_TYPE_VALUE);
			SourceTreeNode* pDup = dupSourceTreeNode(typeParam.pDefaultNode);
			exprReplaceTypeNameIfAny(pDup, typeNameMap);
			s = displaySourceTreeExpr(pDup);
			deleteSourceTreeNode(pDup);
		}

		if (!uniqueId.empty())
			uniqueId += ", ";
		uniqueId += s;

		TypeParam param;
		param.name = typeParam.name;
		param.type = typeParam.type;
		param.bHasDefault = false;
		param.pDefaultNode = dupSourceTreeNode(typeParam.pDefaultNode);
		specializedTypeParams.push_back(param);
	}

	pSpecializedTemplate->m_data_type = m_data_type;
	//pSpecializedTemplate->m_typeParams = typeParams;
	pSpecializedTemplate->m_specializedTypeParams = specializedTypeParams;
	pSpecializedTemplate->m_uniqueId = uniqueId;
	if (body_data)
		pSpecializedTemplate->saveClassBody(pBaseClassDefsBlock, body_data);

	return pSpecializedTemplate;
}

void CTemplate::saveClassBody(void* pBaseClassDefsBlock, void* body_data)
{
	// store into m_classBaseTypeDefs
	m_classBaseTypeDefs.clear(); // TODO: mem leak

	int parentDefCount = 0;
	SourceTreeNode* pBaseClassDefsNode = NULL;
	if (pBaseClassDefsBlock)
	{
		CGrammarAnalyzer ga;
		ga.initWithBlocks(getRealScope(), pBaseClassDefsBlock);
		pBaseClassDefsNode = ga.getBlock();
		MY_ASSERT(ga.isEmpty());
		parentDefCount = baseClassDefsGetCount(pBaseClassDefsNode);
	}
	for (int i = 0; i < parentDefCount; i++)
	{
		ClassBaseTypeDef def;
		SourceTreeNode* pUserDefTypeNode;
		baseClassDefsGetChildByIndex(pBaseClassDefsNode, i, def.bVirtual, def.cam_type, pUserDefTypeNode);
		def.pUserDefTypeNode = dupSourceTreeNode(pUserDefTypeNode);
		def.pBaseScope = NULL;

		TypeDefPointer pTypeDef = getTypeDefByUserDefTypeNode(pUserDefTypeNode, true, true);
		if (pTypeDef) // if base type is a template whose type params depend on this template type param, then it could be NULL
		{
            SemanticDataType type = pTypeDef->getType();
            switch (pTypeDef->getType())
            {
            case SEMANTIC_TYPE_BASIC:
            case SEMANTIC_TYPE_TYPENAME:
                break;
            case SEMANTIC_TYPE_CLASS:
            case SEMANTIC_TYPE_STRUCT:
                while (!pTypeDef->isBaseType() && pTypeDef->getDepth() == 0)
                    pTypeDef = pTypeDef->getBaseType();
                def.pBaseScope = pTypeDef->getClassDef();
                MY_ASSERT(def.pBaseScope);
                break;
            case SEMANTIC_TYPE_TEMPLATE:
                def.pBaseScope = pTypeDef->getTemplate();
                MY_ASSERT(def.pBaseScope);
                break;
            default:
                MY_ASSERT(false);
            }
            if (def.pBaseScope)
            {
                TRACE("CTemplate::%s, i=%d, defNode=%s, base class is %s\n", __FUNCTION__, i, displaySourceTreeUserDefType(pUserDefTypeNode).c_str(), def.pBaseScope->getDebugPath().c_str());
            }
            else
            {
                TRACE("CTemplate::%s, i=%d, base type is %s\n", __FUNCTION__, i, getSemanticTypeName(pTypeDef->getType()).c_str());
            }
		}
		m_classBaseTypeDefs.push_back(def);
	}

	m_body_sv = CGrammarAnalyzer::bracketBlockGetTokens(body_data);

}

void CTemplate::mergeWithBaseClass(CTemplate* pNewTemplate, bool bReplaceName)
{
	MY_ASSERT(isRootTemplate());

	// seems it's legal that one is STRUCT while the other is CLASS
	//if (m_data_type != pNewTemplate->m_data_type)
	//	throw("The type of this template is different from what it's defined previously");
	if (m_typeParams.size() != pNewTemplate->m_typeParams.size())
		throw(std::string("This template has been defined previously with ") + ltoa(pNewTemplate->m_typeParams.size()) + " parameters.");

	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam& param = m_typeParams[i];
		TypeParam& newParam = pNewTemplate->m_typeParams[i];

		if (newParam.type != param.type)
			throw(std::string("The ") + ltoa(i) + " type parameter's type is different from the one previously defined.");
		if (newParam.bHasDefault)
		{
			if (!param.bHasDefault)
			{
				param.bHasDefault = true;
				param.pDefaultNode = dupSourceTreeNode(newParam.pDefaultNode);
			}
			else
			{
				// may need to compare after standardizing typenames
				if (displaySourceTreeExtendedTypeVar(newParam.pDefaultNode) != displaySourceTreeExtendedTypeVar(param.pDefaultNode))
					throw(std::string("The default type of the ") + ltoa(i) + " type parameter is different from the one previously defined.");
			}
		}

		if (bReplaceName)
		{
			if (!param.name.empty())
			{
				SymbolDefMap::iterator it = m_symbol_map.find(param.name);
				MY_ASSERT(it != m_symbol_map.end());
				if (param.type == TEMPLATE_PARAM_TYPE_DATA)
				{
					MY_ASSERT(it->second.type == GO_TYPE_TYPEDEF && it->second.getTypeDef()->getType() == SEMANTIC_TYPE_TYPENAME);
				}
				else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
				{
					MY_ASSERT(it->second.type == GO_TYPE_VAR_DEF);
				}
				else
				{
					MY_ASSERT(false);
				}
				m_symbol_map.erase(it);
				// TODO: memory leak here
			}
			param.name = newParam.name;
			param.pNumValueType = newParam.pNumValueType;
			param.bHasTypename = newParam.bHasTypename;
		}
	}

	if (bReplaceName)
	{
		for (unsigned i = 0; i < m_typeParams.size(); i++)
		{
			TypeParam& param = m_typeParams[i];
			if (!param.name.empty())
			{
				if (param.type == TEMPLATE_PARAM_TYPE_DATA)
					getRealScope()->createClassAsChild(param.name, SEMANTIC_TYPE_TYPENAME);
				else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
					addVarDef(new CVarDef(this, param.name, param.pNumValueType, NULL));
				else
					MY_ASSERT(false);
			}
		}
	}
}

void CTemplate::mergeWithSpecializedClass(CTemplate* pNewTemplate, bool bReplaceName)
{
	MY_ASSERT(!isRootTemplate());

	if (m_data_type != pNewTemplate->m_data_type)
		throw("The type of this template is different from what it's defined previously");
	if (m_typeParams.size() != pNewTemplate->m_typeParams.size())
		throw(std::string("This template has been defined previously with ") + ltoa(m_typeParams.size()) + " parameters.");
	if (m_specializedTypeParams.size() != pNewTemplate->m_specializedTypeParams.size())
		throw(std::string("This template has been defined previously with ") + ltoa(m_specializedTypeParams.size()) + " specialized parameters.");

	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam& param = m_typeParams[i];
		TypeParam& newParam = pNewTemplate->m_typeParams[i];

		if (newParam.type != param.type)
			throw(std::string("The ") + ltoa(i) + " type parameter's type is different from the one previously defined.");
		if (newParam.bHasDefault)
		{
			if (!param.bHasDefault)
			{
				param.bHasDefault = true;
				param.pDefaultNode = dupSourceTreeNode(newParam.pDefaultNode);
			}
			else
			{
				// may need to compare after standardizing typenames
				if (displaySourceTreeExtendedTypeVar(newParam.pDefaultNode) != displaySourceTreeExtendedTypeVar(param.pDefaultNode))
					throw(std::string("The default type of the ") + ltoa(i) + " type parameter is different from the one previously defined.");
			}
		}

		if (bReplaceName)
		{
			if (!param.name.empty())
			{
				SymbolDefMap::iterator it = m_symbol_map.find(param.name);
				MY_ASSERT(it != m_symbol_map.end());
				if (param.type == TEMPLATE_PARAM_TYPE_DATA)
				{
					MY_ASSERT(it->second.type == GO_TYPE_TYPEDEF && it->second.getTypeDef()->getType() == SEMANTIC_TYPE_TYPENAME);
				}
				else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
				{
					MY_ASSERT(it->second.type == GO_TYPE_VAR_DEF);
				}
				else
				{
					MY_ASSERT(false);
				}
				m_symbol_map.erase(it);
				// TODO: memory leak here
			}
			param.name = newParam.name;
			param.pNumValueType = newParam.pNumValueType;
			param.bHasTypename = newParam.bHasTypename;
		}
	}

	if (bReplaceName)
	{
		for (unsigned i = 0; i < m_typeParams.size(); i++)
		{
			TypeParam& param = m_typeParams[i];
			if (!param.name.empty())
			{
				if (param.type == TEMPLATE_PARAM_TYPE_DATA)
					getRealScope()->createClassAsChild(param.name, SEMANTIC_TYPE_TYPENAME);
				else if (param.type == TEMPLATE_PARAM_TYPE_VALUE)
					addVarDef(new CVarDef(this, param.name, param.pNumValueType, NULL));
				else
					MY_ASSERT(false);
			}
		}

		for (unsigned i = 0; i < m_specializedTypeParams.size(); i++)
		{
			TypeParam& param = m_specializedTypeParams[i];
			TypeParam& newParam = pNewTemplate->m_specializedTypeParams[i];

			MY_ASSERT(param.type == newParam.type);
			param.name = newParam.name;
			deleteSourceTreeNode(param.pDefaultNode);
			param.pDefaultNode = dupSourceTreeNode(newParam.pDefaultNode);
		}

		m_uniqueId = pNewTemplate->m_uniqueId;
	}
}

void CTemplate::analyzeClassBody(void* pBaseClassDefsBlock, void* body_data)
{
	TRACE("\nCTemplate::analyzeClassBody, name=%s defined in %s\n", getDebugPath().c_str(), definedIn().c_str());

	//MY_ASSERT(m_classBaseTypeDefs.size() == 0);
	//MY_ASSERT(!m_body_sv.empty());
	MY_ASSERT(!m_bDefined);

	// create instanced class
	CClassDef* pClassDef = new CClassDef(this, SEMANTIC_TYPE_CLASS, getName());
	TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(this, getName(), pClassDef));
	pClassDef->setTypeDef(pTypeDef);
	m_instanced_class.type = GO_TYPE_TYPEDEF;
	m_instanced_class.children.push_back(pTypeDef.get());
	m_instanced_class.pTypeDef = pTypeDef;

	TypeDefPointer pTypeThis = TypeDefPointer(new CTypeDef(NULL, "", pTypeDef, 1));
	CVarDef* pVarDef = new CVarDef(this, "this", pTypeThis, NULL);
	addVarDef(pVarDef);

	TRACE("START ANALYZING CLASS TEMPLATE %s\n", getDebugPath().c_str());
	// roughly analyze template content
	CGrammarAnalyzer ga;
	ga.initWithBlocks(getRealScope(), body_data);

	StringVector blockTokens;
	while (SourceTreeNode* pNode = ga.getBlock(&blockTokens))
	{
		g_cur_file_name = pNode->file_name;
		g_cur_line_no = pNode->line_no;
		TRACE("CTemplate::%s, template=%s, ANALYZING LINE %s:%d\n", __FUNCTION__, getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
		switch (cdbGetType(pNode))
		{
		case CDB_TYPE_CAM:
		case CDB_TYPE_FRIEND:
			break;

		case CDB_TYPE_DEFS:
		{
			pNode = cdbDefGetNode(pNode);
			if (defGetType(pNode) != DEF_TYPE_FUNC_DECL || !defFuncDeclIsFuncDef(pNode))
			{
				CStatement* pStatement = new CStatement(this);
				pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
				pStatement->analyzeDef(&ga, pNode, true);
				addChild(pStatement);
				pStatement->setTemplateTokens(blockTokens);
			}
			else
			{
				//int func_modifier;
				SourceTreeNode* pFuncHeaderNode;
				void *pBaseClassInitBlock2, *bracket_block2;
				bool bPureVirtual;
				std::string asm_string;
				SourceTreeVector attribute_list;
				defFuncDeclGetInfo(pNode, pFuncHeaderNode, asm_string, attribute_list, bPureVirtual, pBaseClassInitBlock2, bracket_block2);
				//MY_ASSERT(!bThrow);
				MY_ASSERT(asm_string.empty());
				MY_ASSERT(attribute_list.empty());

				GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pFuncHeaderNode);

				if (!funcHeaderInfo.scope.empty())
					throw("cannot specify namespace to method name " + funcHeaderInfo.scope.toString());

				CGrammarAnalyzer ga2;
				ga2.initWithBlocks(getRealScope(), funcHeaderInfo.params_block);
				SourceTreeNode* pFuncParamsNode = ga2.getBlock();
				MY_ASSERT(ga2.isEmpty());

				TypeDefPointer type_def_ptr = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(funcHeaderInfo.pReturnExtendedType, true), 0));
				type_def_ptr->setModStrings(funcHeaderInfo.mod_strings);
				type_def_ptr->setFuncReturnTypeNode(dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType));
				type_def_ptr->setThrow(funcHeaderInfo.bThrow, dupSourceTreeNode(funcHeaderInfo.pThrowTypeNode));
				addFuncParamsToFuncType(type_def_ptr, pFuncParamsNode);
				deleteSourceTreeNode(pFuncParamsNode);
				type_def_ptr->setDefLocation(g_cur_file_name, g_cur_line_no);

				SymbolDefObject* pSymbolObj = findSymbol(funcHeaderInfo.name, FIND_SYMBOL_SCOPE_LOCAL);
				if (pSymbolObj && pSymbolObj->type != GO_TYPE_FUNC_DECL)
					throw(funcHeaderInfo.name + " is already defined in " + pSymbolObj->definedIn());

				CFuncDeclare* pFuncDeclare = new CFuncDeclare(this, funcHeaderInfo.name, type_def_ptr);
				pFuncDeclare->setDefLocation(g_cur_file_name, g_cur_line_no);
				addFuncDeclare(pFuncDeclare);

				CFunction* pFunc = new CFunction(this, funcHeaderInfo.name, type_def_ptr, getFlowTypeByModifierBits(funcHeaderInfo.mod_strings), pFuncDeclare);
				//func_def_list.push_back(TempBlock(pFunc, bracket_block2, pNode, memberInitCount));
				addChild(pFunc);
			}
			break;
		}
		default:
			MY_ASSERT(false);
		}
	}
	TRACE("STOP ANALYZING CLASS TEMPLATE %s\n", getDebugPath().c_str());
	m_bDefined = true;

	// some template referenced in this template is defined afterwards. so we have to instance class only on demand
	/*for (SymbolDefMap::iterator it = m_symbol_map.begin(); it != m_symbol_map.end(); it++)
	{
		if (it->second.type != GO_TYPE_TEMPLATE) // instanced templates
			continue;

		MY_ASSERT(it->second.children.size() == 1);
		CTemplate* pInstancedTemplate = it->second.getTemplateAt(0);
		if (!pInstancedTemplate->isInstancedTemplate())
			continue;

		SymbolDefObject* pSymbolObj = pInstancedTemplate->findSymbol(pInstancedTemplate->getName(), FIND_SYMBOL_SCOPE_LOCAL);
		if (!pSymbolObj)
			continue;

		MY_ASSERT(pSymbolObj->type == GO_TYPE_TYPEDEF);
		TypeDefPointer pTypeDef = pSymbolObj->getTypeDef();
		CTypeDef* pTD = pTypeDef.get();
		MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());

		CClassDef* pClassDef = pTypeDef->getBaseType()->getClassDef();
		MY_ASSERT(pClassDef->m_template_name == m_name);
		if (pClassDef->isDefined()) // it might be defined when analyzing other classes before this class
			continue;

		TRACE("CTemplate::%s, instance class %s\n", __FUNCTION__, pClassDef->getName().c_str());
		pClassDef->analyzeClassDef(m_classBaseTypeDefs, NULL, m_body_sv);
		pClassDef->setDefined(true);
	}
	TRACE("CTemplate::%s, template=%s, exit\n", __FUNCTION__, m_name.c_str());*/
}

void CTemplate::analyzeVar(const SourceTreeNode* pBodyNode)
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_VAR);

	SourceTreeNode* pExtendedTypeNode, *pScopeNode;
	void* block_data;
	templateBodyVarGetInfo(pBodyNode, m_mod_strings, pExtendedTypeNode, pScopeNode, block_data);

	m_pFuncReturnExtendedTypeNode = dupSourceTreeNode(pExtendedTypeNode);
	m_varName = scopeGetInfo(pScopeNode);
	//MY_ASSERT(m_varName.getDepth() > 0);
	m_body_sv = CGrammarAnalyzer::bracketBlockGetTokens(block_data);
}

void CTemplate::analyzeFuncVar(const SourceTreeNode* pBodyNode)
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_FUNC_VAR);

	SourceTreeNode* pFuncTypeNode;
	SourceTreeVector expr_list;
	templateBodyFuncVarGetInfo(pBodyNode, m_mod_strings, pFuncTypeNode, expr_list);

	SourceTreeNode* pReturnExtendedType, *pScope, *pFuncParamsNode;
	int nDepth;
	funcTypeGetInfo(pFuncTypeNode, pReturnExtendedType, m_mod2_strings, pScope, nDepth, m_name, pFuncParamsNode, m_mod3_strings);
	MY_ASSERT(nDepth == 1);
	//MY_ASSERT(pScope == NULL);
	//MY_ASSERT(pOptFuncParamsNode == NULL);

	m_pFuncReturnExtendedTypeNode = dupSourceTreeNode(pReturnExtendedType);

	SourceTreeVector paramList = funcParamsGetList(pFuncParamsNode);
	int sz = paramList.size();
	for (unsigned i = 0; i < paramList.size(); i++)
	{
		FuncParamType param_type;
		StringVector mod_strings;
		SourceTreeNode* pTypeNode, *pDeclVarNode;
		void *pInitExprBlock;
		funcParamGetInfo(paramList[i], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);

		SourceTreeNode* pInitExprNode = NULL;
		if (pInitExprBlock)
		{
			CGrammarAnalyzer ga2;
			ga2.initWithBlocks(getRealScope(), pInitExprBlock);
			pInitExprNode = ga2.getBlock();
			MY_ASSERT(ga2.isEmpty());
		}

		FuncParamItem item;
		item.param_type = param_type;
		item.mod_strings = mod_strings;
		item.pTypeNode = dupSourceTreeNode(pTypeNode);
		item.pDeclVarNode = dupSourceTreeNode(pDeclVarNode);
		item.pInitExprNode = pInitExprNode;

		m_funcParams.push_back(item);
	}
	m_func_hasVArgs = funcParamsHasVArgs(pFuncParamsNode);

	for (unsigned i = 0; i < expr_list.size(); i++)
		m_var_array_node_list.push_back(dupSourceTreeNode(expr_list[i]));
}

void CTemplate::analyzeFriendClass(const SourceTreeNode* pBodyNode)
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_FRIEND_CLASS);

	SourceTreeNode* pScope;
	std::string className;
	templateBodyFriendClassGetInfo(pBodyNode, m_csu_type, pScope, className);

	if (pScope)
		m_varName = scopeGetInfo(pScope);
	m_varName.addScope(className);
}

bool CTemplate::isSame(const CTemplate* pTemplate) const
{
	MY_ASSERT(m_nTemplateType == TEMPLATE_TYPE_FUNC);

	if (m_typeParams.size() != pTemplate->m_typeParams.size() ||
		m_funcParams.size() != pTemplate->m_funcParams.size())
		return false;

	if (m_func_hasVArgs != pTemplate->m_func_hasVArgs)
		return false;

	TRACE("CTemplate::%s, name=%s\n", __FUNCTION__, m_template_name.c_str());
	if (m_template_name == "lower_bound")
	{
		int i = 0;
	}

	std::map<std::string, std::string> dict;

	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		const TypeParam& param = m_typeParams.at(i);
		const TypeParam& param2 = pTemplate->m_typeParams.at(i);
		if (param.type != param2.type)
		{
			TRACE("type of typeParam %d is different, %d, %d\n", i, param.type, param2.type);
			return false;
		}
		if (param.type == TEMPLATE_PARAM_TYPE_DATA)
		{
			MY_ASSERT(!param.name.empty());
			MY_ASSERT(!param2.name.empty());
			TRACE("add to dict, %s=%s\n", param2.name.c_str(), param.name.c_str());
			dict[param2.name] = param.name;
		}
		else if (param.type == TEMPLATE_PARAM_TYPE_FUNC)
		{
			MY_ASSERT(false);
		}
		else
		{
			MY_ASSERT(param.type == TEMPLATE_PARAM_TYPE_VALUE);
			MY_ASSERT(!param.name.empty());
			MY_ASSERT(!param2.name.empty());
			TRACE("add to dict, %s=%s\n", param2.name.c_str(), param.name.c_str());
			dict[param2.name] = param.name;
		}
	}

	for (unsigned i = 0; i < pTemplate->m_funcParams.size(); i++)
	{
		const FuncParamItem& funcParam = m_funcParams.at(i);
		const FuncParamItem& funcParam2 = pTemplate->m_funcParams.at(i);

		if (funcParam.param_type != funcParam2.param_type)
		{
			TRACE("type of funcParam %d is different, %d, %d\n", i, funcParam.param_type, funcParam2.param_type);
			return false;
		}
		if (funcParam.param_type == FUNC_PARAM_TYPE_REGULAR)
		{
			if (declVarGetDepth(funcParam.pDeclVarNode) != declVarGetDepth(funcParam2.pDeclVarNode))
			{
				TRACE("FuncParam %d, depth different, %d, %d\n", i, declVarGetDepth(funcParam.pDeclVarNode), declVarGetDepth(funcParam2.pDeclVarNode));
				return false;
			}
			SourceTreeNode* pDup = dupSourceTreeNode(funcParam2.pTypeNode);
			typeReplaceTypeNameIfAny(pDup, dict);
			std::string s2 = displaySourceTreeType(pDup);
			std::string s1 = displaySourceTreeType(funcParam.pTypeNode);
			deleteSourceTreeNode(pDup);
			if (s1 != s2)
			{
				TRACE("FuncParam %d, regular name1='%s', name2='%s'\n", i, s1.c_str(), s2.c_str());
				return false;
			}
		}
		else if (funcParam.param_type == FUNC_PARAM_TYPE_FUNC)
		{
			SourceTreeNode* pDup = dupSourceTreeNode(funcParam2.pTypeNode);
			funcTypeReplaceTypeNameIfAny(pDup, dict);
			std::string s2 = displaySourceTreeFuncType(pDup, true);
			std::string s1 = displaySourceTreeFuncType(funcParam.pTypeNode, true);
			deleteSourceTreeNode(pDup);
			if (s1 != s2)
			{
				TRACE("FuncParam %d, funcType name1='%s', name2='%s'\n", i, s1.c_str(), s2.c_str());
				return false;
			}
		}
	}

	return true;
}

int CTemplate::findResolvedDefParam(const TemplateResolvedDefParamVector& v, const std::string& name)
{
	for (unsigned i = 0; i < v.size(); i++)
	{
		if (v[i].typeName == name)
			return i;
	}

	return -1;
}

CTemplate* CTemplate::duplicateAsChild(const std::string& name)
{
	CTemplate* pTemplate = new CTemplate(this, m_nTemplateType, name);
	TRACE("\nCREATING instanced TEMPLATE %s\n", pTemplate->getDebugPath().c_str());
	pTemplate->setDefLocation(getDefFileName(), getDefLineNo());
	pTemplate->m_data_type = m_data_type;
	pTemplate->m_template_name = m_template_name;
	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam param = m_typeParams[i];
		if (param.pDefaultNode)
			param.pDefaultNode = dupSourceTreeNode(param.pDefaultNode);
		//if (param.type == TEMPLATE_PARAM_TYPE_DATA && param.pDefaultNode)
		//	displaySourceTreeExtendedTypeVar(param.pDefaultNode);
		pTemplate->m_typeParams.push_back(param);
	}
	for (unsigned i = 0; i < m_specializedTypeParams.size(); i++)
	{
		TypeParam param = m_specializedTypeParams[i];
		if (param.pDefaultNode)
			param.pDefaultNode = dupSourceTreeNode(param.pDefaultNode);
		pTemplate->m_specializedTypeParams.push_back(param);
	}
	pTemplate->m_pFuncReturnExtendedTypeNode = NULL;
	pTemplate->m_uniqueId = m_uniqueId;
	pTemplate->m_classBaseTypeDefs = m_classBaseTypeDefs;

	return pTemplate;
}

// according to pTypeDef, try to set type defined in twn and store it into resolvedDefParams, note that twn could be something like Sp<T1, T2>
// return -1 if it's already set with another type value, return 0 if not matched, otherwise return score that matched
int CTemplate::resolveParamNameType(const TokenWithNamespace& twn, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams)
{
    TRACE("CTemplate::%s, twn=%s, pTypeDef=%s\n", __FUNCTION__, twn.toString().c_str(), pTypeDef->toFullString().c_str());

    int ret_n = 0;

    for (unsigned i = 0; i < resolvedDefParams.size(); i++)
    {
        TemplateResolvedDefParam& defParam = resolvedDefParams[i];
        TRACE("xxxxxx    i=%u, typeName=%s\n", i, defParam.typeName.c_str());
        if (defParam.typeName != twn.getLastToken())
            continue;

        MY_ASSERT(defParam.flags & 1); // is a type
        MY_ASSERT(twn.getDepth() == 1);
        if (!twn.scopeHasTemplate(twn.getDepth() - 1))
        {
            if (defParam.flags & 2) // has been set
            {
                if (defParam.pTypeDef->toFullString() != pTypeDef->toFullString())
                {
                    TRACE("CTemplate::%s, template type no match, i=%u, real name=%s, def name=%s, return -1\n", __FUNCTION__, i,
                        pTypeDef->toFullString().c_str(), defParam.pTypeDef->toFullString().c_str());
                    return -1;
                }
            }
            defParam.pTypeDef = pTypeDef;
            defParam.flags |= 2;
            addTypeDef(TypeDefPointer(new CTypeDef(this, defParam.typeName, defParam.pTypeDef, 0)));
            ret_n = 10;
            TRACE("CTemplate::%s, template types match, i=%u, return %d\n", __FUNCTION__, i, ret_n);
            return ret_n;
        }

        if (pTypeDef->getDepth() != 0)
        {
            TRACE("CTemplate::%s, real type's depth is not 0, return -1\n", __FUNCTION__);
            return -1;
        }
        if (pTypeDef->getParent()->getGoType() != GO_TYPE_TEMPLATE ||
            ((CTemplate*)pTypeDef->getParent())->getTemplateType() != TEMPLATE_TYPE_CLASS ||
            !((CTemplate*)pTypeDef->getParent())->isInstancedTemplate())
        {
            TRACE("CTemplate::%s, real type's parent is not an instanced template, return -1\n", __FUNCTION__);
            return -1;
        }
        CTemplate* pInstancedTemplate = (CTemplate*)pTypeDef->getParent();
        CTemplate* pDefTemplate = (CTemplate*)pInstancedTemplate->getParent();

        TypeParamVector* pDefVector = NULL;
        if (pDefTemplate->isRootTemplate())
        {
            pDefVector = &pDefTemplate->m_typeParams;
        }
        else
        {
            MY_ASSERT(pDefTemplate->isSpecializedTemplate());
            pDefVector = &pInstancedTemplate->m_specializedTypeParams;
        }
        if (pDefVector->size() != twn.getTemplateParamCount(twn.getDepth() - 1))
        {
            TRACE("CTemplate::%s, real type's template type parameter count=%lu, no match, return -1\n", __FUNCTION__, pDefVector->size());
            return -1;
        }
        for (unsigned j = 0; j < pDefVector->size(); j++)
        {
            MY_ASSERT((*pDefVector)[j].type == TEMPLATE_PARAM_TYPE_DATA); // handle data only for now
            SourceTreeNode* pExtendedTypeVarNode = (*pDefVector)[j].pTypeNode;
            TypeDefPointer pTypeDef2 = pInstancedTemplate->getTypeDefByExtendedTypeVarNode(pExtendedTypeVarNode);
            MY_ASSERT(pTypeDef2);
            int n = resolveParamType(pExtendedTypeVarNode, pTypeDef2, resolvedDefParams);
            if (n < 0)
                return -1;
            if (n > 0)
                ret_n += n;
        }

        if (defParam.flags & 2) // has been set
        {
            if (defParam.pTypeDef->getType() != SEMANTIC_TYPE_TEMPLATE)
            {
                TRACE("CTemplate::%s, resolved type is not template, return -1\n", __FUNCTION__);
                return -1;
            }
            if (defParam.pTypeDef->getTemplate()->getPath() != pInstancedTemplate->getPath())
            {
                TRACE("CTemplate::%s, resolved template mismatch, i=%u, template1=%s, template2=%s, return -1\n", __FUNCTION__, i,
                    defParam.pTypeDef->getTemplate()->getPath().c_str(), pInstancedTemplate->getPath().c_str());
                return -1;
            }
        }
        else
        {
            defParam.pTypeDef = TypeDefPointer(new CTypeDef(pInstancedTemplate));
            defParam.flags |= 2;
            addTypeDef(TypeDefPointer(new CTypeDef(this, defParam.typeName, defParam.pTypeDef, 0)));
        }
        ret_n += 10;
        break;
    }

    TRACE("CTemplate::%s, return %d\n", __FUNCTION__, ret_n);
    return ret_n;
}

// according to pTypeDef, try to resolve the typename in pExtendedTypeNode, and store it into resolvedDefParams, if it's already set with another type value, then return -1, otherwise return depth that matched
int CTemplate::resolveParamType(const SourceTreeNode* pExtendedTypeVarNode, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams, bool bMatchType)
{
	int ret_n = 0;

	//if (extendedTypeIsReference(pExtendedTypeNode) != pTypeDef->isReference())
	//	return -1;
	//if (extendedTypeIsReference(pExtendedTypeNode))
	//	ret_n++;

	int type_depth = pTypeDef->getDepth();
	bool type_const = pTypeDef->isConst(), type_volatile = pTypeDef->isVolatile(), type_reference = pTypeDef->isReference();
	while (!pTypeDef->isBaseType())
	{
		pTypeDef = pTypeDef->getBaseType();
		type_depth += pTypeDef->getDepth();
		type_const |= pTypeDef->isConst();
		type_volatile |= pTypeDef->isVolatile();
		type_reference |= pTypeDef->isReference();
	}

	SourceTreeNode* pExtendedTypeNode;
	int node_depth = 0;
	extendedTypeVarGetInfo(pExtendedTypeVarNode, pExtendedTypeNode, node_depth);
	node_depth += extendedTypeGetDepth(pExtendedTypeNode);
	StringVector mod_strings, mod2_strings;
	mod_strings = extendedTypeGetModStrings(pExtendedTypeNode);
	bool node_const = isInModifiers(mod_strings, MODBIT_CONST);
	bool node_volatile = isInModifiers(mod_strings, MODBIT_VOLATILE);
	bool node_reference = extendedTypeIsReference(pExtendedTypeNode);
	SourceTreeNode* pTypeNode = extendedTypeGetTypeNode(pExtendedTypeNode);
	typeGetModifierBits(pTypeNode, mod_strings, mod2_strings);
	if (isInModifiers(mod_strings, MODBIT_CONST) || isInModifiers(mod2_strings, MODBIT_CONST))
		node_const = true;
	if (isInModifiers(mod_strings, MODBIT_VOLATILE) || isInModifiers(mod2_strings, MODBIT_VOLATILE))
		node_volatile = true;

	TRACE("CTemplate::%s, NodeType=%s, node_depth=%d, node_const=%d, node_volatile=%d, realType=%s, type_depth=%d, type_const=%d, type_volatile=%d\n",
		__FUNCTION__, displaySourceTreeExtendedTypeVar(pExtendedTypeVarNode).c_str(), node_depth, node_const, node_volatile, pTypeDef->toFullString().c_str(), type_depth, type_const, type_volatile);
	//if (node_const && !type_const)
	//	return -1;
	if (bMatchType && node_reference)
	{
		if (!type_reference)
		{
			TRACE("CTemplate::%s, real type is not reference, return -1\n", __FUNCTION__);
			return -1;
		}
		ret_n += 1;
	}

	if (node_const == type_const)
		ret_n += 1;

	if (node_volatile == type_volatile)
		ret_n += 1;

	if (node_depth > type_depth)
	{
		TRACE("CTemplate::%s, different depth, return -1\n", __FUNCTION__);
		return -1;
	}
	ret_n += node_depth;
	type_depth -= node_depth;

	BasicTypeType tt = typeGetType(pTypeNode);
	if (typeGetType(pTypeNode) == BASICTYPE_TYPE_BASIC)
	{
		if (type_depth > 0 || !pTypeDef->isBaseType() || pTypeDef->getType() != SEMANTIC_TYPE_BASIC)
		{
			TRACE("CTemplate::%s, def type is basic while real type is not:%d, return -1\n", __FUNCTION__, pTypeDef->getType());
			return -1;
		}
		if (pTypeDef->toString() != displaySourceTreeType(pTypeNode))
		{
			TRACE("CTemplate::%s, def type is %s while real type is %s, return -1\n", __FUNCTION__, displaySourceTreeType(pTypeNode).c_str(), pTypeDef->toString().c_str());
			return -1;
		}
		ret_n += 5;
		TRACE("CTemplate::%s, both are basic types, return %d\n", __FUNCTION__, ret_n);
		return ret_n;
	}

    if (typeGetType(pTypeNode) == BASICTYPE_TYPE_DATA_MEMBER_POINTER)
    {
        if (type_depth > 0 || !pTypeDef->isBaseType() || pTypeDef->getType() != SEMANTIC_TYPE_DATA_MEMBER_POINTER)
        {
            TRACE("CTemplate::%s, def type is dmp while real type is not:%d, return -1\n", __FUNCTION__, pTypeDef->getType());
            return -1;
        }
        if (pTypeDef->toString() != displaySourceTreeType(pTypeNode))
        {
            TRACE("CTemplate::%s, def type is %s while real type is %s, return -1\n", __FUNCTION__, displaySourceTreeType(pTypeNode).c_str(), pTypeDef->toString().c_str());
            return -1;
        }
        ret_n += 5;
        TRACE("CTemplate::%s, both are dmp types, return %d\n", __FUNCTION__, ret_n);
        return ret_n;
    }

	SourceTreeNode* pUserDefNode;
	bool bHasTypename;
	CSUType csu_type = typeUserDefinedGetInfo(pTypeNode, bHasTypename, pUserDefNode);
	TokenWithNamespace twn = userDefTypeGetInfo(pUserDefNode);

	if (csu_type != CSU_TYPE_NONE)
	{
		if (type_depth > 0 || !pTypeDef->isBaseType() || pTypeDef->getType() != getSemanticTypeFromCSUType(csu_type))
		{
			TRACE("CTemplate::%s, csu type no match, isBaseType=%d, real type=%d, csu_type=%d, return -1\n", __FUNCTION__, pTypeDef->isBaseType(),
				pTypeDef->getType(), getSemanticTypeFromCSUType(csu_type));
			return -1;
		}
		MY_ASSERT(!twn.scopeHasTemplate(twn.getDepth() - 1));
		if (pTypeDef->getName() != twn.getLastToken())
		{
			TRACE("CTemplate::%s, user def type name no match, real name=%s, def name=%s, return -1\n", __FUNCTION__, pTypeDef->getName().c_str(),
				twn.getLastToken().c_str());
			return -1;
		}
		TRACE("CTemplate::%s, user def type names are the same, return %d\n", __FUNCTION__, ret_n);
		return ret_n;
	}

	if (pTypeDef->getDepth() != type_depth)
	{
		pTypeDef = pTypeDef->getBaseType();
		if (type_depth)
		{
			pTypeDef = TypeDefPointer(new CTypeDef(this, "", pTypeDef, type_depth));
		}
	}

	int n = resolveParamNameType(twn, pTypeDef, resolvedDefParams);
    TRACE("CTemplate::%s, resolveParamNameType returns %d\n", __FUNCTION__, n);
	if (n < 0)
	    return n;
	if (n > 0)
	{
	    ret_n += n;
	    return ret_n;
	}

	SymbolDefObject* pSymbolObj = findSymbolEx(twn);
	if (!pSymbolObj)
		throw("Unknown " + twn.toString());

	TRACE("CTemplate::%s, type=%d\n", __FUNCTION__, pSymbolObj->type);
	switch (pSymbolObj->type)
	{
	case GO_TYPE_TYPEDEF:
	{
		TypeDefPointer pTypeDef2 = pSymbolObj->getTypeDef();
		SemanticDataType sdType = pTypeDef2->getType();
		if (sdType == SEMANTIC_TYPE_TYPENAME)
		{
			TRACE("CTemplate::%s, defined as typename, return %d\n", __FUNCTION__, ret_n);
			return ret_n;
		}
		/*if (twn.scopeHasTemplate(twn.getDepth() - 1))
		{
			std::string s = twn.toString();
			throw(twn.toString() + " is not a template");
		}*/
		if (pTypeDef->toFullString() != pTypeDef2->toFullString())
		{
			TRACE("CTemplate::%s, typedef str no match, real name=0x%lx:%s, def name=0x%lx:%s, return -1\n", __FUNCTION__,
				(unsigned long)pTypeDef.get(), pTypeDef->toFullString().c_str(), (unsigned long)pTypeDef2.get(), pTypeDef2->toFullString().c_str());
			return -1;
		}
		TRACE("CTemplate::%s, typedef str match, return %d\n", __FUNCTION__, ret_n);
		return ret_n;
	}
	case GO_TYPE_TEMPLATE:
		ret_n++;
		break;

	case GO_TYPE_NAMESPACE:
	case GO_TYPE_FUNC_DECL:
	case GO_TYPE_VAR_DEF:
	case GO_TYPE_ENUM:
		throw(twn.toString() + " is neither a type nor a template");

	default:
		MY_ASSERT(false);
	}

	if (type_depth != 0)
		return -1;

	if (pTypeDef->getClassDef() == NULL)
		return -1;

	CClassDef* pClassDef = pTypeDef->getClassDef();
	if (pClassDef->getParent() == NULL)
		return -1;

	CScope* pGrammarObj = pClassDef->getParent();
	if (pGrammarObj->getGoType() != GO_TYPE_TEMPLATE)
		return -1;

	CTemplate* pInstancedTemplate = (CTemplate*)pGrammarObj;

	if (pInstancedTemplate->getTemplateName() != twn.getLastToken())
		return -1;

	for (int i = 0; i < twn.getTemplateParamCount(twn.getDepth() - 1); i++)
	{
		int bType;
		SourceTreeNode* pChild;
		bType = twn.getTemplateParamAt(twn.getDepth() - 1, i, pChild);

		TemplateResolvedDefParam& defParam = pInstancedTemplate->m_resolvedDefParams[i];
		int n;
		if (defParam.pTypeDef)
		{
			if (bType == TEMPLATE_PARAM_TYPE_VALUE)
				return -1;
			if (bType == TEMPLATE_PARAM_TYPE_DATA)
				n = resolveParamType(pChild, defParam.pTypeDef, resolvedDefParams);
			else
				n = resolveParamFunc(pChild, defParam.pTypeDef, resolvedDefParams);
		}
		else
		{
			if (bType != TEMPLATE_PARAM_TYPE_VALUE)
				return -1;
			n = resolveParamNumValue(pChild, defParam.numValue, resolvedDefParams);
		}
		if (n < 0)
			return -1;
		ret_n += n;
	}

	TRACE("CTemplate::%s, return %d\n", __FUNCTION__, ret_n);
	return ret_n;
}

int CTemplate::resolveParamFunc(const SourceTreeNode* pFuncNode, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams)
{
    TRACE("CTemplate::%s, funcNode=%s, realType=%s\n", __FUNCTION__, displaySourceTreeFuncType(pFuncNode).c_str(), pTypeDef->toFullString().c_str());

    if (pTypeDef->getFullDepth() != 0 || pTypeDef->getType() != SEMANTIC_TYPE_FUNC)
        return -1;

    SourceTreeNode* pReturnExtendedType, *pScope, *pFuncParamsNode;
    std::string name;
    StringVector mod_strings, mod2_strings;
	int nDepth;
    funcTypeGetInfo(pFuncNode, pReturnExtendedType, mod_strings, pScope, nDepth, name, pFuncParamsNode, mod2_strings);

    SourceTreeVector param_v = funcParamsGetList(pFuncParamsNode);
    if (pTypeDef->getFuncParamCount() < param_v.size())
        return -1;

    int score = 0;
    for (unsigned i = 0; i < param_v.size(); i++)
    {
        FuncParamType param_type;
        SourceTreeNode* pTypeNode, *pDeclVarNode;
		void *pInitExprBlock;
        funcParamGetInfo(param_v[i], param_type, mod_strings, pTypeNode, pDeclVarNode, pInitExprBlock);

        int n;
        if (param_type == FUNC_PARAM_TYPE_REGULAR)
        {
            SourceTreeNode* pExtendedTypeNode = extendedTypeCreateFromType(dupSourceTreeNode(pTypeNode), pDeclVarNode);
            SourceTreeNode* pExtendedTypeVarNode = extendedTypeVarCreateFromExtendedType(pExtendedTypeNode);
            n = resolveParamType(pExtendedTypeVarNode, pTypeDef->getFuncParamAt(i)->getType(), resolvedDefParams, true);
            deleteSourceTreeNode(pExtendedTypeVarNode);
        }
        else
        {
            MY_ASSERT(param_type == FUNC_PARAM_TYPE_FUNC);
            n = resolveParamFunc(pTypeNode, pTypeDef->getFuncParamAt(i)->getType(), resolvedDefParams);
        }
        TRACE("CTemplate::%s, param:%d returns %d\n", __FUNCTION__, i, n);
        if (n < 0)
        {
            return -1;
        }
        score += n;
    }

    TRACE("CTemplate::%s, returns %d\n", __FUNCTION__, score);
    return score;
}

int CTemplate::resolveParamNumValue(const SourceTreeNode* pExprNode, long numValue, TemplateResolvedDefParamVector& resolvedDefParams)
{
	ExprType exprType = exprGetType(pExprNode);
	if (exprType == EXPR_TYPE_TOKEN_WITH_NAMESPACE)
	{
		TokenWithNamespace twn = tokenWithNamespaceGetInfo(exprGetSecondNode(pExprNode));
		if (twn.getDepth() == 1)
		{
			for (unsigned i = 0; i < resolvedDefParams.size(); i++)
			{
				TemplateResolvedDefParam& defParam = resolvedDefParams[i];
				if (defParam.typeName == twn.getLastToken())
				{
					MY_ASSERT((defParam.flags & 1) == 0); // is a type
					if (defParam.flags & 2) // has been set
						return defParam.numValue == numValue;

					defParam.numValue = numValue;
					defParam.flags |= 2;

					CVarDef* pVarDef = new CVarDef(this, defParam.typeName, g_type_def_int, NULL);
					pVarDef->setValue(numValue);
					addVarDef(pVarDef);
					return 1;
				}
			}
		}
	}

	CExpr expr(this, pExprNode);
	float f;
	if (!expr.calculateNumValue(f))
		throw(std::string("CTemplate::") + __FUNCTION__ + ", cannot calculate " + expr.toString());
	return (f == numValue) ? 1 : -1;
}

CTemplate* CTemplate::classResolveParamForBaseTemplate(const TemplateResolvedDefParamVector& realTypeList)
{
	std::string type_str;
	for (size_t i = 0; i < realTypeList.size(); i++)
	{
		if (!type_str.empty())
			type_str += ",";
		if (realTypeList[i].pTypeDef)
		{
			MY_ASSERT(realTypeList[i].pTypeDef->getType() != SEMANTIC_TYPE_TYPENAME);
			type_str += realTypeList[i].pTypeDef->toFullString();
		}
		else
			type_str += ltoa(realTypeList[i].numValue);
	}
	type_str = m_name + "<" + type_str + " >";

	//TRACE("classResolveParamForBaseTemplate, TEMPLATE=%s, REALTYPE=%s\n", toHeaderString().c_str(), type_str.c_str());

	SymbolDefObject* pSymbolObj = findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_TEMPLATE);
		CTemplate* pInstancedTemplate = pSymbolObj->getTemplateAt(0);
		return pInstancedTemplate;
		/*pSymbolObj = pInstancedTemplate->findSymbol(type_str, false);
		MY_ASSERT(pSymbolObj);
		MY_ASSERT(pSymbolObj->type == GO_TYPE_TYPEDEF);
		TypeDefPointer pTypeDef = pSymbolObj->getTypeDef();
		MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());
		return pTypeDef;*/
	}

	CTemplate* pInstancedTemplate = duplicateAsChild(type_str);
	TemplateResolvedDefParamVector resolvedDefParams;

	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam& defItem = m_typeParams[i];
		TemplateResolvedDefParam rdp;
		rdp.typeName = defItem.name;
		if (!defItem.name.empty())
			MY_ASSERT(pInstancedTemplate->findResolvedDefParam(resolvedDefParams, defItem.name) < 0);

		if (i < realTypeList.size())
		{
			const TemplateResolvedDefParam& realParam = realTypeList[i];

			if (defItem.type != TEMPLATE_PARAM_TYPE_VALUE)
			{
				if (!realParam.pTypeDef)
				{
					TRACE("classResolveParamForBaseTemplate, param %d, declare as a type but realParam is not\n", i);
					delete pInstancedTemplate;
					return NULL;
				}
				rdp.pTypeDef = realParam.pTypeDef;
				MY_ASSERT(rdp.pTypeDef->getType() != SEMANTIC_TYPE_TYPENAME);
				if (!defItem.name.empty())
					pInstancedTemplate->addTypeDef(TypeDefPointer(new CTypeDef(pInstancedTemplate, defItem.name, realParam.pTypeDef, 0)));
			}
			else
			{
				if (realParam.pTypeDef)
				{
					TRACE("classResolveParamForBaseTemplate, param %d, declare as an value but realParam is a type\n", i);
					delete pInstancedTemplate;
					return NULL;
				}
				rdp.numValue = realParam.numValue;
				if (!defItem.name.empty())
				{
					CVarDef* pVarDef = new CVarDef(pInstancedTemplate, defItem.name, g_type_def_int, NULL);
					pVarDef->setValue(rdp.numValue);
					pInstancedTemplate->addVarDef(pVarDef);
				}
			}
		}
		else
		{
			if (!defItem.bHasDefault)
			{
				TRACE("classResolveParamForBaseTemplate, param %d, default value is expected\n", i);
				delete pInstancedTemplate;
				return NULL;
			}

			if (defItem.type != TEMPLATE_PARAM_TYPE_VALUE)
			{
				TypeDefPointer pTypeDef = pInstancedTemplate->getTypeDefByExtendedTypeVarNode(defItem.pDefaultNode);
				if (!pTypeDef)
					throw("Cannot resolve default type of " + displaySourceTreeExtendedTypeVar(defItem.pDefaultNode));

				rdp.pTypeDef = pTypeDef;
				MY_ASSERT(rdp.pTypeDef->getType() != SEMANTIC_TYPE_TYPENAME);
				pInstancedTemplate->addTypeDef(TypeDefPointer(new CTypeDef(pInstancedTemplate, defItem.name, pTypeDef, 0)));
			}
			else
			{
				CExpr expr(this, defItem.pDefaultNode);
				float f;
				if (!expr.calculateNumValue(f))
					throw(std::string("CTemplate::") + __FUNCTION__ + ", cannot calculate " + expr.toString());
				rdp.numValue = f;
				CVarDef* pVarDef = new CVarDef(pInstancedTemplate, defItem.name, g_type_def_int, NULL);
				pVarDef->setValue(rdp.numValue);
				pInstancedTemplate->addVarDef(pVarDef);
			}
		}
		resolvedDefParams.push_back(rdp);
	}

	addTemplate(pInstancedTemplate);
	pInstancedTemplate->setResolvedDefParams(resolvedDefParams);

	return pInstancedTemplate;
}

int CTemplate::classResolveParamForSpecializedTemplate(const TemplateResolvedDefParamVector& realTypeList, CTemplate*& pRetTemplate)
{
	MY_ASSERT(m_specializedTypeParams.size() == realTypeList.size());

	std::string type_str;
	for (size_t i = 0; i < realTypeList.size(); i++)
	{
		if (!type_str.empty())
			type_str += ",";
		if (realTypeList[i].pTypeDef)
			type_str += realTypeList[i].pTypeDef->toFullString();
		else
			type_str += ltoa(realTypeList[i].numValue);
	}
	type_str = m_name + "<" + type_str + " >";

	TRACE("CTemplate::%s, TEMPLATE=%s, REALTYPE=%s\n", __FUNCTION__, toHeaderString(0).c_str(), type_str.c_str());

	SymbolDefObject* pSymbolObj = findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_TEMPLATE);
		pRetTemplate = pSymbolObj->getTemplateAt(0);
		TRACE("CTemplate::%s, calculated before, return %d", __FUNCTION__, pRetTemplate->getScore());
		return pRetTemplate->getScore();
	}

	CTemplate* pInstancedTemplate = duplicateAsChild(type_str);
	TemplateResolvedDefParamVector resolvedDefParams;

	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		TypeParam& defItem = m_typeParams[i];
		TemplateResolvedDefParam resolvedParam;
		resolvedParam.typeName = defItem.name;
		resolvedParam.flags = (defItem.type != TEMPLATE_PARAM_TYPE_VALUE ? 1 : 0);
		resolvedDefParams.push_back(resolvedParam);
	}

	int score = 0;
	TRACE("CTemplate::%s, specializedTypeParams.size()=%lu\n", __FUNCTION__, m_specializedTypeParams.size());
	for (unsigned i = 0; i < m_specializedTypeParams.size(); i++)
	{
		TypeParam& defItem = m_specializedTypeParams[i];
		int n;
		if (defItem.type == TEMPLATE_PARAM_TYPE_DATA)
		{
			n = pInstancedTemplate->resolveParamType(defItem.pDefaultNode, realTypeList[i].pTypeDef, resolvedDefParams, true);
		}
		else if (defItem.type == TEMPLATE_PARAM_TYPE_FUNC)
        {
            n = pInstancedTemplate->resolveParamFunc(defItem.pDefaultNode, realTypeList[i].pTypeDef, resolvedDefParams);
        }
		else
		{
	        MY_ASSERT(defItem.type == TEMPLATE_PARAM_TYPE_VALUE);
			n = pInstancedTemplate->resolveParamNumValue(defItem.pDefaultNode, realTypeList[i].numValue, resolvedDefParams);
		}
		TRACE("CTemplate::%s, param:%d returns %d\n", __FUNCTION__, i, n);
		if (n < 0)
		{
			delete pInstancedTemplate;
			return -1;
		}
		score += n;
	}

	addTemplate(pInstancedTemplate);
	pInstancedTemplate->setResolvedDefParams(resolvedDefParams);
	pInstancedTemplate->setScore(score);
	pRetTemplate = pInstancedTemplate;

	/*for (unsigned i = 0; i < resolvedDefParams.size(); i++)
	{
		TemplateResolvedDefParam& param = resolvedDefParams[i];
		if ((param.flags & 2) == 0)
			continue;

		if (param.flags & 1)
		{
			pInstancedTemplate->addTypeDef(TypeDefPointer(new CTypeDef(pInstancedTemplate, param.typeName, param.pTypeDef, NULL)));
		}
		else
		{
			CVarDef* pVarDef = new CVarDef(pInstancedTemplate, param.typeName, g_type_def_int, NULL);
			pVarDef->setValue(param.numValue);
			pInstancedTemplate->addVarDef(pVarDef);
		}
	}*/

	TRACE("CTemplate::%s, return %d\n", __FUNCTION__, score);
	return score;
}

CTemplate* CTemplate::classMatchForATemplate_(const TemplateResolvedDefParamVector& realTypeList)
{
	TRACE("CTemplate::%s, template=%s, realTypeList size=%lu\n", __FUNCTION__, m_name.c_str(), realTypeList.size());
	MY_ASSERT(m_specializedTypeParams.size() == 0);
	if (realTypeList.size() > m_typeParams.size())
	{
		TRACE("CTemplate::%s, realType size %lu is larger than decl size %lu\n", __FUNCTION__,  realTypeList.size(), m_typeParams.size());
		return NULL;
	}

	CTemplate* pBaseInstanceTemplate = classResolveParamForBaseTemplate(realTypeList);
	if (!pBaseInstanceTemplate)
	{
		TRACE("CTemplate::%s, template=%s, classResolveParamForBaseTemplate failed.\n", __FUNCTION__, m_name.c_str());
		return NULL;
	}
	std::vector<CTemplate*> matched_v;
	int maxMatchedScore = -1;
	for (size_t i = 0; i < m_specializeDefList.size(); i++)
	{
		CTemplate* pTemplate2 = m_specializeDefList[i].getTemplateAt(0);
		CTemplate* pSpecializeInstanceTemplate;
		int n = pTemplate2->classResolveParamForSpecializedTemplate(pBaseInstanceTemplate->m_resolvedDefParams, pSpecializeInstanceTemplate);
		TRACE("CTemplate::%s, template=%s, classResolveParamForSpecializedTemplate %s returns %d\n", __FUNCTION__, m_name.c_str(), pTemplate2->toHeaderString(0).c_str(), n);
		if (n < 0)
			continue;
		if (maxMatchedScore < n)
		{
			maxMatchedScore = n;
			matched_v.clear();
			matched_v.push_back(pSpecializeInstanceTemplate);
		}
		else if (maxMatchedScore == n)
			matched_v.push_back(pSpecializeInstanceTemplate);
	}

	if (matched_v.empty())
	{
		TRACE("CTemplate::%s, template=%s, no matched specialized template, return base template.\n", __FUNCTION__, m_name.c_str());
		return pBaseInstanceTemplate;
	}

	if (matched_v.size() > 1)
	{
		std::string err_s = "Specialized template ambiguity found! Choices are:\n";
		for (unsigned i = 0; i < matched_v.size(); i++)
			err_s += matched_v[i]->toHeaderString(0) + " defined in " + matched_v[i]->definedIn() + "\n";
		throw(err_s);
	}
	TRACE("CTemplate::%s, template=%s, return specialized template %s\n", __FUNCTION__, m_name.c_str(), matched_v[0]->getDebugPath().c_str());
	return matched_v[0];
}

// this template must be the instanced template
TypeDefPointer CTemplate::classGetInstance_(const TemplateResolvedDefParamVector& resolvedDefParams)
{
	MY_ASSERT(isInstancedTemplate());
	TypeDefPointer pTypeDef;

	SymbolDefObject* pSymbolObj = findSymbol(m_name, FIND_SYMBOL_SCOPE_LOCAL);
	if (pSymbolObj)
	{
		MY_ASSERT(pSymbolObj->type == GO_TYPE_TYPEDEF);
		pTypeDef = pSymbolObj->getTypeDef();
		MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());
	}
	else
	{
		pTypeDef = createClassAsChild(m_name, SEMANTIC_TYPE_CLASS);
	}

	/*CClassDef* pClassDef = pTypeDef->getBaseType()->getClassDef();
	if (!pClassDef->isDefined())
	{
		if (((CTemplate*)getParent())->isDefined())
		{
			TRACE("CTemplate::%s for %s\n", __FUNCTION__, pClassDef->getDebugPath().c_str());

			pClassDef->analyzeClassDef(((CTemplate*)getParent())->m_classBaseTypeDefs, NULL, ((CTemplate*)getParent())->m_body_sv);
		}
		else
		{
			TRACE("CTemplate::%s, template hasn't been defined yet\n", __FUNCTION__, getDebugPath().c_str());
		}
	}*/

	return pTypeDef;
}

TypeDefPointer CTemplate::classGetInstance(const TokenWithNamespace& twn, int depth, CScope* pOtherScope)
{
	TRACE("\nCTemplate::classGetInstance, template=%s, twn=%s, depth=%d, other scope=%s\n", getDebugPath().c_str(), twn.toString().c_str(), depth, pOtherScope->getDebugPath().c_str());
	TemplateResolvedDefParamVector realParamTypes;
	bool bHasTypename = false;
	std::string type_str;
	for (int i = 0; i < twn.getTemplateParamCount(depth); i++)
	{
		if (!type_str.empty())
			type_str += ", ";

		SourceTreeNode* pChildNode;
		int bType;
		bType = twn.getTemplateParamAt(depth, i, pChildNode);
		TemplateResolvedDefParam param;
		switch (bType)
		{
		case TEMPLATE_PARAM_TYPE_DATA:
		{
			TRACE("\nCTemplate::classGetInstance, template=%s, i=%d, check type:%s\n", getDebugPath().c_str(), i, displaySourceTreeExtendedTypeVar(pChildNode).c_str());
			TypeDefPointer pTypeDef;
			if (pOtherScope)
				pTypeDef = pOtherScope->getTypeDefByExtendedTypeVarNode(pChildNode);
			TypeDefPointer pTypeDef2 = getTypeDefByExtendedTypeVarNode(pChildNode);
			if (pTypeDef)
			{
				if (!pTypeDef2)
					param.pTypeDef = pTypeDef;
				else
				{
					if (pTypeDef->getType() == SEMANTIC_TYPE_TYPENAME && pTypeDef2->getType() != SEMANTIC_TYPE_TYPENAME)
						param.pTypeDef = pTypeDef2;
					else if (pTypeDef->getType() != SEMANTIC_TYPE_TYPENAME && pTypeDef2->getType() == SEMANTIC_TYPE_TYPENAME)
						param.pTypeDef = pTypeDef;
					else
					{
						param.pTypeDef = pTypeDef;
						/*std::string s = pTypeDef->toFullString();
						std::string s2 = pTypeDef2->toFullString();
						CTypeDef* pTD = pTypeDef.get();
						CTypeDef* pTD2 = pTypeDef2.get();
						MY_ASSERT(false);*/
					}
				}
			}
			else
			{
				if (!pTypeDef2)
				{
					MY_ASSERT(false);
				}
				param.pTypeDef = pTypeDef2;
			}
			type_str += param.pTypeDef->toFullString();
			if (param.pTypeDef->getType() == SEMANTIC_TYPE_TYPENAME)
				bHasTypename = true;
			break;
		}
		case TEMPLATE_PARAM_TYPE_FUNC:
			MY_ASSERT(false);
			break;
		case TEMPLATE_PARAM_TYPE_VALUE:
		{
			TRACE("\nCTemplate::classGetInstance, template=%s, i=%d, calculate expr:%s\n", getDebugPath().c_str(), i, displaySourceTreeExpr(pChildNode).c_str());
			CExpr expr((pOtherScope ? pOtherScope : this), pChildNode);
			float f;
			if (!expr.calculateNumValue(f))
			{
				//std::string s = expr.toString();
				//throw(std::string("CTemplate::") + __FUNCTION__ + ", cannot calculate " + s);
				// when analyzing a template, a template might need to be instanced to check whether a member inside is a type or a var,
				// in this case, it might not be able to get instanced so need to return an empty pointer
				return TypeDefPointer();
			}
			param.numValue = f;
			type_str += ltoa(param.numValue);
			break;
		}
		default:
			MY_ASSERT(false);
		}
		realParamTypes.push_back(param);
	}
	type_str = m_name + "<" + type_str + " >";
	TRACE("CTemplate::%s, type_str = %s, bHasTypename=%d\n", __FUNCTION__, type_str.c_str(), bHasTypename);

	if (bHasTypename)
	{
		CTemplate* pInstancedTemplate;
		SymbolDefObject* pSymbolObj = findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
		if (pSymbolObj)
		{
			MY_ASSERT(pSymbolObj->type == GO_TYPE_TEMPLATE);
			pInstancedTemplate = pSymbolObj->getTemplateAt(0);
			pSymbolObj = pInstancedTemplate->findSymbol(type_str, FIND_SYMBOL_SCOPE_LOCAL);
			MY_ASSERT(pSymbolObj);
			MY_ASSERT(pSymbolObj->type == GO_TYPE_TYPEDEF);
			TypeDefPointer pTypeDef = pSymbolObj->getTypeDef();
			MY_ASSERT(!pTypeDef->isBaseType() && pTypeDef->getBaseType()->isBaseType() && pTypeDef->getBaseType()->getClassDef());
			return pTypeDef;
		}

		pInstancedTemplate = duplicateAsChild(type_str);
		addTemplate(pInstancedTemplate);
		pInstancedTemplate->setResolvedDefParams(realParamTypes);
		return pInstancedTemplate->createClassAsChild(type_str, SEMANTIC_TYPE_CLASS);
	}

	TemplateResolvedDefParamVector resolvedDefParams;
	CTemplate* pTemplate = classMatchForATemplate_(realParamTypes);
	if (!pTemplate)
	{
		TRACE("CTemplate::classGetInstance, cannot find a match\n");
		return TypeDefPointer();
	}
	return pTemplate->classGetInstance_(resolvedDefParams);
}

CTemplate* CTemplate::getTemplateByParams(const TokenWithNamespace& twn, int depth)
{
	MY_ASSERT(isRootTemplate());
	if (twn.getTemplateParamCount(depth) > m_typeParams.size())
		return NULL;

	TRACE("\nCTemplate::%s, template %s is defined at %s, params=<", __FUNCTION__, m_name.c_str(), definedIn().c_str());
	StringVector realParams;
	std::string s;
	for (int j = 0; j < twn.getTemplateParamCount(depth); j++)
	{
		SourceTreeNode* pNode;
		switch (twn.getTemplateParamAt(depth, j, pNode))
		{
		case TEMPLATE_PARAM_TYPE_DATA:
			s = displaySourceTreeExtendedTypeVar(pNode);
			break;
		case TEMPLATE_PARAM_TYPE_FUNC:
			s = displaySourceTreeFuncType(pNode);
			break;
		case TEMPLATE_PARAM_TYPE_VALUE:
			s = displaySourceTreeExpr(pNode);
			break;
		default:
			MY_ASSERT(false);
		}

		TRACE("%s,", s.c_str());
		realParams.push_back(s);
	}
	TRACE(">");

    bool bMatch = true;
	for (unsigned i = 0; i < m_specializeDefList.size(); i++)
	{
		TRACE(", specialized params%u/%lu=<", i, m_specializeDefList.size());
		CTemplate* pSpecializedTemplate = m_specializeDefList[i].getTemplateAt(0);
		bMatch = true;
		for (unsigned j = 0; bMatch && j < pSpecializedTemplate->m_specializedTypeParams.size(); j++)
		{
			const TypeParam& typeParam = pSpecializedTemplate->m_specializedTypeParams[j];
			if (typeParam.type == TEMPLATE_PARAM_TYPE_DATA)
				s = displaySourceTreeExtendedTypeVar(typeParam.pDefaultNode);
			else if (typeParam.type == TEMPLATE_PARAM_TYPE_FUNC)
				s = displaySourceTreeFuncType(typeParam.pDefaultNode);
			else
				s = displaySourceTreeExpr(typeParam.pDefaultNode);
			TRACE("%s,", s.c_str());
			if (j < realParams.size())
			    bMatch = (s == realParams[j]);
			else
			{
			    if (!typeParam.bHasDefault)
			        bMatch = false;
			}
		}
		TRACE(">");
		if (bMatch)
		{
			TRACE(", return specialized template\n");
			return pSpecializedTemplate;
		}
	}

    bMatch = true;
    TRACE(", root params=<");
    for (unsigned i = 0; bMatch && i < m_typeParams.size(); i++)
    {
        const TypeParam& typeParam = m_typeParams[i];
        if (typeParam.type == TEMPLATE_PARAM_TYPE_DATA)
            s = typeParam.name;
        else if (typeParam.type == TEMPLATE_PARAM_TYPE_VALUE)
            s = typeParam.pNumValueType->toString();
        else
            MY_ASSERT(false);
        TRACE("%s,", s.c_str());
        if (i < realParams.size())
            bMatch = (s == realParams[i]);
        else
        {
            if (!typeParam.bHasDefault)
                bMatch = false;
        }
    }
    TRACE(">");

    if (bMatch)
    {
        TRACE(", return root template\n");
        return this;
    }

	TRACE(", no match, return root template\n");
	//MY_ASSERT(false);
	return this;
}

bool CTemplate::canBeSpecialized(const TemplateResolvedDefParamVector& typeList)
{
	if (m_nTemplateType != TEMPLATE_TYPE_CLASS)
		return false;

	if (typeList.size() > m_typeParams.size())
		return false;

	for (size_t i = 0; i < m_typeParams.size(); i++)
	{
		const TypeParam& typeParam = m_typeParams[i];

		if (i < typeList.size())
		{
			const TemplateResolvedDefParam& resolveParam = typeList[i];
			if (typeParam.type != TEMPLATE_PARAM_TYPE_DATA)
			{
				if (!resolveParam.pTypeDef)
					return false;
			}
			else
			{
				if (resolveParam.pTypeDef)
					return false;
				// for now, we can't calculate pExpr to match it.
			}
		}
		else
		{
			if (!typeParam.bHasDefault)
				return false;
		}
	}

	return true;
}

bool CTemplate::compareResolvedDefParams(const TemplateResolvedDefParamVector& typeList, const TemplateResolvedDefParamVector& typeList2)
{
	MY_ASSERT(typeList.size() == m_typeParams.size());
	MY_ASSERT(typeList2.size() == m_typeParams.size());

	for (size_t i = 0; i < m_typeParams.size(); i++)
	{
		if (m_typeParams[i].type != TEMPLATE_PARAM_TYPE_VALUE)
		{
			MY_ASSERT(typeList[i].pTypeDef);
			MY_ASSERT(typeList2[i].pTypeDef);
			if (typeList[i].pTypeDef->toFullString() != typeList2[i].pTypeDef->toFullString())
				return false;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CTemplate::setResolvedDefParams(const TemplateResolvedDefParamVector& v)
{
    MY_ASSERT(m_resolvedDefParams.empty());
    m_resolvedDefParams = v;
    //if (getTemplateType() != TEMPLATE_TYPE_CLASS)
        return;

    MY_ASSERT(isInstancedTemplate());
    MY_ASSERT(m_resolvedTypeParams.empty());

    CTemplate* pDefTemplate = (CTemplate*)getParent();
    TypeParamVector* pDefVector = NULL;
    std::string name = m_template_name + "<";
    if (pDefTemplate->isRootTemplate())
    {
        for (unsigned i = 0; i < m_typeParams.size(); i++)
        {
            if (i > 0)
                name += ", ";

            TypeParam& param = m_typeParams[i];
            //MY_ASSERT(.type == TEMPLATE_PARAM_TYPE_DATA); // handle data only for now
            SymbolDefObject* pObj = findSymbol(param.name, FIND_SYMBOL_SCOPE_LOCAL);
            MY_ASSERT(pObj);
            TemplateResolvedDefParam param2;
            if (param.type == TEMPLATE_PARAM_TYPE_DATA)
            {
                MY_ASSERT(pObj->type == GO_TYPE_TYPEDEF);
                param2.pTypeDef = pObj->getTypeDef();
                name += param2.pTypeDef->toString();
            }
            else
            {
                MY_ASSERT(param.type == TEMPLATE_PARAM_TYPE_VALUE);
                MY_ASSERT(pObj->type == GO_TYPE_VAR_DEF);
                CVarDef* pVarDef = pObj->getVarDef();
                MY_ASSERT(pVarDef->hasValue());
                param2.numValue = pVarDef->getValue();
                name += ltoa(param2.numValue);
            }
            m_resolvedTypeParams.push_back(param2);
        }
    }
    else
    {
        MY_ASSERT(pDefTemplate->isSpecializedTemplate());
        for (unsigned i = 0; i < m_specializedTypeParams.size(); i++)
        {
            if (i > 0)
                name += ", ";

            TypeParam& param = m_specializedTypeParams[i];
            TemplateResolvedDefParam param2;
            if (param.type == TEMPLATE_PARAM_TYPE_DATA)
            {
                TypeDefPointer pTypeDef2 = getTypeDefByExtendedTypeVarNode(param.pDefaultNode);
                MY_ASSERT(pTypeDef2);
                param2.pTypeDef = pTypeDef2;
                name += param2.pTypeDef->toString();
            }
            else
            {
                MY_ASSERT(param.type == TEMPLATE_PARAM_TYPE_VALUE);
                CExpr* pExpr = new CExpr(this, param.pDefaultNode);
                float f;
                if (!pExpr->calculateNumValue(f))
                {
                    std::string s = displaySourceTreeExpr(param.pDefaultNode);
                    MY_ASSERT(false);
                }
                param2.numValue = (long)f;
                name += ltoa(param2.numValue);
            }
            m_resolvedTypeParams.push_back(param2);
        }
    }

    name += ">";
    m_name = name;

    TRACE("CTemplate::%s, set this instanced template's name to %s\n", __FUNCTION__, getName().c_str());
}

std::string CTemplate::toHeaderString(int depth)
{
	std::string ret_s;

	ret_s += "template<";

	for (unsigned i = 0; i < m_typeParams.size(); i++)
	{
		if (i > 0)
			ret_s += ", ";
		TypeParam& param = m_typeParams[i];
		if (param.type == TEMPLATE_PARAM_TYPE_DATA)
		{
			ret_s += param.bClass ? "class" : "typename";
			ret_s += " " + param.name;
			if (param.bHasDefault)
			{
				ret_s += " = ";
				if (param.bHasTypename)
					ret_s += "typename ";
				if (param.bDefaultDataOrFuncType)
					ret_s += displaySourceTreeExtendedTypeVar(param.pDefaultNode);
				else
					ret_s += displaySourceTreeFuncType(param.pDefaultNode);
			}
		}
		else if (param.type == TEMPLATE_PARAM_TYPE_FUNC)
		{
			ret_s += param.name;
			if (param.bHasDefault)
				ret_s += std::string(" = ") + displaySourceTreeFuncType(param.pDefaultNode);
		}
		else
		{
			ret_s += param.pNumValueType->toString() + " " + param.name;
			if (param.bHasDefault)
				ret_s += std::string(" = ") + displaySourceTreeExpr(param.pDefaultNode);
		}
	}
	ret_s += " >\n";

	if (m_nTemplateType == TEMPLATE_TYPE_FUNC || m_nTemplateType == TEMPLATE_TYPE_FUNC_VAR)
	{
		ret_s += printTabs(depth) + combineStrings(m_mod_strings);

		if (m_pFuncReturnExtendedTypeNode)
			ret_s += displaySourceTreeExtendedType(m_pFuncReturnExtendedTypeNode) + " ";

		if (m_nTemplateType == TEMPLATE_TYPE_FUNC)
		{
			ret_s += combineStrings(m_mod2_strings);
			if (!m_mod3_strings.empty())
				ret_s += "(" + combineStrings(m_mod3_strings) + m_name + ")";
			else
				ret_s += m_name;
		}
		else
			ret_s += "(" + combineStrings(m_mod2_strings) + "*" + m_name;

		ret_s += "(";
		for (unsigned i = 0; i < m_funcParams.size(); i++)
		{
			if (i > 0)
				ret_s += ", ";
			FuncParamItem& param = m_funcParams[i];

			switch (param.param_type)
			{
			case FUNC_PARAM_TYPE_REGULAR:
				ret_s += displaySourceTreeType(param.pTypeNode);
				if (param.pDeclVarNode)
				{
					ret_s += " " + displaySourceTreeDeclVar(param.pDeclVarNode);
					if (param.pInitExprNode)
						ret_s += " = " + displaySourceTreeExpr(param.pInitExprNode);
				}
				break;
			case FUNC_PARAM_TYPE_FUNC:
				ret_s += displaySourceTreeFuncType(param.pTypeNode);
				break;
			default:
				MY_ASSERT(false);
			}
		}
		if (m_func_hasVArgs)
			ret_s += ",...";
		ret_s += ")";

		if (m_nTemplateType == TEMPLATE_TYPE_FUNC)
			ret_s += " " + combineStrings(m_mod4_strings) + m_throw_string;
		else
		{
			ret_s += ")";
			for (unsigned i = 0; i < m_var_array_node_list.size(); i++)
				ret_s += "[" + displaySourceTreeExpr(m_var_array_node_list[i]) + "]";
		}
	}
	else if (m_nTemplateType == TEMPLATE_TYPE_CLASS)
		ret_s += printTabs(depth) + getSemanticTypeName(m_data_type) + " " + m_name;
	else if (m_nTemplateType == TEMPLATE_TYPE_VAR)
		ret_s += printTabs(depth) + combineStrings(m_mod_strings) + displaySourceTreeExtendedType(m_pFuncReturnExtendedTypeNode) + " " + m_varName.toString();
	else
		ret_s += printTabs(depth) + "friend " + displayCSUType(m_csu_type) + " " + m_varName.getLastToken();

	if (m_specializedTypeParams.size() > 0)
	{
		ret_s += " <";
		for (unsigned i = 0; i < m_specializedTypeParams.size(); i++)
		{
			if (i > 0)
				ret_s += ", ";

			TypeParam& param = m_specializedTypeParams[i];
			if (param.type == TEMPLATE_PARAM_TYPE_DATA)
				ret_s += displaySourceTreeExtendedTypeVar(param.pDefaultNode);
			else if (param.type == TEMPLATE_PARAM_TYPE_FUNC)
				ret_s += displaySourceTreeFuncType(param.pDefaultNode);
			else
				ret_s += displaySourceTreeExpr(param.pDefaultNode);
		}
		ret_s += " >";
	}

	return ret_s;
}

SymbolDefObject* CTemplate::findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode)
{
	TRACE("CTemplate::findSymbol %s in %s, scope=%d, mode=%d, ", name.c_str(), getDebugPath().c_str(), scope, mode);

	SymbolDefObject* pSymbolObj = NULL;

	if (scope == FIND_SYMBOL_SCOPE_PARENT && getTemplateType() == TEMPLATE_TYPE_CLASS && (name == m_name && m_template_name == name))
	{
		TRACE("FOUND %s IN %s\n", name.c_str(), getDebugPath().c_str());
		if (isRootTemplate())
			return getParent()->findSymbol(m_name, FIND_SYMBOL_SCOPE_LOCAL, mode); //&m_instanced_class; //
		if (isSpecializedTemplate())
		{
			pSymbolObj = ((CTemplate*)getParent())->findSpecializedTemplate(this);
			if (pSymbolObj)
				return pSymbolObj;
			return getParent()->findSymbol(m_name, scope, mode); // handle back to root template
		}
		MY_ASSERT(false);
	}

	pSymbolObj = CScope::findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL, mode);
	if (pSymbolObj)
		return pSymbolObj;

	if (scope >= FIND_SYMBOL_SCOPE_SCOPE)
	{
		for (size_t i = 0; i < m_classBaseTypeDefs.size(); i++)
		{
			ClassBaseTypeDef& cbtd = m_classBaseTypeDefs[i];

			if (!cbtd.pBaseScope)
				continue;

			MY_ASSERT(cbtd.pBaseScope->getGoType() == GO_TYPE_CLASS || cbtd.pBaseScope->getGoType() == GO_TYPE_TEMPLATE);
			pSymbolObj = cbtd.pBaseScope->findSymbol(name, scope, mode);
			if (pSymbolObj)
				return pSymbolObj;
		}
	}

	if (scope == FIND_SYMBOL_SCOPE_PARENT && m_pParent)
		return m_pParent->findSymbol(name, scope, mode);

	//TRACE("not found\n");
	return NULL;
}

std::string CTemplate::toString(bool bDefine, int depth)
{
	std::string ret_s = toHeaderString(depth);

	if (m_nTemplateType == TEMPLATE_TYPE_VAR)
	{
		if (!m_varName.empty())
			ret_s += "::";
		for (size_t i = 0; i < m_body_sv.size(); i++)
		{
			const std::string& s = m_body_sv[i];

			if (CLexer::isCommentWord(s))
			{
				if (s.substr(0, 3) == "//*")
				{
					for (int j = 0; j < atoi(s.c_str() + 3); j++)
						ret_s += "\n" + printTabs(depth + 1);
				}
				continue;
			}
			ret_s += s + " ";
		}
		ret_s += ";\n";
	}
	else if (m_nTemplateType == TEMPLATE_TYPE_FRIEND_CLASS)
	{
		ret_s += ";\n";
	}
	else
	{
		if (!bDefine)
			return ret_s += ";\n";

		if (!m_func_base_init_sv.empty())
			ret_s += " : " + combineStrings(m_func_base_init_sv);

		if (m_classBaseTypeDefs.size() > 0)
		{
			ret_s += " : ";
			for (unsigned i = 0; i < m_classBaseTypeDefs.size(); i++)
			{
				if (i > 0)
					ret_s += ", ";
				const ClassBaseTypeDef& cbtd = m_classBaseTypeDefs[i];
				if (cbtd.bVirtual)
					ret_s += "virtual ";
				ret_s += displayCAMType(cbtd.cam_type) + " ";
				ret_s += displaySourceTreeUserDefType(cbtd.pUserDefTypeNode);
			}
		}
		ret_s += "\n";
		ret_s += printTabs(depth) + "{\n" + printTabs(depth + 1);
		for (size_t i = 0; i < m_body_sv.size(); i++)
		{
			const std::string& s = m_body_sv[i];

			if (CLexer::isCommentWord(s))
			{
				if (s.substr(0, 3) == "//*")
				{
					for (int j = 0; j < atoi(s.c_str() + 3); j++)
						ret_s += "\n" + printTabs(depth + 1);
				}
				continue;
			}
			ret_s += s + " ";
		}
		ret_s += "\n" + printTabs(depth) + "}";
		if (m_nTemplateType == TEMPLATE_TYPE_CLASS)
			ret_s += ";";
		ret_s += "\n";
	}

	return ret_s;
}

CFunction::CFunction(CScope* pParent, const std::string& func_name, TypeDefPointer pFuncType, FlowType flow_type, CFuncDeclare* pFuncDeclare) : CScope(pParent)
{
	m_func_type = pFuncType;
	//m_display_name = func_name;
	m_name = func_name;
	m_flow_type = flow_type;
	m_pFuncDeclare = pFuncDeclare;
	m_cam_type = CAM_TYPE_NONE;

	bool bCallFlow = false;
	for (int i = 0; i < m_func_type->getFuncParamCount(); i++)
	{
		CVarDef* pVarDef = m_func_type->getFuncParamAt(i);
		CVarDef* pVarDef2 = new CVarDef(this, pVarDef->getName(), pVarDef->getType(), NULL);
		pVarDef->setDefLocation(g_cur_file_name, g_cur_line_no);
		if (!pVarDef->getName().empty())
			addVarDef(pVarDef);
		if (!isFlow() && pVarDef->isFlow())
			fatal_error("func %s is declared as not flow but param init expr:%d is flow", m_name.c_str(), i + 1);
	}
}

CFunction::~CFunction()
{
}

std::string CFunction::getDebugName()
{
	return getName() + ":" + ltoa(m_flow_type);
}

void CFunction::analyze(void* pBaseClassInitBlock, void* bracket_block, const SourceTreeNode* pRoot)
{
	TRACE("START ANALYZING FUNC %s(0x%lx)\n", getDebugPath().c_str(), long(this));
	if (pBaseClassInitBlock)
	{
	    //StringVector blockData = CGrammarAnalyzer::bracketBlockGetTokens(pBaseClassInitBlock);
        //TRACE("CFunction::%s, before analyzing, tokens={", __FUNCTION__);
        //for (unsigned i = 0; i < blockData.size(); i++)
        //    TRACE("%s ", blockData[i].c_str());
        //TRACE("}\n");

		CGrammarAnalyzer ga;
		ga.initWithBlocks(getRealScope(), pBaseClassInitBlock);
		SourceTreeVector ret_v = classBaseInitsGetList(ga.getBlock());
		MY_ASSERT(ga.isEmpty());
		for (unsigned i = 0; i < ret_v.size(); i++)
		{
			SourceTreeNode* pUserDefTypeOrMember, *pExpr2Node;

			classBaseInitGetInfo(ret_v[i], pUserDefTypeOrMember, pExpr2Node);
			CExpr2* pExpr2 = new CExpr2(this);
			pExpr2->analyze(pExpr2Node);
			m_memberInitList.push_back(std::pair<TokenWithNamespace, CExpr2*>(userDefTypeGetInfo(pUserDefTypeOrMember), pExpr2));
		}
	}

	CGrammarAnalyzer ga;
	ga.initWithBlocks(getRealScope(), bracket_block);
	while (SourceTreeNode* pNode = ga.getBlock())
	{
		g_cur_file_name = pNode->file_name;
		g_cur_line_no = pNode->line_no;
		TRACE("CFunction::%s, %s, ANALYZING LINE %s:%d\n", __FUNCTION__, getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
		CStatement* pStatement = new CStatement(this);
		pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
		addChild(pStatement);
		pStatement->analyze(&ga, pNode);
		if (!isFlow() && pStatement->isFlow())
			fatal_error("func %s is declared as not flow but statement line %s:%d is flow", m_name.c_str(), pNode->file_name.c_str(), pNode->line_no);
		deleteSourceTreeNode(pNode);
	}
	MY_ASSERT(ga.isEmpty());
	TRACE("STOP ANALYZING FUNC %s(0x%lx)\n", getDebugPath().c_str(), long(this));
}

SymbolDefObject* CFunction::findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode)
{
	TRACE("CFunction::findSymbol %s in %s, scope=%d, mode=%d, ", name.c_str(), getDebugPath().c_str(), scope, mode);
	MY_ASSERT(m_bRealScope);

	SymbolDefObject* pSymbolObj = CScope::findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL, mode);
	if (pSymbolObj)
		return pSymbolObj;

	if (scope >= FIND_SYMBOL_SCOPE_SCOPE)
	{
		if (m_pFuncDeclare->getParent())
		{
			pSymbolObj = m_pFuncDeclare->getParent()->findSymbol(name, scope, mode);
			if (pSymbolObj)
				return pSymbolObj;
		}
	}

	if (scope == FIND_SYMBOL_SCOPE_PARENT && m_pParent)
		return m_pParent->findSymbol(name, scope, mode);

	//TRACE("not found\n");
	return NULL;
}

std::string CFunction::toString(int depth)
{
	std::string ret_s = printTabs(depth);

	//std::string temp_s = m_pFuncDeclare->getDebugPath();
	ret_s += m_func_type->toFuncString(getRelativePath(m_pFuncDeclare->getParent(), m_name));
	/*if (m_func_type->getFuncReturnType())
	  ret_s += m_func_type->getFuncReturnType()->toString() + " ";
	ret_s += getRelativePath(m_pFuncDeclare->getParent(), m_name) + "(";

	for (int i = 0; i < m_func_type->getFuncParamCount(); i++)
	{
		if (i > 0)
			ret_s += ", ";
		CVarDef* pVarDef = m_func_type->getFuncParamAt(i);

		TypeDefPointer pTypeDef = pVarDef->getType();
		if (pTypeDef->getType() != SEMANTIC_TYPE_FUNC)
		{
			while (!pTypeDef->isBaseType())
				pTypeDef = pTypeDef->getBaseType();
			ret_s += pTypeDef->toString();
			ret_s += " " + pVarDef->toString();
		}
		else
		{
			ret_s += pTypeDef->toString();
			break;
		}
	}

	if (m_func_type->hasVArgs())
	{
		if (m_func_type->getFuncParamCount() > 0)
			ret_s += ", ";
		ret_s += "...";
	}

	ret_s += ")";*/
	for (unsigned i = 0; i < m_memberInitList.size(); i++)
	{
		if (i == 0)
			ret_s += " : ";
		else
			ret_s += ", ";
		CExpr2* pExpr2 = m_memberInitList[i].second;
		ret_s += m_memberInitList[i].first.toString() + "(" + pExpr2->toString(0) + ")";
	}
	ret_s += "\n" + printTabs(depth) + "{\n";
	for (int i = 0; i < getChildrenCount(); i++)
	{
		ret_s += ((CStatement*)getChildAt(i))->toString(depth + 1);
	}
	ret_s += printTabs(depth) + "}\n\n";

	return ret_s;
}

CNamespace::CNamespace(CScope* pParent, bool bRealScope, bool bIsNamespace, const std::string& name) : CScope(pParent)
{
	TRACE("Create CNamespace %s(%lx) under %s, bRealScope=%d, bNamespace=%d in %s:%d\n", name.c_str(), long(this), (pParent ? pParent->getDebugPath().c_str() : ""), bRealScope, bIsNamespace, g_cur_file_name.c_str(), g_cur_line_no);

	MY_ASSERT(pParent == NULL || pParent->getGoType() == GO_TYPE_NAMESPACE);
	if (pParent == NULL)
		m_depth = 0;
	else
		m_depth = ((CNamespace*)pParent)->getDepth() + 1;
	m_bNamespace = bIsNamespace;
	m_name = name;
	setRealScope(bRealScope);
	m_extern_modifier = 0;
}

std::string CNamespace::getDebugName()
{
	return getName() + ":" + ltoa(m_bRealScope);
}

void CNamespace::analyzeFuncDef(const SourceTreeNode* pRoot)
{
	//printf("func=%s\n", displaySourceTreeStart(pRoot, depth).c_str());

	// check sync attribute of the func
	SourceTreeNode* pFuncHeaderNode;
	void* pBaseClassInitBlock, *bracket_block;
	bool bPureVirtual;
	std::string asm_string;
	SourceTreeVector attribute_list;
	defFuncDeclGetInfo(pRoot, pFuncHeaderNode, asm_string, attribute_list, bPureVirtual, pBaseClassInitBlock, bracket_block);
	MY_ASSERT(!bPureVirtual);
	MY_ASSERT(asm_string.empty());
	MY_ASSERT(attribute_list.empty());
	MY_ASSERT(bracket_block);

	GrammarFuncHeaderInfo funcHeaderInfo = funcHeaderGetInfo(pFuncHeaderNode);
	FlowType flowType = getFlowTypeByModifierBits(funcHeaderInfo.mod_strings);

	CScope* pScope = this;
	if (!funcHeaderInfo.scope.empty())
	{
		SymbolDefObject* pObj = findSymbolEx(funcHeaderInfo.scope, true);
		switch (pObj->type)
		{
		case GO_TYPE_NAMESPACE:
			pScope = pObj->getNamespace();
			break;
		case GO_TYPE_TYPEDEF:
		{
			TypeDefPointer pTypeDef = pObj->getTypeDef();
			MY_ASSERT(pTypeDef->getType() == SEMANTIC_TYPE_CLASS || pTypeDef->getType() == SEMANTIC_TYPE_STRUCT || pTypeDef->getType() == SEMANTIC_TYPE_UNION);
			CClassDef* pClassDef = pTypeDef->getBaseType()->getClassDef();
			if (!pClassDef->isDefined())
				pClassDef->analyzeByTemplate();
			pScope = pClassDef;
			break;
		}
		case GO_TYPE_TEMPLATE:
			pScope = pObj->getTemplateAt(0);
			break;
		default:
			MY_ASSERT(false);
		}
		MY_ASSERT(pScope);
	}
	funcHeaderInfo.scope.addScope(funcHeaderInfo.name, false);

//printf("***********analyzeFuncDef, name=%s, type=%d\n", name.c_str(), flowType);

	CGrammarAnalyzer ga;
	ga.initWithBlocks(pScope, funcHeaderInfo.params_block);
	SourceTreeNode* pFuncParamsNode = ga.getBlock();
	MY_ASSERT(ga.isEmpty());

	TypeDefPointer type_def_ptr = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, getTypeDefByExtendedTypeNode(funcHeaderInfo.pReturnExtendedType), 0));
	type_def_ptr->setModStrings(funcHeaderInfo.mod_strings);
	type_def_ptr->setMod2Strings(funcHeaderInfo.mod2_strings);
	type_def_ptr->setMod3Strings(funcHeaderInfo.mod3_strings);
	type_def_ptr->setMod4Strings(funcHeaderInfo.mod4_strings);
	type_def_ptr->setFuncReturnTypeNode(dupSourceTreeNode(funcHeaderInfo.pReturnExtendedType));
	type_def_ptr->setThrow(funcHeaderInfo.bThrow, dupSourceTreeNode(funcHeaderInfo.pThrowTypeNode));
	pScope->addFuncParamsToFuncType(type_def_ptr, pFuncParamsNode);
	deleteSourceTreeNode(pFuncParamsNode);
	type_def_ptr->setThrow(funcHeaderInfo.bThrow, funcHeaderInfo.pThrowTypeNode);
	type_def_ptr->setDefLocation(g_cur_file_name, g_cur_line_no);

	SymbolDefObject* pSymbolObj = findSymbolEx(funcHeaderInfo.scope, false);
	CTemplate* pParentTemplate = NULL;
	CFuncDeclare* pFuncDeclare = NULL;
	if (pSymbolObj)
	{
		if (pSymbolObj->type == GO_TYPE_FUNC_DECL)
		{
			pFuncDeclare = findFuncDeclare(pSymbolObj, type_def_ptr);
			if (pFuncDeclare && flowType != type_def_ptr->getFuncFlowType())
				throw("Func " + funcHeaderInfo.scope.toString() + " is declared twice but with different sync attribute");
		}
		else if (pSymbolObj->type == GO_TYPE_TEMPLATE)
		{
			for (unsigned i = 0; i < pSymbolObj->children.size(); i++)
			{
				CTemplate* pTemplate = pSymbolObj->getTemplateAt(i);
				// we don't want to support flow in template for now. So we don't need to care about function template instancing.
				if (pTemplate->funcGetParamCount() == type_def_ptr->getFuncParamCount())
				{
					pParentTemplate = pTemplate;
					break;
				}
			}
			if (!pParentTemplate)
				throw("Cannot find matching template");
		}
		else
			throw(funcHeaderInfo.scope.toString() + " is already defined as a " + getGoTypeName(pSymbolObj->type));
	}

	if (pFuncDeclare == NULL)
	{
		if (funcHeaderInfo.scope.hasRootSign() || funcHeaderInfo.scope.getDepth() > 1)
			throw("Func " + funcHeaderInfo.scope.toString() + " should not have namespace specified");
		pFuncDeclare = new CFuncDeclare(this, funcHeaderInfo.name, type_def_ptr);
		pFuncDeclare->setDefLocation(g_cur_file_name, g_cur_line_no);
		if (pParentTemplate == NULL)
			getRealScope()->addFuncDeclare(pFuncDeclare);
	}

	CFunction* pFunc = new CFunction(getRealScope(), funcHeaderInfo.name, type_def_ptr, flowType, pFuncDeclare);
	addChild(pFunc);

	pFunc->analyze(pBaseClassInitBlock, bracket_block, pRoot);
}

void CNamespace::analyze(CGrammarAnalyzer* pGrammarAnalyzer, const SourceTreeNode* pRoot)
{
	switch (blockGetType(pRoot))
	{
	case BLOCK_TYPE_EXTERN_BLOCK:
	{
		CNamespace* pNamespace = new CNamespace(getRealScope(), false, false, "");
		int modifier_bits;
		void* bracket_block;
		blockExternGetInfo(pRoot, modifier_bits, bracket_block);
		pNamespace->setExternModifier(modifier_bits);
		addChild(pNamespace);

		TRACE("START ANALYZING EXTERN BLOCK %s(0x%lx)\n", pNamespace->getDebugPath().c_str(), long(pNamespace));
		CGrammarAnalyzer ga;
		ga.initWithBlocks((CNamespace*)getRealScope(), bracket_block);
		int idx = 0;
		enterScope(getRealScope());
		while (SourceTreeNode* pNode = ga.getBlock())
		{
			g_cur_file_name = pNode->file_name;
			g_cur_line_no = pNode->line_no;
			TRACE("CNamespace::%s, extern block=%s, ANALYZING LINE %s:%d\n", __FUNCTION__, pNamespace->getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
			pNamespace->analyze(&ga, pNode);
			deleteSourceTreeNode(pNode);
		}
		MY_ASSERT(ga.isEmpty());
		leaveScope();
		TRACE("STOP ANALYZING EXTERN BLOCK %s(0x%lx)\n", pNamespace->getDebugPath().c_str(), long(pNamespace));
		break;
	}
	case BLOCK_TYPE_NAMESPACE:
	{
		std::string name;
		bool bInline;
		void* bracket_block;
		SourceTreeNode* pAttribute;
		blockNamespaceGetInfo(pRoot, bInline, name, pAttribute, bracket_block);
		CNamespace* pNamespace = NULL, *pParentNamespace;
		if (!name.empty())
		{
			SymbolDefObject* pSymbolObj = getRealScope()->findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL);
			if (pSymbolObj)
			{
				if (pSymbolObj->type != GO_TYPE_NAMESPACE)
					throw(name + " is already defined other than namespace");
				pNamespace = pSymbolObj->getNamespace();
			}
			if (pNamespace == NULL)
			{
				pNamespace = new CNamespace(getRealScope(), true, true, name);
				getRealScope()->addNamespace(pNamespace);
			}
			pParentNamespace = pNamespace;
			pNamespace = new CNamespace(pNamespace, false, true, name);
		}
		else
		{
			pParentNamespace = (CNamespace*)getRealScope();
			pNamespace = new CNamespace(getRealScope(), true, true, name);
			((CNamespace*)getRealScope())->addUnnamedNamespace(pNamespace);
		}
		addChild(pNamespace);
		pNamespace->setParent(pParentNamespace); // because the previous addChild() will change its parent

		enterScope(pNamespace->getRealScope());
		CGrammarAnalyzer ga;
		ga.initWithBlocks((CNamespace*)pNamespace->getRealScope(), bracket_block);
		TRACE("START ANALYZING NAMESPACE %s\n", pNamespace->getDebugPath().c_str());
		while (SourceTreeNode* pNode = ga.getBlock())
		{
			g_cur_file_name = pNode->file_name;
			g_cur_line_no = pNode->line_no;
			TRACE("CNamespace::%s, namespace=%s, ANALYZING LINE %s:%d\n", __FUNCTION__, pNamespace->getDebugPath().c_str(), g_cur_file_name.c_str(), g_cur_line_no);
			pNamespace->analyze(&ga, pNode);
			deleteSourceTreeNode(pNode);
		}
		MY_ASSERT(ga.isEmpty());
		leaveScope();
		TRACE("STOP ANALYZING NAMESPACE %s(0x%lx)\n", pNamespace->getDebugPath().c_str(), long(pNamespace));
		break;
	}
	case BLOCK_TYPE_DEF:
	{
		pRoot = blockDefGetNode(pRoot);
		if (defGetType(pRoot) == DEF_TYPE_FUNC_DECL && defFuncDeclIsFuncDef(pRoot))
		{
			analyzeFuncDef(pRoot);
		}
		else
		{
			CStatement* pStatement = new CStatement(this);
			pStatement->setDefLocation(g_cur_file_name, g_cur_line_no);
			pStatement->analyzeDef(pGrammarAnalyzer, pRoot);
			if (pStatement->isFlow())
				fatal_error("global variable cannot be inited with sync expr");
			if (g_cur_file_name == "builtin")
				delete pStatement;
			else
				addChild(pStatement);

			//TRACE("\n===================statement, %s===============\n", pStatement->definedIn().c_str());
			//TRACE("%s\n", pStatement->toString(0).c_str());
			//TRACE("====================================================\n");
		}
		break;
	}
	//case BLOCK_TYPE_FUNC_DEF:
	//	analyzeFuncDef(pRoot);
	//	break;

	default:
		MY_ASSERT(false);
	}
}

std::string CNamespace::toString(int depth)
{
	std::string ret_s;
	int depth2 = depth;

	if (m_pParent)
	{
		if (isNamespace())
			ret_s += printTabs(depth) + "namespace " + getName() + " {\n";
		else
			ret_s += printTabs(depth) + modifierBit2String(m_extern_modifier) + " {\n";
		depth2++;
	}

	enterScope(this);
	for (int i = 0; i < getChildrenCount(); i++)
	{
		CScope* pObject = getChildAt(i);
		//if (pObject->getGoType() == GO_TYPE_STATEMENT || pObject->getGoType() == GO_TYPE_FUNC)
		//	ret_s += g_func_listener(pObject, depth2);
		//else
		ret_s += getChildAt(i)->toString(depth2);
	}

	if (m_pParent)
		ret_s += printTabs(depth) + "}\n";
	leaveScope();

	return ret_s;
}

void CNamespace::addUsingNamespace(CNamespace* pNamespace)
{
	TRACE("CNamespace::%s, path='%s', add one='%s'\n", __FUNCTION__, getDebugPath().c_str(), pNamespace->getDebugPath().c_str());
	m_using_namespaces.push_back(pNamespace);
}

SymbolDefObject* CNamespace::findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode)
{
	TRACE("CNamespace::findSymbol %s in %s, bRealScope=%d, scope=%d, mode=%d, ", name.c_str(), getDebugPath().c_str(), m_bRealScope, scope, mode);

	if (!m_bRealScope)
		return getRealScope()->findSymbol(name, scope, mode);

	SymbolDefObject* pSymbolObj = CScope::findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL, mode);
	if (pSymbolObj)
		return pSymbolObj;

	for (size_t i = 0; i < m_unnamed_namespaces.size(); i++)
	{
		CNamespace* pNamespace = m_unnamed_namespaces[i];
		SymbolDefObject* pSymbolObj = pNamespace->findSymbol(name, FIND_SYMBOL_SCOPE_LOCAL, mode);
		if (pSymbolObj)
			return pSymbolObj;
	}

	if (scope >= FIND_SYMBOL_SCOPE_SCOPE)
	{
		for (size_t i = 0; i < m_using_namespaces.size(); i++)
		{
			CNamespace* pNamespace = m_using_namespaces[i];
			SymbolDefObject* pSymbolObj = pNamespace->findSymbol(name, scope, mode);
			if (pSymbolObj)
				return pSymbolObj;
		}
	}

	if (scope == FIND_SYMBOL_SCOPE_PARENT && m_pParent)
		return m_pParent->findSymbol(name, scope, mode);

	//TRACE("not found\n");
	return NULL;
}

CExpr* createNotExpr(CExpr* pExpr)
{
	ExprType expr_type = pExpr->getExprType();
	if (expr_type != EXPR_TYPE_CONST_VALUE && expr_type != EXPR_TYPE_TOKEN_WITH_NAMESPACE &&
		expr_type != EXPR_TYPE_PARENTHESIS && expr_type != EXPR_TYPE_FUNC_CALL)
	{
		pExpr = new CExpr(NULL, EXPR_TYPE_PARENTHESIS, pExpr);
	}
	return new CExpr(NULL, EXPR_TYPE_NOT, pExpr);
}

TypeDefPointer getRootType(TypeDefPointer pTypeDef, int& depth)
{
	while (!pTypeDef->isBaseType())
	{
		depth += pTypeDef->getDepth();
		pTypeDef = pTypeDef->getBaseType();
	}

	depth += pTypeDef->getDepth();
	return pTypeDef;
}

void semanticPrepareBuildIn()
{
	CGrammarAnalyzer ga;
	ga.initWithBuffer(&g_global_namespace, "builtin", g_buildin_funcs.c_str());
	enterScope(&g_global_namespace);

	try
	{
		while (SourceTreeNode* pNode = ga.getBlock())
		{
			g_cur_file_name = pNode->file_name;
			g_cur_line_no = pNode->line_no;
			g_global_namespace.analyze(&ga, pNode);
			deleteSourceTreeNode(pNode);
		}
	}
	catch (std::string& err_s)
	{
		TRACE("Error in %s:%d, %s\n", g_cur_file_name.c_str(), g_cur_line_no, err_s.c_str());
		MY_ASSERT(false);
	}

	leaveScope();
}

CNamespace* semanticAnalyzeFile(char* file_name, int argc, char* argv[])
{
	//semanticPrepareBuildIn();

	CGrammarAnalyzer ga;
	ga.initWithFile(&g_global_namespace, file_name, argc, argv);
	enterScope(&g_global_namespace);

	try
	{
		while (SourceTreeNode* pNode = ga.getBlock())
		{
			g_cur_file_name = pNode->file_name;
			g_cur_line_no = pNode->line_no;
			TRACE("%s, ANALYZING LINE %s:%d\n", __FUNCTION__, g_cur_file_name.c_str(), g_cur_line_no);
			g_global_namespace.analyze(&ga, pNode);
			deleteSourceTreeNode(pNode);
		}
	}
	catch (std::string& err_s)
	{
		fprintf(stderr, "Error in %s:%d, %s\n", g_cur_file_name.c_str(), g_cur_line_no, err_s.c_str());
		MY_ASSERT(false);
		return NULL;
	}

	leaveScope();
	return &g_global_namespace;
}

bool semanticCheckFunc(void* context, int mode, const SourceTreeNode* pRoot, const GrammarTempDefMap& tempDefMap)
{
	return ((CScope*)context)->onGrammarCheckFunc(mode, pRoot, tempDefMap);
}

bool semanticGrammarCallback(void* context, int mode, std::string& s)
{
	return ((CScope*)context)->onGrammarCallback(mode, s);
}

void semanticInit()
{
	std::vector<std::string> name;
	name.push_back("int");
	g_type_def_int = TypeDefPointer(new CTypeDef(&g_global_namespace, "", name));

	name.clear();
	name.push_back("unsigned");
	g_type_def_unsigned = TypeDefPointer(new CTypeDef(&g_global_namespace, "", name));

	name.clear();
	name.push_back("bool");
	g_type_def_bool = TypeDefPointer(new CTypeDef(&g_global_namespace, "", name));

	name.clear();
	name.push_back("char");
	g_type_def_const_char_ptr = TypeDefPointer(new CTypeDef(&g_global_namespace, "", name));
	g_type_def_const_char_ptr = TypeDefPointer(new CTypeDef(&g_global_namespace, "", g_type_def_const_char_ptr, 1));
	g_type_def_const_char_ptr->setConst();

	name.clear();
	name.push_back("void");
	g_type_def_void = TypeDefPointer(new CTypeDef(&g_global_namespace, "", name));

	g_type_def_void_ptr = TypeDefPointer(new CTypeDef(&g_global_namespace, "", g_type_def_void, 1));

	g_type_def_func_void = TypeDefPointer(new CTypeDef(&g_global_namespace, "", SEMANTIC_TYPE_FUNC, g_type_def_void, 0));

	g_global_symbol_obj.type = GO_TYPE_NAMESPACE;
	g_global_symbol_obj.children.push_back(&g_global_namespace);

	grammarSetCheckFunc(semanticCheckFunc, semanticGrammarCallback);
}

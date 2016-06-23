#ifndef __SEMANTIC__H_
#define __SEMANTIC__H_

#include "grammar.h"
//#include <boost/shared_ptr.hpp>
//#include <boost/enable_shared_from_this.hpp>

struct SymbolDefObject;
class CVarDef;
class CTypeDef;
class CScope;
class CExpr;
class CExpr2;
class CStatement;
class CClassDef;
class CTemplate;
class CFunction;
class CNamespace;
//typedef boost::shared_ptr<CTypeDef>	TypeDefPointer;
class TypeDefPointer;

class TypeDefPointer
{
public:
	TypeDefPointer(CTypeDef* pTypeDef = NULL)
	{
		m_pTypeDef = pTypeDef;
	}

	CTypeDef* operator ->() const
	{
		return m_pTypeDef;
	}

	operator bool() const
	{
		return m_pTypeDef != NULL;
	}

	CTypeDef* get() const
	{
		return m_pTypeDef;
	}

protected:
	CTypeDef*	m_pTypeDef;
};

typedef std::vector<std::string>	StandardType;
typedef std::vector<CScope*>        ScopeVector;
typedef std::vector<CExpr*>			ExprVector;

enum FlowType
{
    FLOW_TYPE_NONE,
    FLOW_TYPE_FLOW,
    FLOW_TYPE_FLOW_ROOT
};

enum GrammarObjectType
{
    // non scopes
    GO_TYPE_TYPEDEF,
    GO_TYPE_FUNC_DECL,
    GO_TYPE_VAR_DEF,
    GO_TYPE_ENUM, // this is only used for now in SymbolDefObject
    GO_TYPE_USING_OBJECTS,
    // scopes
    GO_TYPE_NAMESPACE,
    GO_TYPE_FUNC,
    GO_TYPE_STATEMENT,
    GO_TYPE_CLASS,
    GO_TYPE_TEMPLATE,
    GO_TYPE_EXPR2,
    GO_TYPE_EXPR,
};

enum SemanticDataType
{
	SEMANTIC_TYPE_BASIC,
	SEMANTIC_TYPE_ENUM,
	SEMANTIC_TYPE_UNION,
	SEMANTIC_TYPE_STRUCT,
	SEMANTIC_TYPE_CLASS,
	SEMANTIC_TYPE_FUNC,
    SEMANTIC_TYPE_TYPENAME,
    SEMANTIC_TYPE_TEMPLATE,
    SEMANTIC_TYPE_DATA_MEMBER_POINTER,
    SEMANTIC_TYPE_TYPEOF, // __typeof()
};

enum FindSymbolScope
{
    FIND_SYMBOL_SCOPE_LOCAL,
    FIND_SYMBOL_SCOPE_SCOPE,
    FIND_SYMBOL_SCOPE_PARENT,
};

enum FindSymbolMode
{
    FIND_SYMBOL_MODE_ANY,
    FIND_SYMBOL_MODE_TEMPLATE, // search for template only
    FIND_SYMBOL_MODE_FUNC,
};

class CGrammarObject
{
public:
    CGrammarObject(CScope* pParent) { m_pParent = pParent; m_line_no = 0; m_bHasTypename = false; }
    virtual ~CGrammarObject() {}

    virtual GrammarObjectType getGoType() = 0;

    virtual std::string getName() { return m_name; }
    virtual void setName(const std::string& name) { m_name = name; }
    virtual std::string getDebugName() { return m_name; }

    std::string getPath();
    std::string getDebugPath();

    CScope* getParent() { return m_pParent; }
    void setParent(CScope* pParent) { m_pParent = pParent; }

    void setDefLocation(const StringVector& file_stack, int line_no) { 
		m_file_stack = file_stack; m_line_no = line_no; 
	}
    const std::string getDefFileName() { return m_file_stack.empty() ? "" : m_file_stack.back(); }
    const StringVector& getDefFileStack() { return m_file_stack; }
    int getDefLineNo() { return m_line_no; }
    std::string definedIn() { return (m_file_stack.empty() ? std::string("") : m_file_stack.back()) + ":" + ltoa(m_line_no);}

    // return NULL if not in a tmeplate
    CScope* getParentScope(GrammarObjectType go_type);
    CTemplate* getParentTemplate();
    bool isParentOf(CGrammarObject* pGO);

	bool hasTypename() { return m_bHasTypename; } // the type has typename inside, is not a real type
	void setHasTypename(bool b = true);

protected:
    CScope*         m_pParent;
    std::string     m_name;
    StringVector    m_file_stack;
    int             m_line_no;
	bool			m_bHasTypename;
};

class CTypeDef : /*public boost::enable_shared_from_this<CTypeDef>, */public CGrammarObject
{
public:
	// basic data type
	CTypeDef(CScope* pScope, const std::string& name, const StandardType& basic_tokens);
	// enum or union or struct or class
	CTypeDef(CScope* pScope, const std::string& name, CClassDef* pClassDef);
	// template
	CTypeDef(CScope* pScope, const std::string& name, CTemplate* pTemplate);
	// func or func ptr, depth is 0 means a func declare, otherwise it's a func ptr. pReturnType might be empty pointer for constructor or destructor
	CTypeDef(CScope* pScope, const std::string& name, SemanticDataType type, TypeDefPointer pReturnType, int depth);
	// data member ptr
    CTypeDef(CScope* pScope, const std::string& name, SemanticDataType type, TypeDefPointer pTypeDef, TypeDefPointer pDataScope);
    // template
    CTypeDef(CTemplate* pTemplate);
	// template typename
    CTypeDef(CScope* pScope, const std::string& name, SemanticDataType type);
    // typeof
    CTypeDef(CScope* pScope, const std::string& name, CExpr* pExpr);
	// user def one
	// there's one case that when a struct/class is defined, if want to reference it, we can create a CTypeDef with name set to the name,
	//   pBaseTypeDef set to the real type, pDeclVar set to NULL. We can use isBaseType() to distinguish the two.
	// for typedef types, pDeclVar should not be NULL
	CTypeDef(CScope* pScope, const std::string& name, TypeDefPointer pBaseTypeDef, int extra_depth/*, bool bReference = false, bool bConst = false*/);
	virtual ~CTypeDef();

	//virtual std::string getName() { return m_name; }
	//void setName(const std::string& name) { m_name = name; if (!name.empty()) MY_ASSERT(m_pScope); }
    virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_TYPEDEF; }
    //void setScope(CScope* pScope) { m_pScope = pScope; }
    //CScope* getParent() { return m_pScope; }
	SemanticDataType getType() { return m_type; }
	void setType(SemanticDataType newType) { m_type = newType; }
	//int getModifierBits() { return m_modifier_bits; }
	bool isBaseType() { return !m_pBaseTypeDef; }
	TypeDefPointer sharedFromThis() { return m_shared_from_this; }
	TypeDefPointer getBaseType() { return m_pBaseTypeDef ? m_pBaseTypeDef : m_shared_from_this; }
    CScope* getBaseScope() { return m_pSpecialType; }
    CClassDef* getClassDef() { return (CClassDef*)m_pSpecialType; }
    CTemplate* getTemplate() { return (CTemplate*)m_pSpecialType; }
	int getDepth() { return m_depth; }
    int getFullDepth();
	bool isReference() { return m_nReference > 0; }
	int getReferenceCount() { return m_nReference; }
	void setReference(int n) { m_nReference = n; }
	bool isConst();
	void setConst(bool bConst = true);
    bool isVolatile();
    bool isVirtual();
	std::string getDisplayStr() { return m_display_str; }
	TypeDefPointer getRootBaseType() { if (!m_pBaseTypeDef) return m_shared_from_this; return m_pBaseTypeDef->getRootBaseType(); }
	int getRootDepth() { return m_depth + (m_pBaseTypeDef ? m_pBaseTypeDef->getRootDepth() : 0); }

	bool isVoid();
	bool isNumType(); // char or int or wchar_t or float...
	bool isZero() { return m_bHasValue && m_numValue == 0; }
	void setZero() { m_bHasValue = true; m_numValue = 0; }
	bool hasNumValue() { return m_bHasValue; }
	int getNumValue() { MY_ASSERT(m_bHasValue); return m_numValue; }
	void setHasNumValue(int v) { m_bHasValue = true; m_numValue = v; }

	void setPrefix(const std::string& prefix) { m_prefix = prefix; }
	std::string getPrefix() { return m_prefix; }
	//void setDeclspecStrings(const std::string& declspec_strings) { m_func_declspec_strings = declspec_strings; }

	TypeDefPointer getFuncReturnType() { return m_pFuncReturnType; }
	void setFuncReturnType(TypeDefPointer pTypeDef) { m_pFuncReturnType = pTypeDef; m_pFuncReturnTypeNode = NULL; }
	void addFuncParam(CVarDef* pParam);
	int getFuncParamCount() { return m_func_params.size(); } // vargs doesn't count in param count
	CVarDef* getFuncParamAt(int i);
	bool hasVArgs() { return m_bHasVArgs; }
	void setHasVArgs(bool bHasVArgs = true) { m_bHasVArgs = bHasVArgs; }
	void clearAllFuncParams() { m_func_params.clear(); m_bHasVArgs = false; }
	bool isFuncFlow() { return getFuncFlowType() != FLOW_TYPE_NONE; }
	FlowType getFuncFlowType();
	void setFuncFlowType(FlowType flow);
	bool isPureVirtual() { return m_bPureVirtual; }
    void setPureVirtual(bool bPureVirtual) { m_bPureVirtual = bPureVirtual; }
    int getFuncThrow() { return m_nFuncThrow; }
    SourceTreeNode* getFuncThrowTypeNode() { return m_pThrowTypeNode; }
    void setThrow(int bThrow, SourceTreeNode* pThrowTypeNode);
	void setModStrings(const StringVector& mod_strings);
	void setMod2Strings(const StringVector& mod_strings);
	void setMod3Strings(const StringVector& mod_strings);
	void setMod4Strings(const StringVector& mod_strings);
	void setDisplayFlag(bool bFlag = true) { m_display_flag = bFlag; }
	void setFuncReturnTypeNode(SourceTreeNode* pReturnTypeNode) { m_pFuncReturnTypeNode = pReturnTypeNode; }
    int checkCallParams(const std::vector<TypeDefPointer>& typeList, bool bCallerIsConst);
	const StringVector& getModStrings() { return m_mod_strings; }
	const StringVector& getBasicTokens() { MY_ASSERT(getType() == SEMANTIC_TYPE_BASIC); return m_basic_tokens; }

	virtual std::string toString(int depth = 0, bool bDoxygen = false);
	std::string toFuncString(const std::string& name);
	std::string toBaseTypeString(int depth = 0);
	std::string toFullString();
	void setDisplayString(const std::string& str) { m_display_str = str; }
	static std::string type2String(SemanticDataType basic_type);
	//SourceTreeNode*	getDeclVarNode() { return m_pDeclVarNode; }
	//std::string funcTypeToString(const std::string& name);

    bool is_abstract();
    bool is_class();
	bool is_union();
	bool is_empty();
    bool is_enum();
	bool is_pod();
	bool is_polymorphic();
    bool has_nothrow_assign();
    bool has_nothrow_constructor();
    bool has_nothrow_copy();
    bool has_trivial_assign();
    bool has_trivial_copy();
	bool has_trivial_constructor();
	bool has_trivial_destructor();
	bool has_virtual_destructor();

protected:
	void init();

	//CScope* m_pScope;
	//std::string 	m_name;

	TypeDefPointer	m_shared_from_this;
	SemanticDataType 	m_type;
	TypeDefPointer	m_pBaseTypeDef;
	StandardType 	m_basic_tokens;
	CScope*         m_pSpecialType;
	StringVector	m_mod_strings;
	StringVector	m_mod2_strings;
	StringVector	m_mod3_strings;
	StringVector	m_mod4_strings;
	bool			m_display_flag; // for func type, true means a parenthesis around mod3_strings and name
	std::string		m_prefix;
	bool            m_bHasValue; // only valid when it's basic type int
	int             m_numValue; // only valid when it's basic type int

	int 			m_depth; // relative to m_pBaseTypeDef
	int 			m_nReference;
	std::string		m_display_str;

	//SourceTreeNode*	m_pDeclVarNode;

	CExpr*			m_typeof_expr_node;

	// for base func declare only
	TypeDefPointer	m_pFuncReturnType;
	SourceTreeNode*	m_pFuncReturnTypeNode;
    std::vector<CVarDef*>         m_func_params;
	bool			m_bHasVArgs;
	int             m_nFuncThrow;
	SourceTreeNode* m_pThrowTypeNode;
	bool            m_bPureVirtual;
};

typedef std::vector<TypeDefPointer> TypeDefVector;
typedef std::map<std::string, TypeDefPointer> StringTypeDefMap;

class CFuncDeclare : public CGrammarObject
{
public:
	CFuncDeclare(CScope* pLogicParent, const std::string& name, TypeDefPointer func_type) : CGrammarObject(pLogicParent)
	{
		setName(name);
		m_func_type = func_type;
		m_line_no = 0;
		//m_pLogicParent = pLogicParent;
	}
	virtual ~CFuncDeclare() {}

	//virtual std::string getName() { return m_name; }
    virtual GrammarObjectType getGoType() { return GO_TYPE_FUNC_DECL; }
	TypeDefPointer getType() { return m_func_type; }
	//CScope* getLogicParent() { return m_pLogicParent; }

protected:
	//std::string 	m_name;
	TypeDefPointer	m_func_type;
	//CScope* m_pLogicParent;
};

typedef std::vector<CFuncDeclare*>	FuncDeclareVector;

class CVarDef : public CGrammarObject
{
public:
	// we need pDeclVar here because we might need to support cases like "int a[m][n]";
	CVarDef(CScope* pParent, const std::string& name, TypeDefPointer pBaseType, SourceTreeNode* pDeclVar = NULL, CExpr* pInitExpr = NULL);
	CVarDef(CScope* pParent, const TokenWithNamespace& name, TypeDefPointer pBaseType, SourceTreeNode* pDeclVar = NULL, CExpr* pInitExpr = NULL);
    CVarDef(CScope* pParent, const std::string& name, TypeDefPointer pBaseType, const ExprVector& constructorParamList);
    CVarDef(CScope* pParent, const TokenWithNamespace& name, TypeDefPointer pBaseType, const ExprVector& constructorParamList);
	virtual ~CVarDef();

	//virtual std::string getName() { return m_name; }
    //void setName(const std::string& new_name) { m_name = new_name; }
	virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_VAR_DEF; }
	TypeDefPointer getType() { return m_type; }
	void setType(TypeDefPointer pTypeDef) { m_type = pTypeDef; }

	// if it's a C++ object with constructor parameters
	bool hasConstructor() { return m_bHasConstructor; }

	bool isFlow();

	bool isReference() { return m_pDeclVar ? declVarGetReferenceCount(m_pDeclVar) != 0 : false; }
	void setReference(bool b = true) {
		if (b)
		{
			if (!m_pDeclVar)
				m_pDeclVar = declVarCreateByName("");
			declVarAddModifier(m_pDeclVar, DVMOD_TYPE_REFERENCE);
			if (declVarGetDepth(m_pDeclVar) > declVarGetPointerCount(m_pDeclVar))
			declVarAddModifier(m_pDeclVar, DVMOD_TYPE_PARENTHESIS);
		}
		else
		{
			declVarRemoveModifier(m_pDeclVar, DVMOD_TYPE_REFERENCE);
		}
	}

	void setRestrict(bool bRestrict = true) { m_bRestrict = bRestrict; }
	bool isRestrict() { return m_bRestrict; }
	void setExtern(bool bExtern = true) { m_bExtern = bExtern; }
	bool isExtern() { return m_bExtern; }

	void setDeclVarNode(SourceTreeNode* pDeclVar) { MY_ASSERT(pDeclVar); m_pDeclVar = pDeclVar; }
	SourceTreeNode* getDeclVarNode() { return m_pDeclVar; }
	CExpr* getInitExpr() { return m_pInitExpr; }
	// no need to care about deleting old one
	void setInitExpr(CExpr* pExpr) { m_pInitExpr = pExpr; }
	void addExprInDeclVar(CExpr* pExpr) { m_exprList.push_back(pExpr); }
	const ExprVector& getExprListInDeclVar() { return m_exprList; }

	virtual std::string toString(bool bDumpInitExpr = true);

	bool hasValue() { return m_bHasValue; }
	void setValue(int nValue);
	int getValue() { MY_ASSERT(m_bHasValue); return m_nValue; }

	int getSeqNo() { return m_seq_no; }
	void setSeqNo(int seq) { m_seq_no = seq; }

	bool getAppFlag() { return m_app_flag; }
	void setAppFlag(bool b = true) { m_app_flag = b; }

	// change this var and all statements that use ref it. this var cannot be a global var or defined in function parameters. must be a local var.
	void changeName(const std::string& new_name, CExpr* pNewExpr);
	virtual void setName(const std::string& new_name)
	{
		m_twn.resize(0);
		m_twn.addScope(new_name);
		m_name = new_name;
	}

protected:
	//CScope* m_pParent;

	TokenWithNamespace 	m_twn;
	TypeDefPointer	m_type;
	bool            m_bHasConstructor;
	SourceTreeNode*	m_pDeclVar;
	bool			m_bReference;
	bool			m_bExtern;
	ExprVector		m_exprList; // either expr list in decl var or constructing param list
	CExpr*			m_pInitExpr;

	bool 			m_bRestrict;
    bool            m_bHasValue;
    int             m_nValue;

	int				m_seq_no; // needed by parser
	bool			m_app_flag;
};

class CUsingObject : public CGrammarObject
{
public:
    CUsingObject(const std::string& name, SymbolDefObject* pSymbolObj);
    virtual ~CUsingObject() {}

    virtual GrammarObjectType getGoType() { return GO_TYPE_USING_OBJECTS; }

    SymbolDefObject* getSymbolObj() { return m_pSymbolObj; }

protected:
    SymbolDefObject* m_pSymbolObj;
};

// func template stores as GO_TYPE_FUNC_DECL along with pFuncDeclare.
struct SymbolDefObject {
	GrammarObjectType	type;
	std::vector<CGrammarObject*> children;
	TypeDefPointer pTypeDef;
	int nValue; // for enum only
	//SymbolDefObject*    pUsingObjects;

	TypeDefPointer getTypeDef()
	{
	    MY_ASSERT(type == GO_TYPE_TYPEDEF);
        CGrammarObject* pObj = children[0];
	    if (pTypeDef)
	    {
	        MY_ASSERT(pObj->getGoType() == GO_TYPE_TYPEDEF);
            MY_ASSERT(pTypeDef.get() == (CTypeDef*)pObj);
            return pTypeDef;
	    }
	    MY_ASSERT(pObj->getGoType() == GO_TYPE_USING_OBJECTS);
	    return ((CUsingObject*)pObj)->getSymbolObj()->getTypeDef();
	}

	CGrammarObject* findChildByType(GrammarObjectType t)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			CGrammarObject* pObj = children[i];
			if (pObj->getGoType() == t)
				return pObj;
			if (pObj->getGoType() == GO_TYPE_USING_OBJECTS && ((CUsingObject*)pObj)->getSymbolObj()->type == t)
				return pObj;
		}
		return NULL;
	}

    CTemplate* getTemplateAt(unsigned i = 0)
    {
        MY_ASSERT(type == GO_TYPE_TEMPLATE);
        MY_ASSERT(i >= 0 && i < children.size());
        CGrammarObject* pObj = children[i];
        if (pObj->getGoType() == GO_TYPE_TEMPLATE)
            return (CTemplate*)pObj;
        //printf("getTemplateAt, check using objects 0x%lx\n", (unsigned long)((CUsingObject*)pObj)->getSymbolObj());
        MY_ASSERT(pObj->getGoType() == GO_TYPE_USING_OBJECTS);
        MY_ASSERT(((CUsingObject*)pObj)->getSymbolObj()->children.size() == 1);
        return ((CUsingObject*)pObj)->getSymbolObj()->getTemplateAt(0);
    }
    CNamespace* getNamespace()
    {
        MY_ASSERT(type == GO_TYPE_NAMESPACE);
        MY_ASSERT(children.size() == 1);
        CGrammarObject* pObj = children[0];
        if (pObj->getGoType() == GO_TYPE_NAMESPACE)
            return (CNamespace*)pObj;
        MY_ASSERT(pObj->getGoType() == GO_TYPE_USING_OBJECTS);
        MY_ASSERT(((CUsingObject*)pObj)->getSymbolObj()->children.size() == 1);
        return ((CUsingObject*)pObj)->getSymbolObj()->getNamespace();
    }
    CFuncDeclare* getFuncDeclareAt(unsigned i)
    {
        MY_ASSERT(type == GO_TYPE_FUNC_DECL);
        MY_ASSERT(i >= 0 && i < children.size());
        CGrammarObject* pObj = children[i];
        if (pObj->getGoType() != GO_TYPE_USING_OBJECTS)
	        return (CFuncDeclare*)pObj;
        return ((CUsingObject*)pObj)->getSymbolObj()->getFuncDeclareAt(0);
    }
    CVarDef* getVarDef()
    {
        //MY_ASSERT(type == GO_TYPE_VAR_DEF);
        //MY_ASSERT(children.size() == 1);
		for (size_t i = 0; i < children.size(); i++)
		{
			CGrammarObject* pObj = children[i];
			if (pObj->getGoType() == GO_TYPE_VAR_DEF)
				return (CVarDef*)pObj;
			if (pObj->getGoType() == GO_TYPE_USING_OBJECTS)
			{
				CVarDef* pVarDef = ((CUsingObject*)pObj)->getSymbolObj()->getVarDef();
				if (pVarDef)
					return pVarDef;
			}
		}
		return NULL;
    }
	void removeVarDef()
	{
		for (unsigned i = 0; i < children.size(); i++)
		{
			CGrammarObject* pObj = children[i];
			if (pObj->getGoType() == GO_TYPE_VAR_DEF)
			{
				children.erase(children.begin() + i);
				return;
			}
			if (pObj->getGoType() == GO_TYPE_USING_OBJECTS)
			{
				CVarDef* pVarDef = ((CUsingObject*)pObj)->getSymbolObj()->getVarDef();
				if (pVarDef)
				{
					children.erase(children.begin() + i);
					return;
				}
			}
		}
		MY_ASSERT(false);
	}
	std::string definedIn() const
	{
	    return children[0]->getDefFileName() + ":" + ltoa(children[0]->getDefLineNo());
	}
};

typedef std::map<std::string, SymbolDefObject>	SymbolDefMap;

struct NamespaceTokenPair
{
	CNamespace* pNamespace;
	std::string	token;
};

struct ClassBaseTypeDef {
    bool bVirtual;
    ClassAccessModifierType cam_type;
    SourceTreeNode* pUserDefTypeNode;
    CScope* pBaseScope; // could be class or template
};
typedef std::vector<ClassBaseTypeDef> ClassBaseTypeDefVector;

class CScope : public CGrammarObject
{
public:
    CScope(CScope* pParent, const std::string& name = "");
	virtual ~CScope();

    CFunction* getFunctionScope();
	bool isRealScope() { return m_bRealScope; }
	CScope* getRealScope() { return m_bRealScope ? this : m_pParent->getRealScope(); }
	void setRealScope(bool bRealScope) { m_bRealScope = bRealScope; }
	virtual std::string toString(int depth = 0) = 0;

	void createDoxygenFile();
	FILE* openDoxygenFile();
	const std::string& getDoxygenFName() { return m_doxygen_fname; }
	std::string getDoxygenLink(const std::string& name) { return std::string("<a href=") + m_doxygen_fname + "><b>" + name + "</b></a>"; }

	bool isTransformed() { return m_bTransformed; }
	void setTransformed() { if (getParent() == NULL) return; m_bTransformed = true; if (getParent()) getParent()->setTransformed(); }
	unsigned getChildrenCount() { return m_children.size(); }
	CScope* getChildAt(int idx) { return m_children[idx]; }
	void addChild(CScope* pObj)
	{
	    if (pObj)
	        pObj->setParent(this);
		m_children.push_back(pObj);
		//printf("parent %lx adding %lx, {%s}\n", this, pObj, pObj->toString(2).c_str());
	}
	void removeChildAt(int idx) { m_children.erase(m_children.begin() + idx); }
    void removeAllChildren() { m_children.clear(); }
	void insertChildAt(int idx, CScope* pObj)
	{
		pObj->setParent(this);
		m_children.insert(m_children.begin() + idx, pObj);
	}
	void insertChildrenAt(int idx, const std::vector<CScope*> vec)
	{
		for (size_t i = 0; i < vec.size(); i++)
		{
			CScope* pObj = vec[i];
			pObj->setParent(this);
			m_children.insert(m_children.begin() + idx, pObj);
			idx++;
		}
	}
	void setChildAt(int idx, CScope* pObj)
	{
		pObj->setParent(this);
		m_children[idx] = pObj;
	}

	// the logic to locate a symbol, no matter what type it is, is to first find the symbol in a nearest namespace (based on namespace searching precedence),
	// if that symbol's type is not the one you are looking for, then report error.
	// for func declarations, C++ allows multiple func declaration with the same name but with different parameters. (even one is int another is short is allowed)
	// when looking for a func declaration, it first find the nearest namespace that contains the symbol, if it's not a func declaration, report error.
	// then, check whether it can match any one in all declarations in that namespace. if no match, report error.
	void addNamespace(CNamespace* pNamespace);
	void addFuncDeclare(CFuncDeclare* pFuncDecl);
	void addTypeDef(TypeDefPointer pDef);
	void addVarDef(CVarDef* pVar);
	void removeVarDef(CVarDef* pVar);
	void addEnumDef(const std::string& name, CClassDef* pClassDef, int nValue);
    void addTemplate(CTemplate* pTemplate);
    void addUsingObjects(const std::string& name, SymbolDefObject* pObj);
	virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode = FIND_SYMBOL_MODE_ANY);
	CFuncDeclare* findFuncDeclare(SymbolDefObject* pDefObj, int paramCount = -1);
	CFuncDeclare* findFuncDeclare(SymbolDefObject* pDefObj, TypeDefPointer pTypeDef = TypeDefPointer());
	CFuncDeclare* findFuncDeclare(const std::string& name, const ExprVector& param_list, bool bCallerIsConst = false);
	//TypeDefPointer findTypeDef(const std::string& name);
	//CVarDef* findVarDef(const std::string& name, bool bCheckParent = true);
	//bool findEnumDef(const std::string& name);
	//void addUsingNamespace(CNamespace* pNamespace, const std::string& token); // if token is not empty, it means only the token under this namespace is allowed.
	//std::vector<NamespaceTokenPair> getUsingNamespaceList() { return m_using_namespace_list; }
	// bCheckingParents is only valid when twn doesn't have scope defined.
	SymbolDefObject* findSymbolEx(const TokenWithNamespace& twn, bool bCheckingParents = true, bool bCreateIfNotExists = false, bool bCreateAsType = false);
	CGrammarObject* findRoughSymbol(const TokenWithNamespace& twn);
	CScope* getScope(const TokenWithNamespace& twn);

	int onGrammarCheckFunc(const TokenWithNamespace& twn, const GrammarTempDefMap& tempDefMap);
    bool onGrammarCallback(int mode, std::string& s);

	//std::string typeDescBlock2String(const TypeDescBlock& block);
	/*CTypeDef* funcTypeNode2TypeDef(const SourceTreeNode* pRoot);
	TypeDefPointer typeNode2TypeDef(const SourceTreeNode* pRoot, bool bDefineStructUnion = false);
	TypeDefPointer extendedTypeNode2TypeDef(const SourceTreeNode* pRoot);
	TypeDefPointer extendedTypeVarNode2TypeDef(const SourceTreeNode* pRoot);
	void structUnionAddDefs(TypeDefPointer pTypeDef, const SourceTreeVector& struct_items);*/

	TypeDefPointer getTypeDefByUserDefTypeNode(const SourceTreeNode* pRoot, bool bDefining = false, bool bHasTypename = false);
	TypeDefPointer getTypeDefByTypeNode(const SourceTreeNode* pRoot, bool bDefining = false, bool bAllowUndefinedStruct = false);
	TypeDefPointer getTypeDefBySuperTypeNode(const SourceTreeNode* pRoot, bool bDefining = false);
	TypeDefPointer getTypeDefByExtendedTypeNode(const SourceTreeNode* pRoot, bool bDefining = false);
	TypeDefPointer getTypeDefByExtendedOrFuncTypeNode(const SourceTreeNode* pRoot, bool bDefining = false);
	TypeDefPointer getTypeDefByExtendedTypeVarNode(const SourceTreeNode* pRoot, bool bDefining = false);
	TypeDefPointer getTypeDefByFuncTypeNode(const SourceTreeNode* pRoot);
	TypeDefPointer getTypeDefByDeclVarNode(TypeDefPointer, const SourceTreeNode* pRoot);

    TypeDefPointer createClassAsChild(const std::string& name, SemanticDataType data_type);

	void addFuncParamsToFuncType(TypeDefPointer pTypeDef, const SourceTreeNode* pFuncParamsNode);
	bool checkTemplateParamHasTypename(const TokenWithNamespace& twn, int idx);
	bool checkExprNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkScopeNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkExpr2NodeHasTypename(const SourceTreeNode* pRoot);
	bool checkTypeNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkExtendedTypeNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkExtendedTypeVarNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkFuncTypeNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkUserDefTypeNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkExtendedOrFuncTypeNodeHasTypename(const SourceTreeNode* pRoot);
	bool checkTokenWithNamespaceHasTypename(const TokenWithNamespace& twn);

public:
	bool					m_bRealScope; // extern "C", CExpr, defs with no big brackets, non compound statements should set to false
	bool					m_bTransformed;
	FILE*					m_doxygen_fp;
	std::string				m_doxygen_fname;
	CScope*					m_doxygen_open_next; // for not open too many files
	CScope*					m_doxygen_open_prev;
	//CScope*				m_pParent;
	std::vector<CScope*>	m_children;
	SymbolDefMap 			m_symbol_map;
	//std::vector<NamespaceTokenPair>	m_using_namespace_list;
};

class CExpr : public CScope
{
public:
    CExpr(CScope* pParent);
	CExpr(CScope* pParent, const SourceTreeNode* pRoot);
	virtual ~CExpr();

	CExpr(CScope* pParent, ExprType expr_type, const std::string& val);
	CExpr(CScope* pParent, ExprType expr_type, const TokenWithNamespace& twn);
    CExpr(CScope* pParent, ExprType expr_type, TypeDefPointer pTypeDef);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, const std::string& token);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, const TokenWithNamespace& token);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, CExpr* pRight);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr);
	CExpr(CScope* pParent, ExprType expr_type, TypeDefPointer pTypeDef, CExpr* pLeft); // type cast
    CExpr(CScope* pParent, ExprType expr_type, CFuncDeclare* pFuncDeclare, const std::vector<CExpr*>& param_list); // func call
    CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr, const std::vector<CExpr*>& param_list);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pCondExpr, CExpr* pExpr1, CExpr* pExpr2); // ? : 
    CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr, TypeDefPointer pTypeDef, const std::vector<CExpr*>& param_list); // new replacement

    virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_EXPR; }
	virtual ExprType getExprType() { return m_expr_type; }
	virtual void setExprType(ExprType newType) { m_expr_type = newType; }
	virtual TypeDefPointer getReturnType() { return m_return_type; }
	virtual void setReturnType(TypeDefPointer new_type) { m_return_type = new_type; }
	virtual int getReturnDepth() { return m_return_depth; }
    virtual void setReturnDepth(int returnDepth) { m_return_depth = returnDepth; }
    virtual SymbolDefObject* getReturnSymbolObj() { return m_return_symbol_obj; }
    virtual void setReturnSymbolObj(SymbolDefObject* obj) { m_return_symbol_obj = obj; }
	virtual bool isFlow() { return m_bFlow; }
	virtual void setFlow(bool bFlow) { m_bFlow = bFlow; }
	//SourceTreeNode*	getSourceTreeNode() { return m_pNode; }
	//void setSourceTreeNode(const SourceTreeNode* pRoot);
	std::string getValue() { return m_value; }
	void setValue(const std::string& s) { m_value = s; }
	TokenWithNamespace getTWN() { return m_token_with_namespace; }
	void setTWN(const TokenWithNamespace& twn) { m_token_with_namespace = twn; }
	TypeDefPointer getFuncCallType() { return m_pFuncCallType; }
	void setFuncCallType(TypeDefPointer pTypeDef) { m_pFuncCallType = pTypeDef; }
    CFuncDeclare* getFuncDeclare() { return m_pFuncDeclare; }
    void setFuncDeclare(CFuncDeclare* pFuncDeclare) { m_pFuncDeclare = pFuncDeclare; }

	virtual void analyze(const SourceTreeNode* pRoot);
	virtual std::string toString(int depth = 0);

	virtual void changeRefVarName(const std::string& old_name, CExpr* pNewExpr);

	virtual bool calculateNumValue(float& f);

	static CExpr* copyExpr(CExpr*);

protected:
	void init();

	ExprType		m_expr_type;
	TypeDefPointer	m_return_type;
	int				m_return_depth;
	bool			m_bFlow;
	SymbolDefObject*  m_return_symbol_obj; // this is only intermediate variable after analyzing name but before analyzing func parameters
	bool            m_caller_is_const; // only valid when m_return_symbol_obj is to be used. to determine which func to choose

	// func call type
	TypeDefPointer	m_pFuncCallType;
	CFuncDeclare*	m_pFuncDeclare;

	// const_value, ref_element, ptr_element
	std::string		m_value;

	// token with namespace
	TokenWithNamespace	m_token_with_namespace;

	// sizeof: extended_type_var, new: extended_type_var, type_cast
	//SourceTreeNode*	m_pNode;
	TypeDefPointer	m_pTypeDef, m_pTypeDef2;

	// type_cast
	SourceTreeNode*		m_pSourceTreeNode;
	// delete: has []
	bool 			m_bFlag;
};

typedef std::vector<CExpr*> ExprVector;

class CExpr2 : public CExpr
{
public:
    CExpr2(CScope* pParent, const SourceTreeNode* pRoot = NULL);
    virtual ~CExpr2();

    virtual GrammarObjectType getGoType() { return GO_TYPE_EXPR2; }
    virtual ExprType getExprType() { return EXPR_TYPE_COMMA; }
    virtual void setExprType(ExprType newType) { MY_ASSERT(false); }

    virtual void analyze(const SourceTreeNode* pRoot);
    virtual std::string toString(int depth = 0);

    void addExpr(CExpr* pExpr);
    virtual bool calculateNumValue(float& f);

	void changeRefVarName(const std::string& old_name, CExpr* pNewExpr);
};

class CClassDef : public CScope
{
public:
	CClassDef(CScope* pParent, SemanticDataType basic_type, const std::string& name);
	virtual ~CClassDef() {}

    virtual GrammarObjectType getGoType() { return GO_TYPE_CLASS; }
	virtual std::string toString(int depth = 0);

    virtual std::string getDebugName();
	SemanticDataType getType() { return m_type; }
	virtual std::string getName() { return m_name; }
    std::string getTemplateName() { return m_template_name; }
	std::string getClassName() { return m_template_name.empty() ? getName() : getTemplateName(); }
	bool isDefined() { return m_bDefined; }
	void setDefined(bool bDefined) { m_bDefined = bDefined; }
    bool isWithinTemplate() { return m_bWithinTemplate; }
    void setWithinTemplate(bool b = true) { m_bWithinTemplate = b; }

	virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode = FIND_SYMBOL_MODE_ANY);
	SymbolDefObject* findSymbolInBaseClasses(const std::string& name);

	void analyze(const SourceTreeNode* pRoot);
	void analyzeClassDef(const ClassBaseTypeDefVector& classBaseTypeDefs, void* bracket_block, const StringVector& tokens = StringVector());
	void addDef(CStatement* pStatement);

	void setTypeDef(TypeDefPointer pTypeDef) { m_pTypeDef = pTypeDef; }
	TypeDefPointer getTypeDef() { return m_pTypeDef; }

    bool hasBaseClass(CClassDef* pClassDef);
    bool hasConstructorOrCanBeAssignedWith(TypeDefPointer pTypeDef);

	void addBaseClass(CClassDef* pClassDef, ClassAccessModifierType cam_type = CAM_TYPE_PUBLIC, bool bVirtual = false);

	// if it's a class or struct who has a constructor or has a member who has a constructor
	bool has_constructor();

    bool is_abstract();
    bool is_empty(); // don't have constructor or destructor, don't have protected, private or vitural method
    bool is_pod();
	bool is_polymorphic();
    bool has_nothrow_assign();
    bool has_nothrow_constructor();
    bool has_nothrow_copy();
    bool has_trivial_assign();
    bool has_trivial_copy();
    bool has_trivial_constructor();
    bool has_trivial_destructor();
    bool has_virtual_destructor();

    bool analyzeByTemplate();

protected:
	struct BaseClassCAMPair {
	    bool bVirtual;
	    ClassAccessModifierType cam_type;
	    TypeDefPointer  pTypeDef;
		SourceTreeNode*	pTypeNode;
	};
    struct TempBlock {
	    TempBlock(CFunction* p, void* i, void* b, const SourceTreeNode* r) : pFunc(p), pBaseClassInitBlock(i), bracket_block(b), pRoot(r) {}
        CFunction* pFunc;
        void* pBaseClassInitBlock;
        void* bracket_block;
        const SourceTreeNode* pRoot;
    };
    struct ClassDefBlock {
	    ClassDefBlock(CStatement* p, const SourceTreeNode* r) : pStatement(p), pRoot(r) {}
        CStatement* pStatement;
        const SourceTreeNode* pRoot;
    };
    friend class CTemplate;
	bool			                m_bDefined;	// for struct or union only
	bool                            m_bWithinTemplate; // it's a class within a template, not instanced. its instanced class should be under the instance template of its parent template
	bool							m_enum_has_last_comma;
	SemanticDataType                m_type;
	std::string                     m_template_name;
	std::vector<BaseClassCAMPair>   m_baseTypeList;
	StringVector					m_mod_strings;

	TypeDefPointer                  m_pTypeDef;
};

struct TemplateResolvedDefParam {
    std::string     typeName;
    TypeDefPointer  pTypeDef;
    long            numValue;
    int             flags; // only valid in some calculation, bit 0: bType, bit 1: value assigned
};
typedef std::vector<TemplateResolvedDefParam> TemplateResolvedDefParamVector;

struct FuncParamItem {
    FuncParamType param_type;
    StringVector mod_strings;
    // if it's a recognized type, then pTypeDef is set, pDeclVarNode only carries var name; otherwise, pTypeNode and pDeclVarNode is used.
    SourceTreeNode* pTypeNode, *pDeclVarNode, *pInitExprNode;
    TypeDefPointer pTypeDef;
};

// for class template, there's the root template on the top, below are specialized templates if any.
// under these templates, are instanced templates. And there's one instanced class under each instanced template.
// because in class template implementation, it can directly refer the template name as type name, so we need to create a fake class directly under the root template to refer to this type
class CTemplate : public CScope
{
public:
    struct TypeParam {
        TemplateParamType type;
		bool bClass; // for data type, it's a class or typename
        bool bHasTypename;
        bool bHasDefault;
		bool bDefaultDataOrFuncType;
        std::string name;
        TypeDefPointer  pNumValueType; // data type of num def
        SourceTreeNode* pTypeNode; // only valid for data_type
        SourceTreeNode* pDefaultNode; // default type of type def, or default value of num def
    };
    typedef std::vector<TypeParam> TypeParamVector;

    CTemplate(CScope* pParent, TemplateType type, const std::string& name, const std::string& template_name = "");
    virtual ~CTemplate();

    virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_TEMPLATE; }
    std::string toHeaderString(int depth, bool bDoxygen = false);
    virtual std::string toString(int depth = 0) { MY_ASSERT(false); return ""; }
    virtual std::string toString(bool bDefine, int depth = 0);
    bool isDefined() { return m_bDefined; }
    TemplateType getTemplateType() { return m_nTemplateType; }
    //virtual std::string getName() { return m_name; }
    std::string getTemplateName() { return m_template_name; }
    bool isSame(const CTemplate* pTemplate) const;
    SemanticDataType getClassType() { return m_data_type; } // struct or class
	int getMatchedScore() { MY_ASSERT(isInstancedTemplate()); return m_matched_score; }
	void setMatchedScore(int score) { MY_ASSERT(isInstancedTemplate()); m_matched_score = score; }
	void writeDoxygenHeader();

    bool isRootTemplate() { return m_name == m_template_name && m_specializedTypeParams.empty(); }
    bool isSpecializedTemplate() { MY_ASSERT(getTemplateType() == TEMPLATE_TYPE_CLASS); return m_name == m_template_name && !m_specializedTypeParams.empty(); }
    bool isInstancedTemplate() { return m_name != m_template_name; }
	const std::string getTemplateRoleString() { return isRootTemplate() ? "Root" : (isSpecializedTemplate() ? "Specialized" : (isInstancedTemplate() ? "Instanced" : "Unkonwn")); }

    virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode = FIND_SYMBOL_MODE_ANY);
    SymbolDefObject* findSymbolInBaseClasses(const std::string& name);

    void addHeaderTypeDefs(const std::vector<void*>& header_types);
    void analyzeBody(CGrammarAnalyzer* pGrammarAnalyzer);

    void analyzeFunc(const std::vector<void*>& header_types, const SourceTreeNode* pRoot);
    void analyzeBaseClass(const std::vector<void*>& header_types, const SourceTreeNode* pRoot);
    CTemplate* analyzeSpecializedClass(const std::vector<void*>& header_types, const SourceTreeNode* pRoot);
    void saveClassBody(void* pBaseClassDefsBlock, void* body_data);
    void analyzeClassBody(void* pBaseClassDefsBlock, void* body_data);
    void analyzeVar(const SourceTreeNode* pRoot);
    void analyzeFuncVar(const SourceTreeNode* pRoot);
    void analyzeFriendClass(const SourceTreeNode* pRoot);
    void mergeWithBaseClass(CTemplate* pNewTemplate, bool bReplaceName);
    void mergeWithSpecializedClass(CTemplate* pNewTemplate, bool bReplaceName);

    int funcGetParamCount() { return (int)m_funcParams.size(); }
    int funcRootCheckFitForTypeList(const TypeDefVector& typeList, TemplateResolvedDefParamVector& resovledDefParams);
    int funcInstancedCheckFitForTypeList(const TypeDefVector& typeList, TemplateResolvedDefParamVector& resovledDefParams);
    TypeDefPointer funcGetInstance(const TypeDefVector& typeList, const TemplateResolvedDefParamVector& resovledDefParams);
	CTemplate* funcRootInstanceByTypeParams(const TypeDefVector& typeList);

    TypeDefPointer classGetInstance(const TokenWithNamespace& twn, int idx, CScope* pOtherScope);
    CTemplate* getTemplateByParams(const TokenWithNamespace& twn, int depth);

	bool canBeSpecialized(const TemplateResolvedDefParamVector& typeList);
	CTemplate* analyzeSpecialize(const SourceTreeNode* pRoot, CSUType csu_type, SourceTreeNode* pBaseClassDefsNode, void* body_data, const TemplateResolvedDefParamVector& typeList);

    std::vector<TypeParam> getTypeParams() { return m_typeParams; }
    CClassDef* getRootInstancedClassDef() { return m_instanced_class.getTypeDef()->getClassDef(); }

    std::string getUniqueId() { MY_ASSERT(isSpecializedTemplate()); return m_uniqueId; }
    void addSpecializedTemplate(CTemplate* pTemplate);
    CTemplate* findSpecializedTemplateByUniqueId(const std::string& uniqueId);

    void setResolvedDefParams(const TemplateResolvedDefParamVector& v);
	TemplateResolvedDefParamVector prepareResolvedDefParams();
    int getScore() { return m_score; }
    void setScore(int score) { m_score = score; }

    SymbolDefObject* findSpecializedTemplate(CTemplate* pSpecializedTemplate)
    {
        for (unsigned i = 0; i < m_specializeDefList.size(); i++)
        {
          if (m_specializeDefList[i].getTemplateAt(0) == pSpecializedTemplate)
            return &m_specializeDefList[i];
        }
        return NULL;
    }

    CTemplate* duplicateAsChild(const std::string& name);

protected:
	friend class CClassDef;
	friend class CStatement;
	friend class CScope;

	void createParamTypeFromTemplateHeader(const SourceTreeNode* pRoot);
	void readTemplateHeaderIntoTypeParams(const std::vector<void*>& header_types);
    static int findResolvedDefParam(const TemplateResolvedDefParamVector& v, const std::string& name);
    bool compareResolvedDefParams(const TemplateResolvedDefParamVector& typeList, const TemplateResolvedDefParamVector& typeList2);
    int resolveParamNameType(const TokenWithNamespace& twn, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams);
    int resolveParamType(const SourceTreeNode* pExtendedTypeNode, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams, bool bMatchType = false);
    int resolveParamFunc(const SourceTreeNode* pFuncNode, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams);
    int resolveParamNumValue(const SourceTreeNode* pExprNode, long numValue, TemplateResolvedDefParamVector& resolvedDefParams);
    TypeParam readTemplateTypeParam(const SourceTreeNode* pChild);
    CTemplate* classResolveParamForBaseTemplate(const TemplateResolvedDefParamVector& realTypeList);
    int classResolveParamForSpecializedTemplate(const TemplateResolvedDefParamVector& realTypeList, CTemplate*& pRetTemplate);
    CTemplate* classMatchForATemplate_(const TemplateResolvedDefParamVector& realTypeList);
    TypeDefPointer classGetInstance_();

    bool inParamTypeNames(const std::string s);

    void clear();
    void clearTypeAndVarDefs();

    bool                    m_bDefined;
    TemplateType            m_nTemplateType;
    SemanticDataType		m_data_type; // for class template, is it a struct or a class
    std::string             m_template_name;
    std::vector<TypeParam>  m_typeParams;
    std::vector<TypeParam>  m_specializedTypeParams; // for specialized template, in this list, all type and num values are stored in pDefaultExtendedTypeNode
	unsigned				m_specializedTypeParamCount;
    std::string				m_uniqueId;
	// for root template, store matched type_str and instanced templates. NULL template pointer means unmatched
	// for func template, the key is func param list
	std::map<std::string, CTemplate*>	m_matched_typestr_map; 
	int						m_matched_score; // for instanced template

    SourceTreeNode*         m_pFuncReturnExtendedTypeNode; // for func and var
    std::vector<FuncParamItem>  m_funcParams;
    bool                    m_func_hasVArgs;
	StringVector			m_mod_strings;
	StringVector			m_mod2_strings;
	StringVector			m_mod3_strings;
	StringVector			m_mod4_strings;
	std::string				m_throw_string;

    ClassBaseTypeDefVector  m_classBaseTypeDefs;
    //TypeDefVector           m_classTypeDefList; // class instances stored here. func instances stored in m_children

    std::vector<SymbolDefObject>	m_specializeDefList; // special list of root template

    TemplateResolvedDefParamVector m_resolvedDefParams; // for instanced template only, which type is a type param defined to, e.g. T=int*
    //TemplateResolvedDefParamVector m_resolvedTypeParams; // for instanced template only, list of real types and values e.g. <int*, string>
    TokenWithNamespace             m_varName; // for VAR and friend class only
    CSUType                 m_csu_type; // for friend_class only
    int                     m_score; // for specialized template only

    SymbolDefObject         m_instanced_class; // for root class template only.
	TypeDefPointer			m_instanced_func_type; // for instanced func template only. store func declare type
	StringVector			m_func_base_init_sv;
    StringVector            m_body_sv;
	std::vector<SourceTreeNode*>	m_var_array_node_list;
};

class CStatement : public CScope
{
public:
	CStatement(CScope* pParent);
	CStatement(CScope* pParent, StatementType type); // break, return
	CStatement(CScope* pParent, StatementType type, CExpr* pExpr); // expr, return
	CStatement(CScope* pParent, StatementType type, CExpr2* pExpr2); // expr, return
	CStatement(CScope* pParent, DefType defType, ClassAccessModifierType cam_type);
    CStatement(CScope* pParent, DefType defType, CSUType csu_type, TypeDefPointer pTypeDef);
	CStatement(CScope* pParent, StatementType type, TypeDefPointer pTypeDef);
	CStatement(CScope* pParent, StatementType type, CVarDef* pVar, SourceTreeNode* pTypeNode = NULL); // def
	CStatement(CScope* pParent, StatementType type, CExpr* pExpr, CStatement* pStatement); // if or while
	CStatement(CScope* pParent, StatementType type, const std::vector<CScope*>& vec); // compound
	virtual ~CStatement();

	virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_STATEMENT; }
	StatementType getStatementType() { return m_statement_type; }
    void setStatementType(StatementType newType) { m_statement_type = newType; }
	DefType getDefType() { return m_def_type; }
	bool isFlow() { return m_bFlow; }
	void setFlow(bool bFlow) { m_bFlow = bFlow; }
	TypeDefPointer getTypeDef() { return m_pTypeDef; }
	void setTypeDef(TypeDefPointer pTypeDef) { m_pTypeDef = pTypeDef; }
	CFuncDeclare* getFuncDeclare() { return m_pFuncDeclare; }
	void setFuncDeclare(CFuncDeclare* pFuncDeclare) { m_pFuncDeclare = pFuncDeclare; }
	bool isAppFlagSet() { return m_bAppFlag; }
	void setAppFlag(bool bSet = true) { m_bAppFlag = bSet; }

	// for var def statement;
	//std::string getDefTypeString() { return displaySourceTreeType(m_pSourceNode); }
	int getVarCount() { return m_var_list.size(); }
	CVarDef* getVarAt(int idx) { return m_var_list[idx]; }
	void removeVarAt(int idx) { m_var_list.erase(m_var_list.begin() + idx); }
	void removeAllVars() { m_var_list.clear(); }
	//SourceTreeNode*	getTypeSourceNode() { return m_pSourceNode; }

	void analyze(CGrammarAnalyzer* pGrammarAnalyzer, const SourceTreeNode* pRoot);
	void analyzeDef(CGrammarAnalyzer* pGrammarAnalyzer, const SourceTreeNode* pRoot, bool bTemplate = false);
	virtual std::string toString(int depth);
	std::string toDefString(int depth);

	// this statement is a def var type and contains this var, change the name in its source node tree.
	void changeDefVarName(const std::string& old_name, const std::string& new_name);
	// need to change ref of a var with old_name in this statement and its children to new_name
	bool changeRefVarName(const std::string& old_name, CExpr* pNewExpr);

	void setElseStatement(CStatement* pStatement);
	void addSwitchCase(CExpr* pExpr, const ScopeVector& statement_v);
	void addSwitchDefault(const ScopeVector& statement_v);

	// for template defs, store tokens of this definition so that it can be instanced later
	void setTemplateTokens(const StringVector& tokens) { m_templateTokens = tokens; }
	const StringVector& getTemplateTokens() { return m_templateTokens; }

    void setClassAccessModifierType(ClassAccessModifierType cam_type) { m_cam_type = cam_type; }
    ClassAccessModifierType getClassAccessModifierType() { return m_cam_type; }

protected:
	void init();
	TypeDefPointer analyzeSuperType(const SourceTreeNode* pSuperTypeNode, bool bAllowUndefinedStruct = false);
	void analyzeDeclVar(SourceTreeNode* pChildNode, SourceTreeVector& var_v);

public:
	// pound_line: line: m_asm_string
	// def_pre_decl, typedef func, typedef func ptr: m_pTypeDef
	// using_namespace: m_pGrammarObj, m_asm_string, m_bFlag
	// typedef, data: m_typedefType, m_pTypeDef, m_declVarList
	// typedef, func, functype: m_typedefType, m_pTypeDef
	// typedef, typeof: m_typedefType, m_pTypeDef, m_pExpr, m_name;
	// var def: m_modifier_bits, m_pTypeDef, m_attribute_list, m_var_list,
	// func decl: m_modifier_bits, m_pFuncDeclare, m_bThrow, m_asm_string, m_attribute_list
	// func var: m_modifier_bits, m_pTypeDef, m_var_list[0]->getName()
	// template_func, template_class: m_pTemplate, m_bFlag: decl(0) or define(1)
	// template_var:    m_pTypeDef
	// extern template class: m_bFlag: bClass, class: m_pTypeDef, func: m_pFuncDeclare
    // extern template func: m_declVarList(only 1)
	// class cam: m_modifier_bits: cam_type
    // class friend: m_modifier_bits: csu_type, m_pTypeDef
	StatementType	    m_statement_type;
	bool			    m_bFlow;

	// used in parser
	bool				m_bAppFlag;

	// def
	DefType			    m_def_type;
	//TypeDefPointer	m_pVarTypeDef;
	std::vector<CVarDef*>	m_var_list;
	// for def statement, m_pSourceNode copies the whole line except var_def which only copies type;
	TypeDefType         m_typedefType;
	SourceTreeVector    m_declVarList;
	TypeDefPointer      m_pTypeDef;
	CFuncDeclare*       m_pFuncDeclare;
	//int	                m_modifier_bits;
	StringVector		m_mod_strings;
	StringVector		m_mod2_strings;
	CTemplate*          m_pTemplate;
	CScope*             m_pGrammarObj;
	CExpr*              m_pExpr; // only used in typedef typeof
	CSUType             m_csu_type;
	ClassAccessModifierType m_cam_type;

	// number of elseif, number of statements of all cases
	std::vector<unsigned>	m_int_vector;
	// if/exprif/else: whether have else; switch: whether have default
    std::vector<FuncParamItem>    m_func_param_vector; // for try catch only
	bool		        m_bFlag;

	SourceTreeVector    m_attribute_list;
	std::string         m_asm_string;
	//int                 m_bThrow;

	StringVector        m_templateTokens;
};

class CFunction : public CScope
{
public:
	CFunction(CScope* pParent, const std::string& func_name, TypeDefPointer funcType, FlowType flow_type, CFuncDeclare* pFuncDeclare);
	virtual ~CFunction();

    virtual GrammarObjectType getGoType() { return GO_TYPE_FUNC; }
	void analyze(void* pBaseClassInitBlock, void* bracket_block, const SourceTreeNode* pRoot);

	virtual std::string getDebugName();
	bool isFlow() { return m_flow_type != FLOW_TYPE_NONE; }
	bool isFlowRoot() { return m_flow_type == FLOW_TYPE_FLOW_ROOT; }
	FlowType getFlowType() { return m_flow_type; }
    void setFlowType(FlowType newType) { m_flow_type = newType; }
	TypeDefPointer getFuncType() { return m_func_type; }
	void setFuncType(TypeDefPointer pTypeDef) { m_func_type = pTypeDef; }
	//int getParamVarCount() { return m_funcParams.size(); }
	//CVarDef* getParamVarAt(int idx) { return m_funcParams[idx]; }
    //void addParamVar(CVarDef* pVarDef) { return m_funcParams.push_back(pVarDef); }
	CFuncDeclare* getFuncDeclare() { return m_pFuncDeclare; }
	void setFuncDeclare(CFuncDeclare* pFuncDeclare) { m_pFuncDeclare = pFuncDeclare; }

    virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode = FIND_SYMBOL_MODE_ANY);

	virtual std::string toString(int depth);

    void setClassAccessModifierType(ClassAccessModifierType cam_type) { m_cam_type = cam_type; }
    ClassAccessModifierType getClassAccessModifierType() { return m_cam_type; }

public:
	TypeDefPointer				m_func_type;
	//TokenWithNamespace          m_display_name;
	//std::vector<CVarDef*>		m_funcParams;
	//std::vector<FuncParam>		m_displayFuncParams;
	FlowType					m_flow_type;
	CFuncDeclare*               m_pFuncDeclare;

	std::vector< std::pair<TokenWithNamespace, CExpr2*> >   m_memberInitList;
    ClassAccessModifierType     m_cam_type;
};

// There are two types of CNamespaces, logical namespaces and physical namespaces.
// logical ones, isRealScope=true, have defs only, physical ones, isRealScope=false, have children only. g_root_namespace has both
// for namespace that have names, it has a physical namespace whose parent is a logical namespace whose parent points to its real logical parent namespace
// extern block, isRealScope=false, it's parent points to its real logical parent namespace
// unnamed namespace, isRealScope=true, it's parent points to its real logical parent namespace
class CNamespace : public CScope
{
public:
	CNamespace(CScope* pParent, bool bRealScope, bool bIsNamespace, const std::string& name); // if not a namespace, then it's a extern "C" or "C++"
	virtual ~CNamespace() {}

    virtual GrammarObjectType getGoType() { return GO_TYPE_NAMESPACE; }
	virtual std::string toString(int depth);

    virtual std::string getDebugName();
    bool isNamespace() { return m_bNamespace; }
    virtual std::string getName() { return m_name; }
    int getDepth() { return m_depth; }
	void setExternModifier(int modifier) { m_extern_modifier = modifier; }
	void addUnnamedNamespace(CNamespace* pNamespace) { m_unnamed_namespaces.push_back(pNamespace); }
    void addUsingNamespace(CNamespace* pNamespace);
	void setModStrings(const StringVector& mod_strings) { m_mod_strings = mod_strings; }

	void analyze(CGrammarAnalyzer* pGrammarAnalyzer, const SourceTreeNode* pRoot);
	void analyzeFuncDef(const SourceTreeNode* pRoot);

	virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolScope scope, FindSymbolMode mode = FIND_SYMBOL_MODE_ANY);

protected:
	bool		m_bNamespace;
	int         m_depth;
	int	        m_extern_modifier;
	StringVector	m_mod_strings;
	std::vector<CNamespace*>    m_unnamed_namespaces;
    std::vector<CNamespace*>    m_using_namespaces;
};

//bool analyzeDef(const SourceTreeNode* pRoot, TypeDescBlock& type_desc_block, std::vector<CFuncVar*>& var_list);

typedef std::string (*FuncListener)(CScope*, int depth);
extern TypeDefPointer g_type_def_int, g_type_def_unsigned, g_type_def_bool, g_type_def_void, g_type_def_void_ptr, g_type_def_func_void;

CExpr* createNotExpr(CExpr* pExpr);
void enterNamespace(CNamespace* pNamespace);
void leaveNamespace();
FlowType getFlowTypeByModifierBits(int modifier_bits);
std::string getSemanticTypeName(SemanticDataType type);
SemanticDataType getSemanticTypeFromCSUType(CSUType type);
std::string getGoTypeName(GrammarObjectType t);
void extendedTypeReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict);
TokenWithNamespace getRelativeTWN(CScope* pTarget, const std::string& name = "");
std::string getRelativePath(CScope* pTarget, const std::string& name = "");
std::string typeListToFullString(const TypeDefVector& typeList);
int comparableWith(TypeDefPointer pFirstTypeDef, TypeDefPointer pSecondTypeDef);
TypeDefPointer createTypeByDepth(TypeDefPointer pTypeDef, int depth);
void checkBestMatchedFunc(const std::vector<CGrammarObject*>& list, const std::vector<TypeDefPointer>& param_v, bool bCallerIsConst, int& minUnmatchCount, std::vector<CGrammarObject*>& matched_v);
bool isSameFuncType(TypeDefPointer pType1, TypeDefPointer pType2);
void enterScope(CScope* pScope);
void leaveScope();
// for a pTypeDef, return its base type and depth to the root, depth is added to its original value, so remember to set to 0 if needed
TypeDefPointer getRootType(TypeDefPointer pTypeDef, int& depth);

void semanticInit();
CNamespace* semanticAnalyzeFile(char* file_name, int argc, char* argv[]);

#endif // __SEMANTIC__H_

#ifndef __SEMANTIC__H_
#define __SEMANTIC__H_

#include "grammar.h"
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/enable_shared_from_this.hpp>

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

typedef std::vector<std::string>	StandardType;
typedef boost::shared_ptr<CTypeDef>	TypeDefPointer;
typedef std::vector<CExpr*>			ExprVector;
typedef std::vector<CScope*>        ScopeVector;

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

enum FindSymbolMode
{
    FIND_SYMBOL_MODE_LOCAL,
    FIND_SYMBOL_MODE_SCOPE,
    FIND_SYMBOL_MODE_PARENT,
};

class CGrammarObject
{
public:
    CGrammarObject(CScope* pParent) { m_pParent = pParent; m_line_no = 0; }
    virtual ~CGrammarObject() {}

    virtual GrammarObjectType getGoType() = 0;

    virtual std::string getName() { return m_name; }
    virtual void setName(const std::string& name) { m_name = name; }
    virtual std::string getDebugName() { return m_name; }

    std::string getPath();

    CScope* getParent() { return m_pParent; }
    void setParent(CScope* pParent) { m_pParent = pParent; }

    void setDefLocation(const std::string& file_name, int line_no) { m_file_name = file_name; m_line_no = line_no; }
    const std::string& getDefFileName() { return m_file_name; }
    int getDefLineNo() { return m_line_no; }
    std::string definedIn() { return m_file_name + ":" + ltoa(m_line_no);}

    // return NULL if not in a tmeplate
    CScope* getParentScope(GrammarObjectType go_type);
    CTemplate* getParentTemplate();
    bool isParentOf(CGrammarObject* pGO);

protected:
    CScope*         m_pParent;
    std::string     m_name;
    std::string     m_file_name;
    int             m_line_no;
};

class CTypeDef : public boost::enable_shared_from_this<CTypeDef>, public CGrammarObject
{
public:
	// basic data type
	CTypeDef(CScope* pScope, const std::string& name, const StandardType& basic_tokens);
	// enum or union or struct or class
	CTypeDef(CScope* pScope, const std::string& name, CClassDef* pClassDef);
	// func or func ptr, depth can be 0 or 1. pReturnType might be empty pointer for constructor or destructor
	CTypeDef(CScope* pScope, const std::string& name, TypeDefPointer pReturnType, int modifier_bits, int depth);
	// data member ptr
    CTypeDef(CScope* pScope, const std::string& name, SemanticDataType type, TypeDefPointer pTypeDef, TypeDefPointer pDataScope);
    // template
    CTypeDef(CTemplate* pTemplate);
	// template typename
    CTypeDef(CScope* pScope, const std::string& name);
    // typeof
    CTypeDef(CScope* pScope, const std::string& name, const TokenWithNamespace& twn);
	// user def one
	// there's one case that when a struct/class is defined, if want to reference it, we can create a CTypeDef with name set to the name,
	//   pBaseTypeDef set to the real type, pDeclVar set to NULL. We can use isBaseType() to distinguish the two.
	// for typedef types, pDeclVar should not be NULL
	CTypeDef(CScope* pScope, const std::string& name, TypeDefPointer pBaseTypeDef, SourceTreeNode* pDeclVar);
	virtual ~CTypeDef();

	//virtual std::string getName() { return m_name; }
	//void setName(const std::string& name) { m_name = name; if (!name.empty()) MY_ASSERT(m_pScope); }
    virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_TYPEDEF; }
    //void setScope(CScope* pScope) { m_pScope = pScope; }
    //CScope* getParent() { return m_pScope; }
	SemanticDataType getType() { return m_type; }
	void setType(SemanticDataType newType) { m_type = newType; }
	int getModifierBits() { return m_modifier_bits; }
	bool isConst() { return m_modifier_bits & MODBIT_CONST; }
	void setConst(bool bConst = true) { if (bConst) m_modifier_bits |= MODBIT_CONST; else m_modifier_bits &= ~MODBIT_CONST; }
    bool isVolatile() { return m_modifier_bits & MODBIT_VOLATILE; }
    bool isVirtual() { return m_modifier_bits & MODBIT_VIRTUAL; }
	bool isBaseType() { return !m_pBaseTypeDef; }
	TypeDefPointer getBaseType() { return m_pBaseTypeDef ? m_pBaseTypeDef : shared_from_this(); }
    CClassDef* getClassDef() { return (CClassDef*)m_pSpecialType; }
    CTemplate* getTemplate() { return (CTemplate*)m_pSpecialType; }
	int getDepth() { return m_depth; }
    int getFullDepth();
	bool isReference() { return m_bReference; }
	void setReference(bool b) { m_bReference = b; }
	bool isVoid();
	bool isZero() { return m_bZero; }
	void setZero() { m_bZero = true; }

	TypeDefPointer getFuncReturnType() { return m_pFuncReturnType; }
	void addFuncParam(CVarDef* pParam);
	int getFuncParamCount() { return m_func_params.size(); } // vargs doesn't count in param count
	CVarDef* getFuncParamAt(int i);
	bool hasVArgs() { return m_bHasVArgs; }
	void setHasVArgs(bool bHasVArgs = true) { m_bHasVArgs = bHasVArgs; }
	bool isFuncFlow() { return getFuncFlowType() != FLOW_TYPE_NONE; }
	FlowType getFuncFlowType();
	bool isPureVirtual() { return m_bPureVirtual; }
    void setPureVirtual(bool bPureVirtual) { m_bPureVirtual = bPureVirtual; }
    bool hasFuncThrow() { return m_bFuncThrow; }
    SourceTreeNode* getFuncThrowTypeNode() { return m_pThrowTypeNode; }
    void setThrow(bool bThrow, SourceTreeNode* pThrowTypeNode);
    int checkCallParams(const std::vector<TypeDefPointer>& typeList, bool bCallerIsConst);

	virtual std::string toString(int depth = 0);
	std::string toBaseTypeString(int depth = 0);
	std::string toFullString();
	static std::string type2String(SemanticDataType basic_type);
	SourceTreeNode*	getDeclVarNode() { return m_pDeclVarNode; }
	std::string funcTypeToString(const std::string& name);

    bool is_abstract();
    bool is_class();
	bool is_empty();
	bool is_pod();
    bool has_nothrow_assign();
    bool has_nothrow_copy();
    bool has_trivial_assign();
    bool has_trivial_copy();
	bool has_trivial_destructor();

protected:
	void init();

	//CScope* m_pScope;
	//std::string 	m_name;

	SemanticDataType 	m_type;
	TypeDefPointer	m_pBaseTypeDef;
	StandardType 	m_basic_tokens;
	CScope*         m_pSpecialType;
	int             m_modifier_bits;
	bool            m_bZero; // only valid when it's basic type int

	int 			m_depth; // relative to m_pBaseTypeDef
	bool 			m_bReference;

	SourceTreeNode*	m_pDeclVarNode;

	TokenWithNamespace  m_typeof_twn;

	// for base func declare only
	TypeDefPointer	m_pFuncReturnType;
    std::vector<CVarDef*>         m_func_params;
	bool			m_bHasVArgs;
	bool            m_bFuncThrow;
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
	CVarDef(CScope* pParent, const std::string& name, TypeDefPointer pTypeDef, SourceTreeNode* pDeclVar = NULL, CExpr* pInitExpr = NULL);
    CVarDef(CScope* pParent, const std::string& name, TypeDefPointer pTypeDef, const ExprVector& constructorParamList);
	virtual ~CVarDef();

	//virtual std::string getName() { return m_name; }
    //void setName(const std::string& new_name) { m_name = new_name; }
	virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_VAR_DEF; }
	TypeDefPointer getType() { return m_type; }
	void setType(TypeDefPointer pTypeDef) { m_type = pTypeDef; }
	bool hasConstructor() { return m_bHasConstructor; }
	bool isFlow();
	bool isReference() { return declVarIsReference(m_pDeclVar); }
	void setReference() { declVarSetReference(m_pDeclVar); }
	//CScope* getParent() { return m_pParent; }
	//void setParent(CScope* pParent) { m_pParent = pParent; }
	void setRestrict(bool bRestrict = true) { m_bRestrict = bRestrict; }
	bool isRestrict() { return m_bRestrict; }

	void setDeclVarNode(SourceTreeNode* pDeclVar) { MY_ASSERT(pDeclVar); m_pDeclVar = pDeclVar; }
	SourceTreeNode* getDeclVarNode() { return m_pDeclVar; }
	CExpr* getInitExpr() { return m_pInitExpr; }
	// no need to care about deleting old one
	void setInitExpr(CExpr* pExpr) { m_pInitExpr = pExpr; }
	void addExprInDeclVar(CExpr* pExpr) { m_exprList.push_back(pExpr); }
	ExprVector getExprListInDeclVar() { return m_exprList; }

	virtual std::string toString(bool bDumpInitExpr = true);

	bool hasValue() { return m_bHasValue; }
	void setValue(int nValue);
	int getValue() { MY_ASSERT(m_bHasValue); return m_nValue; }

	// change this var and all statements that use ref it. this var cannot be a global var or defined in function parameters. must be a local var.
	void changeName(const std::string& new_name);

protected:
	//CScope* m_pParent;

	//std::string 	m_name;
	TypeDefPointer	m_type;
	bool            m_bHasConstructor;
	SourceTreeNode*	m_pDeclVar;
	bool			m_bReference;
	ExprVector		m_exprList; // either expr list in decl var or constructing param list
	CExpr*			m_pInitExpr;

	bool 			m_bRestrict;
    bool            m_bHasValue;
    int             m_nValue;
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
    CTemplate* getTemplateAt(int i = 0)
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
    CFuncDeclare* getFuncDeclareAt(int i)
    {
        MY_ASSERT(type == GO_TYPE_FUNC_DECL);
        MY_ASSERT(i >= 0 && i < children.size());
        return (CFuncDeclare*)children[i];
    }
    CVarDef* getVarDef()
    {
        MY_ASSERT(type == GO_TYPE_VAR_DEF);
        MY_ASSERT(children.size() == 1);
        CGrammarObject* pObj = children[0];
        if (pObj->getGoType() == GO_TYPE_VAR_DEF)
            return (CVarDef*)pObj;
        MY_ASSERT(pObj->getGoType() == GO_TYPE_USING_OBJECTS);
        return ((CUsingObject*)pObj)->getSymbolObj()->getVarDef();
    }
	std::string definedIn() const
	{
	    return children[0]->getDefFileName() + ":" + ltoa(children[0]->getDefLineNo());
	}

	void checkBestMatchedFunc(const std::vector<TypeDefPointer>& param_v, bool bCallerIsConst, int& minUnmatchCount, std::vector<CGrammarObject*>& matched_v);
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
    CScope(CScope* pParent) : CGrammarObject(pParent)
	{
		//m_pParent = pParent;
		m_bRealScope = true;
	}
	virtual ~CScope()
	{
		//printf("destruction %lx\n", this);
		BOOST_FOREACH(CScope* pObj, m_children)
		{
			//printf("deleting %lx, {%s}\n", pObj, pObj->toString(2).c_str());
			delete pObj;
		}
	}

	CScope* getRealScope() { return m_bRealScope ? this : m_pParent->getRealScope(); }
    CFunction* getFunctionScope();
	void setRealScope(bool bRealScope) { m_bRealScope = bRealScope; }
	virtual std::string toString(int depth = 0) = 0;

	int getChildrenCount() { return (int)m_children.size(); }
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
		BOOST_FOREACH(CScope* pObj, vec)
		{
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
	void addEnumDef(const std::string& name, CClassDef* pClassDef, int nValue);
    void addTemplate(CTemplate* pTemplate);
    void addUsingObjects(const std::string& name, SymbolDefObject* pObj);
	virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolMode mode);
	CFuncDeclare* findFuncDeclare(SymbolDefObject* pDefObj, int paramCount = -1);
	CFuncDeclare* findFuncDeclare(SymbolDefObject* pDefObj, TypeDefPointer pTypeDef = TypeDefPointer());
	//TypeDefPointer findTypeDef(const std::string& name);
	//CVarDef* findVarDef(const std::string& name, bool bCheckParent = true);
	//bool findEnumDef(const std::string& name);
	//void addUsingNamespace(CNamespace* pNamespace, const std::string& token); // if token is not empty, it means only the token under this namespace is allowed.
	//std::vector<NamespaceTokenPair> getUsingNamespaceList() { return m_using_namespace_list; }
	// bCheckingParents is only valid when twn doesn't have scope defined.
	SymbolDefObject* findSymbolEx(const TokenWithNamespace& twn, bool bCheckingParents = true, bool bCreateIfNotExists = false, bool bCreateAsType = false);
	CScope* getScope(const TokenWithNamespace& twn);

	bool onGrammarCheckFunc(int mode, const SourceTreeNode* pRoot, const GrammarTempDefMap& tempDefMap);
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

public:
	bool							m_bRealScope; // extern "C", CExpr, defs with no big brackets, non compound statements should set to false
	//CScope*					m_pParent;
	std::vector<CScope*>	m_children;
	SymbolDefMap 					m_symbol_map;
	//std::vector<NamespaceTokenPair>	m_using_namespace_list;
};

class CExpr : public CScope
{
public:
    CExpr(CScope* pParent);
	CExpr(CScope* pParent, const SourceTreeNode* pRoot);
	virtual ~CExpr();

	CExpr(CScope* pParent, ExprType expr_type, const std::string val);
    CExpr(CScope* pParent, ExprType expr_type, TypeDefPointer pTypeDef);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, const std::string token);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pLeft, CExpr* pRight);
	CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr);
	CExpr(CScope* pParent, ExprType expr_type, TypeDefPointer pTypeDef, CExpr* pLeft);
    CExpr(CScope* pParent, ExprType expr_type, CFuncDeclare* pFuncDeclare, const ScopeVector& param_list);
    CExpr(CScope* pParent, ExprType expr_type, CExpr* pExpr, const ScopeVector& param_list);

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
	//virtual void setFlow(bool bFlow) { m_bFlow = bFlow; }
	//SourceTreeNode*	getSourceTreeNode() { return m_pNode; }
	//void setSourceTreeNode(const SourceTreeNode* pRoot);
	std::string getValue() { return m_value; }
	CFuncDeclare* getFuncDeclare() { return m_pFuncDeclare; }
    void setFuncDeclare(CFuncDeclare* pFuncDeclare) { m_pFuncDeclare = pFuncDeclare; }

	virtual void analyze(const SourceTreeNode* pRoot);
	virtual std::string toString(int depth = 0);

	virtual void changeRefVarName(const std::string& old_name, const std::string& new_name);

	virtual bool calculateNumValue(float& f);

protected:
	void init();

	ExprType		m_expr_type;
	TypeDefPointer	m_return_type;
	int				m_return_depth;
	bool			m_bFlow;
	SymbolDefObject*  m_return_symbol_obj; // this is only intermediate variable after analyzing name but before analyzing func parameters
	bool            m_caller_is_const; // only valid when m_return_symbol_obj is to be used. to determine which func to choose

	// func call, first obj is funcExpr, remainder are paramExpr
	CFuncDeclare*	m_pFuncDeclare;

	// const_value, ref_element, ptr_element
	std::string		m_value;

	// token with namespace
	TokenWithNamespace	m_token_with_namespace;

	// token
	//CVarDef* 		m_pVarDef;

	// sizeof: extended_type_var, new: extended_type_var, type_cast
	//SourceTreeNode*	m_pNode;
	TypeDefPointer	m_pTypeDef;

	// delete: has []
	bool 			m_bFlag;
};

typedef std::vector<CExpr*> ExprVector;

class CExpr2 : public CExpr
{
public:
    CExpr2(CScope* pParent);
    virtual ~CExpr2();

    virtual GrammarObjectType getGoType() { return GO_TYPE_EXPR2; }
    virtual ExprType getExprType() { return EXPR_TYPE_COMMA; }
    virtual void setExprType(ExprType newType) { MY_ASSERT(false); }

    virtual void analyze(const SourceTreeNode* pRoot);
    virtual std::string toString(int depth = 0);

    void addExpr(CExpr* pExpr);
	virtual void changeRefVarName(const std::string& old_name, const std::string& new_name);
    virtual bool calculateNumValue(float& f);
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
	bool isDefined() { return m_bDefined; }
	void setDefined(bool bDefined) { m_bDefined = bDefined; }
    bool isWithinTemplate() { return m_bWithinTemplate; }
    void setWithinTemplate(bool b = true) { m_bWithinTemplate = b; }

	virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolMode mode);
	SymbolDefObject* findSymbolInBaseClasses(const std::string& name);

	void analyze(const SourceTreeNode* pRoot);
	void analyzeClassDef(const ClassBaseTypeDefVector& classBaseTypeDefs, void* bracket_block, const StringVector& tokens = StringVector());
	void addDef(CStatement* pStatement);

	void setTypeDef(TypeDefPointer pTypeDef) { m_pTypeDef = pTypeDef; }
	TypeDefPointer getTypeDef() { return m_pTypeDef; }

    bool hasBaseClass(CClassDef* pClassDef);
    bool hasConstructorOrCanBeAssignedWith(TypeDefPointer pTypeDef);

    bool is_abstract();
    bool is_empty(); // don't have constructor or destructor, don't have protected, private or vitural method
    bool is_pod();
    bool has_nothrow_assign();
    bool has_nothrow_copy();
    bool has_trivial_assign();
    bool has_trivial_copy();
    bool has_trivial_destructor();

    void analyzeByTemplate();

protected:
	struct BaseClassCAMPair {
	    bool bVirtual;
	    ClassAccessModifierType cam_type;
	    TypeDefPointer  pTypeDef;
	};
    struct TempBlock {
	    TempBlock(CFunction* p, void* i, void* b, const SourceTreeNode* r) : pFunc(p), pBaseClassInitBlock(i), bracket_block(b), pRoot(r) {}
        CFunction* pFunc;
        void* pBaseClassInitBlock;
        void* bracket_block;
        const SourceTreeNode* pRoot;
    };
    friend class CTemplate;
	bool			                m_bDefined;	// for struct or union only
	bool                            m_bWithinTemplate; // it's a class within a template, not instanced. its instanced class should be under the instance template of its parent template
	SemanticDataType                m_type;
	std::string                     m_template_name;
	std::vector<BaseClassCAMPair>   m_baseTypeList;

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
    int modifierBits;
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
        bool bHasDefault;
        bool bHasTypename;
        std::string name;
        TypeDefPointer  pNumValueType; // data type of num def
        SourceTreeNode* pTypeNode; // only valid for data_type
        SourceTreeNode* pDefaultNode; // default type of type def, or default value of num def
    };
    typedef std::vector<TypeParam> TypeParamVector;

    CTemplate(CScope* pParent, TemplateType type, const std::string& name);
    virtual ~CTemplate();

    virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_TEMPLATE; }
    std::string toHeaderString();
    virtual std::string toString(int depth = 0) { MY_ASSERT(false); return ""; }
    virtual std::string toString(bool bDefine, int depth = 0);
    bool isDefined() { return m_bDefined; }
    TemplateType getTemplateType() { return m_nTemplateType; }
    //virtual std::string getName() { return m_name; }
    std::string getTemplateName() { return m_template_name; }
    bool isSame(const CTemplate* pTemplate) const;
    SemanticDataType getClassType() { return m_data_type; } // struct or class

    bool isRootTemplate() { MY_ASSERT(getTemplateType() == TEMPLATE_TYPE_CLASS); return m_name == m_template_name && m_specializedTypeParams.empty(); }
    bool isSpecializedTemplate() { MY_ASSERT(getTemplateType() == TEMPLATE_TYPE_CLASS); return m_name == m_template_name && !m_specializedTypeParams.empty(); }
    bool isInstancedTemplate() { MY_ASSERT(getTemplateType() == TEMPLATE_TYPE_CLASS); return m_name != m_template_name; }

    virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolMode mode);
    SymbolDefObject* findSymbolInBaseClasses(const std::string& name);

    void analyzeFunc(const SourceTreeNode* pRoot);
    void analyzeBaseClass(const SourceTreeNode* pRoot);
    CTemplate* analyzeSpecializedClass(const SourceTreeNode* pRoot);
    void saveClassBody(void* pBaseClassDefsBlock, void* body_data);
    void analyzeClassBody(void* pBaseClassDefsBlock, void* body_data);
    void analyzeVar(const SourceTreeNode* pRoot);
    void analyzeFriendClass(const SourceTreeNode* pRoot);
    void mergeWithBaseClass(CTemplate* pNewTemplate, bool bReplaceName);
    void mergeWithSpecializedClass(CTemplate* pNewTemplate, bool bReplaceName);

    int funcGetParamCount() { return (int)m_funcParams.size(); }
    int funcCheckFitForTypeList(const TypeDefVector& typeList, TemplateResolvedDefParamVector& resovledDefParams);
    TypeDefPointer funcGetInstance(const TypeDefVector& typeList, const TemplateResolvedDefParamVector& resovledDefParams);

    TypeDefPointer classGetInstance(const TokenWithNamespace& twn, int idx, CScope* pOtherScope);
    CTemplate* getTemplateByParams(const TokenWithNamespace& twn, int depth);

	bool canBeSpecialized(const TemplateResolvedDefParamVector& typeList);
	CTemplate* analyzeSpecialize(const SourceTreeNode* pRoot, CSUType csu_type, SourceTreeNode* pBaseClassDefsNode, void* body_data, const TemplateResolvedDefParamVector& typeList);

    std::vector<TypeParam> getTypeParams() { return m_typeParams; }
    CClassDef* getRootInstancedClassDef() { return m_instanced_class.getTypeDef()->getClassDef(); }

    std::string getUniqueId() { MY_ASSERT(isSpecializedTemplate()); return m_uniqueId; }
    void addSpecializedTemplate(CTemplate* pTemplate);
    CTemplate* findSpecializedTemplateByUniqueId(const std::string& uniqueId);

    void setResolvedDefParams(const TemplateResolvedDefParamVector& v) { m_resolvedDefParams = v; }
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

	void createParamTypeFromTemplateHeader(const SourceTreeNode* pRoot);
	void readTemplateHeaderIntoTypeParams(const SourceTreeNode* pRoot);
    static int findResolvedDefParam(const TemplateResolvedDefParamVector& v, const std::string& name);
    bool compareResolvedDefParams(const TemplateResolvedDefParamVector& typeList, const TemplateResolvedDefParamVector& typeList2);
    int resolveParamType(const SourceTreeNode* pExtendedTypeNode, TypeDefPointer pTypeDef, TemplateResolvedDefParamVector& resolvedDefParams, bool bMatchType = false);
    int resolveParamNumValue(const SourceTreeNode* pExprNode, long numValue, TemplateResolvedDefParamVector& resolvedDefParams);
    TypeParam readTemplateTypeParam(const SourceTreeNode* pChild);
    CTemplate* classResolveParamForBaseTemplate(const TemplateResolvedDefParamVector& realTypeList);
    int classResolveParamForSpecializedTemplate(const TemplateResolvedDefParamVector& realTypeList, CTemplate*& pRetTemplate);
    CTemplate* classMatchForATemplate_(const TemplateResolvedDefParamVector& realTypeList);
    TypeDefPointer classGetInstance_(const TemplateResolvedDefParamVector& resolvedDefParams);

    bool inParamTypeNames(const std::string s);

    void clear();
    void clearTypeAndVarDefs();

    bool                    m_bDefined;
    TemplateType            m_nTemplateType;
    SemanticDataType		m_data_type; // for class template, is it a struct or a class
    std::string             m_template_name;
    std::vector<TypeParam>  m_typeParams;
    std::vector<TypeParam>  m_specializedTypeParams; // for specialized template only, in this list, all type and num values are stored in pDefaultExtendedTypeNode
    std::string				m_uniqueId;

    SourceTreeNode*         m_pFuncReturnExtendedTypeNode; // for func and var
    std::vector<FuncParamItem>  m_funcParams;
    bool                    m_func_hasVArgs;
    int                     m_func_modifier_bits;

    ClassBaseTypeDefVector  m_classBaseTypeDefs;
    //TypeDefVector           m_classTypeDefList; // class instances stored here. func instances stored in m_children

    std::vector<SymbolDefObject>	m_specializeDefList; // special list of root template

    TemplateResolvedDefParamVector m_resolvedDefParams; // for instanced template only
    TokenWithNamespace             m_varName; // for VAR and friend class only
    CSUType                 m_csu_type; // for friend_class only
    int                     m_score; // for specialized template only

    SymbolDefObject         m_instanced_class; // for root class template only.
    StringVector            m_body_sv;
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
	CStatement(CScope* pParent, StatementType type, CExpr* pExpr, CStatement* pStatement); // if
	CStatement(CScope* pParent, StatementType type, const std::vector<CScope*>& vec); // compound
	virtual ~CStatement();

	virtual std::string getDebugName();
    virtual GrammarObjectType getGoType() { return GO_TYPE_STATEMENT; }
	StatementType getStatementType() { return m_statement_type; }
    void setStatementType(StatementType newType) { m_statement_type = newType; }
	DefType getDefType() { return m_def_type; }
	bool isFlow() { return m_bFlow; }
	//void setFlow(bool bFlow) { m_bFlow = bFlow; }

	// for var def statement;
	//std::string getDefTypeString() { return displaySourceTreeType(m_pSourceNode); }
	int getVarCount() { return m_var_list.size(); }
	CVarDef* getVarAt(int idx) { return m_var_list[idx]; }
	//SourceTreeNode*	getTypeSourceNode() { return m_pSourceNode; }

	void analyze(const SourceTreeNode* pRoot);
	void analyzeDef(const SourceTreeNode* pRoot, bool bTemplate = false);
	virtual std::string toString(int depth);
	std::string toDefString(int depth);

	// this statement is a def var type and contains this var, change the name in its source node tree.
	void changeDefVarName(const std::string& old_name, const std::string& new_name);
	// need to change ref of a var with old_name in this statement and its children to new_name
	bool changeRefVarName(const std::string& old_name, const std::string& new_name);

	void addElseIf(CExpr* pExpr, CStatement* pStatement);
	void addElseStatement(CStatement* pStatement);

	// for template defs, store tokens of this definition so that it can be instanced later
	void setTemplateTokens(const StringVector& tokens) { m_templateTokens = tokens; }
	const StringVector& getTemplateTokens() { return m_templateTokens; }

    void setClassAccessModifierType(ClassAccessModifierType cam_type) { m_cam_type = cam_type; }
    ClassAccessModifierType getClassAccessModifierType() { return m_cam_type; }

protected:
	TypeDefPointer analyzeSuperType(const SourceTreeNode* pSuperTypeNode, bool bAllowUndefinedStruct = false);

public:
	// def_pre_decl, typedef func, typedef func ptr: m_pTypeDef
	// using_namespace: m_pGrammarObj, m_asm_string, m_bFlag
	// typedef, data: m_typedefType, m_pTypeDef, m_pDeclVarNode
	// typedef, func, functype: m_typedefType, m_pTypeDef
	// typedef, typeof: m_typedefType, m_pTypeDef, m_pExpr, m_name;
	// var def: m_modifier_bits, m_pTypeDef, m_attribute_list, m_var_list,
	// func decl: m_modifier_bits, m_pFuncDeclare, m_bThrow, m_asm_string, m_attribute_list
	// func var: m_modifier_bits, m_pTypeDef, m_var_list[0]->getName()
	// template_func, template_class: m_pTemplate, m_bFlag: decl(0) or define(1)
	// template_var:    m_pTypeDef
	// extern template class: m_bFlag: bClass, class: m_pTypeDef, func: m_pFuncDeclare
    // extern template func: m_pDeclVarNode
	// class cam: m_modifier_bits: cam_type
    // class friend: m_modifier_bits: csu_type, m_pTypeDef
	StatementType	    m_statement_type;
	bool			    m_bFlow;

	// def
	DefType			    m_def_type;
	//TypeDefPointer	m_pVarTypeDef;
	std::vector<CVarDef*>	m_var_list;
	// for def statement, m_pSourceNode copies the whole line except var_def which only copies type;
	TypeDefType         m_typedefType;
	SourceTreeNode*	    m_pDeclVarNode;
	TypeDefPointer      m_pTypeDef;
	CFuncDeclare*       m_pFuncDeclare;
	int	                m_modifier_bits;
	CTemplate*          m_pTemplate;
	CScope*             m_pGrammarObj;
	CExpr*              m_pExpr; // only used in typedef typeof
	CSUType             m_csu_type;
	ClassAccessModifierType m_cam_type;

	// number of elseif, number of statements of all cases
	std::vector<int>	m_int_vector;
	// if/exprif/else: whether have else; switch: whether have default
    std::vector<FuncParamItem>    m_func_param_vector; // for try catch only
	bool		        m_bFlag;

	SourceTreeVector    m_attribute_list;
	std::string         m_asm_string;
	bool                m_bThrow;

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
	int getParamVarCount() { return m_funcParams.size(); }
	CVarDef* getParamVarAt(int idx) { return m_funcParams[idx]; }
    void addParamVar(CVarDef* pVarDef) { return m_funcParams.push_back(pVarDef); }
	//FuncParam getParamDisplayInfo(int idx) { return m_displayFuncParams[idx]; }

    virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolMode mode);

	virtual std::string toString(int depth);

    void setClassAccessModifierType(ClassAccessModifierType cam_type) { m_cam_type = cam_type; }
    ClassAccessModifierType getClassAccessModifierType() { return m_cam_type; }

public:
	TypeDefPointer				m_func_type;
	//TokenWithNamespace          m_display_name;
	std::vector<CVarDef*>		m_funcParams;
	//std::vector<FuncParam>		m_displayFuncParams;
	FlowType					m_flow_type;
	CFuncDeclare*               m_pFuncDeclare;

	std::vector< std::pair<TokenWithNamespace, CExpr*> >   m_memberInitList;
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

	void analyze(const SourceTreeNode* pRoot);
	void analyzeFuncDef(const SourceTreeNode* pRoot);

	virtual SymbolDefObject* findSymbol(const std::string& name, FindSymbolMode mode);

protected:
	bool		m_bNamespace;
	std::string m_name;
	int         m_depth;
	int	        m_extern_modifier;
	std::vector<CNamespace*>    m_unnamed_namespaces;
    std::vector<CNamespace*>    m_using_namespaces;
};

//bool analyzeDef(const SourceTreeNode* pRoot, TypeDescBlock& type_desc_block, std::vector<CFuncVar*>& var_list);

typedef std::string (*FuncListener)(CScope*, int depth);
extern TypeDefPointer g_type_def_int, g_type_def_bool, g_type_def_void, g_type_def_void_ptr, g_type_def_func_void;

CExpr* createNotExpr(CExpr* pExpr);
void enterNamespace(CNamespace* pNamespace);
void leaveNamespace();
FlowType getFlowTypeByModifierBits(int modifier_bits);
std::string getSemanticTypeName(SemanticDataType type);
SemanticDataType getSemanticTypeFromCSUType(CSUType type);
std::string getGoTypeName(GrammarObjectType t);
void extendedTypeReplaceTypeNameIfAny(SourceTreeNode* pRoot, const std::map<std::string, std::string>& dict);
std::string getRelativePath(CScope* pTarget, const std::string& name = "");
int comparableWith(TypeDefPointer pFirstTypeDef, TypeDefPointer pSecondTypeDef);
TypeDefPointer createTypeByDepth(TypeDefPointer pTypeDef, int depth);

void semanticInit();
CNamespace* semanticAnalyzeFile(char* file_name, int argc, char* argv[]);

#endif // __SEMANTIC__H_

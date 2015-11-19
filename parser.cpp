#include "parser.h"
/*#ifdef WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif*/

#define TRACE(fmt, ...)	do { if (true) write_log(fmt, __VA_ARGS__); } while (false)

CMyFunc::CMyFunc(CFunction* pFunc, bool bFork)
{
    m_pFunc = pFunc;
    m_bFork = bFork;

    m_temp_var_index = 0;
    m_signal_index = 0;
    m_sub_func_counter = 0;
	m_local_obj_var_index = 0;
}

CFuncDeclare* CMyFunc::findFuncDecl(const std::string& func_name)
{
	SymbolDefObject* pSymbolObj = m_pFunc->findSymbol(func_name, FIND_SYMBOL_SCOPE_PARENT);
	MY_ASSERT(pSymbolObj && pSymbolObj->type == GO_TYPE_FUNC_DECL && pSymbolObj->children.size() == 1);
	return pSymbolObj->getFuncDeclareAt(0);
}

TypeDefPointer CMyFunc::findTypeDef(const std::string& type_name)
{
	SymbolDefObject* pSymbolObj = m_pFunc->findSymbol(type_name, FIND_SYMBOL_SCOPE_PARENT);
	MY_ASSERT(pSymbolObj && pSymbolObj->type == GO_TYPE_TYPEDEF);
	return pSymbolObj->getTypeDef();
}

void CMyFunc::addToStatementVector(ScopeVector& ret_v, CStatement* pNewStatement, unsigned nSignalNo)
{
	CStatement* pStatement;

	if (m_pFunc->isFlowRoot() || pNewStatement->isAppFlagSet())
	{
		ret_v.push_back(pNewStatement);
		return;
	}
	if (!ret_v.empty())
	{
		pStatement = (CStatement*)ret_v.back();
		if (pStatement->getStatementType() == STATEMENT_TYPE_IF)
		{
			CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
			if (pExpr->getExprType() == EXPR_TYPE_LESS_EQUAL &&
				((CExpr*)pExpr->getChildAt(0))->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE && ((CExpr*)pExpr->getChildAt(0))->getValue() == PARAM_VAR_NAME_SIGNAL &&
				((CExpr*)pExpr->getChildAt(1))->getExprType() == EXPR_TYPE_CONST_VALUE && ((CExpr*)pExpr->getChildAt(1))->getValue() == ltoa(nSignalNo))
			{
				pStatement = (CStatement*)pStatement->getChildAt(1);
				MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_COMPOUND);
				pStatement->addChild(pNewStatement);
				return;
			}
		}
	}

	ScopeVector v;
	v.push_back(pNewStatement);
	ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
		new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalNo))),
		new CStatement(NULL, STATEMENT_TYPE_COMPOUND, v)));
}

void CMyFunc::addBatchToStatementVector(ScopeVector& ret_v, ScopeVector& new_v, unsigned nSignalNo)
{
	for (size_t i = 0; i < new_v.size(); i++)
	{
		CScope* pObj = new_v[i];
		MY_ASSERT(pObj->getGoType() == GO_TYPE_STATEMENT);
		addToStatementVector(ret_v, (CStatement*)pObj, nSignalNo);
	}
}

CExpr* composeSignalLogicExpr(CExpr* pExpr, unsigned nSignalEnter, unsigned nSignalStart, unsigned nSignalEnd)
{
	CExpr* pExpr2 = new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter)));
	if (pExpr && !(pExpr->getExprType() == EXPR_TYPE_CONST_VALUE && pExpr->getValue() == "true"))
		pExpr2 = new CExpr(NULL, EXPR_TYPE_AND, pExpr2, new CExpr(NULL, EXPR_TYPE_PARENTHESIS, pExpr));
	if (nSignalStart > nSignalEnd)
		return pExpr2;
	if (nSignalStart == nSignalEnd)
		pExpr = new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalStart)));
	else
		pExpr = new CExpr(NULL, EXPR_TYPE_AND,
			new CExpr(NULL, EXPR_TYPE_GREATER_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalStart))),
			new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnd))));
	return new CExpr(NULL, EXPR_TYPE_OR, pExpr2, pExpr);
}

bool isVarDefinedBetweenScopeAndFunction(const std::string& varName, CScope* pScope, CScope* pCurrent, bool bOutsideScope)
{
	if (pCurrent->findSymbol(varName, FIND_SYMBOL_SCOPE_LOCAL))
		return bOutsideScope;

	if (pCurrent->getGoType() == GO_TYPE_FUNC)
		return false;

	if (pCurrent == pScope)
		bOutsideScope = true;

	CScope* pParent = pCurrent->getParent();
	MY_ASSERT(pParent);
	MY_ASSERT(pParent != pCurrent);

	return isVarDefinedBetweenScopeAndFunction(varName, pScope, pParent, bOutsideScope);
}

/*void changeVarToUnderStructInExpr(CScope* pScope, const std::string& struct_name, CExpr* pExpr)
{
	if (pExpr->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE)
	{
		TokenWithNamespace twn = pExpr->getTWN();
		if (twn.getDepth() == 1 && isVarDefinedBetweenScopeAndFunction(twn.getLastToken(), pScope, pExpr, false))
		{
			pExpr->setExprType(EXPR_TYPE_PTR_ELEMENT);
			pExpr->addChild(new CExpr(pExpr, EXPR_TYPE_TOKEN_WITH_NAMESPACE, struct_name));
			pExpr->setValue(twn.getLastToken());
		}
		return;
	}

	for (int i = 0; i < pExpr->getChildrenCount(); i++)
	{
		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(i);
		MY_ASSERT(pExpr2->getGoType() == GO_TYPE_EXPR || pExpr2->getGoType() == GO_TYPE_EXPR2);
		changeVarToUnderStructInExpr(pScope, struct_name, pExpr2);
	}
}

// check every var to see whether it is defined outside the pScope but under the function. If true, change it to pStruct->var_name
void changeVarToUnderStructInStatement(CScope* pScope, const std::string& struct_name, CStatement* pStatement)
{
	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_DEF:
	{
		int var_count = pStatement->getVarCount();
		for (int i = 0; i < var_count; i++)
		{
			CVarDef* pVar = pStatement->getVarAt(i);
			if (pVar->getInitExpr())
			  changeVarToUnderStructInExpr(pScope, struct_name, pVar->getInitExpr());
		}
		break;
	}
	default:
	{
		for (int i = 0; i < pStatement->getChildrenCount(); i++)
		{
			CScope* pObj = pStatement->getChildAt(i);
			if (!pObj)
				continue;
			if (pObj->getGoType() == GO_TYPE_STATEMENT)
				changeVarToUnderStructInStatement(pScope, struct_name, (CStatement*)pObj);
			else
				changeVarToUnderStructInExpr(pScope, struct_name, (CExpr*)pObj);
		}
	}
	}
}*/

// for all vars whose name in local_var_names, change it to parent_name->name
void moveVarUnderParentInExpr(CExpr* pExpr, const std::string& parent_name, const LocalVarSet& local_var_names)
{
	if (pExpr->getGoType() == GO_TYPE_EXPR && pExpr->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE)
	{
		TokenWithNamespace twn = pExpr->getTWN();
		LocalVarSet::const_iterator it;
		if (twn.getDepth() == 1 && (it = local_var_names.find(twn.getLastToken())) != local_var_names.end())
		{
			pExpr->setExprType(EXPR_TYPE_PTR_ELEMENT);
			pExpr->addChild(new CExpr(pExpr, EXPR_TYPE_TOKEN_WITH_NAMESPACE, parent_name));
			pExpr->setValue(it->second.new_name);

			if (it->second.bRef) // a reference, need to change to (*pVar->name)
			{
				CExpr* pNewExpr = new CExpr(NULL);
				*pNewExpr = *pExpr;
				pExpr->removeAllChildren();
				pExpr->setExprType(EXPR_TYPE_PARENTHESIS);
				pExpr->addChild(new CExpr(pExpr, EXPR_TYPE_INDIRECTION, pNewExpr));
			}
		}
		return;
	}

	for (int i = 0; i < pExpr->getChildrenCount(); i++)
	{
		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(i);
		MY_ASSERT(pExpr2->getGoType() == GO_TYPE_EXPR || pExpr2->getGoType() == GO_TYPE_EXPR2);
		moveVarUnderParentInExpr(pExpr2, parent_name, local_var_names);
	}
}

void moveVarUnderParentInStatement(CStatement* pStatement, const std::string& parent_name, const LocalVarSet& local_var_names)
{
	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_DEF:
	{
		int var_count = pStatement->getVarCount();
		for (int i = 0; i < var_count; i++)
		{
			CVarDef* pVar = pStatement->getVarAt(i);
			if (pVar->getInitExpr())
			  moveVarUnderParentInExpr(pVar->getInitExpr(), parent_name, local_var_names);
		}
		break;
	}
	default:
	{
		for (int i = 0; i < pStatement->getChildrenCount(); i++)
		{
			CScope* pObj = pStatement->getChildAt(i);
			if (!pObj)
				continue;
			if (pObj->getGoType() == GO_TYPE_STATEMENT)
				moveVarUnderParentInStatement((CStatement*)pObj, parent_name, local_var_names);
			else
				moveVarUnderParentInExpr((CExpr*)pObj, parent_name, local_var_names);
		}
	}
	}
}

CStatement* addFuncDefStruct(CScope* pParent, CFuncDeclare* pFuncDeclare)
{
	std::string new_name = FUNC_DEF_STRUCT_PREFIX + pFuncDeclare->getName();
	SymbolDefObject* pDefObj = pParent->findSymbol(new_name, FIND_SYMBOL_SCOPE_PARENT);
	if (pDefObj)
		return NULL;
	CClassDef* pStruct = new CClassDef(pParent, SEMANTIC_TYPE_STRUCT, new_name);
	TypeDefPointer pStructTypeDef = TypeDefPointer(new CTypeDef(pParent, new_name, pStruct));
	pParent->addTypeDef(TypeDefPointer(new CTypeDef(pParent, new_name, pStructTypeDef, 0)));
	CStatement* pStatement = new CStatement(pParent, STATEMENT_TYPE_DEF, pStructTypeDef);

	// add basic member into the struct
	pStruct->addDef(new CStatement(pStruct, STATEMENT_TYPE_DEF, 
		new CVarDef(NULL, FLOW_BASIC_MEMBER_NAME, pParent->findSymbol(FLOW_BASIC_TYPE_NAME, FIND_SYMBOL_SCOPE_PARENT)->getTypeDef(), NULL)));

	for (int i = 0; i < pFuncDeclare->getType()->getFuncParamCount(); i++)
	{
		CVarDef* pVarDef = pFuncDeclare->getType()->getFuncParamAt(i);

		TypeDefPointer pVarType = pVarDef->getType()->getBaseType();
		SourceTreeNode* pVarNode = dupSourceTreeNode(pVarDef->getDeclVarNode());
		if (pVarDef->isReference())
		{
			declVarRemoveModifier(pVarNode, DVMOD_TYPE_REFERENCE);
			declVarAddModifier(pVarNode, DVMOD_TYPE_INTERNAL_POINTER);
		}
		CVarDef* pVarDef2 = new CVarDef(pVarDef->getParent(), pVarDef->getName(), pVarType, pVarNode);
		CStatement* pStatement2 = new CStatement(pStruct, STATEMENT_TYPE_DEF, pVarDef2, NULL);
		pStruct->addDef(pStatement2);
	}

	if (pParent->getGoType() == GO_TYPE_CLASS)
	{
		CStatement* pStatement = new CStatement(pStruct, STATEMENT_TYPE_DEF, new CVarDef(NULL, FLOW_PARAM_NAME_CALLER_THIS, 
			TypeDefPointer(new CTypeDef(NULL, "", 
				TypeDefPointer(new CTypeDef(pParent->getParent(), pParent->getName(), ((CClassDef*)pParent)->getTypeDef(), 0)), 1)), NULL));
		pStruct->addDef(pStatement);
	}

	// add return var into the struct
	TypeDefPointer return_type = pFuncDeclare->getType()->getFuncReturnType();
	if (!return_type->isVoid())
	{
		pStruct->addDef(new CStatement(pStruct, STATEMENT_TYPE_DEF,
			new CVarDef(NULL, PARAM_NAME_CALLER_RET, return_type, NULL)));
	}

	return pStatement;
}

void CMyFunc::replaceExprWithStatement(ScopeVector& ret_v, CExpr*& pExpr, unsigned signalNo)
{
	ScopeVector ret_v2;

	TypeDefPointer return_type = pExpr->getReturnType();
	std::string varName = getTempVarName();
	SourceTreeNode* pDeclVarNode = declVarCreateByName(varName);
	for (int i = 0; i < pExpr->getReturnDepth(); i++)
		declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
	CVarDef* pVarDef = new CVarDef(NULL, varName, return_type, pDeclVarNode);

	//CVarDef* pVarDef2 = new CVarDef(pVarDef->getParent(), pVarDef->getName(), pVarDef->getType(), dupSourceTreeNode(pVarDef->getDeclVarNode()));
	CStatement* pStatement2 = new CStatement(NULL, STATEMENT_TYPE_DEF, pVarDef, NULL);
	getStructType()->getClassDef()->addDef(pStatement2);

	CScope* pScope = pExpr->getRealScope();
	//ret_v2.push_back(new CStatement(pScope, STATEMENT_TYPE_DEF, pVarDef, (return_type->getBaseType() ? return_type->getBaseType()->getBasicNode() : return_type->getBasicNode())));
	ret_v2.push_back(new CStatement(pScope, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
		new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName()),
		pExpr)));
	pExpr = new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName());

	addBatchToStatementVector(ret_v, ret_v2, signalNo);
}

void CMyFunc::checkExprDecompose(ScopeVector& ret_v, CExpr*& pExpr, unsigned& inAndOutSignalNo, const LocalVarSet& local_var_names)
{
	checkNonFlowExprDecompose(pExpr, local_var_names);

	if (!pExpr->isFlow())
		return;

	switch (pExpr->getExprType())
	{
	case EXPR_TYPE_CONST_VALUE:			// const_value
	case EXPR_TYPE_TOKEN_WITH_NAMESPACE:				// token
		break;

	case EXPR_TYPE_REF_ELEMENT:		// expr token
	case EXPR_TYPE_PTR_ELEMENT:		// expr token
	case EXPR_TYPE_ARRAY:			// expr expr
	case EXPR_TYPE_RIGHT_INC: 		// expr
	case EXPR_TYPE_RIGHT_DEC:		// expr
	case EXPR_TYPE_LEFT_INC:		// expr
	case EXPR_TYPE_LEFT_DEC:		// expr
	case EXPR_TYPE_POSITIVE:		// expr
	case EXPR_TYPE_NEGATIVE:		// expr
	case EXPR_TYPE_NOT:				// expr
	case EXPR_TYPE_BIT_NOT:			// expr
	case EXPR_TYPE_TYPE_CAST:		// type expr
	case EXPR_TYPE_INDIRECTION:		// expr
	case EXPR_TYPE_ADDRESS_OF:		// expr
	case EXPR_TYPE_SIZEOF:			// (extended_type_var | expr)
	case EXPR_TYPE_NEW_C:	    	// scope extended_type_var
	case EXPR_TYPE_NEW_OBJECT:       // scope expr2
	case EXPR_TYPE_NEW_ADV:          // scope expr, user_def_type, expr2
	case EXPR_TYPE_DELETE:			// expr
	case EXPR_TYPE_MULTIPLE:		// expr expr
	case EXPR_TYPE_DIVIDE:			// expr expr
	case EXPR_TYPE_REMAINDER:		// expr expr
	case EXPR_TYPE_ADD:				// expr expr
	case EXPR_TYPE_SUBTRACT:		// expr expr
	case EXPR_TYPE_LEFT_SHIFT:		// expr expr
	case EXPR_TYPE_RIGHT_SHIFT:		// expr expr
	case EXPR_TYPE_LESS_THAN:		// expr expr
	case EXPR_TYPE_LESS_EQUAL:		// expr expr
	case EXPR_TYPE_GREATER_THAN:	// expr expr
	case EXPR_TYPE_GREATER_EQUAL:	// expr expr
	case EXPR_TYPE_EQUAL:			// expr expr
	case EXPR_TYPE_NOT_EQUAL:		// expr expr
	case EXPR_TYPE_BIT_AND:			// expr expr
	case EXPR_TYPE_BIT_XOR:			// expr expr
	case EXPR_TYPE_BIT_OR:			// expr expr
	case EXPR_TYPE_THROW:				// expr
	case EXPR_TYPE_COMMA:				// expr expr
	case EXPR_TYPE_PARENTHESIS:			// expr
	{
		MY_ASSERT(pExpr->getChildrenCount() > 0);

		CExpr* pExpr1 = (CExpr*)pExpr->getChildAt(0);
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo, local_var_names);
		pExpr->setChildAt(0, pExpr1);
		MY_ASSERT(!pExpr1->isFlow());

		if (pExpr->getChildrenCount() > 1)
		{
			CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(1);
			// if pExpr2->isFlow, then we need to keep the value of pExpr1 before calculating pExpr2 except pExpr1 is a simple unchanged value
			if (pExpr2->isFlow() && !(pExpr1->getExprType() == EXPR_TYPE_CONST_VALUE || pExpr1->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE && isTempVarName(pExpr1->getValue())))
			{
//printf("replaceExprWithStatement, expr=%s\n", pExpr1->toString().c_str());
				replaceExprWithStatement(ret_v, pExpr1, inAndOutSignalNo);
				pExpr->setChildAt(0, pExpr1);
			}

			checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo, local_var_names);
			pExpr->setChildAt(1, pExpr2);
			MY_ASSERT(!pExpr2->isFlow());
		}
		pExpr->setFlow(false);
		break;
	}
	case EXPR_TYPE_ASSIGN:		  // expr expr
	case EXPR_TYPE_ADD_ASSIGN:	  // expr expr
	case EXPR_TYPE_SUBTRACT_ASSIGN: // expr expr
	case EXPR_TYPE_MULTIPLE_ASSIGN: // expr expr
	case EXPR_TYPE_DIVIDE_ASSIGN:   // expr expr
	case EXPR_TYPE_REMAINDER_ASSIGN:	// expr expr
	case EXPR_TYPE_LEFT_SHIFT_ASSIGN:   // expr expr
	case EXPR_TYPE_RIGHT_SHIFT_ASSIGN:  // expr expr
	case EXPR_TYPE_BIT_AND_ASSIGN:	  // expr expr
	case EXPR_TYPE_BIT_XOR_ASSIGN:	  // expr expr
	case EXPR_TYPE_BIT_OR_ASSIGN:	   // expr expr
	{
		MY_ASSERT(pExpr->getChildrenCount() > 0);

		CExpr* pExpr1 = (CExpr*)pExpr->getChildAt(0);
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo, local_var_names);
		pExpr->setChildAt(0, pExpr1);
		MY_ASSERT(!pExpr1->isFlow());

		if (pExpr->getChildrenCount() > 1)
		{
			CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(1);
			// if pExpr2->isFlow, then we need to keep the value of pExpr1 before calculating pExpr2 except pExpr1 is a simple unchanged value
			checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo, local_var_names);
			pExpr->setChildAt(1, pExpr2);
			MY_ASSERT(!pExpr2->isFlow());
		}
		pExpr->setFlow(false);
		break;
	}
	case EXPR_TYPE_FUNC_CALL:		// expr *[expr: ':']
	{
		ScopeVector ret_v2;
		bool bFuncFlow = false;
		TypeDefPointer pFuncCallType;
		int nStart = 0, i;
		CExpr* pExpr2 = NULL, *pFuncExpr = NULL;
		std::string func_name;
		if (!pExpr->getValue().empty())
		{
			pFuncCallType = pExpr->getFuncCallType();
			bFuncFlow = pFuncCallType->isFuncFlow();
			func_name = pExpr->getValue();
		}
		else
		{
			pFuncExpr = (CExpr*)pExpr->getChildAt(0);
			func_name = pFuncExpr->toString();
			pFuncCallType = pFuncExpr->getReturnType();
			MY_ASSERT(pFuncCallType->getType() == SEMANTIC_TYPE_FUNC);
			bFuncFlow = pFuncCallType->isFuncFlow();
			nStart = 1;
			checkExprDecompose(ret_v, pFuncExpr, inAndOutSignalNo, local_var_names);
			MY_ASSERT(!pFuncExpr->isFlow());
			pExpr->setChildAt(0, pFuncExpr);
		}
		int nLastFlowExpr = -1;
		for (i = nStart; i < pExpr->getChildrenCount(); i++)
		{
			if (((CExpr*)pExpr->getChildAt(i))->isFlow())
			{
				nLastFlowExpr = i;
				break;
			}
		}
		if (nLastFlowExpr >= 0 && nStart > 0 && !(pFuncExpr->getExprType() == EXPR_TYPE_CONST_VALUE || pFuncExpr->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE && isTempVarName(pFuncExpr->getValue())))
		{
			replaceExprWithStatement(ret_v, pFuncExpr, inAndOutSignalNo);
			pExpr->setChildAt(0, pFuncExpr);
		}
		for (i = pExpr->getChildrenCount() - 1; i >= nStart; i--)
		{
			CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(i);
			checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo, local_var_names);
			MY_ASSERT(!pExpr2->isFlow());
			pExpr->setChildAt(i, pExpr2);
			if (nLastFlowExpr >= 0 && i > nLastFlowExpr && !(pExpr2->getExprType() == EXPR_TYPE_CONST_VALUE || pExpr2->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE && isTempVarName(pExpr2->getValue())))
			{
				replaceExprWithStatement(ret_v, pExpr2, inAndOutSignalNo);
				pExpr->setChildAt(i, pExpr2);
			}
		}

		if (bFuncFlow)
		{
			ScopeVector ret_v2, ret_v3;
			CScope* pScope = pExpr->getRealScope();

			CFuncDeclare* pCallFuncDeclare = pFuncExpr->getFuncDeclare();
			CClassDef* pCallClassDef = NULL;
			if (pCallFuncDeclare->getParent()->getGoType() == GO_TYPE_CLASS)
				pCallClassDef = (CClassDef*)pCallFuncDeclare->getParent();

			TypeDefPointer pCallStructTypeDef;
			if (pCallClassDef == NULL)
				pCallStructTypeDef = findTypeDef(FUNC_DEF_STRUCT_PREFIX + func_name);
			else
				pCallStructTypeDef = pCallClassDef->findSymbol(FUNC_DEF_STRUCT_PREFIX + pCallFuncDeclare->getName(), FIND_SYMBOL_SCOPE_LOCAL)->getTypeDef();
			MY_ASSERT(pCallStructTypeDef);

			std::string pStruVarName = getTempVarName();
			TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(NULL, "", pCallStructTypeDef, 1));

            CVarDef* pVarDef = new CVarDef(pScope, pStruVarName, pTypeDef2, NULL);
            m_pStructTypeDef->getClassDef()->addDef(new CStatement(m_pStructTypeDef->getClassDef(), STATEMENT_TYPE_DEF, pVarDef, NULL));

			// pFuncVar->pTemp = (Struct*)__flow_func_enter(pVar->basic.flow, caller_func, caller_data, caller_signal, sizeof(Struct));
			unsigned nSignalEnter = allocSignalNo(); // this is exiting signal
			inAndOutSignalNo = allocSignalNo();

			std::vector<CExpr*> param_v;
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
					FLOW_BASIC_FLOW_MEMBER_NAME));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, (m_pFunc->getParent()->getGoType() == GO_TYPE_CLASS ? 
					FLOW_STATIC_METHOD_PREFIX + pScope->getFunctionScope()->getName() : 
					pScope->getFunctionScope()->getName()
				)));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)));
			param_v.push_back(new CExpr(pExpr, EXPR_TYPE_SIZEOF, pCallStructTypeDef));
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pStruVarName),
				new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pTypeDef2,
					new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_FUNC_ENTER_FUNC_NAME), param_v)
				)
			)));

			// pFuncVar->pTemp->__flow_caller_this = ...
			if (pCallClassDef)
			{
				CExpr* pCallerExpr = NULL;
				if (pFuncExpr->getExprType() == EXPR_TYPE_REF_ELEMENT || pFuncExpr->getExprType() == EXPR_TYPE_PTR_ELEMENT)
				{
					pCallerExpr = (CExpr*)pFuncExpr->getChildAt(0);
					if (pFuncExpr->getExprType() == EXPR_TYPE_REF_ELEMENT)
					{
						pCallerExpr = new CExpr(NULL, EXPR_TYPE_ADDRESS_OF, pCallerExpr);
						pFuncExpr->setExprType(EXPR_TYPE_PTR_ELEMENT);
					}
					pFuncExpr->setChildAt(0, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
						new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pStruVarName), FLOW_PARAM_NAME_CALLER_THIS));
				}
				else
					pCallerExpr = new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, "this");

				ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN, 
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
						new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pStruVarName), FLOW_PARAM_NAME_CALLER_THIS), 
					pCallerExpr
				)));
			}
			// setting parameters into struct
			int nStart = pExpr->getValue().empty() ? 1 : 0;
			int n = pFuncCallType->getFuncParamCount();
			int n2 = pExpr->getChildrenCount() - nStart;
			MY_ASSERT(n >= n2);
			for (int i = 0; i < n2; i++)
			{
				// pLocal->pStru->pVar = param;
				CVarDef* pParamVarDef = pFuncCallType->getFuncParamAt(i);
				CExpr* pValueExpr = (CExpr*)pExpr->getChildAt(nStart);
				if (pParamVarDef->isReference())
					pValueExpr = new CExpr(NULL, EXPR_TYPE_ADDRESS_OF, pValueExpr);
				ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
						new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
							new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME),
							pStruVarName),
						pParamVarDef->getName()),
					pValueExpr)));
				pExpr->removeChildAt(nStart);
			}
			for (; n2 < n; n2++)
			{
				CVarDef* pVarDef = pFuncCallType->getFuncParamAt(n2);
				MY_ASSERT(pVarDef->getInitExpr());
				ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
						new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
							new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME),
							pStruVarName),
					pVarDef->getName()),
					pVarDef->getInitExpr())));
			}

			// if (!func(signal_first_enter, pFuncVar->pTempVar)) return false;
			pExpr->setFuncCallType(findTypeDef(FLOW_FUNC_TYPE_NAME));
			pExpr->setReturnType(g_type_def_bool);
			pExpr->setFlow(false);
			pExpr->addChild(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
			pExpr->addChild(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), 
				pStruVarName)
			);
			CExpr* pDeleteObjExpr = CExpr::copyExpr(pExpr);
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
				new CExpr(NULL, EXPR_TYPE_NOT, pExpr),
				new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, "false"))
			));

			/*// __flow_signal = n;
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)))));*/

			addBatchToStatementVector(ret_v, ret_v2, nSignalEnter);

			// pFuncVar->retTemp = pFuncVar->pTempVar->ret;
			std::string retVarName;
			if (!pFuncCallType->getFuncReturnType()->isVoid())
			{
				retVarName = getTempVarName();
				CVarDef* pVarDef = new CVarDef(NULL, retVarName, pFuncCallType->getFuncReturnType());
				m_pStructTypeDef->getClassDef()->addDef(new CStatement(m_pStructTypeDef->getClassDef(), STATEMENT_TYPE_DEF, pVarDef, NULL));

				addToStatementVector(ret_v, new CStatement(pScope, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), retVarName),
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
						new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pStruVarName),
						PARAM_NAME_CALLER_RET)
				)), inAndOutSignalNo);
			}

			// deleting objects
			pDeleteObjExpr->setChildAt(pDeleteObjExpr->getChildrenCount() - 2, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_DELETE_OBJECT)));
			addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR2, pDeleteObjExpr), inAndOutSignalNo);

			// __flow_func_leave(pFuncVar->basic.flow, pFuncVar->pTempVar);
			param_v.clear();
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
				FLOW_BASIC_FLOW_MEMBER_NAME
			));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pStruVarName));
			addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR2,
				new CExpr(pScope, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_FUNC_LEAVE_FUNC_NAME), param_v)
			), inAndOutSignalNo);

			if (!retVarName.empty())
				pExpr = new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), retVarName);
			else
				pExpr = NULL;
		}
		if (pExpr)
			pExpr->setFlow(false);
		break;
	}
	case EXPR_TYPE_AND:				// expr expr
	case EXPR_TYPE_OR:				// expr expr
	{
		MY_ASSERT(pExpr->getChildrenCount() == 2);

		CExpr* pExpr1 = (CExpr*)pExpr->getChildAt(0);
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo, local_var_names);
		pExpr->setChildAt(0, pExpr1);
		MY_ASSERT(!pExpr1->isFlow());

		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(1);
		if (!pExpr2->isFlow())
		{
			checkNonFlowExprDecompose(pExpr2, local_var_names);
			break;
		}
		pExpr1->setReturnType(g_type_def_bool);
		replaceExprWithStatement(ret_v, pExpr1, inAndOutSignalNo);
		unsigned nSignalEnter = inAndOutSignalNo;

		ScopeVector ret_v2;
		checkExprDecompose(ret_v2, pExpr2, inAndOutSignalNo, local_var_names);
		pExpr2 = new CExpr(NULL, EXPR_TYPE_ASSIGN, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pExpr1->getValue()), pExpr2);
		ret_v2.push_back(new CStatement(pExpr2));

		pExpr2 = new CExpr(pExpr->getParent(), EXPR_TYPE_TOKEN_WITH_NAMESPACE, pExpr1->getValue());
		if (pExpr->getExprType() == EXPR_TYPE_OR)
			pExpr2 = createNotExpr(pExpr2);
		addToStatementVector(ret_v, new CStatement(pExpr->getRealScope(), STATEMENT_TYPE_IF, pExpr2, new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2)), nSignalEnter);

		pExpr->m_children.clear();
		delete pExpr;
		pExpr = pExpr1;
		pExpr->setFlow(false);
		break;
	}
	case EXPR_TYPE_TERNARY:			// expr expr expr
	{
		MY_ASSERT(pExpr->getChildrenCount() == 3);

		CExpr* pExpr0 = (CExpr*)pExpr->getChildAt(0);
		checkExprDecompose(ret_v, pExpr0, inAndOutSignalNo, local_var_names);
		pExpr->setChildAt(0, pExpr0);
		MY_ASSERT(!pExpr0->isFlow());

		CExpr* pExpr1 = (CExpr*)pExpr->getChildAt(1);
		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(2);
		if (!pExpr1->isFlow() && !pExpr2->isFlow())
		{
			checkNonFlowExprDecompose(pExpr1, local_var_names);
			checkNonFlowExprDecompose(pExpr2, local_var_names);
			break;
		}
		TypeDefPointer return_type = pExpr->getReturnType();
		CVarDef* pVarDef = new CVarDef(NULL, getTempVarName(), return_type, NULL);
		SourceTreeNode* pDeclVarNode = declVarCreateByName(pVarDef->getName());
		for (int i = 0; i < pExpr->getReturnDepth(); i++)
			declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
		pVarDef->setDeclVarNode(pDeclVarNode);
		deleteSourceTreeNode(pDeclVarNode);
		CStatement* pStatement = new CStatement(pExpr->getRealScope(), STATEMENT_TYPE_EXPR2, pVarDef, NULL); //return_type->getBaseType()->getBasicNode());
		addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
		unsigned nSignalEnter = inAndOutSignalNo;

		ScopeVector ret_v2;

		checkExprDecompose(ret_v2, pExpr1, inAndOutSignalNo, local_var_names);
		MY_ASSERT(!pExpr1->isFlow());
		pExpr1 = new CExpr(NULL, EXPR_TYPE_ASSIGN, new CExpr(pExpr->getParent(), EXPR_TYPE_TOKEN_WITH_NAMESPACE, pVarDef->getName()), pExpr1);
		ret_v2.push_back(new CStatement(pExpr1));

		CStatement* pStatementIf = new CStatement(pExpr->getRealScope(), STATEMENT_TYPE_IF, pExpr0, new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));

		ret_v2.clear();
		checkExprDecompose(ret_v2, pExpr2, nSignalEnter, local_var_names);
		MY_ASSERT(!pExpr2->isFlow());
		pExpr2 = new CExpr(NULL, EXPR_TYPE_ASSIGN, new CExpr(pExpr->getParent(), EXPR_TYPE_TOKEN_WITH_NAMESPACE, pVarDef->getName()), pExpr2);
		ret_v2.push_back(new CStatement(pExpr2));

		pStatementIf->setElseStatement(new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));
		addToStatementVector(ret_v, pStatementIf, nSignalEnter);

		pExpr->m_children.clear();
		CScope* pParent = pExpr->getParent();
		delete pExpr;
		pExpr = new CExpr(pParent, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pVarDef->getName());
		break;
	}
	default:
		MY_ASSERT(false);
	}
}

// try to decompose one statement to a few ones if necessary. pass in signal via inAndOutSignalNo, function should set signal to new value in inAndOutSignalNo when returns.
void CMyFunc::checkStatementDecompose(ScopeVector& ret_v, CStatement* pStatement, unsigned nContinueSignal, unsigned& inAndOutSignalNo, LocalVarSet& local_var_names, unsigned& local_obj_var_cnt)
{
	bool bWithinFlowBlock = !(pStatement->getParent()->getGoType() == GO_TYPE_STATEMENT && !((CStatement*)pStatement->getParent())->isFlow());
	ScopeVector ret_v2, sv_nf;
	std::vector<CExpr*> param_v;

	if (pStatement->getStatementType() == STATEMENT_TYPE_DEF)
	{
		if (pStatement->getDefType() == DEF_TYPE_FUNC_VAR_DEF || 
			pStatement->getDefType() == DEF_TYPE_VAR_DEF || 
			pStatement->getDefType() == DEF_TYPE_SUPER_TYPE_VAR_DEF)
		{
			int currentSignal = inAndOutSignalNo;
			for (int i = 0; i < pStatement->getVarCount(); i++)
			{
				CStatement* pStatement2;
				CVarDef* pVarDef = pStatement->getVarAt(i);

				if (pVarDef->isReference())
				{
					pVarDef->setAppFlag(true);
					if (pVarDef->getInitExpr())
						pVarDef->setInitExpr(new CExpr(NULL, EXPR_TYPE_ADDRESS_OF, pVarDef->getInitExpr()));
				}

				std::string name = pVarDef->getName();
				MY_ASSERT(local_var_names.find(name) == local_var_names.end());
				NewVarInfo nvi;
				nvi.bParam = false;
				nvi.bRef = pVarDef->getAppFlag();
				nvi.new_name = name;
				local_var_names[name] = nvi;

				// add define into struct
				pVarDef->getParent()->getRealScope()->removeVarDef(pVarDef);
				TypeDefPointer pVarType = pVarDef->getType()->getBaseType();
				SourceTreeNode* pVarNode = dupSourceTreeNode(pVarDef->getDeclVarNode());
				if (pVarDef->getAppFlag())
				{
					declVarRemoveModifier(pVarNode, DVMOD_TYPE_REFERENCE);
					declVarAddModifier(pVarNode, DVMOD_TYPE_INTERNAL_POINTER);
				}
				MY_ASSERT(!pVarType->isReference());
				MY_ASSERT(!declVarIsReference(pVarNode));
				CVarDef* pVarDef2 = new CVarDef(pVarDef->getParent(), pVarDef->getName(), pVarType, pVarNode);
				pStatement2 = new CStatement(m_pStructTypeDef->getClassDef(), STATEMENT_TYPE_DEF, pVarDef2, NULL);
				m_pStructTypeDef->getClassDef()->addDef(pStatement2);

				int depth = 0;
				TypeDefPointer pTypeDef = getRootType(pVarDef->getType(), depth);
				if (depth == 0 && pTypeDef->getClassDef() && pTypeDef->getClassDef()->has_constructor())
				{
					// new (&a) A(...);
					pVarDef2->setSeqNo(m_local_obj_var_index++);
					local_obj_var_cnt++;

					CClassDef* pClassDef = pTypeDef->getClassDef();
					TokenWithNamespace twn = getRelativeTWN(pClassDef);
					/*twn.addScope(pClassDef->getName());
					CFuncDeclare* pFuncDeclare = pClassDef->findFuncDeclare(pClassDef->getName(), pVarDef->getExprListInDeclVar());
					MY_ASSERT(pFuncDeclare);
					CExpr* pFuncExpr = new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
						new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName()), twn);
					pFuncExpr->setFuncDeclare(pFuncDeclare);
					addToStatementVector(ret_v, new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR2, 
						new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pFuncExpr, pVarDef->getExprListInDeclVar())), inAndOutSignalNo);*/
					addToStatementVector(ret_v, new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR2, 
						new CExpr(NULL, EXPR_TYPE_NEW_ADV, new CExpr(NULL, EXPR_TYPE_ADDRESS_OF, new CExpr(NULL, EXPR_TYPE_PARENTHESIS, 
							new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName()))), 
							pVarDef->getType(), pVarDef->getExprListInDeclVar())), inAndOutSignalNo);

					// pLocalVar->__flow_delete_table[pLocalVar->basic.delete_counter++] = seq;
					addToStatementVector(ret_v, new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR2, 
						new CExpr(NULL, EXPR_TYPE_ASSIGN, 
							new CExpr(NULL, EXPR_TYPE_ARRAY, 
								new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_DELETE_TABLE), 
								new CExpr(NULL, EXPR_TYPE_RIGHT_INC, new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
									new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME), FLOW_DELETE_COUNTER))), 
							new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, ltoa(pVarDef2->getSeqNo())))
					), inAndOutSignalNo);
				}
				else
				{
					// add "int i;" in pStruct
					CExpr* pInitExpr = pVarDef->getInitExpr();

					if (pInitExpr)
					{
						// i = 3;
						pStatement2 = new CStatement(pStatement->getParent(), STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
							new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName()), 
							pInitExpr));
						checkStatementDecompose(ret_v, pStatement2, nContinueSignal, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
					}
				}

				pStatement->removeVarAt(i);
				i--;
			}
		}
		else
			addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
		return;
	}

	if (!pStatement->isFlow())
	{
		checkNonFlowStatementDecompose(ret_v2, pStatement, nContinueSignal, local_var_names, local_obj_var_cnt);
		addBatchToStatementVector(ret_v, ret_v2, inAndOutSignalNo);
		return;
	}

	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_EXPR2:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 1);
		CExpr2* pExpr2 = (CExpr2*)pStatement->getChildAt(0);
		int n = 0;
		for (int i = 0; i < pExpr2->getChildrenCount(); i++)
		{
			CExpr* pExpr = (CExpr*)pExpr2->getChildAt(i);
			if (!pExpr)
				continue;
			//std::string s = pExpr->toString();
			checkExprDecompose(ret_v, pExpr, inAndOutSignalNo, local_var_names);
			if (!pExpr)
				continue;
			MY_ASSERT(!pExpr->isFlow());
			if (pExpr->getExprType() != EXPR_TYPE_TOKEN_WITH_NAMESPACE)
			{
				pExpr2->setChildAt(n++, pExpr);
			}
		}
		while (pExpr2->getChildrenCount() > n)
			pExpr2->removeChildAt(pExpr2->getChildrenCount() - 1);

		if (n > 0)
		{
			pExpr2->setFlow(false);
			pStatement->setFlow(false);
			addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
		}
		break;
	}
	case STATEMENT_TYPE_COMPOUND:
	{
		LocalVarSet local_var_names2 = local_var_names;
		unsigned local_obj_var_cnt2 = local_obj_var_cnt;
		ret_v2 = pStatement->m_children;
		pStatement->m_children.clear();
		int nOldLocalObjVarIndex = m_local_obj_var_index;
		for (size_t i = 0; i < ret_v2.size(); i++)
		{
			CScope* pGrammarObj = ret_v2[i];

			MY_ASSERT(pGrammarObj->getGoType() == GO_TYPE_STATEMENT);
			checkStatementDecompose(pStatement->m_children, (CStatement*)pGrammarObj, nContinueSignal, inAndOutSignalNo, local_var_names2, local_obj_var_cnt2);
		}
		if (m_local_obj_var_index > nOldLocalObjVarIndex)
		{
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, ltoa(__FLOW_CALL_SIGNAL_DELETE_OBJECT + nOldLocalObjVarIndex)));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
			pStatement->m_children.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_FUNC_CALL, 
				m_pFunc->getFuncDeclare(), param_v)));
		}
		ret_v.push_back(pStatement);
		break;
	}
	case STATEMENT_TYPE_IF:
	{
		bool bFlow = false;
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		if (pExpr->isFlow())
		{
			checkExprDecompose(ret_v, pExpr, inAndOutSignalNo, local_var_names);
			MY_ASSERT(!pExpr->isFlow());
			pStatement->setChildAt(0, pExpr);
		}
		else
			checkNonFlowExprDecompose(pExpr, local_var_names);
		ret_v.push_back(pStatement);

		unsigned nSignalEnter = inAndOutSignalNo;
		unsigned nSignalStart = getNextSignalNo() + 1;

		CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(1);
		if (pStatement2->isFlow())
		{
			if (pStatement2->getStatementType() != STATEMENT_TYPE_COMPOUND)
			{
				ret_v2.clear();
				ret_v2.push_back(pStatement2);
				pStatement2 = new CStatement(pStatement2->getParent()->getRealScope(), STATEMENT_TYPE_COMPOUND, ret_v2);
			}
			ret_v2.clear();
			checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
			MY_ASSERT(ret_v2.size() == 1);
			pStatement->setChildAt(1, pStatement2);
		}
		else
			checkNonFlowStatementDecompose(sv_nf, pStatement2, 0, local_var_names, local_obj_var_cnt);
		bFlow |= pStatement2->isFlow();
		pStatement->setChildAt(0, composeSignalLogicExpr((CExpr*)pStatement->getChildAt(0), nSignalEnter, nSignalStart, inAndOutSignalNo));
		nSignalStart = getNextSignalNo();

		/*int n = pStatement->m_int_vector[0];
		for (int i = 0; i < n; i++)
		{
			CExpr* pExpr = (CExpr*)pStatement->getChildAt(2 + i * 2);
			CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(2 + i * 2 + 1);
			if (pExpr->isFlow())
			{
				pStatement2 = new CStatement(NULL, STATEMENT_TYPE_IF, pExpr, pStatement2);
				for (int j = i + 1; j < n; j++)
				{
					pStatement2->addElseIf((CExpr*)pStatement->getChildAt(2 + j * 2), (CStatement*)pStatement->getChildAt(2 + j * 2 + 1));
				}
				if (pStatement->m_bFlag)
				{
					pStatement2->addElseStatement((CStatement*)pStatement->getChildAt(2 + n * 2));
				}
				pStatement->m_int_vector[0] = n = i;
				pStatement->m_bFlag = false;
				pStatement->m_children.resize(2 + i * 2);
				pStatement->addElseStatement(pStatement2);
				bFlow |= true;
				break;
			}

			ret_v2.clear();
			if (pStatement2->isFlow())
			{
			  if (pStatement2->getStatementType() != STATEMENT_TYPE_COMPOUND)
			  {
				  ret_v2.clear();
				  ret_v2.push_back(pStatement2);
				  pStatement2 = new CStatement(pStatement2->getParent()->getRealScope(), STATEMENT_TYPE_COMPOUND, ret_v2);
			  }
			  ret_v2.clear();
			  checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo);
			  MY_ASSERT(ret_v2.size() == 1 && ret_v2[0] == pStatement2);
			  pStatement->setChildAt(2 + i * 2 + 1, pStatement2);
			}
			bFlow |= pStatement2->isFlow();
			pStatement->setChildAt(2 + i * 2, composeSignalLogicExpr((CExpr*)pStatement->getChildAt(2 + i * 2), nSignalEnter, nSignalStart, inAndOutSignalNo));
			nSignalStart = getNextSignalNo();
		}*/

		CStatement* pElseStatement = (CStatement*)pStatement->getChildAt(2);
		if (pElseStatement)
		{
			if (pElseStatement->isFlow())
			{
				if (pElseStatement->getStatementType() != STATEMENT_TYPE_COMPOUND)
				{
					ret_v2.clear();
					ret_v2.push_back(pElseStatement);
					pElseStatement = new CStatement(pElseStatement->getParent()->getRealScope(), STATEMENT_TYPE_COMPOUND, ret_v2);
				}
				ret_v2.clear();
				checkStatementDecompose(ret_v2, pElseStatement, nContinueSignal, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
				MY_ASSERT(ret_v2.size() == 1 && ret_v2[0] == pStatement2);
			}
			else
				checkNonFlowStatementDecompose(sv_nf, pElseStatement, 0, local_var_names, local_obj_var_cnt);
			bFlow |= pStatement2->isFlow();
			pStatement->setElseStatement(new CStatement(NULL, STATEMENT_TYPE_IF, 
				composeSignalLogicExpr(NULL, nSignalEnter, nSignalStart, inAndOutSignalNo), pElseStatement));
		}

		pStatement->setFlow(bFlow);
		break;
	}
	case STATEMENT_TYPE_WHILE:
	case STATEMENT_TYPE_DO:
	case STATEMENT_TYPE_FOR:
	{
		CExpr* pExpr1 = NULL, *pExpr2 = NULL;
		CStatement* pStatement2;

		if (pStatement->getStatementType() == STATEMENT_TYPE_WHILE)
		{
			MY_ASSERT(pStatement->getChildrenCount() == 2);
			pExpr1 = (CExpr*)pStatement->getChildAt(0);
			pStatement2 = (CStatement*)pStatement->getChildAt(1);
		}
		else if (pStatement->getStatementType() == STATEMENT_TYPE_DO)
		{
			MY_ASSERT(pStatement->getChildrenCount() == 2);
			pStatement2 = (CStatement*)pStatement->getChildAt(0);
			pExpr1 = (CExpr*)pStatement->getChildAt(1);
		}
		else // STATEMENT_TYPE_FOR
		{
			MY_ASSERT(pStatement->getChildrenCount() == 4);
			pExpr1 = (CExpr*)pStatement->getChildAt(0);

			// if (signal <= 0)
            //   expr1; // for only
			if (pExpr1)
			{
				checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo, local_var_names);
				MY_ASSERT(!pExpr1->isFlow());
				addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR2, pExpr1), inAndOutSignalNo);
			}

			pExpr1 = (CExpr*)pStatement->getChildAt(1);
			pExpr2 = (CExpr*)pStatement->getChildAt(2);
			pStatement2 = (CStatement*)pStatement->getChildAt(3);
		}

		// int nTempMode = signal <= 0 ? 0 : 1;
		std::string tempVarName = getTempVarName();
		//CVarDef* pVarDef = new CVarDef(NULL, tempVarName, g_type_def_bool, declVarCreateByName(tempVarName)); // var of bFirstEnter
        //m_pStructTypeDef->getClassDef()->addDef(new CStatement(m_pStructTypeDef->getClassDef(), STATEMENT_TYPE_DEF, pVarDef, NULL));

//#define VAR_bFirstEnter new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName())

		CVarDef* pVarDef = new CVarDef(NULL, tempVarName, g_type_def_int, NULL, new CExpr(NULL, EXPR_TYPE_TERNARY, 
			new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, 
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), 
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo))), 
			new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "0"), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "1")));
		ret_v.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pVarDef));
		/*
          if (signal <= nSignalEnter)
		    expr1; // for only
          bFirstEnter = true;
          while (signal <= lastSignal)
          {
            if (nTempMode > 1 && signal > nSignalEnter + 1)
              signal = nSignalEnter + 1;
            if (signal <= nSignalEnter + 1)
            {
              if (nTempMode != 0) // for only
                expr2;
              if (!expr) // while and for only
                break;
              if (nTempMode != 0 && !expr) // do only
                break;
            }
            bFirstEnter = false;
		*/
		unsigned nSignalEnter = getNextSignalNo();
		if (nSignalEnter == 0)
			nSignalEnter = allocSignalNo() + 1;

		if (pStatement2->getStatementType() != STATEMENT_TYPE_COMPOUND)
		{
			ret_v2.clear();
			ret_v2.push_back(pStatement2);
			pStatement2 = new CStatement(pStatement, STATEMENT_TYPE_COMPOUND, ret_v2);
		}

		// nTempMode = 2;
		CStatement* pStatement3 = new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN, 
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, tempVarName), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "2")));
		pStatement3->setAppFlag();
		pStatement2->insertChildAt(0, pStatement3);

        //if (!expr) // while and for only
        //break;
		//if (nTempMode != 0 && !expr) // do only
		//  break;
		pExpr1 = createNotExpr(pExpr1);
		if (pStatement->getStatementType() == STATEMENT_TYPE_DO)
			pExpr1 = new CExpr(NULL, EXPR_TYPE_AND,
				new CExpr(NULL, EXPR_TYPE_NOT_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, tempVarName), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "0")), 
				pExpr1);
		pStatement2->insertChildAt(0, new CStatement(NULL, STATEMENT_TYPE_IF, pExpr1,
			new CStatement(NULL, STATEMENT_TYPE_BREAK)));

		//if (nTempMode != 0) // for only
		//  expr2;
		if (pStatement->getStatementType() == STATEMENT_TYPE_FOR)
		{
			pStatement2->insertChildAt(0, new CStatement(NULL, STATEMENT_TYPE_IF,
				new CExpr(NULL, EXPR_TYPE_NOT_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, tempVarName), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "0")), 
				new CStatement(NULL, STATEMENT_TYPE_EXPR2, pExpr2)));
		}

		m_continue_stack.push_back(local_obj_var_cnt);
		m_break_stack.push_back(local_obj_var_cnt);

		ret_v2.clear();
		checkStatementDecompose(ret_v2, pStatement2, nSignalEnter, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
		MY_ASSERT(ret_v2.size() == 1);
		pStatement2 = (CStatement*)ret_v2[0];
		pStatement->setChildAt(1, pStatement2);

		m_continue_stack.pop_back();
		m_break_stack.pop_back();

        // if (nTempMode > 1 && signal > nSignalEnter)
        //   signal = nSignalEnter;
		pStatement2->insertChildAt(0, new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_AND, 
				new CExpr(NULL, EXPR_TYPE_GREATER_THAN, 
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, tempVarName), 
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "1")), 
				new CExpr(NULL, EXPR_TYPE_GREATER_THAN, 
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), 
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter)))), 
			new CStatement(NULL, STATEMENT_TYPE_EXPR2, 
				new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))))));

		pStatement->m_statement_type = STATEMENT_TYPE_WHILE;
		pStatement->m_children.clear();
		pStatement->addChild(new CExpr(NULL, EXPR_TYPE_LESS_EQUAL,
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
			new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo))));
		pStatement->addChild(pStatement2);
		ret_v.push_back(pStatement);

		//inAndOutSignalNo = nSignalEnter;
		break;
	}
	case STATEMENT_TYPE_SWITCH:
	{
		/*
			if (signal <= nSignalLast)
			{
				var temp = expr1;
				switch (temp)
				{
				......
				}
				if (signal >= nEnter)
					signal = nEnter;
			}
		*/
		m_break_stack.push_back(local_obj_var_cnt);

		ScopeVector ret_v3;
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		if (pExpr->isFlow())
		{
			checkExprDecompose(ret_v3, pExpr, inAndOutSignalNo, local_var_names);
			MY_ASSERT(!pExpr->isFlow());
			pStatement->setChildAt(0, pExpr);
		}
		else
			checkNonFlowExprDecompose(pExpr, local_var_names);
		ret_v3.push_back(pStatement);

		unsigned nSignalEnter = allocSignalNo(); // this is exiting signal

		int n = 1;
		for (int i = 0; i < pStatement->m_int_vector.size(); i++)
		{
			int m = pStatement->m_int_vector[i];
			CExpr* pExpr = (CExpr*)pStatement->getChildAt(n);
			MY_ASSERT(!pExpr->isFlow());
			for (int j = 1; j < m;)
			{
				CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(n + j);
				ret_v2.clear();
				checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
				pStatement->removeChildAt(n + j);
				pStatement->insertChildrenAt(n + j, ret_v2);
				m += ret_v2.size() - 1;
				j += ret_v2.size();
			}
			pStatement->m_int_vector[i] = m;
			n += m;
		}
		if (pStatement->m_bFlag)
		{
			for (; n < pStatement->getChildrenCount();)
			{
				CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(n);
				ret_v2.clear();
				checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
				pStatement->removeChildAt(n);
				pStatement->insertChildrenAt(n, ret_v2);
				n += ret_v2.size();
			}
		}

		ret_v3.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_GREATER_THAN,
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))),
			new CStatement(NULL, STATEMENT_TYPE_EXPR2,
				new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))))));

		addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v3), inAndOutSignalNo);
		inAndOutSignalNo = nSignalEnter;
		m_break_stack.pop_back();
		break;
	}
	case STATEMENT_TYPE_RETURN:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 1);
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		MY_ASSERT(pExpr->isFlow());
		checkExprDecompose(ret_v, pExpr, inAndOutSignalNo, local_var_names);
		MY_ASSERT(!pExpr->isFlow());
		pStatement->setChildAt(0, pExpr);
		pStatement->setFlow(false);

		checkNonFlowStatementDecompose(ret_v, pStatement, nContinueSignal, local_var_names, local_obj_var_cnt);
		break;
	}
	case STATEMENT_TYPE_FLOW_WAIT:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 2);
		CExpr* pExpr1 = (CExpr*)pStatement->getChildAt(0);
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo, local_var_names);
		MY_ASSERT(!pExpr1->isFlow());

		CExpr* pExpr2 = (CExpr*)pStatement->getChildAt(1);
		checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo, local_var_names);
		MY_ASSERT(!pExpr2->isFlow());

		pStatement->removeAllChildren();

		// if (!__flow_wait(pExpr1, this_func, signal, pFuncVar, pExpr2))
		//   return false;
		CFuncDeclare* pFuncDeclare = findFuncDecl(FLOW_WAIT_FUNC_NAME);
		MY_ASSERT(pFuncDeclare);
		CExpr* pExpr = new CExpr(pStatement->getParent()->getRealScope(), EXPR_TYPE_FUNC_CALL, pFuncDeclare, param_v);

		unsigned nSignalEnter = allocSignalNo(); // this is exiting signal
		inAndOutSignalNo = allocSignalNo();

		CScope* pFunc = pStatement;
		while (pFunc->getGoType() != GO_TYPE_FUNC)
			pFunc = pFunc->getParent();
		pExpr->addChild(pExpr1);
		pExpr->addChild(getMyCallFuncName());
		pExpr->addChild(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)));
		pExpr->addChild(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
		pExpr->addChild(pExpr2);

		pStatement = new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_NOT, pExpr),
			new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false")));
		addToStatementVector(ret_v, pStatement, nSignalEnter);

		//addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN, 
		//		new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
		//		new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)))), 
		//	nSignalEnter);
		break;
	}
	case STATEMENT_TYPE_FLOW_FORK:
	case STATEMENT_TYPE_FLOW_NEW:
	{
		bool bIsFlowNew = pStatement->getStatementType() == STATEMENT_TYPE_FLOW_NEW;

		pStatement->setRealScope(true); // so that when changing vars, it won't search in it's parent scope

		TypeDefPointer pFuncType = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, g_type_def_void, 0));
		pFuncType->setFuncFlowType(FLOW_TYPE_FLOW);

		TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), m_pStructTypeDef->getName(), m_pStructTypeDef, 1));
		pFuncType->addFuncParam(new CVarDef(NULL, FLOW_FORK_PARENT_PARAM_NAME, pTypeDef, NULL));

		std::string new_sub_name = getNewSubFuncName();
		CFunction* pNewFunc = new CFunction(m_pFunc->getParent()->getRealScope(), new_sub_name, pFuncType, FLOW_TYPE_FLOW, 
			new CFuncDeclare(m_pFunc->getParent()->getRealScope(), new_sub_name, pFuncType));
		pNewFunc->setDefLocation(m_pFunc->getDefFileStack(), 0);

		MY_ASSERT(pStatement->getChildrenCount() == (bIsFlowNew ? 2 : 1));
		CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(bIsFlowNew ? 1 : 0);
		if (pStatement2->getStatementType() == STATEMENT_TYPE_COMPOUND)
		{
			for (int i = 0; i < pStatement2->getChildrenCount(); i++)
			{
				CStatement* pStatement3 = (CStatement*)pStatement2->getChildAt(i);
				moveVarUnderParentInStatement(pStatement3, FLOW_FORK_PARENT_PARAM_NAME, local_var_names);
				pNewFunc->addChild(pStatement3);
			}
		}
		else
		{
			moveVarUnderParentInStatement(pStatement2, FLOW_FORK_PARENT_PARAM_NAME, local_var_names);
			pNewFunc->addChild(pStatement2);
		}
		pNewFunc->addChild(new CStatement(pNewFunc, STATEMENT_TYPE_RETURN));

		CMyFunc* pNewMyFunc = new CMyFunc(pNewFunc, true);
		ScopeVector ret_v3;
		ret_v3 = pNewMyFunc->analyze();
		m_result_v.insert(m_result_v.end(), ret_v3.begin(), ret_v3.end());

		unsigned nSignalEnter = inAndOutSignalNo;

		CExpr* pFlowNewExpr = NULL;
		if (bIsFlowNew)
		{
			pFlowNewExpr = (CExpr*)pStatement->getChildAt(0);
			if (pFlowNewExpr->isFlow())
			{
				checkExprDecompose(ret_v2, pFlowNewExpr, inAndOutSignalNo, local_var_names);
				MY_ASSERT(!pFlowNewExpr->isFlow());
			}
		}
		// void* pTempFlow = __flow_start(pVar->basic.pFlow, 0);
		param_v.push_back(
			bIsFlowNew ?
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER) : 
				new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME), FLOW_BASIC_FLOW_MEMBER_NAME)
		);
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "0"));
		CVarDef* pFlowVar = new CVarDef(pStatement->getParent()->getRealScope(), getTempVarName(), g_type_def_void_ptr, NULL,
			new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_START_FUNC_NAME), param_v)
		);
		ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pFlowVar));

		if (bIsFlowNew)
		{
			// expr = pTempFlow;
			ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN, 
				pFlowNewExpr, 
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pFlowVar->getName())
			)));
		}

		// struct* pTemp1 = (struct*)__flow_func_enter(pTempFlow, NULL, NULL, 0, sizeof(struct));
		TypeDefPointer pBaseTypeDef = findTypeDef(FUNC_DEF_STRUCT_PREFIX + pNewFunc->getName());
		MY_ASSERT(pBaseTypeDef);
		TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(pNewFunc->getParent()->getRealScope(), pBaseTypeDef->getName(), pBaseTypeDef, 1));

		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pFlowVar->getName()));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		param_v.push_back(new CExpr(pStatement, EXPR_TYPE_SIZEOF, findTypeDef(pBaseTypeDef->getName())));

		CVarDef* pFuncVar = new CVarDef(pNewFunc->getParent()->getRealScope(), getTempVarName(), pTypeDef2, NULL,
			new CExpr(pStatement->getRealScope(), EXPR_TYPE_TYPE_CAST, pTypeDef2,
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_FUNC_ENTER_FUNC_NAME), param_v)
		));
		ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pFuncVar));

	    //pTempFunc->parent_param = pFuncVar;
		ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pFuncVar->getName()), FLOW_FORK_PARENT_PARAM_NAME),
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME)
		)));

		// func_call(0, pTemp1);
		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pFuncVar->getName()));
		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2,
			new CExpr(pStatement->getParent()->getRealScope(), EXPR_TYPE_FUNC_CALL, pNewFunc->getFuncDeclare(), param_v)
		));

		addBatchToStatementVector(ret_v, ret_v2, inAndOutSignalNo);

		delete pNewMyFunc;
		break;
	}
	case STATEMENT_TYPE_FLOW_TRY:
	{
		pStatement->setRealScope(true); // so that when changing vars, it won't search in it's parent scope
		int nCatches = pStatement->m_int_vector[0];
		MY_ASSERT(nCatches > 0);

	    // move try block to a sub function
		TypeDefPointer pFuncType = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, g_type_def_void, 0));
		pFuncType->setFuncFlowType(FLOW_TYPE_FLOW);

		TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), m_pStructTypeDef->getName(), m_pStructTypeDef, 1));
		pFuncType->addFuncParam(new CVarDef(NULL, FLOW_FORK_PARENT_PARAM_NAME, pTypeDef, NULL));

		CFunction* pSubFunc = new CFunction(m_pFunc->getParent()->getRealScope(), getNewSubFuncName(), pFuncType, FLOW_TYPE_FLOW,  
			new CFuncDeclare(m_pFunc->getParent()->getRealScope(), getNewSubFuncName(), pFuncType));
		pSubFunc->setDefLocation(m_pFunc->getDefFileStack(), 0);

		CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(0);
		if (pStatement2->getStatementType() == STATEMENT_TYPE_COMPOUND)
		{
			for (int i = 0; i < pStatement2->getChildrenCount(); i++)
			{
				CStatement* pStatement3 = (CStatement*)pStatement2->getChildAt(i);
				moveVarUnderParentInStatement(pStatement3, FLOW_FORK_PARENT_PARAM_NAME, local_var_names);
				pSubFunc->addChild(pStatement3);
			}
		}
		else
		{
			moveVarUnderParentInStatement(pStatement2, FLOW_FORK_PARENT_PARAM_NAME, local_var_names);
			pSubFunc->addChild(pStatement2);
		}
		pStatement2 = NULL;

		ScopeVector ret_v3;

		CMyFunc* pMySubFunc = new CMyFunc(pSubFunc, false);
		ret_v3 = pMySubFunc->analyze();
		m_result_v.insert(m_result_v.end(), ret_v3.begin(), ret_v3.end());

		ret_v3.clear();
		unsigned nSignalEnter = inAndOutSignalNo;

		// pVar->sub_flow = __flow_start(pVar->basic.pFlow);
        CVarDef* pVarSubFlow = new CVarDef(pStatement, getTempVarName(), g_type_def_void_ptr);
        m_pStructTypeDef->getClassDef()->addDef(new CStatement(m_pStructTypeDef->getClassDef(), STATEMENT_TYPE_DEF, pVarSubFlow, NULL));

		param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
			FLOW_BASIC_FLOW_MEMBER_NAME));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "0"));
		addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
            new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarSubFlow->getName()),
            new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_START_FUNC_NAME), param_v)
		)), nSignalEnter);

		//pVar->pTempWait1 = &object1;
		//pVar->pTempWait2 = &object2;
		int n;
		TypeDefPointer pBaseTypeDef = findTypeDef(FLOW_OBJECT_TYPE_NAME);
		MY_ASSERT(pBaseTypeDef);

		TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getClassDef(), pBaseTypeDef->getName(), pBaseTypeDef, 1));

		std::vector<std::string> tempWaitVarNameList;
		for (n = 0; n < nCatches; n++)
		{
			CExpr* pWaitObjExpr = (CExpr*)pStatement->getChildAt(1 + n * 3);
			MY_ASSERT(!pWaitObjExpr->isFlow());
			checkNonFlowExprDecompose(pWaitObjExpr, local_var_names);
	        CVarDef* pVarWait = new CVarDef(pStatement, getTempVarName(), pTypeDef2);
	        m_pStructTypeDef->getClassDef()->addDef(new CStatement(m_pStructTypeDef->getClassDef(), STATEMENT_TYPE_DEF, pVarWait, NULL));
	        tempWaitVarNameList.push_back(pVarWait->getName());
			addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
	            new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarWait->getName()),
	            pWaitObjExpr
			)), nSignalEnter);
		}

	    /*if (__flow_wait(pVar->pTempWait1, this_func, pVar, 1, param1))
	      signal = 1;
	    else if (__flow_wait(pVar->pTempWait2, this_func, pVar, 2, param2))
	      signal = 2;
	    else
	    {
          pFuncTemp->parent_param = pFuncVar;
	      struct* pFuncTemp = __flow_func_enter(sub_flow, caller_func, caller_data, 3, sizeof(struct));
	      if (!func_sub1(0, pFuncTemp))
	        return false;
	    }
*/
		CStatement* pLastIfStatement = NULL;
		for (n = 0; n < nCatches; n++)
		{
			CExpr* pWaitParamExpr = (CExpr*)pStatement->getChildAt(1 + n * 3 + 1);
			MY_ASSERT(!pWaitParamExpr->isFlow());
			checkNonFlowExprDecompose(pWaitParamExpr, local_var_names);

			param_v.clear();
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), tempWaitVarNameList[n]));
			param_v.push_back(getMyCallFuncName());
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1)));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
			param_v.push_back(pWaitParamExpr);

			CStatement* pTempStatement = new CStatement(NULL, STATEMENT_TYPE_IF, 
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_WAIT_FUNC_NAME), param_v),
				new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))))
			);

			if (n == 0)
				pStatement2 = pLastIfStatement = pTempStatement;
			else
			{
				pLastIfStatement->setElseStatement(pTempStatement);
				pLastIfStatement = pTempStatement;
			}
			allocSignalNo();
		}

		ret_v2.clear();

		pBaseTypeDef = findTypeDef(FUNC_DEF_STRUCT_PREFIX + pSubFunc->getName());
		MY_ASSERT(pBaseTypeDef);

		pTypeDef2 = TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), pBaseTypeDef->getName(), pBaseTypeDef, 1));

		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarSubFlow->getName()));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, m_pFunc->getName()));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1)));
		param_v.push_back(new CExpr(pStatement, EXPR_TYPE_SIZEOF, findTypeDef(FUNC_DEF_STRUCT_PREFIX + pSubFunc->getName())));
		CVarDef* pFuncVar = new CVarDef(m_pFunc, getTempVarName(), pTypeDef2, NULL,
			new CExpr(pStatement->getRealScope(), EXPR_TYPE_TYPE_CAST, pTypeDef2,
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_FUNC_ENTER_FUNC_NAME), param_v)
		));
		ret_v2.push_back(new CStatement(m_pFunc, STATEMENT_TYPE_DEF, pFuncVar));
		inAndOutSignalNo = allocSignalNo() + 1;

		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pFuncVar->getName()), FLOW_FORK_PARENT_PARAM_NAME),
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME)
		)));

		// func_call(0, pTemp1);
		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pFuncVar->getName()));
		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_NOT, new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, pSubFunc->getFuncDeclare(), param_v)),
			new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false"))
		));

		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))
		)));

		pLastIfStatement->setElseStatement(new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));

		addToStatementVector(ret_v, pStatement2, nSignalEnter);
		pStatement2 = NULL;

		/*if (signal == 1)
		{
		    flow_end(pVar->sub_flow);
		    flow_cancel_wait(&object2);
		    ...
		}
		else if (signal == 2)
		{
		    flow_end(pVar->sub_flow);
		    flow_cancel_wait(&object1);
		    ...
		}
		else if (signal == 0 || signal == 3)
		{
			flow_end(pFuncVar->sub_flow);
			flow_cancel_wait(pFuncVar->pTempWait1);
			flow_cancel_wait(pFuncVar->pTempWait2);
		}*/

		pLastIfStatement = NULL;
		for (n = 0; n < nCatches; n++)
		{
			ret_v2.clear();
			for (int i = 0; i < nCatches; i++)
			{
				if (i == n)
					continue;
				param_v.clear();
				param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), tempWaitVarNameList[i]));
				ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2,
					new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_CANCEL_WAIT_FUNC_NAME), param_v)));
			}
			param_v.clear();
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarSubFlow->getName()));
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2,
					new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_END_FUNC_NAME), param_v)));

			CStatement* pCatchStatement = (CStatement*)pStatement->getChildAt(1 + n * 3 + 2);
			if (pCatchStatement->getStatementType() == STATEMENT_TYPE_COMPOUND)
			{
				for (int i = 0; i < pCatchStatement->getChildrenCount(); i++)
				{
					CStatement* pStatement3 = (CStatement*)pCatchStatement->getChildAt(i);
					checkStatementDecompose(ret_v2, pStatement3, -1, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
				}
			}
			else
			{
				checkStatementDecompose(ret_v2, pCatchStatement, -1, inAndOutSignalNo, local_var_names, local_obj_var_cnt);
			}

			CStatement* pTempStatement = new CStatement(NULL, STATEMENT_TYPE_IF,
					new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))),
					new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2)
				);
			if (n == 0)
				pStatement2 = pTempStatement;
			else
				pLastIfStatement->setElseStatement(pTempStatement);
			pLastIfStatement = pTempStatement;
		}

		ret_v2.clear();
		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarSubFlow->getName()));
		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2,
				new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_END_FUNC_NAME), param_v)));
		for (int i = 0; i < nCatches; i++)
		{
			param_v.clear();
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), tempWaitVarNameList[i]));
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2,
				new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_CANCEL_WAIT_FUNC_NAME), param_v)));
		}
		pLastIfStatement->setElseStatement(new CStatement(NULL, STATEMENT_TYPE_IF, 
			new CExpr(NULL, EXPR_TYPE_OR, 
				new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))), 
				new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + nCatches + 1)))
			),
			new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2)));
		ret_v.push_back(pStatement2);

		pStatement->removeAllChildren();
		delete pMySubFunc;
		break;
	}
	//case STATEMENT_TYPE_FLOW_ENGINE:
	//	MY_ASSERT(pStatement->getChildrenCount() == 1);
	//	ret_v.push_back(pStatement->getChildAt(0));
	//	break;
	case STATEMENT_TYPE_TRY:
		printf("\n\nflow func is not allowed in try block\n");
		MY_ASSERT(false);
		break;
	case STATEMENT_TYPE_BREAK:
	case STATEMENT_TYPE_CONTINUE:
		MY_ASSERT(false); // these are always non-sync statements
		break;
	default:
		MY_ASSERT(false);
	}

//printf("checkStatementDecompose, this=%lx, return size=%d\n", (long)pStatement, ret_v.size());
}

void CMyFunc::checkDupNames(CStatement* pStatement)
{
	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_DEF:
	{
		for (int i = 0; i < pStatement->getVarCount(); i++)
		{
			CVarDef* pVar = pStatement->getVarAt(i);

			std::string name = pVar->getName();
			if (findVar(name))
			{
				name = getTempVarName();

				CExpr* pExpr = new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, name); 
				pVar->changeName(name, pExpr);
				delete pExpr;
			}
			addVar(name);
		}
		break;
	}
	case STATEMENT_TYPE_COMPOUND:
	{
		if (!pStatement->isFlow())
			return;

		for (int i = 0; i < pStatement->getChildrenCount(); i++)
		{
			CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(i);

			checkDupNames(pStatement2);
		}
		break;
	}
	case STATEMENT_TYPE_FOR:
	{
		if (pStatement->getChildAt(0) == NULL && pStatement->getVarCount() > 0) // decl var
		{
			CStatement* pNewStatement = new CStatement(pStatement);
			*pNewStatement = *pStatement;
			pNewStatement->setParent(pStatement);
			pStatement->setStatementType(STATEMENT_TYPE_COMPOUND);
			pStatement->setRealScope(true);
			pStatement->removeAllChildren();

			for (int i = 0; i < pNewStatement->getVarCount(); i++)
			{
				CVarDef* pVarDef = pNewStatement->getVarAt(i);
				pStatement->removeVarDef(pVarDef);
				pStatement->addChild(new CStatement(pStatement, STATEMENT_TYPE_DEF, pVarDef));
			}
			pNewStatement->removeAllVars();
			pStatement->addChild(pNewStatement);
		}
		//break; no break here, let it go through
	}
	default:
	{
		for (int i = 0; i < pStatement->getChildrenCount(); i++)
		{
			CScope* pObj = pStatement->getChildAt(i);
			if (!pObj)
				continue;
			if (pObj->getGoType() == GO_TYPE_STATEMENT)
			  checkDupNames((CStatement*)pObj);
		}
	}
	}
}

// if var is in local_var_names, then change it to LOCALVAR_NAME->var
void CMyFunc::checkNonFlowExprDecompose(CExpr* pExpr, const LocalVarSet& local_var_names)
{
	if (pExpr->getExprType() == EXPR_TYPE_TOKEN_WITH_NAMESPACE)
	{
		TokenWithNamespace twn = pExpr->getTWN();
		LocalVarSet::const_iterator it;
		if (twn.getDepth() == 1 && (it = local_var_names.find(twn.getLastToken())) != local_var_names.end())
		{
			pExpr->setExprType(EXPR_TYPE_PTR_ELEMENT);
			pExpr->addChild(new CExpr(pExpr, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
			pExpr->setValue(twn.getLastToken());

			if (it->second.bRef) // a reference, need to change to (*pVar->name)
			{
				CExpr* pNewExpr = new CExpr(NULL);
				*pNewExpr = *pExpr;
				pExpr->removeAllChildren();
				pExpr->setExprType(EXPR_TYPE_PARENTHESIS);
				pExpr->addChild(new CExpr(pExpr, EXPR_TYPE_INDIRECTION, pNewExpr));
			}
		}
		return;
	}

	for (int i = 0; i < pExpr->getChildrenCount(); i++)
	{
		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(i);
		MY_ASSERT(pExpr2->getGoType() == GO_TYPE_EXPR || pExpr2->getGoType() == GO_TYPE_EXPR2);
		checkNonFlowExprDecompose(pExpr2, local_var_names);
	}
}

// for non-sync statements, we want to leave it unchanged except return statement. We also need to change all local var names
void CMyFunc::checkNonFlowStatementDecompose(ScopeVector& ret_v, CStatement* pStatement, unsigned nContinueSignal, LocalVarSet& local_var_names, unsigned local_obj_var_cnt)
{
	ScopeVector ret_v2;

	MY_ASSERT(!pStatement->isFlow());

	if (pStatement->getStatementType() == STATEMENT_TYPE_COMPOUND)
	{
		LocalVarSet local_var_names2 = local_var_names;
		ret_v2 = pStatement->m_children;
		pStatement->m_children.clear();
		for (size_t i = 0; i < ret_v2.size(); i++)
		{
			CScope* pGrammarObj = ret_v2[i];

			MY_ASSERT(pGrammarObj->getGoType() == GO_TYPE_STATEMENT);
			checkNonFlowStatementDecompose(pStatement->m_children, (CStatement*)pGrammarObj, nContinueSignal, local_var_names2, local_obj_var_cnt);
		}
		ret_v.push_back(pStatement);
		return;
	}

	if (pStatement->getStatementType() == STATEMENT_TYPE_DEF)
	{
		// if there's a var redefined, then the use of this var in all statements below within the compound refer to this non-flow var
		for (int i = 0; i < pStatement->getVarCount(); i++)
		{
			CVarDef* pVarDef = pStatement->getVarAt(i);
			if (local_var_names.find(pVarDef->getName()) != local_var_names.end())
				local_var_names.erase(pVarDef->getName());
		}
		ret_v.push_back(pStatement);
		return;
	}

	for (int i = 0; i < pStatement->getChildrenCount(); i++)
	{
		CScope* pObj = pStatement->getChildAt(i);
		if (!pObj)
			continue;
		if (pObj->getGoType() == GO_TYPE_EXPR || pObj->getGoType() == GO_TYPE_EXPR2)
		{
			checkNonFlowExprDecompose((CExpr*)pObj, local_var_names);
		}
		else if (pObj->getGoType() == GO_TYPE_STATEMENT)
		{
			ScopeVector ret_v2;
			checkNonFlowStatementDecompose(ret_v2, (CStatement*)pObj, nContinueSignal, local_var_names, local_obj_var_cnt);
			MY_ASSERT(ret_v2.size() > 0);
			if (ret_v2.size() == 1)
				pStatement->setChildAt(i, ret_v2[0]);
			else
				pStatement->setChildAt(i, new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));
		}
	}

	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_RETURN:
		if (isFork())
		{
			// __flow_end(pVar->basic.pFlow);
			ExprVector param_v;
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
				FLOW_BASIC_FLOW_MEMBER_NAME));
			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, 
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_END_FUNC_NAME), param_v)));
			// return false;
			MY_ASSERT(pStatement->getChildrenCount() == 0);
			pStatement->addChild(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false"));
			ret_v.push_back(pStatement);
		}
		else if (m_pFunc->isFlowRoot())
		{
			// __flow_end(pVar->basic.pFlow);
			ExprVector param_v;
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
				FLOW_BASIC_FLOW_MEMBER_NAME));
			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, 
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_END_FUNC_NAME), param_v)));
			// return
			ret_v.push_back(pStatement);
		}
		else
		{
			// assign ret value;
			if (pStatement->getChildrenCount() > 0)
			{
				ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), PARAM_NAME_CALLER_RET),
					(CExpr*)pStatement->getChildAt(0)
				)));
				pStatement->removeChildAt(0);
			}

			// if (signal != 0 && pLocalVar->basic.caller_func)
			//	pLocalVar->basic.caller_func(pLocalVar->basic.signal, pLocalVar->basic.caller_data);
			ExprVector param_v;
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
					PARAM_NAME_CALLER_SIGNAL));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
					PARAM_NAME_CALLER_DATA));

			CExpr* pFuncExpr = new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
								new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
								PARAM_NAME_CALLER_FUNC);
			pFuncExpr->setFuncDeclare(m_pFunc->getFuncDeclare()); // not exactly correct, but this is the func declare I can find
			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
				new CExpr(NULL, EXPR_TYPE_AND,
					new CExpr(NULL, EXPR_TYPE_NOT_EQUAL,
						new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
						new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER))
					),
					new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
							new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
							PARAM_NAME_CALLER_FUNC)
				),
				new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pFuncExpr, param_v))
			));

			// return signal == 0;
			pStatement->addChild(new CExpr(NULL, EXPR_TYPE_EQUAL,
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER))
			));
			ret_v.push_back(pStatement);
		}
		break;

	case STATEMENT_TYPE_CONTINUE:
	case STATEMENT_TYPE_BREAK:
	{
		unsigned entering_cnt = pStatement->getStatementType() == STATEMENT_TYPE_CONTINUE ? m_continue_stack.back() : m_break_stack.back();
		if (local_obj_var_cnt > entering_cnt)
		{
			ExprVector param_v;
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, ltoa(__FLOW_CALL_SIGNAL_DELETE_OBJECT + entering_cnt)));
			param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_FUNC_CALL, 
				m_pFunc->getFuncDeclare(), param_v)));
		}
		ret_v.push_back(pStatement);
		break;
	}

	case STATEMENT_TYPE_EXPR2:
	case STATEMENT_TYPE_IF:
	case STATEMENT_TYPE_WHILE:
	case STATEMENT_TYPE_DO:
	case STATEMENT_TYPE_FOR:
	case STATEMENT_TYPE_SWITCH:
	{
		ret_v.push_back(pStatement);
		break;
	}
	case STATEMENT_TYPE_TRY:
	case STATEMENT_TYPE_FLOW_WAIT:
	case STATEMENT_TYPE_FLOW_TRY:
	case STATEMENT_TYPE_FLOW_FORK:
	case STATEMENT_TYPE_FLOW_NEW:
		MY_ASSERT(false);
	}
}

CExpr* CMyFunc::getMyCallFuncName()
{
	if (m_pFunc->getFuncDeclare()->getParent()->getRealScope()->getGoType() == GO_TYPE_CLASS)
	{
		//TokenWithNamespace twn;
		//twn.addScope(((CClassDef*)m_pFunc->getParent()->getRealScope())->getName());
		//twn.addScope();
		return new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, FLOW_STATIC_METHOD_PREFIX + m_pFunc->getName());
	}
	return new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, m_pFunc->getName());
}

CFunction* createStaticFunction(CClassDef* pClassDef, const std::string& funcName)
{
	std::string new_name = FLOW_STATIC_METHOD_PREFIX + funcName;

	if (pClassDef->findSymbol(new_name, FIND_SYMBOL_SCOPE_LOCAL) != NULL)
		return NULL;

	TypeDefPointer pFuncType = TypeDefPointer(new CTypeDef(NULL, "", SEMANTIC_TYPE_FUNC, g_type_def_bool, 0));

	CFunction* pStaticFunction = new CFunction(pClassDef, new_name, 
		pFuncType, FLOW_TYPE_NONE, new CFuncDeclare(pClassDef, new_name, pFuncType));
	StringVector mod_strings;
	addToModifiers(mod_strings, MODBIT_STATIC);
	pFuncType->setModStrings(mod_strings);
	CVarDef* pVarDef = new CVarDef(NULL, PARAM_VAR_NAME_SIGNAL, g_type_def_unsigned, NULL);
	pFuncType->addFuncParam(pVarDef);
	pStaticFunction->addVarDef(pVarDef);
	pVarDef = new CVarDef(NULL, PARAM_VAR_NAME_DATA, g_type_def_void_ptr, NULL);
	pFuncType->addFuncParam(pVarDef);
	pStaticFunction->addVarDef(pVarDef);

	pClassDef->addFuncDeclare(pStaticFunction->getFuncDeclare());

	// return ((struct* )param)->pThis->func(signal, param);
	CExpr* pFuncExpr = new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
		new CExpr(NULL, EXPR_TYPE_PARENTHESIS, new CExpr(NULL, EXPR_TYPE_TYPE_CAST, 
			TypeDefPointer(new CTypeDef(NULL, "", pClassDef->findSymbol(FUNC_DEF_STRUCT_PREFIX + funcName, FIND_SYMBOL_SCOPE_LOCAL)->getTypeDef(), 1)), 
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_DATA))), FLOW_PARAM_NAME_CALLER_THIS), funcName);
	pFuncExpr->setFuncDeclare(pStaticFunction->getFuncDeclare());

	ExprVector param_v;
	param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL));
	param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_DATA));
	pStaticFunction->addChild(new CStatement(pStaticFunction, STATEMENT_TYPE_RETURN, 
		new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pFuncExpr, param_v)));

	return pStaticFunction;
}

ScopeVector CMyFunc::analyze()
{
	// step 1, move func params into struct
	std::vector<CScope*> ret_v, ret_v2;
	ExprVector param_v;

	if (m_pFunc->getChildrenCount() == 0 ||
		((CStatement*)m_pFunc->getChildAt(m_pFunc->getChildrenCount() - 1))->getStatementType() != STATEMENT_TYPE_RETURN/* && m_pFunc->getFuncType()->getFuncReturnType()->isVoid()*/)
	{
		m_pFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_RETURN));
	}

	// put all param names into local_var_names;
	LocalVarSet local_var_names;

	CScope* pParentScope = m_pFunc->getFuncDeclare()->getParent();
	std::string def_struct_name = FUNC_DEF_STRUCT_PREFIX + m_pFunc->getName();
	SymbolDefObject* pDefObj = pParentScope->findSymbol(def_struct_name, FIND_SYMBOL_SCOPE_PARENT);
	CClassDef* pDefStruct = NULL, *pImplStruct = NULL;

	if (pDefObj)
	{
		pDefStruct = pDefObj->getTypeDef()->getBaseType()->getClassDef();
	}
	else
	{
		CStatement* pDefStatement = addFuncDefStruct(pParentScope, m_pFunc->getFuncDeclare());
		m_result_v.push_back(pDefStatement);
		pDefStatement->setDefLocation(m_pFunc->getDefFileStack(), m_pFunc->getDefLineNo());
		pDefStruct = pDefStatement->getTypeDef()->getBaseType()->getClassDef();
	}
	MY_ASSERT(pDefStruct);

	std::string impl_struct_name = FUNC_IMPL_STRUCT_PREFIX + m_pFunc->getName();
	pImplStruct = new CClassDef(m_pFunc->getParent()->getRealScope(), SEMANTIC_TYPE_STRUCT, impl_struct_name);
	pImplStruct->addBaseClass(pDefStruct);
	m_pStructTypeDef = TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), impl_struct_name, pImplStruct));
	m_pFunc->getParent()->getRealScope()->addTypeDef(TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), impl_struct_name, m_pStructTypeDef, 0)));
	CStatement* pImplStatement = new CStatement(m_pFunc->getParent()->getRealScope(), STATEMENT_TYPE_DEF, m_pStructTypeDef);
	pImplStatement->setDefLocation(m_pFunc->getDefFileStack(), m_pFunc->getDefLineNo());
	m_result_v.push_back(pImplStatement);

	// add func params
	int n = 0;
	for (int i = 0; i < m_pFunc->getFuncType()->getFuncParamCount(); i++)
	{
		CVarDef* pVarDef = m_pFunc->getFuncType()->getFuncParamAt(i);

		NewVarInfo nvi;
		nvi.bParam = true;
		nvi.bRef = pVarDef->isReference();
		nvi.new_name = ((CStatement*)pDefStruct->getChildAt(1 + i))->getVarAt(0)->getName();
		local_var_names[pVarDef->getName()] = nvi;

		m_pFunc->removeVarDef(pVarDef); // because we don't re-define those params as references in root_flow func.

		/*TypeDefPointer pTypeDef = pVarDef->getType();
			
		CVarDef* pVarDef2 = new CVarDef(m_pFunc, pVarDef->getName(), pTypeDef, NULL);
		if (pVarDef->isReference())
			pVarDef2->setReference();

		m_pFunc->insertChildAt(n, new CStatement(m_pFunc, STATEMENT_TYPE_DEF, pVarDef2, NULL));

		n++;*/
	}

	// change func type to standard flow func type
	if (!m_pFunc->isFlowRoot())
	{
		m_pFunc->getParent()->getRealScope()->addFuncDeclare(
			new CFuncDeclare(m_pFunc, m_pFunc->getName(), findTypeDef(FLOW_FUNC_TYPE_NAME)));

		// set func type to flow func
		//m_pFunc->m_funcParams.clear();
		m_pFunc->setFuncType(TypeDefPointer(new CTypeDef(m_pFunc, FLOW_FUNC_TYPE_NAME, SEMANTIC_TYPE_FUNC, g_type_def_bool, 0)));
		//m_pFunc->getFuncType()->setFuncReturnType(g_type_def_bool);
		//m_pFunc->getFuncType()->clearAllFuncParams();
		CVarDef* pVarDef = new CVarDef(NULL, PARAM_VAR_NAME_SIGNAL, g_type_def_unsigned, NULL);
		m_pFunc->getFuncType()->addFuncParam(pVarDef);
		m_pFunc->addVarDef(pVarDef);
		pVarDef = new CVarDef(NULL, PARAM_VAR_NAME_DATA, g_type_def_void_ptr, NULL);
		m_pFunc->getFuncType()->addFuncParam(pVarDef);
		m_pFunc->addVarDef(pVarDef);
		m_pFunc->setFuncDeclare(new CFuncDeclare(m_pFunc->getFuncDeclare()->getParent(), m_pFunc->getName(), m_pFunc->getFuncType()));

		// declare local var in the very begining of the func
		TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(NULL, "", new CTypeDef(m_pStructTypeDef->getParent(), m_pStructTypeDef->getName(), m_pStructTypeDef, 0), 1));
		pVarDef = new CVarDef(m_pFunc, LOCALVAR_NAME, pTypeDef, NULL,
			new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pTypeDef, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_DATA)));
		CStatement* pStatement = new CStatement(m_pFunc, STATEMENT_TYPE_DEF, pVarDef, NULL);
		m_pFunc->insertChildAt(0, pStatement);
	}

	m_pFunc->getFuncType()->setFuncFlowType(FLOW_TYPE_NONE);

	std::string c_func_name = m_pFunc->getName();
	// if it's a method in a class, define its static func
	if (m_pFunc->getFuncDeclare()->getParent()->getGoType() == GO_TYPE_CLASS)
	{
		CFunction* pStaticFunction = createStaticFunction((CClassDef*)m_pFunc->getFuncDeclare()->getParent(), m_pFunc->getName());
		if (pStaticFunction)
			m_result_v.push_back(pStaticFunction);

		c_func_name = FLOW_STATIC_METHOD_PREFIX + m_pFunc->getName();
	}

	// step 2, check dup names. when found, rename them.
	for (int i = 0; i < m_pFunc->getChildrenCount(); i++)
	{
		CStatement* pStatement = (CStatement*)m_pFunc->getChildAt(i);
		checkDupNames(pStatement);
	}

	// step 2, decompose
	ret_v = m_pFunc->m_children;
	m_pFunc->m_children.clear();
	if (!m_pFunc->isFlowRoot())
	{
		// skip the first statement which is local_var definition
		for (int i = 0; i < 1; i++)
		{
			m_pFunc->addChild(ret_v.front());
			ret_v.erase(ret_v.begin());
		}
	}

	// __flow_func_expand_stack(__flow_func_var->__flow_basic.flow_block, __flow_func_var, sizeof(__FLOW_FUNC_IMPL_func));
	param_v.clear();
	param_v.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
		new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME), FLOW_BASIC_FLOW_MEMBER_NAME));
	param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME));
	param_v.push_back(new CExpr(m_pFunc, EXPR_TYPE_SIZEOF, findTypeDef(FUNC_IMPL_STRUCT_PREFIX + m_pFunc->getName())));
	addToStatementVector(m_pFunc->m_children, new CStatement(m_pFunc, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_FUNC_CALL, 
		findFuncDecl(FLOW_FUNC_EXPAND_STACK), param_v)), 0);

	if (!m_pFunc->isFlowRoot())
	{
		// pLocalVar->basic.this_func = func_name;
		addToStatementVector(m_pFunc->m_children, new CStatement(m_pFunc, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN, 
			new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, 
				LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME), PARAM_NAME_THIS_FUNC), 
			new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, c_func_name)
		)), 0);
	}

	unsigned local_obj_var_cnt = 0;
	for (size_t i = 0; i < ret_v.size(); i++)
	{
		CScope* pGrammarObj = ret_v[i];

		MY_ASSERT(pGrammarObj->getGoType() == GO_TYPE_STATEMENT);
		CStatement* pStatement = (CStatement*)pGrammarObj;
		checkStatementDecompose(m_pFunc->m_children, pStatement, -1, m_signal_index, local_var_names, local_obj_var_cnt);
	}

	std::string flowName;
	if (m_pFunc->isFlowRoot())
	{
		// pFlow = __flow_start(NULL);
		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "0"));
		flowName = getTempVarName();
		m_pFunc->insertChildAt(0, new CStatement(m_pFunc, STATEMENT_TYPE_DEF, new CVarDef(m_pFunc, flowName, g_type_def_void_ptr, NULL, 
			new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_START_FUNC_NAME), param_v))));

		// Struct* pFuncData = (Struct*)__flow_func_enter(pFlow, NULL, NULL, 0, sizeof(Struct));
		std::string tempDefVarName = getTempVarName();
		TypeDefPointer pFuncStructDefType = findTypeDef(FUNC_DEF_STRUCT_PREFIX + m_pFunc->getName());
		MY_ASSERT(pFuncStructDefType);
		TypeDefPointer pFuncStructDefPointerType = TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), pFuncStructDefType->getName(), pFuncStructDefType, 1));

		param_v.clear();
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, flowName));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		param_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		param_v.push_back(new CExpr(m_pFunc, EXPR_TYPE_SIZEOF, findTypeDef(FUNC_DEF_STRUCT_PREFIX + m_pFunc->getName())));

		m_pFunc->insertChildAt(1, new CStatement(m_pFunc, STATEMENT_TYPE_DEF, new CVarDef(m_pFunc, tempDefVarName, pFuncStructDefPointerType, NULL,
			new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pFuncStructDefPointerType,
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, findFuncDecl(FLOW_FUNC_ENTER_FUNC_NAME), param_v)
			)
		)));

		// setting parameters into struct, pFuncData->.... = ...;
		for (int i = 0; i < m_pFunc->getFuncType()->getFuncParamCount(); i++)
		{
			CVarDef* pVarDef = m_pFunc->getFuncType()->getFuncParamAt(i);
			m_pFunc->insertChildAt(2 + i, new CStatement(NULL, STATEMENT_TYPE_EXPR2, new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, tempDefVarName), pVarDef->getName()),
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, pVarDef->getName())
			)));
		}

		// __FLOW_FUNC_IMPL_func* __flow_func_var = (__FLOW_FUNC_IMPL_func*)tempVarName;
		TypeDefPointer pFuncStructImplType = findTypeDef(FUNC_IMPL_STRUCT_PREFIX + m_pFunc->getName());
		MY_ASSERT(pFuncStructImplType);
		TypeDefPointer pFuncStructImplPointerType = TypeDefPointer(new CTypeDef(m_pFunc->getParent()->getRealScope(), pFuncStructImplType->getName(), pFuncStructImplType, 1));
		m_pFunc->insertChildAt(2 + m_pFunc->getFuncType()->getFuncParamCount(), new CStatement(m_pFunc, STATEMENT_TYPE_DEF, new CVarDef(m_pFunc, LOCALVAR_NAME, pFuncStructImplPointerType, NULL,
			new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pFuncStructImplPointerType, 
				new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, tempDefVarName)
			)
		)));
	}
	/*else
	{
		// remove the last if
		CStatement* pStatement = (CStatement*)m_pFunc->getChildAt(m_pFunc->getChildrenCount() - 1);
		MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_IF);
		pStatement = (CStatement*)pStatement->getChildAt(1);
		m_pFunc->removeChildAt(m_pFunc->getChildrenCount() - 1);
		if (pStatement->getStatementType() == STATEMENT_TYPE_COMPOUND)
		{
			for (int i = 0; i < pStatement->getChildrenCount(); i++)
				m_pFunc->addChild(pStatement->getChildAt(i));
		}
		else
			m_pFunc->addChild(pStatement);
	}*/

	if (!m_pFunc->isFlowRoot())
	{
		if (m_local_obj_var_index > 0)
		{
			// add delete_table to struct
			SourceTreeNode* pExprNode = declVarCreateByName(FLOW_DELETE_TABLE);
			declVarAddArrayExpr(pExprNode, exprCreateConst(EXPR_TYPE_CONST_VALUE, ltoa(m_local_obj_var_index)));

			CClassDef* pStruct = m_pStructTypeDef->getClassDef();
			CVarDef* pVarDef2 = new CVarDef(NULL, FLOW_DELETE_TABLE, g_type_def_unsigned, pExprNode);
			pStruct->addDef(new CStatement(pStruct, STATEMENT_TYPE_DEF, pVarDef2, NULL));

			// switch (__flow_func_var->__flow_delete_table[--__flow_func_var->__flow_basic.delete_counter])
			CStatement* pSwitchStatement = new CStatement(NULL, STATEMENT_TYPE_SWITCH, new CExpr(NULL, EXPR_TYPE_ARRAY, 
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_DELETE_TABLE), 
				new CExpr(NULL, EXPR_TYPE_LEFT_DEC, new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME), FLOW_DELETE_COUNTER)))); 
			for (int i = 0; i < pStruct->getChildrenCount(); i++)
			{
				CScope* pScope = pStruct->getChildAt(i);
				MY_ASSERT(pScope->getGoType() == GO_TYPE_STATEMENT);

				CStatement* pStatement = (CStatement*)pScope;
				MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_DEF);

				for (int j = 0; j < pStatement->getVarCount(); j++)
				{
					CVarDef* pVarDef = pStatement->getVarAt(j);
					if (pVarDef->getSeqNo() < 0)
						continue;

					// __flow_func_var->xxx.TTT::~TTT();
					int depth = 0;
					TypeDefPointer pTypeDef = getRootType(pVarDef->getType(), depth);
					MY_ASSERT(depth == 0);
					CClassDef* pClassDef = pTypeDef->getClassDef();
					TokenWithNamespace twn = getRelativeTWN(pClassDef);
					twn.addScope("~" + pClassDef->getName());
					ExprVector e_v;
					CFuncDeclare* pFuncDeclare = pClassDef->findFuncDeclare("~" + pClassDef->getName(), e_v);
					MY_ASSERT(pFuncDeclare);
					CExpr* pFuncExpr = new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
						new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), pVarDef->getName()), twn);
					pFuncExpr->setFuncDeclare(pFuncDeclare);

					ScopeVector s_v;
					s_v.push_back(new CStatement(pSwitchStatement, STATEMENT_TYPE_EXPR2, 
						new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pFuncExpr, e_v)));
					s_v.push_back(new CStatement(pSwitchStatement, STATEMENT_TYPE_BREAK));
					pSwitchStatement->addSwitchCase(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(pVarDef->getSeqNo())), s_v);
				}
			}
			ScopeVector delete_v;
			delete_v.push_back(pSwitchStatement);

			// while (0x80000 + __flow_func_var->__flow_basic.delete_counter > signal) ...
			m_pFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_WHILE, 
				new CExpr(NULL, EXPR_TYPE_GREATER_THAN, 
					new CExpr(NULL, EXPR_TYPE_ADD, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, __FLOW_CALL_SIGNAL_DELETE_OBJECT_STR), 
						new CExpr(NULL, EXPR_TYPE_REF_ELEMENT, new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, 
							new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME), FLOW_DELETE_COUNTER)), 
					new CExpr(NULL, EXPR_TYPE_TOKEN_WITH_NAMESPACE, PARAM_VAR_NAME_SIGNAL)), 
				new CStatement(NULL, STATEMENT_TYPE_COMPOUND, delete_v)));
		}

		// return false;
		m_pFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false")));
	}

	//std::string s = m_pFunc->toString(1);
	m_result_v.push_back(m_pFunc);

	return m_result_v;
}

ScopeVector parseGrammarObject(CScope* pScope)
{
	std::vector<CScope*> ret_v;

	GrammarObjectType goType = pScope->getGoType();
	if (goType == GO_TYPE_FUNC)
	{
		CFunction* pFunc = (CFunction*)pScope;

		//printf("onFuncRead, flowType=%d\n", pFunc->getFlowType());
		if (pFunc->getFlowType() != FLOW_TYPE_NONE)
		{
			pFunc->setTransformed();
			CMyFunc* pMyFunc = new CMyFunc(pFunc);
			return pMyFunc->analyze();
		}
	}
	else if (goType == GO_TYPE_NAMESPACE)
	{
		for (int i = 0; i < pScope->getChildrenCount();)
		{
			CScope* pChild = pScope->getChildAt(i);
			// sometimes a scope's child's parent is not itself, etc. a namespace is created as a parent namespace with realscope and 
			// a child namespace with no realscope. the scope adds the child namespace as its child but the child namespace's parent is
			// its parent namespace
			CScope* pParent = pChild->getParent(); 													
			std::vector<CScope*> ret_v2 = parseGrammarObject(pChild);
			pScope->removeChildAt(i);
			for (int j = 0; j < ret_v2.size(); j++, i++)
			{
				pChild = ret_v2[j];
				pScope->insertChildAt(i, pChild);
				pChild->setParent(pParent);
			}
		}
	}
	else if (goType == GO_TYPE_STATEMENT && ((CStatement*)pScope)->getStatementType() == STATEMENT_TYPE_DEF)
	{
		CStatement* pStatement = (CStatement*)pScope;
		switch (pStatement->getDefType())
		{
		case DEF_TYPE_FUNC_DECL:
		{
			//define structure type
			CFuncDeclare* pFuncDeclare = pStatement->getFuncDeclare();
			if (!isInModifiers(pFuncDeclare->getType()->getModStrings(), MODBIT_FLOW))
				break;

			CScope* pParentScope = pFuncDeclare->getParent();
			CStatement* pStructStatement = addFuncDefStruct(pFuncDeclare->getParent(), pFuncDeclare);
			if (pStructStatement)
			{
				pStructStatement->setDefLocation(pStatement->getDefFileStack(), pStatement->getDefLineNo());
				ret_v.push_back(pStructStatement);
			}
			if (pFuncDeclare->getParent()->getGoType() == GO_TYPE_CLASS)
			{
				CFunction* pStaticFunction = createStaticFunction((CClassDef*)pFuncDeclare->getParent(), pFuncDeclare->getName());
				ret_v.push_back(pStaticFunction);
			}

			pStatement->setTypeDef(TypeDefPointer(new CTypeDef(pParentScope, FLOW_FUNC_TYPE_NAME, SEMANTIC_TYPE_FUNC, g_type_def_bool, 0)));
			CVarDef* pVarDef = new CVarDef(NULL, PARAM_VAR_NAME_SIGNAL, g_type_def_unsigned, NULL);
			pStatement->getTypeDef()->addFuncParam(pVarDef);
			pVarDef = new CVarDef(NULL, PARAM_VAR_NAME_DATA, g_type_def_void_ptr, NULL);
			pStatement->getTypeDef()->addFuncParam(pVarDef);
			pStatement->setFuncDeclare(new CFuncDeclare(pParentScope, pFuncDeclare->getName(), pStatement->getTypeDef()));

			pStatement->setTransformed();
			pStatement->getTypeDef()->setFuncFlowType(FLOW_TYPE_NONE);
			break;
		}
		case DEF_TYPE_SUPER_TYPE_VAR_DEF:
		{
			TypeDefPointer pTypeDef = ((CStatement*)pScope)->getTypeDef();
			CClassDef* pClassDef = pTypeDef->getClassDef();
			for (int i = 0; i < pClassDef->getChildrenCount();)
			{
				CScope* pChild = pClassDef->getChildAt(i);
				// sometimes a scope's child's parent is not itself, etc. a namespace is created as a parent namespace with realscope and 
				// a child namespace with no realscope. the scope adds the child namespace as its child but the child namespace's parent is
				// its parent namespace
				CScope* pParent = pChild->getParent(); 													
				std::vector<CScope*> ret_v2 = parseGrammarObject(pChild);
				pClassDef->removeChildAt(i);
				for (int j = 0; j < ret_v2.size(); j++, i++)
				{
					pChild = ret_v2[j];
					pClassDef->insertChildAt(i, pChild);
					pChild->setParent(pParent);
				}
			}
			break;
		}
		}
	}

	ret_v.push_back(pScope);
	return ret_v;
}

void AnalyzeFile(char* file_name, char* dest_path, int argc, char* argv[])
{
	try {
		CNamespace* pNamespace = semanticAnalyzeFile(file_name, argc, argv);
		enterScope(pNamespace);
		parseGrammarObject(pNamespace);
		std::string ret_s = pNamespace->toString(0);
		leaveScope();

		FILE* wfp = fopen(dest_path, "wt");
		if (!wfp)
			printf("cannot open dest file %s\n", dest_path);
		else
		{
			fprintf(wfp, "%s\n", ret_s.c_str());
			fclose(wfp);
		}
	}
	catch (std::string& s)
	{
		printf("analyzeFile failed, err=%s\n", s.c_str());
	}
}

int main(int argc, char* argv[])
{
#if 0
#ifdef _WIN32
	StringVector include_paths = get_sys_include_path();
	if (include_paths.empty())
	{
		printf("\n\nCannot find INCLUDE environment variable\n");
		return 1;
	}
#endif
#endif
	if (argc != 3)
	{
		printf("\n\nUsage: %s <src_file> <dest_file>\n\n", argv[0]);
		return 1;
	}

	semanticInit();
	AnalyzeFile(argv[1], argv[2], 0, NULL);

	/*DIR *d;
	struct dirent *dir;
	d = opendir(".");

	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, "test.cpp") != 0)
			continue;
		char* postfix = strrchr(dir->d_name, '.');
		if (postfix == NULL)
			continue;
		if (strcmp(postfix, ".cpp") == 0 || strcmp(postfix, "*.c") == 0)
			AnalyzeFile(dir->d_name, argc, argv);
	}
	closedir(d);*/

	return 0;
}

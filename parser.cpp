#include "parser.h"
#include <dirent.h>

std::vector<CGrammarObject* > g_solvedBlocks;

void CMyFunc::addToStatementVector(std::vector<CGrammarObject*>& ret_v, CStatement* pNewStatement, int nSignalNo)
{
	CStatement* pStatement;

	if (m_pFunc->isFlowRoot())
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
				((CExpr*)pExpr->getChildAt(0))->getExprType() == EXPR_TYPE_TOKEN && ((CExpr*)pExpr->getChildAt(0))->getValue() == PARAM_VAR_NAME_SIGNAL &&
				((CExpr*)pExpr->getChildAt(1))->getExprType() == EXPR_TYPE_CONST_VALUE && ((CExpr*)pExpr->getChildAt(1))->getValue() == ltoa(nSignalNo))
			{
				pStatement = (CStatement*)pStatement->getChildAt(1);
				MY_ASSERT(pStatement->getStatementType() == STATEMENT_TYPE_COMPOUND);
				pStatement->addChild(pNewStatement);
				return;
			}
		}
	}

	std::vector<CGrammarObject*> v;
	v.push_back(pNewStatement);
	ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
		new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalNo))),
		new CStatement(NULL, STATEMENT_TYPE_COMPOUND, v)));
}

void CMyFunc::addBatchToStatementVector(GrammarObjectVector& ret_v, GrammarObjectVector& new_v, int nSignalNo)
{
	BOOST_FOREACH(CGrammarObject* pObj, new_v)
	{
		MY_ASSERT(pObj->getNodeType() == SCOPE_TYPE_STATEMENT);
		addToStatementVector(ret_v, (CStatement*)pObj, nSignalNo);
	}
}

CExpr* composeSignalLogicExpr(CExpr* pExpr, int nSignalEnter, int nSignalStart, int nSignalEnd)
{
	CExpr* pExpr2 = new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter)));
	if (pExpr && !(pExpr->getExprType() == EXPR_TYPE_CONST_VALUE && pExpr->getValue() == "true"))
		pExpr2 = new CExpr(NULL, EXPR_TYPE_AND, pExpr2, new CExpr(NULL, EXPR_TYPE_PARENTHESIS, pExpr));
	if (nSignalStart > nSignalEnd)
		return pExpr2;
	if (nSignalStart == nSignalEnd)
		pExpr = new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalStart)));
	else
		pExpr = new CExpr(NULL, EXPR_TYPE_AND,
			new CExpr(NULL, EXPR_TYPE_GREATER_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalStart))),
			new CExpr(NULL, EXPR_TYPE_LESS_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnd))));
	return new CExpr(NULL, EXPR_TYPE_OR, pExpr2, pExpr);
}

bool isVarDefinedBetweenScopeAndFunction(const std::string& varName, CGrammarObject* pScope, CGrammarObject* pCurrent, bool bOutsideScope)
{
	if (pCurrent->findVarDef(varName, false))
		return bOutsideScope;

	if (pCurrent->getNodeType() == SCOPE_TYPE_FUNC)
		return false;

	if (pCurrent == pScope)
		bOutsideScope = true;

	CGrammarObject* pParent = pCurrent->getParent();
	MY_ASSERT(pParent);
	MY_ASSERT(pParent != pCurrent);

	return isVarDefinedBetweenScopeAndFunction(varName, pScope, pParent, bOutsideScope);
}

void changeVarToUnderStructInExpr(CGrammarObject* pScope, const std::string& struct_name, CExpr* pExpr)
{
	if (pExpr->getExprType() == EXPR_TYPE_TOKEN)
	{
		if (isVarDefinedBetweenScopeAndFunction(pExpr->getValue(), pScope, pExpr, false))
		{
			pExpr->setExprType(EXPR_TYPE_PTR_ELEMENT);
			pExpr->addChild(new CExpr(pExpr, EXPR_TYPE_TOKEN, struct_name));
		}
		return;
	}

	for (int i = 0; i < pExpr->getChildrenCount(); i++)
	{
		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(i);
		MY_ASSERT(pExpr2->getNodeType() == SCOPE_TYPE_EXPR);
		changeVarToUnderStructInExpr(pScope, struct_name, pExpr2);
	}
}

// check every var to see whether it is defined outside the pScope but under the function. If true, change it to pStruct->var_name
void changeVarToUnderStructInStatement(CGrammarObject* pScope, const std::string& struct_name, CStatement* pStatement)
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
			CGrammarObject* pObj = pStatement->getChildAt(i);
			if (pObj->getNodeType() == SCOPE_TYPE_STATEMENT)
				changeVarToUnderStructInStatement(pScope, struct_name, (CStatement*)pObj);
			else
				changeVarToUnderStructInExpr(pScope, struct_name, (CExpr*)pObj);
		}
	}
	}
}

CStatement* CMyFunc::checkFuncGetParamStructDef()
{
	std::vector<CGrammarObject*> ret_v;

	m_pStructTypeDef = TypeDefPointer(new CStruct(m_pFunc->getParent()->getRealScope(), CALL_PARAM_STRUCT_PREFIX + m_pFunc->getName(), BASICTYPE_TYPE_STRUCT));
	m_pFunc->getParent()->getRealScope()->addTypeDef(m_pStructTypeDef);
	CStatement* pRetStatement = new CStatement(m_pFunc->getParent()->getRealScope(), STATEMENT_TYPE_DEF, m_pStructTypeDef);
	CStruct* pStruct = (CStruct*)m_pStructTypeDef.get();

	if (!m_pFunc->isFlowRoot())
		m_pFunc->getParent()->getRealScope()->addFuncDeclare(new CFuncDeclare(m_pFunc->getName(), m_pFunc->findTypeDef(FLOW_FUNC_TYPE_NAME)));

	TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(FLOW_BASIC_TYPE_NAME, m_pFunc->findTypeDef(FLOW_BASIC_TYPE_NAME), NULL));
	pStruct->addDef(new CStatement(pStruct, STATEMENT_TYPE_DEF, new CVarDef(NULL, FLOW_BASIC_MEMBER_NAME, pTypeDef, NULL)));

	// add func params
	int n = 0;
	for (int i = 0; i < m_pFunc->getParamVarCount(); i++)
	{
		CVarDef* pVarDef = m_pFunc->getParamVarAt(i);
		pStruct->addDef(new CStatement(pStruct, STATEMENT_TYPE_DEF, pVarDef));

		if (!m_pFunc->isFlowRoot())
		{
			TypeDefPointer pTypeDef = pVarDef->getType();
			if (pTypeDef->isBaseType())
			{
				SourceTreeNode* pDeclVar = declVarCreateByName("");
				declVarAddModifier(pDeclVar, DVMOD_TYPE_REFERENCE);
				pTypeDef = TypeDefPointer(new CTypeDef("", pTypeDef, pDeclVar));
			}
			else
			{
				SourceTreeNode* pDeclVar = dupSourceTreeNode(pTypeDef->getDeclVarNode());
				declVarAddModifier(pDeclVar, DVMOD_TYPE_REFERENCE);
				pTypeDef = TypeDefPointer(new CTypeDef("", pTypeDef->getBaseType(), pDeclVar));
			}

			CVarDef* pVarDef2 = new CVarDef(m_pFunc, pVarDef->getName(), pTypeDef, NULL,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarDef->getName()));
			pVarDef2->setReference();
			m_pFunc->insertChildAt(n, new CStatement(m_pFunc, STATEMENT_TYPE_DEF, pVarDef2, NULL));
			n++;
		}
	}

	if (!m_pFunc->isFlowRoot())
	{
		TypeDefPointer return_type = m_pFunc->getFuncType()->getFuncReturnType();
		if (!return_type->isVoid())
		{
			pStruct->addDef(new CStatement(pStruct, STATEMENT_TYPE_DEF,
				new CVarDef(NULL, PARAM_NAME_CALLER_RET, return_type, NULL)));
		}

		m_pFunc->m_funcParams.clear();
		m_pFunc->setFuncType(m_pFunc->findTypeDef(FLOW_FUNC_TYPE_NAME));
		m_pFunc->addParamVar(new CVarDef(NULL, PARAM_VAR_NAME_SIGNAL, g_type_def_int, NULL));
		m_pFunc->addParamVar(new CVarDef(NULL, PARAM_VAR_NAME_DATA, g_type_def_void_ptr, NULL));

		SourceTreeNode* pDeclVar = declVarCreateByName(LOCALVAR_NAME);
		declVarAddModifier(pDeclVar, DVMOD_TYPE_POINTER);
		pTypeDef = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getName(), m_pStructTypeDef, NULL));
		pTypeDef = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getName(), pTypeDef, pDeclVar));
		CVarDef* pVarDef = new CVarDef(m_pFunc, LOCALVAR_NAME, pTypeDef, NULL,
			new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pTypeDef, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_DATA)));
		CStatement* pStatement = new CStatement(m_pFunc, STATEMENT_TYPE_DEF, pVarDef, NULL);
		m_pFunc->insertChildAt(0, pStatement);
	}
	//printf("add statement=%s\n", pStatement->toString(0).c_str());
	return pRetStatement;
}

void CMyFunc::replaceExprWithStatement(std::vector<CGrammarObject*>& ret_v, CExpr*& pExpr, int signalNo)
{
	std::vector<CGrammarObject*> ret_v2;

	TypeDefPointer return_type = pExpr->getReturnType();
	std::string varName = getTempVarName();
	SourceTreeNode* pDeclVarNode = declVarCreateByName(varName);
	for (int i = 0; i < pExpr->getReturnDepth(); i++)
		declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
	CVarDef* pVarDef = new CVarDef(NULL, varName, return_type, pDeclVarNode);

	//CVarDef* pVarDef2 = new CVarDef(pVarDef->getParent(), pVarDef->getName(), pVarDef->getType(), dupSourceTreeNode(pVarDef->getDeclVarNode()));
	CStatement* pStatement2 = new CStatement((CStruct*)getStructType().get(), STATEMENT_TYPE_DEF, pVarDef, NULL);
	((CStruct*)getStructType().get())->addDef(pStatement2);

	CGrammarObject* pScope = pExpr->getRealScope();
	//ret_v2.push_back(new CStatement(pScope, STATEMENT_TYPE_DEF, pVarDef, (return_type->getBaseType() ? return_type->getBaseType()->getBasicNode() : return_type->getBasicNode())));
	ret_v2.push_back(new CStatement(pScope, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
		new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarDef->getName()),
		pExpr)));
	pExpr = new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarDef->getName());

	addBatchToStatementVector(ret_v, ret_v2, signalNo);
}

void CMyFunc::checkExprDecompose(std::vector<CGrammarObject*>& ret_v, CExpr*& pExpr, int& inAndOutSignalNo)
{
	if (!pExpr->isFlow())
		return;

	switch (pExpr->getExprType())
	{
	case EXPR_TYPE_CONST_VALUE:			// const_value
	case EXPR_TYPE_TOKEN:				// token
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
	case EXPR_TYPE_XOR:				// expr
	case EXPR_TYPE_TYPE_CAST:		// type expr
	case EXPR_TYPE_INDIRECTION:		// expr
	case EXPR_TYPE_ADDRESS_OF:		// expr
	case EXPR_TYPE_SIZE_OF:			// (extended_type_var | expr)
	case EXPR_TYPE_NEW:				// extended_type_var
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
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo);
		pExpr->setChildAt(0, pExpr1);
		MY_ASSERT(!pExpr1->isFlow());

		if (pExpr->getChildrenCount() > 1)
		{
			CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(1);
			// if pExpr2->isFlow, then we need to keep the value of pExpr1 before calculating pExpr2 except pExpr1 is a simple unchanged value
			if (pExpr2->isFlow() && !(pExpr1->getExprType() == EXPR_TYPE_CONST_VALUE || pExpr1->getExprType() == EXPR_TYPE_TOKEN && isTempVarName(pExpr1->getValue())))
			{
//printf("replaceExprWithStatement, expr=%s\n", pExpr1->toString().c_str());
				replaceExprWithStatement(ret_v, pExpr1, inAndOutSignalNo);
				pExpr->setChildAt(0, pExpr1);
			}

			checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo);
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
	  checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo);
	  pExpr->setChildAt(0, pExpr1);
	  MY_ASSERT(!pExpr1->isFlow());

	  if (pExpr->getChildrenCount() > 1)
	  {
		  CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(1);
		  // if pExpr2->isFlow, then we need to keep the value of pExpr1 before calculating pExpr2 except pExpr1 is a simple unchanged value
		  checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo);
		  pExpr->setChildAt(1, pExpr2);
		  MY_ASSERT(!pExpr2->isFlow());
	  }
	  pExpr->setFlow(false);
	  break;
	}
	case EXPR_TYPE_FUNC_CALL:		// expr *[expr: ':']
	{
		std::vector<CGrammarObject*> ret_v2;
		bool bFuncFlow = false;
		CFuncDeclare* pFuncDeclare = NULL;
		int nStart = 0, i;
		CExpr* pExpr2 = NULL;
		if (!pExpr->getValue().empty())
		{
			bFuncFlow = pExpr->getFuncDeclare()->getType()->isFlow();
			pFuncDeclare = pExpr->getFuncDeclare();
		}
		else
		{
			pExpr2 = (CExpr*)pExpr->getChildAt(0);
			MY_ASSERT(pExpr2->getReturnType()->getBasicType() == BASICTYPE_TYPE_FUNC);
			bFuncFlow = pExpr2->getReturnType()->isFlow();
			pFuncDeclare = pExpr2->getFuncDeclare();
			nStart = 1;
			checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo);
			MY_ASSERT(!pExpr2->isFlow());
			pExpr->setChildAt(0, pExpr2);
		}
		int nLastFlowExpr = -1;
		for (i = nStart; i < pExpr->getChildrenCount(); i++)
			if (((CExpr*)pExpr->getChildAt(i))->isFlow())
			{
				nLastFlowExpr = i;
				break;
			}
		if (nLastFlowExpr >= 0 && nStart > 0 && !(pExpr2->getExprType() == EXPR_TYPE_CONST_VALUE || pExpr2->getExprType() == EXPR_TYPE_TOKEN && isTempVarName(pExpr2->getValue())))
		{
			replaceExprWithStatement(ret_v, pExpr2, inAndOutSignalNo);
			pExpr->setChildAt(0, pExpr2);
		}
		for (i = pExpr->getChildrenCount() - 1; i >= nStart; i--)
		{
			pExpr2 = (CExpr*)pExpr->getChildAt(i);
			checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo);
			MY_ASSERT(!pExpr2->isFlow());
			pExpr->setChildAt(i, pExpr2);
			if (nLastFlowExpr >= 0 && i > nLastFlowExpr && !(pExpr2->getExprType() == EXPR_TYPE_CONST_VALUE || pExpr2->getExprType() == EXPR_TYPE_TOKEN && isTempVarName(pExpr2->getValue())))
			{
				replaceExprWithStatement(ret_v, pExpr2, inAndOutSignalNo);
				pExpr->setChildAt(i, pExpr2);
			}
		}

		if (bFuncFlow)
		{
			std::vector<CGrammarObject*> ret_v2, ret_v3;
			int nSignalEnter = inAndOutSignalNo;
			CGrammarObject* pScope = pExpr->getRealScope();

			TypeDefPointer pBaseTypeDef = pExpr->findTypeDef(CALL_PARAM_STRUCT_PREFIX + pFuncDeclare->getName());
			MY_ASSERT(pBaseTypeDef);
			pBaseTypeDef = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, NULL));

			std::string pStruVarName = getTempVarName();
			SourceTreeNode* pDeclVarNode = declVarCreateByName(pStruVarName);
			declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
			TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, pDeclVarNode));

            CVarDef* pVarDef = new CVarDef(pScope, pStruVarName, pTypeDef2, dupSourceTreeNode(pDeclVarNode));
            ((CStruct*)m_pStructTypeDef.get())->addDef(new CStatement((CStruct*)m_pStructTypeDef.get(), STATEMENT_TYPE_DEF, pVarDef, NULL));

			// pFuncVar->pTemp = (Struct*)__flow_func_enter(pVar->basic.flow, caller_func, caller_data, caller_signal, sizeof(Struct));
			inAndOutSignalNo = allocSignalNo();

			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
					FLOW_BASIC_FLOW_MEMBER_NAME));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, pScope->getFunctionScope()->getName()));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)));
			ret_v3.push_back(new CExpr(pExpr, EXPR_TYPE_SIZE_OF,
				extendedTypeVarCreateFromExtendedType(extendedTypeCreateFromType(typeCreateUserDefined(CALL_PARAM_STRUCT_PREFIX + pFuncDeclare->getName())))));
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pStruVarName),
				new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pTypeDef2,
					new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pExpr->findFuncDeclare(FLOW_FUNC_ENTER_FUNC_NAME, -1), ret_v3)
				)
			)));

			// setting parameters into struct
			int nStart = pExpr->getValue().empty() ? 1 : 0;
			int n = pFuncDeclare->getType()->getFuncParamCount();
			int n2 = pExpr->getChildrenCount();
			MY_ASSERT(n - nStart == pExpr->getChildrenCount());
			for (int i = 0; i < pFuncDeclare->getType()->getFuncParamCount(); i++)
			{
				CVarDef* pVarDef = pFuncDeclare->getType()->getFuncParamAt(i);
				ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
						new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
							new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME),
							pStruVarName),
					pVarDef->getName()),
					((CExpr*)pExpr->getChildAt(nStart)))));
				pExpr->removeChildAt(nStart);
			}

			// if (!func(signal_first_enter, pFuncVar->pTempVar)) return false;
			pExpr->setFuncDeclare(pExpr->findFuncDeclare(FLOW_FUNC_TYPE_NAME, -1));
			pExpr->setReturnType(g_type_def_bool);
			pExpr->setFlow(false);
			pExpr->addChild(new CExpr(NULL, EXPR_TYPE_TOKEN, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
			pExpr->addChild(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME),
				pStruVarName)
			);
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
				new CExpr(NULL, EXPR_TYPE_NOT, pExpr),
				new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_TOKEN, "false"))
			));

			// __flow_signal = n;
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)))));

			addBatchToStatementVector(ret_v, ret_v2, nSignalEnter);

			// pFuncVar->retTemp = pFuncVar->pTempVar->ret;
			std::string retVarName;
			if (!pFuncDeclare->getType()->isVoid())
			{
				retVarName = getTempVarName();
				CVarDef* pVarDef = new CVarDef(NULL, retVarName, pFuncDeclare->getType()->getFuncReturnType());
				((CStruct*)m_pStructTypeDef.get())->addDef(new CStatement((CStruct*)m_pStructTypeDef.get(), STATEMENT_TYPE_DEF, pVarDef, NULL));

				addToStatementVector(ret_v, new CStatement(pScope, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), retVarName),
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
						new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pStruVarName),
						PARAM_NAME_CALLER_RET)
				)), inAndOutSignalNo);
			}

			// __flow_func_leave(pFuncVar->basic.flow, pFuncVar->pTempVar);
			ret_v3.clear();
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
				FLOW_BASIC_FLOW_MEMBER_NAME
			));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pStruVarName));
			addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR,
				new CExpr(pScope, EXPR_TYPE_FUNC_CALL, pScope->findFuncDeclare(FLOW_FUNC_LEAVE_FUNC_NAME, -1), ret_v3)
			), inAndOutSignalNo);

			if (!retVarName.empty())
				pExpr = new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), retVarName);
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
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo);
		pExpr->setChildAt(0, pExpr1);
		MY_ASSERT(!pExpr1->isFlow());

		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(1);
		if (!pExpr2->isFlow())
			break;

		pExpr1->setReturnType(g_type_def_bool);
		replaceExprWithStatement(ret_v, pExpr1, inAndOutSignalNo);
		int nSignalEnter = inAndOutSignalNo;

		std::vector<CGrammarObject*> ret_v2;
		checkExprDecompose(ret_v2, pExpr2, inAndOutSignalNo);
		pExpr2 = new CExpr(NULL, EXPR_TYPE_ASSIGN, new CExpr(NULL, EXPR_TYPE_TOKEN, pExpr1->getValue()), pExpr2);
		ret_v2.push_back(new CStatement(pExpr2));

		pExpr2 = new CExpr(pExpr->getParent(), EXPR_TYPE_TOKEN, pExpr1->getValue());
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
		checkExprDecompose(ret_v, pExpr0, inAndOutSignalNo);
		pExpr->setChildAt(0, pExpr0);
		MY_ASSERT(!pExpr0->isFlow());

		CExpr* pExpr1 = (CExpr*)pExpr->getChildAt(1);
		CExpr* pExpr2 = (CExpr*)pExpr->getChildAt(2);
		if (!pExpr1->isFlow() && !pExpr2->isFlow())
			break;

		TypeDefPointer return_type = pExpr->getReturnType();
		CVarDef* pVarDef = new CVarDef(NULL, getTempVarName(), return_type, NULL);
		SourceTreeNode* pDeclVarNode = declVarCreateByName(pVarDef->getName());
		for (int i = 0; i < pExpr->getReturnDepth(); i++)
			declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
		pVarDef->setDeclVarNode(pDeclVarNode);
		deleteSourceTreeNode(pDeclVarNode);
		CStatement* pStatement = new CStatement(pExpr->getRealScope(), STATEMENT_TYPE_EXPR, pVarDef, return_type->getBaseType()->getBasicNode());
		addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
		int nSignalEnter = inAndOutSignalNo;

		std::vector<CGrammarObject*> ret_v2;

		checkExprDecompose(ret_v2, pExpr1, inAndOutSignalNo);
		MY_ASSERT(!pExpr1->isFlow());
		pExpr1 = new CExpr(NULL, EXPR_TYPE_ASSIGN, new CExpr(pExpr->getParent(), EXPR_TYPE_TOKEN, pVarDef->getName()), pExpr1);
		ret_v2.push_back(new CStatement(pExpr1));

		CStatement* pStatementIf = new CStatement(pExpr->getRealScope(), STATEMENT_TYPE_IF, pExpr0, new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));

		ret_v2.clear();
		checkExprDecompose(ret_v2, pExpr2, nSignalEnter);
		MY_ASSERT(!pExpr2->isFlow());
		pExpr2 = new CExpr(NULL, EXPR_TYPE_ASSIGN, new CExpr(pExpr->getParent(), EXPR_TYPE_TOKEN, pVarDef->getName()), pExpr2);
		ret_v2.push_back(new CStatement(pExpr2));

		pStatementIf->addElseStatement(new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));
		addToStatementVector(ret_v, pStatementIf, nSignalEnter);

		pExpr->m_children.clear();
		CGrammarObject* pParent = pExpr->getParent();
		delete pExpr;
		pExpr = new CExpr(pParent, EXPR_TYPE_TOKEN, pVarDef->getName());
		break;
	}
	default:
		MY_ASSERT(false);
	}
}

// try to decompose one statement to a few ones if necessary. pass in signal via inAndOutSignalNo, function should set out signal in inAndOutSignalNo when returns.
void CMyFunc::checkStatementDecompose(std::vector<CGrammarObject*>& ret_v, CStatement* pStatement, int nContinueSignal, int& inAndOutSignalNo)
{
	bool bWithinFlowBlock = !(pStatement->getParent()->getNodeType() == SCOPE_TYPE_STATEMENT && !((CStatement*)pStatement->getParent())->isFlow());
//printf("\ncheckStatementDecompose, this=%lx, <<<<%s>>>>, type=%d, bFlow=%d, inAndOutSignalNo=%d\n", (long)pStatement, pStatement->toString(4).c_str(), pStatement->getStatementType(), pStatement->isFlow(), inAndOutSignalNo);
	std::vector<CGrammarObject*> ret_v2;

	if (pStatement->getStatementType() == STATEMENT_TYPE_DEF)
	{
		switch (pStatement->getDefType())
		{
		case DEF_TYPE_STRUCT_DECL:
		case DEF_TYPE_UNION_DECL:
		case DEF_TYPE_TYPEDEF:
		case DEF_TYPE_FUNC_DECL:
		case DEF_TYPE_FUNC_VAR_DEF:
			addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
			break;

		case DEF_TYPE_VAR_DEF:
		{
			int currentSignal = inAndOutSignalNo;
			for (int i = 0; i < pStatement->getVarCount(); i++)
			{
				CStatement* pStatement2;
				CVarDef* pVarDef = pStatement->getVarAt(i);

				if (pVarDef->isReference())
				{
					pStatement2 = new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pVarDef, NULL);
					ret_v.push_back(pStatement2);
					continue;
				}

				// add "int i;" in pStruct
				CExpr* pInitExpr = pVarDef->getInitExpr();

				// add define into struct
				CVarDef* pVarDef2 = new CVarDef(pVarDef->getParent(), pVarDef->getName(), pVarDef->getType(), dupSourceTreeNode(pVarDef->getDeclVarNode()));
				pStatement2 = new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pVarDef2, NULL);
				((CStruct*)m_pStructTypeDef.get())->addDef(pStatement2);

				// int& i = pStruct->i;
				pVarDef->setReference();
				pVarDef->setInitExpr(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME),
					pVarDef->getName()));
				pStatement2 = new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pVarDef, NULL);
				ret_v.push_back(pStatement2);

				if (pInitExpr)
				{
					// i = 3;
					pStatement2 = new CStatement(pStatement->getParent(), STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
						new CExpr(NULL, EXPR_TYPE_TOKEN, pVarDef->getName()),
						pInitExpr));
					checkStatementDecompose(ret_v, pStatement2, nContinueSignal, inAndOutSignalNo);
				}
			}
			//pStatement->setFlow(false);
			//addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
			break;
		}
		default:
			MY_ASSERT(false);
		}
//printf("checkStatementDecompose, this=%lx, return size=%d\n", (long)pStatement, ret_v.size());
		return;
	}

	if (!pStatement->isFlow())
	{
		checkNonFlowStatementDecompose(ret_v2, pStatement, nContinueSignal);
		addBatchToStatementVector(ret_v, ret_v2, inAndOutSignalNo);
//printf("checkStatementDecompose, this=%lx, return size=%d\n", (long)pStatement, ret_v.size());
		return;
	}

	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_EXPR:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 1);
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		checkExprDecompose(ret_v, pExpr, inAndOutSignalNo);
		if (pExpr && pExpr->getExprType() != EXPR_TYPE_TOKEN)
		{
			MY_ASSERT(!pExpr->isFlow());
			pStatement->setChildAt(0, pExpr);
			pStatement->setFlow(false);
			addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
		}
		break;
	}
	case STATEMENT_TYPE_COMPOUND:
	{
		ret_v2 = pStatement->m_children;
		pStatement->m_children.clear();
		BOOST_FOREACH (CGrammarObject* pGrammarObj, ret_v2)
		{
			MY_ASSERT(pGrammarObj->getNodeType() == SCOPE_TYPE_STATEMENT);
			checkStatementDecompose(pStatement->m_children, (CStatement*)pGrammarObj, nContinueSignal, inAndOutSignalNo);
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
			checkExprDecompose(ret_v, pExpr, inAndOutSignalNo);
			MY_ASSERT(!pExpr->isFlow());
			pStatement->setChildAt(0, pExpr);
		}
		ret_v.push_back(pStatement);

		int nSignalEnter = inAndOutSignalNo;
		int nSignalStart = getNextSignalNo();

		CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(1);
		if (!pStatement2->isFlow())
		{
			if (pStatement2->getStatementType() != STATEMENT_TYPE_COMPOUND)
			{
				ret_v2.clear();
				ret_v2.push_back(pStatement2);
				pStatement2 = new CStatement(pStatement2->getParent()->getRealScope(), STATEMENT_TYPE_COMPOUND, ret_v2);
			}
			ret_v2.clear();
			checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo);
			MY_ASSERT(ret_v2.size() == 1);
			pStatement->setChildAt(1, pStatement2);
		}
		bFlow |= pStatement2->isFlow();
		pStatement->setChildAt(0, composeSignalLogicExpr((CExpr*)pStatement->getChildAt(0), nSignalEnter, nSignalStart, inAndOutSignalNo));
		nSignalStart = getNextSignalNo();

		int n = pStatement->m_int_vector[0];
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
		}
		if (pStatement->m_bFlag)
		{
			CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(2 + n * 2);
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
			  pStatement->setChildAt(2 + n * 2, pStatement2);
			}
			bFlow |= pStatement2->isFlow();
			pStatement->insertChildAt(2 + n * 2, composeSignalLogicExpr(NULL, nSignalEnter, nSignalStart, inAndOutSignalNo));
			pStatement->m_int_vector[0] = n + 1;
			pStatement->m_bFlag = false;
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

			checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo);
			MY_ASSERT(!pExpr1->isFlow());
			addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR, pExpr1), inAndOutSignalNo);

			pExpr1 = (CExpr*)pStatement->getChildAt(1);
			pExpr2 = (CExpr*)pStatement->getChildAt(2);
			pStatement2 = (CStatement*)pStatement->getChildAt(3);
		}

		std::string tempVarName = getTempVarName();
		CVarDef* pVarDef = new CVarDef(NULL, tempVarName, g_type_def_bool, declVarCreateByName(tempVarName)); // var of bFirstEnter
        ((CStruct*)m_pStructTypeDef.get())->addDef(new CStatement((CStruct*)m_pStructTypeDef.get(), STATEMENT_TYPE_DEF, pVarDef, NULL));
		addToStatementVector(ret_v, new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarDef->getName()),
			new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "true")
		)), inAndOutSignalNo);

		/*
			while (signal <= lastSignal)
			{
			  if (!bFirstEnter)
			    pExpr2; // for only
			  if (signal > nSignalEnter)
			    signal = nSignalEnter;
			  if (!pExpr1) // for DO: if (!bFirstEnter && !pExpr1)
			    break;
			  bFirstEnter = false;
			  ...
			  ...
			}
		*/
		int nSignalEnter = allocSignalNo(); // this is exiting signal
		int nSignalStart = getNextSignalNo();

		if (pStatement2->getStatementType() != STATEMENT_TYPE_COMPOUND)
		{
			ret_v2.clear();
			ret_v2.push_back(pStatement2);
			pStatement2 = new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2);
			pStatement->setChildAt(1, pStatement2);
		}

		pStatement2->insertChildAt(0, new CStatement(NULL, STATEMENT_TYPE_EXPR,
			new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarDef->getName()),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false"))));

		pExpr1 = createNotExpr(pExpr1);
		if (pStatement->getStatementType() == STATEMENT_TYPE_DO)
			pExpr1 = new CExpr(NULL, EXPR_TYPE_AND,
				new CExpr(NULL, EXPR_TYPE_NOT, new CExpr(NULL, EXPR_TYPE_TOKEN, pVarDef->getName())),
				pExpr1);
		pStatement2->insertChildAt(0, new CStatement(NULL, STATEMENT_TYPE_IF, pExpr1,
			new CStatement(NULL, STATEMENT_TYPE_BREAK)));

		ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_GREATER_THAN,
				new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))),
			new CStatement(NULL, STATEMENT_TYPE_EXPR,
				new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))))));

		if (pStatement->getStatementType() == STATEMENT_TYPE_FOR)
		{
			pStatement2->insertChildAt(0, new CStatement(NULL, STATEMENT_TYPE_IF,
				new CExpr(NULL, EXPR_TYPE_NOT, new CExpr(NULL, EXPR_TYPE_TOKEN, pVarDef->getName())),
				new CStatement(NULL, STATEMENT_TYPE_EXPR, pExpr2)));
		}

		ret_v2.clear();
		checkStatementDecompose(ret_v2, pStatement2, nSignalEnter, inAndOutSignalNo);
		MY_ASSERT(ret_v2.size() == 1);
		pStatement->setChildAt(1, ret_v2[0]);

		pStatement->m_statement_type = STATEMENT_TYPE_WHILE;
		pStatement->m_children.clear();
		pStatement->addChild(new CExpr(NULL, EXPR_TYPE_LESS_EQUAL,
			new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
			new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo))));
		pStatement->addChild(pStatement2);
		ret_v.push_back(pStatement);

		inAndOutSignalNo = nSignalEnter;
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
		std::vector<CGrammarObject*> ret_v3;
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		if (pExpr->isFlow())
		{
			checkExprDecompose(ret_v3, pExpr, inAndOutSignalNo);
			MY_ASSERT(!pExpr->isFlow());
			pStatement->setChildAt(0, pExpr);
		}
		ret_v3.push_back(pStatement);

		int nSignalEnter = allocSignalNo(); // this is exiting signal

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
				checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo);
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
				checkStatementDecompose(ret_v2, pStatement2, nContinueSignal, inAndOutSignalNo);
				pStatement->removeChildAt(n);
				pStatement->insertChildrenAt(n, ret_v2);
				n += ret_v2.size();
			}
		}

		ret_v3.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_GREATER_THAN,
				new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))),
			new CStatement(NULL, STATEMENT_TYPE_EXPR,
				new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
					new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter))))));

		addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v3), inAndOutSignalNo);
		inAndOutSignalNo = nSignalEnter;
		break;
	}
	case STATEMENT_TYPE_RETURN:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 1);
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		MY_ASSERT(pExpr->isFlow());
		checkExprDecompose(ret_v, pExpr, inAndOutSignalNo);
		MY_ASSERT(!pExpr->isFlow());
		pStatement->setChildAt(0, pExpr);
		pStatement->setFlow(false);

		checkNonFlowStatementDecompose(ret_v, pStatement, nContinueSignal);
		break;
	}
	/*case STATEMENT_TYPE_FLOW_SIGNAL:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 2);
		CExpr* pExpr = (CExpr*)pStatement->getChildAt(0);
		checkExprDecompose(ret_v, pExpr, inAndOutSignalNo);
		MY_ASSERT(!pExpr->isFlow());
		pStatement->setChildAt(0, pExpr);

		pExpr = (CExpr*)pStatement->getChildAt(1);
		checkExprDecompose(ret_v, pExpr, inAndOutSignalNo);
		MY_ASSERT(!pExpr->isFlow());
		pStatement->setChildAt(1, pExpr);

		pStatement->setFlow(false);
		addToStatementVector(ret_v, pStatement, inAndOutSignalNo);
		break;
	}*/
	case STATEMENT_TYPE_FLOW_WAIT:
	{
		MY_ASSERT(pStatement->getChildrenCount() == 2);
		CExpr* pExpr1 = (CExpr*)pStatement->getChildAt(0);
		checkExprDecompose(ret_v, pExpr1, inAndOutSignalNo);
		MY_ASSERT(!pExpr1->isFlow());

		CExpr* pExpr2 = (CExpr*)pStatement->getChildAt(1);
		checkExprDecompose(ret_v, pExpr2, inAndOutSignalNo);
		MY_ASSERT(!pExpr2->isFlow());

		pStatement->removeAllChildren();

		// if (!__flow_wait(pExpr1, this_func, signal, pFuncVar, pExpr2))
		//   return false;
		CFuncDeclare* pFuncDeclare = pStatement->findFuncDeclare(FLOW_WAIT_FUNC_NAME, -1);
		MY_ASSERT(pFuncDeclare);
		ret_v2.clear();
		CExpr* pExpr = new CExpr(pStatement->getParent()->getRealScope(), EXPR_TYPE_FUNC_CALL, pFuncDeclare, ret_v2);

		int nSignalEnter = allocSignalNo(); // this is exiting signal
		inAndOutSignalNo = allocSignalNo();

		CGrammarObject* pFunc = pStatement;
		while (pFunc->getNodeType() != SCOPE_TYPE_FUNC)
			pFunc = pFunc->getParent();
		pExpr->addChild(pExpr1);
		pExpr->addChild(new CExpr(NULL, EXPR_TYPE_TOKEN, ((CFunction*)pFunc)->getName()));
		pExpr->addChild(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(inAndOutSignalNo)));
		pExpr->addChild(new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME));
		pExpr->addChild(pExpr2);

		pStatement = new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_NOT, pExpr),
			new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false")));
		addToStatementVector(ret_v, pStatement, nSignalEnter);
		break;
	}
	case STATEMENT_TYPE_FLOW_FORK:
	{
		TypeDefPointer pFuncType = TypeDefPointer(new CTypeDef("", g_type_def_void, FLOW_TYPE_FLOW, 0));

		SourceTreeNode* pDeclVar = declVarCreateByName(LOCALVAR_NAME);
		declVarAddModifier(pDeclVar, DVMOD_TYPE_POINTER);
		TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getName(), m_pStructTypeDef, NULL));
		pTypeDef = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getName(), pTypeDef, pDeclVar));
		pFuncType->addFuncParam(new CVarDef(NULL, FLOW_FORK_PARENT_PARAM_NAME, pTypeDef, NULL));

		CFunction* pNewFunc = new CFunction(m_pFunc->getParent()->getRealScope(), getNewSubFuncName(), pFuncType, FLOW_TYPE_FLOW);
		pNewFunc->addParamVar(new CVarDef(NULL, FLOW_FORK_PARENT_PARAM_NAME, pTypeDef, NULL));

		MY_ASSERT(pStatement->getChildrenCount() == 1);
		CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(0);
		if (pStatement2->getStatementType() == STATEMENT_TYPE_COMPOUND)
		{
			for (int i = 0; i < pStatement2->getChildrenCount(); i++)
			{
				CStatement* pStatement3 = (CStatement*)pStatement2->getChildAt(i);
				changeVarToUnderStructInStatement(pStatement, FLOW_FORK_PARENT_PARAM_NAME, pStatement3);
				pNewFunc->addChild(pStatement3);
			}
		}
		else
		{
			changeVarToUnderStructInStatement(pStatement, FLOW_FORK_PARENT_PARAM_NAME, pStatement2);
			pNewFunc->addChild(pStatement2);
		}

		CMyFunc* pNewMyFunc = new CMyFunc(pNewFunc, true);
		pNewMyFunc->analyze();

		std::vector<CGrammarObject*> ret_v3;
		int nSignalEnter = inAndOutSignalNo;

		// void* pFlow = __flow_start(pVar->basic.pFlow);
		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
			FLOW_BASIC_FLOW_MEMBER_NAME));
		CVarDef* pFlowVar = new CVarDef(pStatement->getParent()->getRealScope(), getTempVarName(), g_type_def_void_ptr, NULL,
			new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_START_FUNC_NAME, -1), ret_v3)
		);
		ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pFlowVar));

		// struct* pTemp1 = (struct*)__flow_func_enter(pTemp1->basic.flow, NULL, NULL, 0, sizeof(struct));
		TypeDefPointer pBaseTypeDef = pStatement->findTypeDef(CALL_PARAM_STRUCT_PREFIX + pNewFunc->getName());
		MY_ASSERT(pBaseTypeDef);
		pBaseTypeDef = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, NULL));

		SourceTreeNode* pDeclVarNode = declVarCreateByName("");
		declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
		TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, pDeclVarNode));

		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, pFlowVar->getName()));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		ret_v3.push_back(new CExpr(pStatement, EXPR_TYPE_SIZE_OF,
			extendedTypeVarCreateFromExtendedType(extendedTypeCreateFromType(typeCreateUserDefined(CALL_PARAM_STRUCT_PREFIX + pNewFunc->getName())))));

		CVarDef* pFuncVar = new CVarDef(pStatement->getParent()->getRealScope(), getTempVarName(), pTypeDef2, dupSourceTreeNode(pDeclVarNode),
			new CExpr(pStatement->getRealScope(), EXPR_TYPE_TYPE_CAST, pTypeDef2,
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_FUNC_ENTER_FUNC_NAME, -1), ret_v3)
		));
		ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_DEF, pFuncVar));

	    //pTempFunc->parent_param = pFuncVar;
		ret_v2.push_back(new CStatement(pStatement->getParent()->getRealScope(), STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, pFuncVar->getName()), FLOW_FORK_PARENT_PARAM_NAME),
			new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME)
		)));

		// func_call(0, pTemp1);
		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, pFuncVar->getName()));
		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR,
			new CExpr(pStatement->getParent()->getRealScope(), EXPR_TYPE_FUNC_CALL, new CExpr(NULL, EXPR_TYPE_TOKEN, pNewFunc->getName()), ret_v3)
		));

		addBatchToStatementVector(ret_v, ret_v2, inAndOutSignalNo);

		delete pNewMyFunc;
		break;
	}
	case STATEMENT_TYPE_FLOW_TRY:
	{
		int nCatches = pStatement->m_int_vector[0];
		MY_ASSERT(nCatches > 0);

	    // move try block to a sub function
		TypeDefPointer pFuncType = TypeDefPointer(new CTypeDef("", g_type_def_void, FLOW_TYPE_FLOW, 0));

		SourceTreeNode* pDeclVar = declVarCreateByName(LOCALVAR_NAME);
		declVarAddModifier(pDeclVar, DVMOD_TYPE_POINTER);
		TypeDefPointer pTypeDef = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getName(), m_pStructTypeDef, NULL));
		pTypeDef = TypeDefPointer(new CTypeDef(m_pStructTypeDef->getName(), pTypeDef, pDeclVar));
		pFuncType->addFuncParam(new CVarDef(NULL, FLOW_FORK_PARENT_PARAM_NAME, pTypeDef, NULL));

		CFunction* pSubFunc = new CFunction(m_pFunc->getParent()->getRealScope(), getNewSubFuncName(), pFuncType, FLOW_TYPE_FLOW);
		pSubFunc->addParamVar(new CVarDef(NULL, FLOW_FORK_PARENT_PARAM_NAME, pTypeDef, NULL));

		CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(0);
		if (pStatement2->getStatementType() == STATEMENT_TYPE_COMPOUND)
		{
			for (int i = 0; i < pStatement2->getChildrenCount(); i++)
			{
				CStatement* pStatement3 = (CStatement*)pStatement2->getChildAt(i);
				changeVarToUnderStructInStatement(pStatement, FLOW_FORK_PARENT_PARAM_NAME, pStatement3);
				pSubFunc->addChild(pStatement3);
			}
		}
		else
		{
			changeVarToUnderStructInStatement(pStatement, FLOW_FORK_PARENT_PARAM_NAME, pStatement2);
			pSubFunc->addChild(pStatement2);
		}
		pStatement2 = NULL;

		CMyFunc* pMySubFunc = new CMyFunc(pSubFunc, false);
		pMySubFunc->analyze();

		std::vector<CGrammarObject*> ret_v3;
		int nSignalEnter = inAndOutSignalNo;

		// pVar->sub_flow = __flow_start(pVar->basic.pFlow);
        CVarDef* pVarSubFlow = new CVarDef(pStatement, getTempVarName(), g_type_def_void_ptr);
        ((CStruct*)m_pStructTypeDef.get())->addDef(new CStatement((CStruct*)m_pStructTypeDef.get(), STATEMENT_TYPE_DEF, pVarSubFlow, NULL));

		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
			FLOW_BASIC_FLOW_MEMBER_NAME));
		addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
            new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarSubFlow->getName()),
            new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_START_FUNC_NAME, -1), ret_v3)
		)), nSignalEnter);

		//pVar->pTempWait1 = &object1;
		//pVar->pTempWait2 = &object2;
		int n;
		TypeDefPointer pBaseTypeDef = pStatement->findTypeDef(FLOW_OBJECT_TYPE_NAME);
		MY_ASSERT(pBaseTypeDef);
		pBaseTypeDef = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, NULL));

		SourceTreeNode* pDeclVarNode = declVarCreateByName("");
		declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
		TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, pDeclVarNode));

		std::vector<std::string> tempWaitVarNameList;
		for (n = 0; n < nCatches; n++)
		{
	        CVarDef* pVarWait = new CVarDef(pStatement, getTempVarName(), pTypeDef2);
	        ((CStruct*)m_pStructTypeDef.get())->addDef(new CStatement((CStruct*)m_pStructTypeDef.get(), STATEMENT_TYPE_DEF, pVarWait, NULL));
	        tempWaitVarNameList.push_back(pVarWait->getName());
			addToStatementVector(ret_v, new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
	            new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarWait->getName()),
	            (CExpr*)pStatement->getChildAt(1 + n * 3)
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
	      signal = 3;
	    }*/
		for (n = 0; n < nCatches; n++)
		{
			ret_v3.clear();
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), tempWaitVarNameList[n]));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, m_pFunc->getName()));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1)));
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME));
			ret_v3.push_back((CExpr*)pStatement->getChildAt(1 + n * 3 + 1));
			if (n == 0)
				pStatement2 = new CStatement(NULL, STATEMENT_TYPE_IF,
					new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_WAIT_FUNC_NAME, -1), ret_v3),
					new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
						new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
						new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + 1))))
				);
			else
				pStatement2->addElseIf(
					new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_WAIT_FUNC_NAME, -1), ret_v3),
					new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
						new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
						new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))))
				);
			allocSignalNo();
		}

		ret_v2.clear();

		pBaseTypeDef = pStatement->findTypeDef(CALL_PARAM_STRUCT_PREFIX + pSubFunc->getName());
		MY_ASSERT(pBaseTypeDef);
		pBaseTypeDef = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, NULL));

		pDeclVarNode = declVarCreateByName("");
		declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
		pTypeDef2 = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, pDeclVarNode));

		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarSubFlow->getName()));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, m_pFunc->getName()));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1)));
		ret_v3.push_back(new CExpr(pStatement, EXPR_TYPE_SIZE_OF,
			extendedTypeVarCreateFromExtendedType(extendedTypeCreateFromType(typeCreateUserDefined(CALL_PARAM_STRUCT_PREFIX + pSubFunc->getName())))));
		CVarDef* pFuncVar = new CVarDef(m_pFunc, getTempVarName(), pTypeDef2, dupSourceTreeNode(pDeclVarNode),
			new CExpr(pStatement->getRealScope(), EXPR_TYPE_TYPE_CAST, pTypeDef2,
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_FUNC_ENTER_FUNC_NAME, -1), ret_v3)
		));
		ret_v2.push_back(new CStatement(m_pFunc, STATEMENT_TYPE_DEF, pFuncVar));
		inAndOutSignalNo = allocSignalNo();

		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, pFuncVar->getName()), FLOW_FORK_PARENT_PARAM_NAME),
			new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME)
		)));

		// func_call(0, pTemp1);
		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, pFuncVar->getName()));
		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
			new CExpr(NULL, EXPR_TYPE_NOT, new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, new CExpr(NULL, EXPR_TYPE_TOKEN, pSubFunc->getName()), ret_v3)),
			new CStatement(NULL, STATEMENT_TYPE_RETURN, new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false"))
		));

		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))
		)));

		pStatement2->addElseStatement(new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));

		addToStatementVector(ret_v, pStatement2, nSignalEnter);
		pStatement2 = NULL;

		//if (signal == 1)
		//{
		//    flow_cancel_wait(&object2);
		//    flow_end(pVar->sub_flow);
		//    ...
		//}
		//else if (signal == 2)
		//{
		//    flow_cancel_wait(&object1);
		//    flow_end(pVar->sub_flow);
		//    ...
		//}
		//else if (signal == 3)
		//{
		//    flow_end(pVar->sub_flow);
		//    flow_cancel_wait(&object1);
		//    flow_cancel_wait(&object2);
		//}

		for (n = 0; n < nCatches; n++)
		{
			ret_v2.clear();
			for (int i = 0; i < nCatches; i++)
			{
				if (i == n)
					continue;
				ret_v3.clear();
				ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), tempWaitVarNameList[i]));
				ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR,
					new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_CANCEL_WAIT_FUNC_NAME, -1), ret_v3)));
			}
			ret_v3.clear();
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarSubFlow->getName()));
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR,
					new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_END_FUNC_NAME, -1), ret_v3)));

			CStatement* pCatchStatement = (CStatement*)pStatement->getChildAt(1 + n * 3 + 2);
			if (pCatchStatement->getStatementType() == STATEMENT_TYPE_COMPOUND)
			{
				for (int i = 0; i < pCatchStatement->getChildrenCount(); i++)
				{
					CStatement* pStatement3 = (CStatement*)pCatchStatement->getChildAt(i);
					checkStatementDecompose(ret_v2, pStatement3, -1, inAndOutSignalNo);
				}
			}
			else
			{
				checkStatementDecompose(ret_v2, pCatchStatement, -1, inAndOutSignalNo);
			}
			if (n == 0)
				pStatement2 = new CStatement(NULL, STATEMENT_TYPE_IF,
					new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))),
					new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2)
				);
			else
				pStatement2->addElseIf(
					new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + n + 1))),
					new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));
		}

		ret_v2.clear();
		ret_v3.clear();
		ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarSubFlow->getName()));
		ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR,
				new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_END_FUNC_NAME, -1), ret_v3)));
		for (int i = 0; i < nCatches; i++)
		{
			ret_v3.clear();
			ret_v3.push_back(new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), tempWaitVarNameList[i]));
			ret_v2.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR,
				new CExpr(m_pFunc, EXPR_TYPE_FUNC_CALL, pStatement->findFuncDeclare(FLOW_CANCEL_WAIT_FUNC_NAME, -1), ret_v3)));
		}
		pStatement2->addElseIf(
			new CExpr(NULL, EXPR_TYPE_EQUAL, new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL), new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(nSignalEnter + nCatches + 1))),
			new CStatement(NULL, STATEMENT_TYPE_COMPOUND, ret_v2));
		ret_v.push_back(pStatement2);

		pStatement->removeAllChildren();
		delete pMySubFunc;
		break;
	}
	case STATEMENT_TYPE_FLOW_ENGINE:
		MY_ASSERT(pStatement->getChildrenCount() == 1);
		ret_v.push_back(pStatement->getChildAt(0));
		break;
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

void CMyFunc::checkDupNames(CStatement* pStatement/*, std::string& pre_string*/)
{
	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_DEF:
	{
		int var_count = pStatement->getVarCount();
		if (var_count == 0)
			return;

		for (int i = 0; i < var_count; i++)
		{
			CVarDef* pVar = pStatement->getVarAt(i);
			if (findVar(pVar->getName()))
			{
				std::string s = getTempVarName();
				pVar->changeName(s);
			}
			addVar(pVar->getName());
			/*if (!pVar->isReference())
			{
				CExpr* pExpr = new CExpr(pStatement, EXPR_TYPE_TOKEN, LOCALVAR_NAME);
				pExpr = new CExpr(pStatement, EXPR_TYPE_REF_ELEMENT, pExpr, pVar->getName());
				if (pVar->getInitExpr())
					pVar->setInitExpr(new CExpr(pStatement, EXPR_TYPE_ASSIGN, pExpr, pVar->getInitExpr()));
				else
					pVar->setInitExpr(pExpr);
				pre_string += printTabs(1) + displaySourceTreeType(pStatement->getTypeSourceNode()) + " " + pVar->toString(false) + ";\n";
				pVar->setReference();
			}*/
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
	default:
	{
		for (int i = 0; i < pStatement->getChildrenCount(); i++)
		{
			CGrammarObject* pObj = pStatement->getChildAt(i);
			if (pObj->getNodeType() == SCOPE_TYPE_STATEMENT)
			  checkDupNames((CStatement*)pObj);
		}
	}
	}
}

// for non-sync statements, we want to leave it unchanged except two kinds of statements inside: return and continue
void CMyFunc::checkNonFlowStatementDecompose(std::vector<CGrammarObject*>& ret_v, CStatement* pStatement, int nContinueSignal)
{
	std::vector<CGrammarObject*> ret_v2;
	CStatement* pStatement2;

	MY_ASSERT(!pStatement->isFlow());

	switch (pStatement->getStatementType())
	{
	case STATEMENT_TYPE_RETURN:
		if (isFork())
		{
			// __flow_end(pVar->basic.pFlow);
			ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
				FLOW_BASIC_FLOW_MEMBER_NAME));
			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_FUNC_CALL,
					pStatement->findFuncDeclare(FLOW_END_FUNC_NAME, -1), ret_v2)));
			// return false;
			MY_ASSERT(pStatement->getChildrenCount() == 0);
			pStatement->addChild(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, "false"));
			ret_v.push_back(pStatement);
		}
		else if (m_pFunc->isFlowRoot())
		{
			// __flow_end(pVar->basic.pFlow);
			ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
				FLOW_BASIC_FLOW_MEMBER_NAME));
			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_FUNC_CALL,
					pStatement->findFuncDeclare(FLOW_END_FUNC_NAME, -1), ret_v2)));
			// return
			ret_v.push_back(pStatement);
		}
		else
		{
			// assign ret value;
			if (pStatement->getChildrenCount() > 0)
			{
				ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), PARAM_NAME_CALLER_RET),
					(CExpr*)pStatement->getChildAt(0)
				)));
				pStatement->removeChildAt(0);
			}

			// if (signal != 0 && pLocalVar->basic.caller_func)
			//	pLocalVar->basic.caller_func(pLocalVar->basic.signal, pLocalVar->basic.caller_data);
			ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
					PARAM_NAME_CALLER_SIGNAL));
			ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
					new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
					PARAM_NAME_CALLER_DATA));

			ret_v.push_back(new CStatement(NULL, STATEMENT_TYPE_IF,
				new CExpr(NULL, EXPR_TYPE_AND,
					new CExpr(NULL, EXPR_TYPE_NOT_EQUAL,
						new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
						new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER))
					),
					new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
							new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
							PARAM_NAME_CALLER_FUNC)
				),
				new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_FUNC_CALL,
						new CExpr(NULL, EXPR_TYPE_REF_ELEMENT,
								new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), FLOW_BASIC_MEMBER_NAME),
								PARAM_NAME_CALLER_FUNC),
						ret_v2
				))
			));

			// return signal == 0;
			pStatement->addChild(new CExpr(NULL, EXPR_TYPE_EQUAL,
				new CExpr(NULL, EXPR_TYPE_TOKEN, PARAM_VAR_NAME_SIGNAL),
				new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER))
			));
			ret_v.push_back(pStatement);
		}
		break;

	case STATEMENT_TYPE_DEF:
	case STATEMENT_TYPE_EXPR:
	case STATEMENT_TYPE_CONTINUE:
	case STATEMENT_TYPE_BREAK:
	//case STATEMENT_TYPE_FLOW_SIGNAL:
		ret_v.push_back(pStatement);
		break;

	case STATEMENT_TYPE_COMPOUND:
		ret_v2 = pStatement->m_children;
		pStatement->m_children.clear();
		BOOST_FOREACH (CGrammarObject* pGrammarObj, ret_v2)
		{
			MY_ASSERT(pGrammarObj->getNodeType() == SCOPE_TYPE_STATEMENT);
			checkNonFlowStatementDecompose(pStatement->m_children, (CStatement*)pGrammarObj, nContinueSignal);
		}
		ret_v.push_back(pStatement);
		break;

	case STATEMENT_TYPE_IF:
	case STATEMENT_TYPE_WHILE:
	case STATEMENT_TYPE_DO:
	case STATEMENT_TYPE_FOR:
	{
		for (int i = 0; i < pStatement->getChildrenCount(); i++)
		{
			CGrammarObject* pObj = pStatement->getChildAt(i);
			if (pObj->getNodeType() != SCOPE_TYPE_STATEMENT)
				continue;

			CStatement* pStatement2 = (CStatement*)pObj;
			ret_v2.clear();
			checkNonFlowStatementDecompose(ret_v2, pStatement2, nContinueSignal);
			if (ret_v2.size() == 1)
				pStatement2 = (CStatement*)ret_v2[0];
			else
				pStatement2 = new CStatement(pStatement, STATEMENT_TYPE_COMPOUND, ret_v2);
			pStatement->setChildAt(i, pStatement2);
		}
		ret_v.push_back(pStatement);
		break;
	}
	case STATEMENT_TYPE_SWITCH:
	{
		int n = 1;
		for (int i = 0; i < pStatement->m_int_vector.size(); i++)
		{
			int m = pStatement->m_int_vector[i];
			for (int j = 1; j < m;)
			{
				CStatement* pStatement2 = (CStatement*)pStatement->getChildAt(n + j);
				ret_v2.clear();
				checkNonFlowStatementDecompose(ret_v2, pStatement2, nContinueSignal);
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
				checkNonFlowStatementDecompose(ret_v2, pStatement2, nContinueSignal);
				pStatement->removeChildAt(n);
				pStatement->insertChildrenAt(n, ret_v2);
				n += ret_v2.size();
			}
		}
		ret_v.push_back(pStatement);
		break;
	}
	case STATEMENT_TYPE_TRY:
	case STATEMENT_TYPE_FLOW_WAIT:
	case STATEMENT_TYPE_FLOW_TRY:
	case STATEMENT_TYPE_FLOW_FORK:
	case STATEMENT_TYPE_FLOW_ENGINE:
		MY_ASSERT(false);
	}
}

void CMyFunc::analyze()
{
	// step 1, move func params into struct
	GrammarObjectVector ret_v, ret_v2;

	if (m_pFunc->getChildrenCount() == 0 ||
		((CStatement*)m_pFunc->getChildAt(m_pFunc->getChildrenCount() - 1))->getStatementType() != STATEMENT_TYPE_RETURN && m_pFunc->getFuncType()->getFuncReturnType()->isVoid())
	{
		m_pFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_RETURN));
	}

	CStatement* pFuncParamStructDefStatement = checkFuncGetParamStructDef();
	g_solvedBlocks.push_back(pFuncParamStructDefStatement);

	// step 2, check dup names. when found, rename them.
	for (int i = 0; i < m_pFunc->getChildrenCount(); i++)
	{
		CStatement* pStatement = (CStatement*)m_pFunc->getChildAt(i);
		checkDupNames(pStatement);
	}

	// step 2, decompose
	int nSignalStart = 0;
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

	BOOST_FOREACH(CGrammarObject* pGrammarObj, ret_v)
	{
		MY_ASSERT(pGrammarObj->getNodeType() == SCOPE_TYPE_STATEMENT);
		CStatement* pStatement = (CStatement*)pGrammarObj;
		checkStatementDecompose(m_pFunc->m_children, pStatement, -1, nSignalStart);
	}

	std::string flowName;
	if (m_pFunc->isFlowRoot())
	{
		// pFlow = __flow_start(NULL);
		ret_v.clear();
		ret_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		flowName = getTempVarName();
		m_pFunc->insertChildAt(0, new CStatement(m_pFunc, STATEMENT_TYPE_DEF, new CVarDef(m_pFunc, flowName, g_type_def_void_ptr, NULL, new CExpr(
			m_pFunc, EXPR_TYPE_FUNC_CALL, m_pFunc->findFuncDeclare(FLOW_START_FUNC_NAME, -1), ret_v))));

		// Struct* pFuncData = (Struct*)__flow_func_enter(pFlow, NULL, NULL, 0, sizeof(Struct));
		TypeDefPointer pBaseTypeDef = m_pFunc->findTypeDef(CALL_PARAM_STRUCT_PREFIX + m_pFunc->getName());
		MY_ASSERT(pBaseTypeDef);
		pBaseTypeDef = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, NULL));

		SourceTreeNode* pDeclVarNode = declVarCreateByName("");
		declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
		TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, pDeclVarNode));

		ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, flowName));
		ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
		ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
		ret_v2.push_back(new CExpr(m_pFunc, EXPR_TYPE_SIZE_OF,
			extendedTypeVarCreateFromExtendedType(extendedTypeCreateFromType(typeCreateUserDefined(CALL_PARAM_STRUCT_PREFIX + m_pFunc->getName())))));

		m_pFunc->insertChildAt(1, new CStatement(m_pFunc, STATEMENT_TYPE_DEF, new CVarDef(m_pFunc, LOCALVAR_NAME, pTypeDef2, dupSourceTreeNode(pDeclVarNode),
			new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pTypeDef2,
				new CExpr(NULL, EXPR_TYPE_FUNC_CALL, m_pFunc->findFuncDeclare(FLOW_FUNC_ENTER_FUNC_NAME, -1), ret_v2)
			)
		)));

		// setting parameters into struct, pFuncData->.... = ...;
		for (int i = 0; i < m_pFunc->getFuncType()->getFuncParamCount(); i++)
		{
			CVarDef* pVarDef = m_pFunc->getFuncType()->getFuncParamAt(i);
			m_pFunc->insertChildAt(2 + i, new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
				new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT, new CExpr(NULL, EXPR_TYPE_TOKEN, LOCALVAR_NAME), pVarDef->getName()),
				new CExpr(NULL, EXPR_TYPE_TOKEN, pVarDef->getName())
			)));
		}
	}

	g_solvedBlocks.push_back(m_pFunc);

	/*if (!pRootFunc)
		return;

	clearAllVars();
	resetTempVarName();
	resetSignalNo();
	resetSubFuncCounter();

	// pFlow = __flow_start(NULL);
	ret_v.clear();
	ret_v.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
	std::string flowName = getTempVarName();
	pRootFunc->addChild(new CStatement(pRootFunc, STATEMENT_TYPE_DEF, new CVarDef(pRootFunc, flowName, g_type_def_void_ptr, NULL, new CExpr(
		pRootFunc, EXPR_TYPE_FUNC_CALL, pRootFunc->findFuncDeclare(FLOW_START_FUNC_NAME, -1), ret_v
	))));

	// Struct* pFuncData = (Struct*)__flow_func_enter(pFlow, NULL, NULL, 0, sizeof(Struct));
	std::vector<CGrammarObject*> ret_v2;

	TypeDefPointer pBaseTypeDef = pRootFunc->findTypeDef(CALL_PARAM_STRUCT_PREFIX + m_pFunc->getName());
	MY_ASSERT(pBaseTypeDef);
	pBaseTypeDef = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, NULL));

	SourceTreeNode* pDeclVarNode = declVarCreateByName("");
	declVarAddModifier(pDeclVarNode, DVMOD_TYPE_POINTER);
	TypeDefPointer pTypeDef2 = TypeDefPointer(new CTypeDef(pBaseTypeDef->getName(), pBaseTypeDef, pDeclVarNode));

	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, flowName));
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, EMPTY_POINTER));
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
	ret_v2.push_back(new CExpr(pRootFunc, EXPR_TYPE_SIZE_OF,
		extendedTypeVarCreateFromExtendedType(extendedTypeCreateFromType(typeCreateUserDefined(CALL_PARAM_STRUCT_PREFIX + m_pFunc->getName())))));

	std::string funcName = getTempVarName();
	pRootFunc->addChild(new CStatement(pRootFunc, STATEMENT_TYPE_DEF, new CVarDef(pRootFunc, funcName, pTypeDef2, dupSourceTreeNode(pDeclVarNode),
		new CExpr(NULL, EXPR_TYPE_TYPE_CAST, pTypeDef2,
			new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pRootFunc->findFuncDeclare(FLOW_FUNC_ENTER_FUNC_NAME, -1), ret_v2)
		)
	)));

	// setting parameters into struct, pFuncData->.... = ...;
	for (int i = 0; i < pRootFunc->getFuncType()->getFuncParamCount(); i++)
	{
		CVarDef* pVarDef = pRootFunc->getFuncType()->getFuncParamAt(i);
		pRootFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_EXPR, new CExpr(NULL, EXPR_TYPE_ASSIGN,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_TOKEN, funcName),
				pVarDef->getName()),
			new CExpr(NULL, EXPR_TYPE_TOKEN, pVarDef->getName())
		)));
	}

	// func(0, pFuncData);
	ret_v2.clear();
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_CONST_VALUE, ltoa(__FLOW_CALL_SIGNAL_FIRST_ENTER)));
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, funcName));
	pRootFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_EXPR,
		new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pRootFunc->findFuncDeclare(m_pFunc->getName(), -1), ret_v2)
	));

	// type ret = pFuncData->ret;
	std::string retVarName;
	TypeDefPointer pReturnType = pRootFunc->getFuncType()->getFuncReturnType();
	if (!pReturnType->isVoid())
	{
		retVarName = getTempVarName();
		pRootFunc->addChild(new CStatement(pRootFunc, STATEMENT_TYPE_DEF, new CVarDef(NULL, retVarName, pReturnType, NULL,
			new CExpr(NULL, EXPR_TYPE_PTR_ELEMENT,
				new CExpr(NULL, EXPR_TYPE_TOKEN, funcName),
				PARAM_NAME_CALLER_RET))
		));
	}

	// __flow_func_leave(pFlow, pVar);
	ret_v2.clear();
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, flowName));
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, funcName));
	pRootFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_EXPR,
		new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pRootFunc->findFuncDeclare(FLOW_FUNC_LEAVE_FUNC_NAME, -1), ret_v2)
	));

	// flow_end(pFlow)
	ret_v2.clear();
	ret_v2.push_back(new CExpr(NULL, EXPR_TYPE_TOKEN, flowName));
	pRootFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_EXPR,
		new CExpr(NULL, EXPR_TYPE_FUNC_CALL, pRootFunc->findFuncDeclare(FLOW_END_FUNC_NAME, -1), ret_v2)
	));

	// return ret;
	if (!retVarName.empty())
	{
		pRootFunc->addChild(new CStatement(NULL, STATEMENT_TYPE_RETURN,
			new CExpr(NULL, EXPR_TYPE_TOKEN, retVarName)
		));
	}

	g_solvedBlocks.push_back(pRootFunc);*/
}

std::string onFuncRead(CGrammarObject* pObject, int depth)
{
	if (pObject->getNodeType() != SCOPE_TYPE_FUNC)
		return pObject->toString(depth);

	CFunction* pFunc = (CFunction*)pObject;

	//printf("onFuncRead, flowType=%d\n", pFunc->getFlowType());
	if (pFunc->getFlowType() == FLOW_TYPE_NONE)
		return pFunc->toString(depth);

	g_solvedBlocks.clear();

	CMyFunc* pMyFunc = new CMyFunc(pFunc);
	pMyFunc->analyze();

	std::string ret_s;
	BOOST_FOREACH(CGrammarObject* pObj, g_solvedBlocks)
	{
		ret_s += pObj->toString(depth);
	}

	return ret_s;
}

int main(int argc, char* argv[])
{
	argc--;
	argv++;

	semanticInit(onFuncRead);

	DIR *d;
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
	closedir(d);

	//printf("===================print logic object tree=============\n");
	//dump_htmls();

	//printf("done\n");
	return 0;
}

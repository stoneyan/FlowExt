#ifndef __PARSER__H_
#define __PARSER__H_

#include "semantic.h"
#include <set>

const std::string EMPTY_POINTER                 = "0";
const std::string FLOW_FUNC_TYPE_NAME           = "FLOW_FUNC";
const std::string FLOW_OBJECT_TYPE_NAME         = "FLOW_OBJECT";
const std::string FUNC_DEF_STRUCT_PREFIX        = "__FLOW_FUNC_DEF_";
const std::string FUNC_IMPL_STRUCT_PREFIX       = "__FLOW_FUNC_IMPL_";
const std::string LOCALVAR_STRUCT_PREFIX        = "__FLOW_LOCALVARS_";
const std::string TEMP_VAR_PREFIX               = "__flow_temp_";
const std::string PARAM_VAR_NAME_SIGNAL         = "__flow_signal";
const std::string PARAM_VAR_NAME_DATA           = "__flow_param";
const std::string LOCALVAR_NAME                 = "__flow_func_var";
//const std::string FIRST_SIGNAL_NAME             = "__FLOW_CALL_SIGNAL_FIRST_ENTER";
const std::string FLOW_BASIC_TYPE_NAME          = "__FLOW_FUNC_BLOCK";
const std::string FLOW_BASIC_MEMBER_NAME        = "__flow_basic";
const std::string FLOW_BASIC_FLOW_MEMBER_NAME   = "this_flow";
const std::string PARAM_NAME_THIS_FUNC          = "this_func";
const std::string PARAM_NAME_CALLER_FUNC        = "caller_func";
const std::string PARAM_NAME_CALLER_DATA        = "caller_data";
const std::string PARAM_NAME_CALLER_SIGNAL      = "caller_signal";
const std::string FLOW_DELETE_COUNTER			= "delete_counter";
const std::string PARAM_NAME_CALLER_RET         = "__flow_caller_ret";
const std::string FLOW_WAIT_FUNC_NAME           = "__flow_wait";
const std::string FLOW_CANCEL_WAIT_FUNC_NAME    = "flow_cancel_wait";
const std::string FLOW_FORK_PARENT_PARAM_NAME   = "__flow_parent_param";
const std::string PREFIX_OF_FLOW_ROOT_FUNC      = "__flow_";
const std::string FLOW_START_FUNC_NAME          = "__flow_start";
const std::string FLOW_END_FUNC_NAME            = "__flow_end";
const std::string FLOW_FUNC_ENTER_FUNC_NAME     = "__flow_func_enter";
const std::string FLOW_FUNC_EXPAND_STACK        = "__flow_func_expand_stack";
const std::string FLOW_FUNC_LEAVE_FUNC_NAME     = "__flow_func_leave";
const std::string FLOW_PARAM_NAME_CALLER_THIS   = "__flow_caller_this";
const std::string FLOW_STATIC_METHOD_PREFIX     = "__flow_static_";
const std::string FLOW_DELETE_TABLE             = "__flow_delete_table";

#define __FLOW_CALL_SIGNAL_DELETE_OBJECT  0x80000000
#define __FLOW_CALL_SIGNAL_DELETE_OBJECT_STR  "0x80000000"

#define __FLOW_CALL_SIGNAL_FIRST_ENTER  0

struct NewVarInfo {
	bool bParam;
	bool bRef;
	std::string new_name;
};

typedef std::map<std::string, NewVarInfo> LocalVarSet;

class CMyFunc
{
public:

	CMyFunc(CFunction* pFunc, bool bFork = false);

    CFunction* getFunc() { return m_pFunc; }
    std::string getFuncName() { return m_pFunc->getName(); }
    bool isFork() { return m_bFork; }

    void clearAllVars() { m_var_list.clear(); }
    bool findVar(std::string name) { return m_var_list.find(name) != m_var_list.end(); }
    void addVar(std::string name) { m_var_list.insert(name); }
    std::string getTempVarName() { return TEMP_VAR_PREFIX + ltoa(m_temp_var_index++); }
    bool isTempVarName(const std::string& name) { return name.substr(0, TEMP_VAR_PREFIX.size()) == TEMP_VAR_PREFIX; }
    int allocSignalNo() { return m_signal_index++; }
    int getNextSignalNo() { return m_signal_index; }
    std::string getNewSubFuncName()
    {
        int n = ++m_sub_func_counter;
        return "_flow_" + getFuncName() + "_sub" + ltoa(n);
    }

    void setStructType(TypeDefPointer pTypeDef) { m_pStructTypeDef = pTypeDef; }
    TypeDefPointer getStructType() { return m_pStructTypeDef; }

    ScopeVector analyze();

    ScopeVector checkFuncGetParamStructDef();
    void replaceExprWithStatement(ScopeVector& ret_v, CExpr*& pExpr, unsigned signalNo);
    void checkExprDecompose(ScopeVector& ret_v, CExpr*& pExpr, unsigned& inAndOutSignalNo, const LocalVarSet& local_var_names);
    void checkStatementDecompose(ScopeVector& ret_v, CStatement* pStatement, unsigned nContinueSignal, unsigned& inAndOutSignalNo, LocalVarSet& local_var_names, unsigned& local_obj_var_cnt);
    void checkDupNames(CStatement* pStatement/*, std::string& pre_string*/);
    void checkNonFlowStatementDecompose(ScopeVector& ret_v, CStatement* pStatement, unsigned nContinueSignal, LocalVarSet& local_var_names, unsigned local_obj_var_cnt);
	void checkNonFlowExprDecompose(CExpr* pExpr, const LocalVarSet& local_var_names);
    void addToStatementVector(ScopeVector& ret_v, CStatement* pNewStatement, unsigned nSignalNo);
    void addBatchToStatementVector(ScopeVector& ret_v, ScopeVector& new_v, unsigned nSignalNo);
	CFuncDeclare* findFuncDecl(const std::string& func_name);
	TypeDefPointer findTypeDef(const std::string& type_name);
	CExpr* getMyCallFuncName();

protected:
    CFunction*      m_pFunc;
    bool            m_bFork;

    std::set<std::string> m_var_list;
    unsigned		m_temp_var_index;
	unsigned		m_local_obj_var_index;
    unsigned		m_signal_index;
    unsigned		m_sub_func_counter;
    TypeDefPointer  m_pStructTypeDef;

	std::vector<CScope*>	m_result_v;

	std::vector<unsigned>	m_continue_stack;
	std::vector<unsigned>	m_break_stack;
};

CExpr* composeSignalLogicExpr(CExpr* pExpr, int nSignalEnter, int nSignalStart, int nSignalEnd);
bool isVarDefinedBetweenScopeAndFunction(const std::string& varName, CGrammarObject* pScope, CGrammarObject* pCurrent, bool bOutsideScope);
void changeVarToUnderStructInExpr(CGrammarObject* pScope, const std::string& struct_name, CExpr* pExpr);
void changeVarToUnderStructInStatement(CGrammarObject* pScope, const std::string& struct_name, CStatement* pStatement);

/*
typedef void (*__FLOW_FUNC)(int, void*);

#define __FLOW_CALL_SIGNAL_FIRST_ENTER  0

struct __FLOW_CALLERPARAMS_http_get {
    __FLOW_FUNC __flow_caller_func;
    void*       __flow_caller_data;
    int         __flow_caller_signal;
    const char* url;
};

struct __FLOW_LOCALVARS_http_get {
    __FLOW_CALLERPARAMS_http_get __flow_call_param;
    int s;
    struct sockaddr_in ret_addr;
    int ret;
    char* http_request;
    char* http_response;
};

const std::string FLOW_FUNC_TYPE_NAME       = "FLOW_FUNC";
const std::string CALL_PARAM_STRUCT_PREFIX  = "__FLOW_CALLERPARAMS_";
const std::string LOCALVAR_STRUCT_PREFIX    = "__FLOW_LOCALVARS_";
const std::string TEMP_VAR_PREFIX           = "__FLOW_TEMP_VAR_";
const std::string PARAM_VAR_NAME_SIGNAL     = "__flow_signal";
const std::string PARAM_VAR_NAME_DATA       = "__flow_param";
const std::string LOCALVAR_NAME             = "__flow_local_var";
//const std::string LOCALVAR_NAME_PARAM     = "__flow_param_var";
const std::string FIRST_SIGNAL_NAME         = "__FLOW_CALL_SIGNAL_FIRST_ENTER";
const std::string PARAM_NAME_CALLER_FUNC    = "__flow_caller_func";
const std::string PARAM_NAME_CALLER_DATA    = "__flow_caller_data";
const std::string PARAM_NAME_CALLER_SIGNAL  = "__flow_caller_signal";
const std::string PARAM_NAME_CALLER_RET     = "__flow_caller_ret";
const std::string FLOW_WAIT_FUNC_NAME          = "__flow_wait";


==========================================
example 1:

a();
flow_wait(&obj, param);
b();

transform to:

if (signal <= 0)
{
  a();

  if (!__flow_wait(this_func, &localvar, 1, &obj, param))
    return false;
}

if (signal <= 1)
{
  b();
}


==========================================
example 2:

a();
int ret = flow_func(1);
b();

transform to:

bool flow_func(int signal_no, void* param)
{
    ...
	func_var->ret = ...
    if (signal_no != 0 && pVar->basic.caller_func != NULL)
        pVar->basic.caller_func(pVar->basic.caller_signal, pVar->basic.caller_data);
    return signal_no == 0;
}


if (signal <= 0)
{
  a();

  pVar->pTemp1 = new struct;
  pVar->pTemp1->basic.flow = pVar->basic.pFlow;
  pVar->pTemp1->basic.caller_func = this_func;
  pVar->pTemp1->basic.caller_data = pVar;
  pVar->pTemp1->basic.caller_signal = 0;
  pVar->pTemp1->param = ....
  __flow_func_enter(pVar->basic.pFlow, pVar->pTemp1);
  __flow_func_var->__flow_temp_0 = ...
  if (!flow_func(1, fp))
    return false;
  signal = 1;
}

if (signal <= 1)
{
  ret = pStruct->ret;
  __flow_func_leave(pVar->basic.pFlow, pVar->pTemp1);

  b();
}


==========================================
example 3:

a();
if (a < b)
{
  b();
  flow(1);
  c();
}
else if (a < b2)
{
  if (a < b1)
  {
    b();
    flow(2);
  }
  c();
  flow(3);
}
else
{
  b();
  flow(4);
  c();
}
d();

transform to:

if (signal <= 0)
{
  a();
}

  if (signal <= 0 && (a < b) || signal == 1)
  {
    if (signal <= 0)
    {
      b();
      if (!flow(1))
        return false;
      signal = 1;
    }
    c();
  }
  else if (signal <= 0 && (a < b2) || signal >= 2 && signal <= 3)
  {
    if (signal <= 0 && (a < b1) || signal <= 2)
    {
      b();
      if (!flow(2))
        return false;
      signal = 2;
    }
    if (signal <= 2)
    {
      c();
      if (!flow(3))
        return false;
      signal = 3;
    }
  }
  else if (signal <= 0 || signal == 4)
  {
    if (signal == 0)
    {
      b();
      if (!flow(4))
        return false;
      signal = 4;
    }
    if (signal == 4)
      c();
  }

if (signal <= 4)
  d();


==========================================
example 4: while, for, do

transform to:

if (signal <= 0)
{
  expr1; // for only
}
int bMode = signal <= 0 ? 0 : 1;
while (signal <= 3)
{
  if (bMode > 1 && signal > 1)
    signal = 1;
  if (signal <= 1)
  {
	if (bMode != 0) // for only
      expr3;
    if (!expr) // while and for only
      break;
    if (bMode != 0 && !expr) // do only
      break;
  }
  bMode = 2;

  ...
}

===========================================
flow int aaa()
{
  n = 5;

  a();
  flow_fork {
    b();
    if (n > 3)
      return;
    d();
  }

  a2();
}

translate to:

bool aaa_sub1(int signal_no, void* param)
{
    for return:
    __flow_end(pVar->basic.pFlow);
    return false;
}

bool aaa(int signal_no, void* param)
{
  pVar = (struct_aaa*)param;

  if (signal_no <= 0)
  {
    if (!a())
      return false;
    signal_no = 1;
  }

  if (signal_no <= 1)
  {
    pTempFlow = __flow_start(pVar->basic.pFlow);
    struct* pTempFunc = __flow_func_enter(pTempFlow, NULL, NULL, 0, sizeof(struct));
    aaa_sub1(0, pTempFunc);

    a2();
  }
}

========================================
flow int aaa()
{
  n = 5;

  a();
  void* ptr = flow_new {
    b();
    if (n > 3)
      return;
    d();
  }

  a2();
}

translate to:

bool aaa_sub1(int signal_no, void* param)
{
    for return:
    __flow_end(pVar->basic.pFlow);
    return false;
}

bool aaa(int signal_no, void* param)
{
  pVar = (struct_aaa*)param;

  if (signal_no <= 0)
  {
    if (!a())
      return false;
    signal_no = 1;
  }

  if (signal_no <= 1)
  {
    ptr = __flow_start(NULL);
    struct* pTempFunc = __flow_func_enter(pTempFlow, NULL, NULL, 0, sizeof(struct));
    aaa_sub1(0, pTempFunc);

    a2();
  }
}


===========================================
flow_root int main(int argc, char* argv[])
{
    flow_fork
    {
	    ...
    }

	loop();
    return 0;
}

translate to:

int main(int argc, char* argv[])
{
    pFlow = __flow_start(NULL);
    struct* pFuncVar = __flow_func_enter(pFlow, NULL, NULL, 0, sizeof(struct));
    pFuncVar->argc = argc;
    pFuncVar->argv = argv;
    pFuncVar->temp1 = __flow_func_enter(pFlow, NULL, NULL, 0, sizeof(struct main_sub1));
    __flow_main_sub1(0, pFuncVar->temp1);

    loop();
    __flow_end(pFlow);
    return ret;
}


============================================
flow_try {
    a();
    if (n > 5)
      return 3;
    b();
}
flow_catch (&object1, param1)
{
}
flow_catch (&object2, param2)
{
}

translate to:

if (signal <= 0)
{
    pFuncVar->sub_flow = __flow_start(pVar->basic.flow);
	pFuncVar->pTempWait1 = &object1;
	pFuncVar->pTempWait2 = &object2;
    if (__flow_wait(pFuncVar->pTempWait1, this_func, pFuncVar, 1, param1))
      signal = 1;
    else if (__flow_wait(pFuncVar->pTempWait2, this_func, pFuncVar, 2, param2))
      signal = 2;
    else
    {
      struct* pFuncTemp = __flow_func_enter(sub_flow, caller_func, caller_data, 3, sizeof(struct));
      pFuncTemp->parent_param = pFuncVar;
      if (!func_sub1(0, pFuncTemp))
        return false;
    }
}
if (signal == 1)
{
    flow_end(pFuncVar->sub_flow);
    flow_cancel_wait(pFuncVar->pTempWait2);
    ...
}
else if (signal == 2)
{
    flow_end(pFuncVar->sub_flow);
    flow_cancel_wait(pFuncVar->pTempWait1);
    ...
}
else if (signal == 0 || signal == 3)
{
    flow_end(pFuncVar->sub_flow);
    flow_cancel_wait(pFuncVar->pTempWait1);
    flow_cancel_wait(pFuncVar->pTempWait2);
}

============================================

class A
{
	flow int b();
};

A a;
a.b();

translate to:

class A
{
	static bool __flow_static_b(int signal, void* param)
	{
		return ((struct *)param)->__flow_this->__flow_b(signal, param);
	}
	bool b(int signal, void* param);
};

  A a;
  pVar->pTemp1 = new struct;
  pVar->pTemp1->basic.flow = pVar->basic.pFlow;
  pVar->pTemp1->basic.caller_func = this_func;
  pVar->pTemp1->basic.caller_data = pVar;
  pVar->pTemp1->basic.caller_signal = 0;
  pVar->pTemp1->__flow_this = &a;
  pVar->pTemp1->param = ....
  __flow_func_enter(pVar->basic.pFlow, pVar->pTemp1);
  __flow_func_var->__flow_temp_0 = ...
  if (!a.b(1, fp))
    return false;
  signal = 1;
*/
#endif // __PARSER__H_

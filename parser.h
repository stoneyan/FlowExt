#ifndef __PARSER__H_
#define __PARSER__H_

#include "semantic.h"
#include <set>

const std::string EMPTY_POINTER                 = "__null";
const std::string FLOW_FUNC_TYPE_NAME           = "FLOW_FUNC";
const std::string FLOW_OBJECT_TYPE_NAME           = "FLOW_OBJECT";
const std::string CALL_PARAM_STRUCT_PREFIX      = "__FLOW_CALLERPARAMS_";
const std::string LOCALVAR_STRUCT_PREFIX        = "__FLOW_LOCALVARS_";
const std::string TEMP_VAR_PREFIX               = "__flow_temp_";
const std::string PARAM_VAR_NAME_SIGNAL         = "__flow_signal";
const std::string PARAM_VAR_NAME_DATA           = "__flow_param";
const std::string LOCALVAR_NAME                 = "__flow_func_var";
//const std::string FIRST_SIGNAL_NAME             = "__FLOW_CALL_SIGNAL_FIRST_ENTER";
const std::string FLOW_BASIC_TYPE_NAME          = "__FLOW_FUNCTION_BLOCK";
const std::string FLOW_BASIC_MEMBER_NAME        = "__flow_basic";
const std::string FLOW_BASIC_FLOW_MEMBER_NAME   = "flow_block";
const std::string PARAM_NAME_CALLER_FUNC        = "caller_func";
const std::string PARAM_NAME_CALLER_DATA        = "caller_data";
const std::string PARAM_NAME_CALLER_SIGNAL      = "caller_signal";
const std::string PARAM_NAME_CALLER_RET         = "__flow_caller_ret";
const std::string FLOW_WAIT_FUNC_NAME           = "__flow_wait";
const std::string FLOW_CANCEL_WAIT_FUNC_NAME    = "flow_cancel_wait";
const std::string FLOW_FORK_PARENT_PARAM_NAME   = "__flow_parent_param";
const std::string PREFIX_OF_FLOW_ROOT_FUNC      = "__flow_";
const std::string FLOW_START_FUNC_NAME          = "__flow_start";
const std::string FLOW_END_FUNC_NAME            = "__flow_end";
const std::string FLOW_FUNC_ENTER_FUNC_NAME     = "__flow_func_enter";
const std::string FLOW_FUNC_LEAVE_FUNC_NAME     = "__flow_func_leave";

#define __FLOW_CALL_SIGNAL_FIRST_ENTER  0

class CMyFunc
{
public:
    CMyFunc(CFunction* pFunc, bool bFork = false)
    {
        m_pFunc = pFunc;
        m_bFork = bFork;

        resetTempVarName();
        resetSignalNo();
        resetSubFuncCounter();
    }

    CFunction* getFunc() { return m_pFunc; }
    std::string getFuncName() { return m_pFunc->getName(); }
    bool isFork() { return m_bFork; }

    void clearAllVars() { m_var_list.clear(); }
    bool findVar(std::string name) { return m_var_list.find(name) != m_var_list.end(); }
    void addVar(std::string name) { m_var_list.insert(name); }
    void resetTempVarName() { m_temp_var_index = 0; }
    std::string getTempVarName() { return TEMP_VAR_PREFIX + ltoa(m_temp_var_index++); }
    bool isTempVarName(const std::string& name) { return name.substr(0, TEMP_VAR_PREFIX.size()) == TEMP_VAR_PREFIX; }
    void resetSignalNo() { m_signal_index = 1; }
    int allocSignalNo() { return m_signal_index++; }
    int getNextSignalNo() { return m_signal_index; }
    void resetSubFuncCounter() { m_sub_func_counter = 0; }
    std::string getNewSubFuncName()
    {
        int n = ++m_sub_func_counter;
        return "_flow_" + getFuncName() + "_sub" + ltoa(n);
    }

    void setStructType(TypeDefPointer pTypeDef) { m_pStructTypeDef = pTypeDef; }
    TypeDefPointer getStructType() { return m_pStructTypeDef; }

    void analyze();

    CStatement* checkFuncGetParamStructDef();
    void replaceExprWithStatement(GrammarObjectVector& ret_v, CExpr*& pExpr, int signalNo);
    void checkExprDecompose(GrammarObjectVector& ret_v, CExpr*& pExpr, int& inAndOutSignalNo);
    void checkStatementDecompose(GrammarObjectVector& ret_v, CStatement* pStatement, int nContinueSignal, int& inAndOutSignalNo);
    void checkDupNames(CStatement* pStatement/*, std::string& pre_string*/);
    void checkNonFlowStatementDecompose(GrammarObjectVector& ret_v, CStatement* pStatement, int nContinueSignal);
    void addToStatementVector(std::vector<CGrammarObject*>& ret_v, CStatement* pNewStatement, int nSignalNo);
    void addBatchToStatementVector(GrammarObjectVector& ret_v, GrammarObjectVector& new_v, int nSignalNo);

protected:
    CFunction*      m_pFunc;
    bool            m_bFork;

    std::set<std::string> m_var_list;
    int             m_temp_var_index;
    int             m_signal_index;
    int             m_sub_func_counter;
    TypeDefPointer  m_pStructTypeDef;
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

  if (!flow_wait(this_func, &localvar, 1, &obj, param))
    return;
  signal =  1;
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
    for return:

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
  if (!flow(1))
    return false;
  signal = 1;
}

if (signal <= 1)
{
  ret = pStruct->ret;
  __flow_func_leave(pVar->basic.pFlow, pVar->pTemp1);
  delete pVar->pTemp1;

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
example 4: while

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


==========================================
example 5:

a();
while (a < b)
{
  b();
  if (a < c)
  {
    flow(1);
    if (c > d)
      break;
    flow(2);
    continue;
  }
  d();
  wait(2);
}

transform to:

if (signal == 0)
  a();

while (signal == 0 && a < b || signal >= 1 && signal <= 3)
{
  if (signal == 0)
  {
    b();
  }
  if (signal == 0 && a < c || signal >= 1 && signal <= 2)
  {
    if (signal == 0)
    {
      if (!flow(1))
        return false;
      signal = 1;
    }
    if (c > d)
    {
      signal = 0;
      break;
    }
    if (signal == 1)
    {
      if (!flow(2))
        return false;
      signal = 2;
    }
    if (b < d)
    {
      signal = 0;
      continue;
    }
  }
  if (signal == 2)
    signal = 0;
  if (signal == 0)
  {
    d();
    if (!wait(3))
      return false;
    signal = 3;
  }
  if (signal == 3)
    signal = 0;
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
    pTempFunc->parent_param = pFuncVar;
    aaa_sub1(0, pTempFunc);

    d2();
  }
}


===========================================
flow_root int main(int argc, char* argv[])
{
	flow_fork {
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
    pFuncVar->temp1 = __flow_func_enter(pFlow, NULL, NULL, 0, sizeof(struct sub1));
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
      signal = 3;
    }
}
if (signal == 1)
{
    flow_cancel_wait(pFuncVar->pTempWait2);
    flow_end(pFuncVar->sub_flow);
    ...
}
else if (signal == 2)
{
    flow_cancel_wait(pFuncVar->pTempWait1);
    flow_end(pFuncVar->sub_flow);
    ...
}
else if (signal == 3)
{
    flow_end(pFuncVar->sub_flow);
    flow_cancel_wait(pFuncVar->pTempWait1);
    flow_cancel_wait(pFuncVar->pTempWait2);
}

...
*/
#endif // __PARSER__H_

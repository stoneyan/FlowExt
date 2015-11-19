#ifndef _FLOW_IMPL_H_
#define _FLOW_IMPL_H_

extern "C" {

typedef bool (*FLOW_FUNC)(unsigned, void*);

struct FLOW_OBJECT {
	FLOW_FUNC	callback_func;
	void*       callback_data;
    unsigned    callback_signal;
	void*		param;
	int   		value;
	void*		waiting_flow;
	FLOW_OBJECT*	prev_waiting_obj;
	FLOW_OBJECT*	next_waiting_obj;
};

void flow_init(FLOW_OBJECT* pObj);
void flow_signal(FLOW_OBJECT* pObj, void* param);
void flow_delete(void* pFlow);
void flow_cancel_wait(FLOW_OBJECT* pObj);

struct __FLOW_FUNC_BLOCK {
    void*        this_flow;
	FLOW_FUNC	 this_func;
    FLOW_FUNC    caller_func;
    void*        caller_data;
    unsigned     caller_signal;
	unsigned     delete_counter;
};

bool __flow_wait(FLOW_OBJECT* pObj, FLOW_FUNC callback_func, unsigned signal, void* callback_data, void** param);
void* __flow_start(void* pFlow, unsigned long stack_size); // 0 means using default value
void __flow_end(void* pFlow);
void* __flow_func_enter(void* pFlow, FLOW_FUNC caller_func, void* caller_data, unsigned caller_signal, long block_size);
void __flow_func_expand_stack(void* pFlow, void* caller_data, long block_size);
void __flow_func_leave(void* pFlow, void* pFuncBlock);

}

#endif // _FLOW_IMPL_H_


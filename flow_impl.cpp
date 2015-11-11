#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "flow_impl.h"

#define TRACE(fmt)	do { if (false) printf(fmt); } while (false)
#define TRACE2(fmt, ...)	do { if (false) printf(fmt, __VA_ARGS__); } while (false)

#define __FLOW_CALL_SIGNAL_DELETE_OBJECT  0x80000000
#define FLOW_STACK_DEFAULT_SIZE		512000

struct __FlowBlock
{
	__FlowBlock*			parent_flow;
	__FlowBlock*			next_flow;
	__FlowBlock*			prev_flow;
	__FlowBlock*			sub_flows;
	__FLOW_FUNCTION_BLOCK*	cur_stack;
	FLOW_OBJECT*			first_waiting_object;
	unsigned long			stack_allocated;
	unsigned long			stack_used;
	char*					stack;
};

void __flow_assert(bool b)
{
	assert(b);
}

void flow_init(FLOW_OBJECT* pObj)
{
  	memset(pObj, 0, sizeof(FLOW_OBJECT));
}

void flow_signal(FLOW_OBJECT* pObj, void* param)
{
	TRACE2("flow_signal, pObj=0x%lx, ", (unsigned long)pObj);
	FLOW_FUNC callback_func = pObj->callback_func;
	if (callback_func)
	{
		TRACE("already wait, call it\n");
		flow_cancel_wait(pObj);
		pObj->callback_func = NULL;
		if (pObj->param)
			*((void**)pObj->param) = param;
		callback_func(pObj->callback_signal, pObj->callback_data);
	}
	else
	{
		TRACE("not waiting, set flag\n");
		pObj->value = 1;
		pObj->param = param;
	}
}

bool __flow_wait(FLOW_OBJECT* pObj, FLOW_FUNC callback_func, unsigned signal, void* callback_data, void** param)
{
	TRACE2("__flow_wait, pObj=0x%lx, ", (unsigned long)pObj);
	__FLOW_FUNCTION_BLOCK* pCallingBlock = (__FLOW_FUNCTION_BLOCK*)callback_data;

  	if (pObj->value)
  	{
		TRACE("already signaled, return\n");
		pObj->value = 0;
		if (param)
			*param = pObj->param;
		return true;
  	}

	TRACE("not signaled, wait\n");
	__FlowBlock* pCallingFlow = (__FlowBlock*)pCallingBlock->flow_block;
	pObj->next_waiting_obj = pCallingFlow->first_waiting_object;
	if (pCallingFlow->first_waiting_object)
		pCallingFlow->first_waiting_object->prev_waiting_obj = pObj;
  	pCallingFlow->first_waiting_object = pObj;

	pObj->waiting_flow = pCallingFlow;
	pObj->callback_func = callback_func;
	pObj->callback_data = callback_data;
	pObj->callback_signal = signal;
	pObj->param = (void*)param;
	return false;
}

void flow_cancel_wait(FLOW_OBJECT* pObj)
{
	if (!pObj->callback_func)
		return;

	__FlowBlock* pWaitingFlow = (__FlowBlock*)pObj->waiting_flow;
	__flow_assert(pWaitingFlow != NULL);
	if (pWaitingFlow->first_waiting_object == pObj)
	{
		__flow_assert(pObj->prev_waiting_obj == NULL);
		pWaitingFlow->first_waiting_object = pObj->next_waiting_obj;
	}
	else if (pObj->prev_waiting_obj)
		pObj->prev_waiting_obj->next_waiting_obj = pObj->next_waiting_obj;
	if (pObj->next_waiting_obj)
		pObj->next_waiting_obj->prev_waiting_obj = pObj->prev_waiting_obj;

	pObj->prev_waiting_obj = pObj->next_waiting_obj = NULL;
	pObj->value = 0;
}

void* __flow_start(void* parent_flow, unsigned long stack_size)
{
	__FlowBlock* pParentFlow = (__FlowBlock*)parent_flow;
	if (stack_size == 0)
		stack_size = FLOW_STACK_DEFAULT_SIZE;
	__FlowBlock* pFlow = (__FlowBlock*)malloc(sizeof(__FlowBlock) + stack_size);
	memset(pFlow, 0, sizeof(__FlowBlock));
	pFlow->parent_flow = pParentFlow;
	if (pParentFlow)
	{
		pFlow->next_flow = pParentFlow->sub_flows;
		if (pParentFlow->sub_flows)
			pParentFlow->sub_flows->prev_flow = pFlow;
		pParentFlow->sub_flows = pFlow;
	}
	pFlow->stack = (char*)pFlow + sizeof(__FlowBlock);
	pFlow->stack_allocated = stack_size;
	TRACE2("__flow_start, parent=0x%lx, child=0x%lx\n", (long)pParentFlow, (long)pFlow);
	return pFlow;
}

void* __flow_func_enter(void* flow, FLOW_FUNC caller_func, void* caller_data, unsigned caller_signal, long block_size)
{
	__FlowBlock* pFlow = (__FlowBlock*)flow;

	__flow_assert(pFlow->cur_stack == NULL || caller_data == pFlow->cur_stack);
	__flow_assert(pFlow->stack_used + block_size < pFlow->stack_allocated);
	__FLOW_FUNCTION_BLOCK* pFuncBlock = (__FLOW_FUNCTION_BLOCK*)(pFlow->stack + pFlow->stack_used);
	TRACE2("__flow_func_enter, flow=0x%lx, parent=0x%lx, func=0x%lx\n", (long)flow, (long)caller_data, (long)pFuncBlock);
	pFuncBlock->flow_block = pFlow;
	pFuncBlock->this_func = NULL;
	pFuncBlock->caller_func = caller_func;
	pFuncBlock->caller_data = caller_data;
	pFuncBlock->caller_signal = caller_signal;
	pFuncBlock->delete_counter = 0;
	pFlow->cur_stack = pFuncBlock;
	pFlow->stack_used += block_size;

	return pFuncBlock;
}

void __flow_func_expand_stack(void* flow, void* caller_data, long block_size)
{
	__FlowBlock* pFlow = (__FlowBlock*)flow;

	__flow_assert(caller_data == pFlow->cur_stack);
	__flow_assert(block_size >= (char*)pFlow->cur_stack - (char*)caller_data);
	block_size -= (char*)pFlow->cur_stack - (char*)caller_data;
	__flow_assert(pFlow->stack_used + block_size < pFlow->stack_allocated);
	pFlow->stack_used += block_size;
}

void __flow_func_leave(void* flow, void* func_data)
{
	TRACE2("__flow_func_leave, flow=0x%lx, func=0x%lx\n", (long)flow, (long)func_data);
	__FlowBlock* pFlow = (__FlowBlock*)flow;
	__FLOW_FUNCTION_BLOCK* pFuncBlock = (__FLOW_FUNCTION_BLOCK*)func_data;

	__flow_assert(pFuncBlock == pFlow->cur_stack);

	pFlow->stack_used = (char*)pFuncBlock - pFlow->stack;
	pFlow->cur_stack = (__FLOW_FUNCTION_BLOCK*)pFuncBlock->caller_data;
}

// delete the whole stack, all sub_flows, remove itself from parent flow
void __flow_end(void* flow)
{
	TRACE2("__flow_end, flow=0x%lx\n", (long)flow);
	__FlowBlock* pFlow = (__FlowBlock*)flow;

	// cancel all waiting objects
	while (pFlow->first_waiting_object)
	{
		flow_cancel_wait(pFlow->first_waiting_object);
	}

	while (pFlow->cur_stack)
	{
		__FLOW_FUNCTION_BLOCK* pPrevStack = (__FLOW_FUNCTION_BLOCK*)pFlow->cur_stack->caller_data;
		TRACE2("__flow_end, flow=0x%lx, delete func=0x%lx, parent=0x%lx\n", (long)flow, (long)pFlow->cur_stack, (long)pPrevStack);
		if (pFlow->cur_stack->this_func)
			pFlow->cur_stack->this_func(__FLOW_CALL_SIGNAL_DELETE_OBJECT, pFlow->cur_stack);
		pFlow->cur_stack = pPrevStack;
	}

	while (pFlow->sub_flows)
	{
		//if (pFlow->sub_flows->waiting_object)
		//{
		TRACE2("__flow_end, flow=0x%lx, deleting sub flow 0x%lx\n", (long)flow, (long)pFlow->sub_flows);
		__flow_end(pFlow->sub_flows);
		//}
		/*else
		{
			TRACE("__flow_end, flow=0x%lx, skipping sub flow 0x%lx\n", (long)flow, (long)pFlow->sub_flows);
			pFlow->sub_flows->parent_flow = NULL;
			pFlow->sub_flows = pFlow->sub_flows->next_flow;
		}*/
	}

	__FlowBlock* pParent = pFlow->parent_flow;
	TRACE2("__flow_end, flow=0x%lx, remove myself from parent flow 0x%lx, my next flow is 0x%lx\n", (long)flow, (long)pParent, (long)pFlow->next_flow);
	if (pParent)
	{
		if (pParent->sub_flows == pFlow)
		{
			pParent->sub_flows = pFlow->next_flow;
			if (pParent->sub_flows)
				pParent->sub_flows->prev_flow = NULL;
		}
		else
		{
			pFlow->prev_flow->next_flow = pFlow->next_flow;
			if (pFlow->next_flow)
				pFlow->next_flow->prev_flow = pFlow->prev_flow;
		}
	}

	TRACE2("__flow_end, delete myself 0x%lx\n", (long)pFlow);
	free(pFlow);
}

void flow_delete(void* flow)
{
	__FlowBlock* pFlow = (__FlowBlock*)flow;

	__flow_assert(pFlow->parent_flow == NULL);
	__flow_end(pFlow);
}


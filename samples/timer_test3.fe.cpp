#include "flow_helper.h"

flow void start_timer(int interval, FLOW_OBJECT* pObj, void* param)
{
	flow_fork
	{
		FLOW_OBJECT* pObj2 = pObj;
		void* param2 = param;

	    CFlowObject fo;
        fo.time_wait(interval);
		//printf("start_timer, signal, pObj=0x%lx\n", pObj2);
		flow_signal(pObj2, param2);
	}
}

flow void heartbeat(int col, int max, int interval)
{
    CFlowObject fo;
    for (int i = 0; i < max; i += interval)
    {
		for (int j = 0; j < col; j++)
			printf("\t");
        printf("%d\n", i);
        fo.time_wait(interval * 1000);
    }
}

flow_root int main(int argc, char** argv)
{
	VSOCK::startup();

	flow_fork
	{
		CFlowObject stop_obj;
		srand(get_cur_tick());

		void* flow_table[4];
		for (int i = 0; i < 4; i++)
		{
			int interval = rand() % 4 + 1;
			int duration = rand() % 30000;
			printf("start flow %d, interval=%d, duration=%d\n", i, interval, duration);
			flow_table[i] = flow_new
			{
				heartbeat(i, 1000, interval);
			}
			//printf("***call start timer, pObj=%lx\n", stop_obj.getObj());
			start_timer(duration, stop_obj.getObj(), (void*)i);
		}

		for (int i = 0; i < 4; i++)
		{
			void* param;
			//printf("***wait for stop_obj, pObj=%lx\n", stop_obj.getObj());
			stop_obj.wait(&param);
			printf("flow %d terminated\n", (int)param);
			flow_delete(flow_table[(int)param]);
		}
		CFlowSocket::stop_event_loop();
	}

    CFlowSocket::event_loop();
    return 0;
}

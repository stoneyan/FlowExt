#include "flow_helper.h"

flow void start_timer(int interval, FLOW_OBJECT* pObj)
{
	flow_fork
	{
	    CFlowObject fo;
        fo.time_wait(interval);
		flow_signal(pObj, NULL);
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
		int duration = rand() % 30000;
		printf("timer is set to %dms\n", duration);
		start_timer(duration, stop_obj.getObj());

		flow_try
		{
			flow_fork
			{
				heartbeat(0, 1000, 2);
			}
			//flow_fork
			//{
				heartbeat(1, 1000, 1);
			//}
		}
		flow_catch(stop_obj.getObj(), NULL)
		{
			printf("program terminated\n");
			CFlowSocket::stop_event_loop();
		}
	}

    CFlowSocket::event_loop();
    return 0;
}

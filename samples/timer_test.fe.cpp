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
		heartbeat(0, 10, 2);
	}
	flow_fork
	{
		heartbeat(1, 10, 1);
		CFlowSocket::stop_event_loop();
	}

    CFlowSocket::event_loop();
    return 0;
}

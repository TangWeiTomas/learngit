#include "PanalarmDevice.h" 

typedef struct PanalarmDevice_s
{
	tu_evtimer_t *evtimer;
	uint8_t IEEEAddr[8];
	uint16_t nwkAddr; 				// Network address
	uint16_t deviceID; 				// Device identifier
	uint8_t endpoint; 				// Endpoint identifier
	uint8_t onlineflag;    			// �ڵ��Ƿ����ߣ�1 ���ߣ�0������
	uint8_t currentState;			//��ǰ״̬
}PanalarmDevice_t;


static PanalarmDevice_t *PanalarmDeviceInfo = NULL;


void PanalarmDevice_ProceeHandler_CB(void *args)
{
	PanalarmDevice_t *panalarmInfo = (PanalarmDevice_t *)args;

	if(panalarmInfo->deviceID == ZB_DEV_INFRARED_BODY_SENSOR)
	{
		SRPC_ComAlarmStateInd(panalarmInfo->IEEEAddr,panalarmInfo->endpoint,panalarmInfo->deviceID,panalarmInfo->currentState);
	}

	tu_evtimer_free(PanalarmDeviceInfo->evtimer);
	free(PanalarmDeviceInfo);
	PanalarmDeviceInfo = NULL;
}


//��������ⱻ�����󣬵�����1min��û���ϱ����ϱ���ǰû��״̬
int8 PanalarmDevice_UpdateDeviceState(epInfo_t *epinfo)
{
	int ret = -1;

	log_debug("PanalarmDevice_UpdateDeviceState++\n");
	
	if(epinfo == NULL)
	{
		log_debug("epinfo is NULL");
		return -1;
	}
		
	if(PanalarmDeviceInfo == NULL)
	{

		PanalarmDeviceInfo = malloc(sizeof(PanalarmDevice_t));
		if(PanalarmDeviceInfo == NULL)
		{
			log_debug("malloc failed\n");
			return -1;
		}

		PanalarmDeviceInfo->evtimer = tu_evtimer_new(main_base_event_loop);
		if(PanalarmDeviceInfo->evtimer == NULL)
		{
			log_debug("tu_evtimer_new failed\n");
			free(PanalarmDeviceInfo);
			return -1;
		}

		memcpy(PanalarmDeviceInfo->IEEEAddr,epinfo->IEEEAddr,8);
		PanalarmDeviceInfo->nwkAddr = epinfo->nwkAddr;
		PanalarmDeviceInfo->endpoint = epinfo->endpoint;
		PanalarmDeviceInfo->deviceID = epinfo->deviceID; 				// Device identifier
		PanalarmDeviceInfo->onlineflag = epinfo->onlineflag;    		// �ڵ��Ƿ����ߣ�1 ���ߣ�0������
		PanalarmDeviceInfo->currentState = 0;							//��ǰ״̬
		
		ret = tu_set_evtimer_realtime(PanalarmDeviceInfo->evtimer,PANALRM_DEVICE_INCOMING_TIMER,ONCE,PanalarmDevice_ProceeHandler_CB,PanalarmDeviceInfo);
		if(ret == -1)
		{
			tu_evtimer_free(PanalarmDeviceInfo->evtimer);
			free(PanalarmDeviceInfo);
			return -1;
		}
		
	}
	else
	{
		ret = tu_reset_evtimer(PanalarmDeviceInfo->evtimer,PANALRM_DEVICE_INCOMING_TIMER,ONCE);
	}

	log_debug("PanalarmDevice_UpdateDeviceState--\n");
	
	return ret;
}


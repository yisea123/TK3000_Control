#include "AppTasks.h"
#include "MainForm.h"
//#include "IOTest.h"
#include "main.h"
#include "usbd_usr.h"
#include "USBFunc.h"
//#include "MoveFileExImportForm.h"
//#include "MainFormCustomMove.h"
//#include "LogFile.h"

portTickType _xLastWakeTime;
xTaskHandle _showRealDataHandle;    //��ʾʵʱ�����������
xTaskHandle _workRunLeftHandle;  	//��ƽ̨����˿���
xTaskHandle _workRunRightHandle;  	//��ƽ̨����˿���
xTaskHandle _positionResetHandle;   //λ�ø�λ���
xTaskHandle _alarmHandle;           //��������˸���
xTaskHandle _IOStateShowHandle;		//IO��״̬��ʾ
xTaskHandle _dateTimeShowHandle;	//������ʱ�����ʾ
xTaskHandle _usbHostHandle;			//USBHost
xTaskHandle _fatfsOperateHandle;	//FATFS��ز�����������,����USB�Ĳ���
xTaskHandle _setUSBDirverHandle;	//��ʼUSB�洢�豸��������
xTaskHandle _inportEventHandle;		//������¼�������

static BaseType_t xHigherPriorityTaskWoken;  
static BaseType_t xHigherPriorityTaskWokenRight;  
//�ش��ź���
SemaphoreHandle_t _xSemaphoreRedo;
//��һ���ź���
SemaphoreHandle_t _xSemaphoreNext;
//�Ƿ��ѽ�����ϵȴ�״̬
bool _isWaitSemaphore=false;
//��ƽ̨����˿����������ڲ��á�������ͨ������arg��ȡ
WorkTaskStartParaStruct _leftStartPara={LeftPlatform,Key,TaskNoRun};
//��ƽ̨����˿����������ڲ��á�������ͨ������arg��ȡ
WorkTaskStartParaStruct _rightStartPara={RightPlatform,Key,TaskNoRun};
//һ��ִ��FATFS������صĺ������
EventHandlerNoPara _oneFatfsOperateFunc=NULL;

//������¼���Ⲣִ��
void InportEventCheckRun(void);
void InportEventTask(void *arg);

void Delay_ms_OS(u16 ms)
{
	//vTaskDelay(ms/portTICK_RATE_MS);
	_xLastWakeTime=xTaskGetTickCount();
	vTaskDelayUntil(&_xLastWakeTime,ms/portTICK_RATE_MS);
}

//��ʱ����
void Delay_s_OS(float time_s)
{
	u32 time_int = (u32) (time_s * 1000.0);
	Delay_ms_OS(time_int);	
}

//��ʾʵʱ����
void ShowRealData(void *arg)
{
	while(true)
	{
		InportEventCheckRun();
		MultiAxisCoordStruct* mas=GetRealTimeCoord();
		if(Parameter.ProdType==CustomMovePlat)
		{
			//ShowCurrCoord_cm(mas);
		}
		else
		{
			ShowCurrCoord(mas);
		}
		Delay_ms_OS(200);   
	}
}

void USBHost(void *arg)
{
	while(true)
	{
        USBH_Process(&USB_OTG_Core, &USB_Host);
        Delay_ms_OS(10);
	}
}

void AlarmTwinkle(void *arg)
{
	while(true)
	{
		SetOutPortState(AlarmLampPort,true);
		Delay_ms_OS(500);
		SetOutPortState(AlarmLampPort,false);
		Delay_ms_OS(500);        
	}
}

void PositionResetTask(void *arg)
{
	for(;;)
	{
		/* �ȴ�֪ͨ���������� */  
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY);  
		//xTaskNotifyWait();
		PositionResetMove();
	}
}

bool _isOpenUSBDirver=false;
//�豸��Ϊ������̵Ĵ򿪻�رյ������ڲ�
void SetOpenMassTask(void *arg)
{
	for(;;)
	{
		/* �ȴ�֪ͨ���������� */  
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY);  
		if(_isOpenUSBDirver)
		{
			_currUSBMode=DirverMode;
			usbd_OpenMassStorage();
		}
		else
		{
			_currUSBMode=HostMode;
			usbd_CloseMassStorage();
		}
	}
}

//������ر��豸��Ϊ������̣��ⲿ����
void SetUSBMassStorage(bool isOpen)
{
	_isOpenUSBDirver=isOpen;
	xTaskNotifyGive(_setUSBDirverHandle);
}

//����Y��׼������
void RunYReadyTask(LeftRightPlatformEnum plat)
{
	_yReadyPlat=plat;
	_oneFatfsOperateFunc=YReadyTask;
	StartFATFSTask();
}

void DeviceAndTaskInit(void)
{
	DeviceInit();
	xTaskCreate(PositionResetTask, "PosReset", configMINIMAL_STACK_SIZE, NULL, 3, &_positionResetHandle); 
	xTaskCreate(AlarmTwinkle, "AlarmTwin", configMINIMAL_STACK_SIZE, NULL, 3, &_alarmHandle);
	vTaskSuspend(_alarmHandle);
	xTaskCreate(WorkRun, "WorkRunLeft", 4096, &_leftStartPara, 3, &_workRunLeftHandle ); 
	xTaskCreate(WorkRun, "WorkRunRight", 4096, &_rightStartPara, 3, &_workRunRightHandle ); 
  //xTaskCreate(ShowIOState, "IOState", configMINIMAL_STACK_SIZE, NULL, 3, &_IOStateShowHandle);
	//vTaskSuspend(_IOStateShowHandle);
  //xTaskCreate(ShowDataTime, "DateTimeShow", configMINIMAL_STACK_SIZE, NULL, 3, &_dateTimeShowHandle);
  xTaskCreate(ShowRealData, "ShowRealData", configMINIMAL_STACK_SIZE, NULL, 3, &_showRealDataHandle);
	xTaskCreate(InportEventTask, "InportEvnet", configMINIMAL_STACK_SIZE, NULL, 3, &_inportEventHandle);	
}

//ִ����FATFS������в�������ĺ���
void FATFSOperate_ALL(void *arg)
{
	DeviceAndTaskInit();//�豸��ʼ���������ʼ��ֻ����һ��
    for(;;)
	{
		//�ȴ�֪ͨ���������� 
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
		if(_oneFatfsOperateFunc!=NULL)
		{
			_oneFatfsOperateFunc();
			_oneFatfsOperateFunc=NULL;
			continue;
		}
		
		//USB�Ĳ�������Ҫȷ���������񲻻�ִ�е�����
		SetUSBHostTaskEnable(false);//ִ��USB�������ʱҪ�ȹر�Host���񣬷����дUSB�����
		USBOpereateTask();
    //MoveOpereateTask();
		//LogFileOpereateTask();//��USB�������ܷ��ڴ˴������USBHost��������ռ����Դ
		SetUSBHostTaskEnable(true);//�ڴ�Ҫ��Host���񣬷����ܶ����USB
    }        
}

//��ʼFATFS�������������
void StartFATFSTask(void)
{
	if(eTaskGetState(_fatfsOperateHandle)==eRunning)
		return;
    xTaskNotifyGive(_fatfsOperateHandle);  
}

//�����ʼ��
void InitTask(void *arg)
{
	//�Ƿ��ѽ�����ϵȴ�״̬
	_isWaitSemaphore=false;		
	//�ش��ź���
	_xSemaphoreRedo= xSemaphoreCreateBinary();  //��ֵ�ź���;
	//��һ���ź���
	_xSemaphoreNext= xSemaphoreCreateBinary();  //��ֵ�ź���;

	xTaskCreate(FATFSOperate_ALL, "USBOP", 6000, NULL, 3, &_fatfsOperateHandle);
	xTaskCreate(USBHost, "USBHost", configMINIMAL_STACK_SIZE, NULL, 3, &_usbHostHandle);
    vTaskSuspend(_usbHostHandle);    
	xTaskCreate(SetOpenMassTask, "OpenMass", configMINIMAL_STACK_SIZE, NULL, 3, &_setUSBDirverHandle);    
	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_cb); //��ʼ��USBHost
    //WriteLog("�豸��ʼ���ɹ���");
    vTaskDelete(NULL);
    for(;;)
    {
    
    }
}

bool _isMotionTest=false;//�Ƿ����ڶ�������
void StartTestCoordMove()
{
	if(IsWorkRunning())
		return;
	ShowMainForm(0);
	//xTaskNotifyGive(_testCoordMoveHandle);
	_oneFatfsOperateFunc=TestCoordMove;
	StartFATFSTask();
	_isMotionTest=true;
}

//ֹͣ���е�����
void StopTasks(void)
{	
	vTaskSuspend(_fatfsOperateHandle);  
    vTaskSuspend(_showRealDataHandle); 
	vTaskSuspend(_workRunLeftHandle);
	vTaskSuspend(_workRunRightHandle); 
	vTaskSuspend(_positionResetHandle); 
	vTaskSuspend(_alarmHandle);
	vTaskSuspend(_IOStateShowHandle);  	
    vTaskSuspend(_dateTimeShowHandle); 		
    //vTaskSuspend(_usbHostHandle);	
    //vTaskSuspend(_setUSBDirverHandle);
	//vTaskSuspendAll();//���ܹ����������������ʱͣ������
}

//��ͣ
void ScramStop(StopSource stopSource)
{
//	if(stopSource==ScreenStop)
//	{
//		WriteLog("��LCD�ϰ����˼�ͣ��");
//	}
//	else if(stopSource==KeyStop)
//	{
//		WriteLog("�����˼�ͣ��ť��");
//	}
	
	StopTasks();
	StopAllAxis();
	//StopTasks();
	SetOutPortState(ScrewDriverPort,false);	//�����źŹر�
	SetOutPortState(VacuumValvePort,false);	//����źŹر�
	CloseAlarm();
	ShowSysResetForm(stopSource);	
	//WriteLogBuffToFile();//д����־���浽�ļ�
}

void btnInportEvent_OKClick(u16 keyCode)
{
	CloseAlarm();	
	SystemReset();
}

void btnInportEventCancelClick(u16 keyCode)
{
	*(__IO uint8_t *)(InportEventCancelTag)=1;
	CloseAlarm();	
	SystemReset();
}

InportEventStruct* _ies;
void InportEventTask(void *arg)
{
	for(;;)
	{
		/* �ȴ�֪ͨ���������� */  
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
		StopTasks();		
		StopAllAxis();
		SetOutPortState(ScrewDriverPort,false);	//�����źŹر�
		SetOutPortState(VacuumValvePort,false);	//����źŹر�
		OpenAlarm();
		ShowMessgeForm(_ies->Msg,btnInportEvent_OKClick,btnInportEventCancelClick);
	}
}

//������¼���Ⲣִ��
void InportEventCheckRun(void)
{
	if(Moves.InportEventCount==0)
		return;
	if(*(__IO uint8_t *)(InportEventCancelTag)==1)
		return;
	for(u16 i=0;i<Moves.InportEventCount;i++)
	{
		InportEventStruct* ies = &Moves.InportEvents[i].InportEvent;
		if(GetInPortState_Index(ies->PortIndex)==ies->InputState)
		{
			_ies=ies;
			xTaskNotifyGive(_inportEventHandle);
		}
	}
}

//������ڹ����С����ϵȴ��У������λ�з���true�����򷵻�false
bool IsWorkRunning(void)
{
	if(_isMotionTest)
		return true;
	if(eTaskGetState(_positionResetHandle)==eRunning)
		return true;
	if(eTaskGetState(_positionResetHandle)==eRunning)
		return true;
	if(eTaskGetState(_workRunLeftHandle)==eRunning) 
		return true;
	if(eTaskGetState(_workRunRightHandle)==eRunning)
		return true;
	if(eTaskGetState(_workRunLeftHandle)==eBlocked) //���ϵȴ�ʱ
		return true;
	if(eTaskGetState(_workRunRightHandle)==eBlocked)
		return true;
	return false;
}

//��ʼλ�ø�λ����
void StartPositionReset(void)
{
	if(IsWorkRunning())
		return;
	//vTaskNotifyGiveFromISR( _positionResetHandle ,&xHigherPriorityTaskWoken);  //�����������
	xTaskNotifyGive(_positionResetHandle);
}

//˫ƽ̨ʱ�����ڹ�����ƽ̨����������ô˺�����ʹ��һ��ƽ̨����׼��״̬ƽ̨������������
//platformComplate:���ô˺���������ƽ̨�����磺LeftPlat��ʾ��ƽ̨�����(����ǰ�ź�ȡʱ�ĵȴ�ȡ��)����ƽ̨���Կ�ʼ������
void ContinueNextPlatWork(LeftRightPlatformEnum platformComplate)
{
	if(Parameter.ProdType==SinglePlatform)//����ǵ�ƽ̨�򲻴��������Ĳ���
		return;
	if(platformComplate==RightPlatform)
	{
		if(_leftStartPara.WorkTaskState!=YReadyed)//�������׼��״̬����ʾ�û�δ������һ������ť
			return;
		ShowWorkMsg_Plat(LeftPlatform, "��ʼ����");
		_leftStartPara.WorkTaskState=TaskRunning;
		StartWorkTask(LeftPlatform,_leftStartPara.StartSource);
	}
	else
	{
		if(_rightStartPara.WorkTaskState!=YReadyed)
			return;
		ShowWorkMsg_Plat(RightPlatform, "��ʼ����");
		_rightStartPara.WorkTaskState=TaskRunning;
		StartWorkTask(RightPlatform,_rightStartPara.StartSource);
	}
}

//��ʼ����˿����
void StartWorkTask(LeftRightPlatformEnum platform,StartXinhaoSource sxs)
{
	//��������������򲻽����κζ���
	//if(_currFormIndex!=1 && _currFormIndex!=71 && _currFormIndex!=91)
		//return;		
	if(platform==RightPlatform && Parameter.ProdType==SinglePlatform)//����ǵ�ƽ̨����Ӧ��ƽ̨����
		return;
	if(eTaskGetState(_positionResetHandle)==eRunning)//�������λ����Ӧ
		return;
	if(_isMotionTest)//������ڶ�������
		return;
    char* msg= "δ��⵽������";
	if(platform==LeftPlatform)
	{
		//�����ƽ̨���ڹ���(�����������������ϵȴ�)������ƽ̨����׼��״̬(Y1�Ƶ���һ������)
		if(_rightStartPara.WorkTaskState==TaskRunning)
		{
			_leftStartPara.WorkTaskState=YReadyed;//��ƽ̨Y����׼������
			RunYReadyTask(LeftPlatform);
			return;
		}
		eTaskState taskState=eTaskGetState(_workRunLeftHandle);
		if(taskState==eSuspended) //��Y�����׼��״̬������һƽ̨�������˴���ʱ����״̬���eReady��why??
		{
			if(Parameter.IsWorkpieceCheck && GetInPortState(LeftWorkpiecePort))//��������˹�����⣬��û��⵽����
			{
        //WriteLog(msg);
				ShowWorkMsg(msg);//������ʾ��Ҫ�У���Ҫд����־�ļ�
				return;
			}
			_leftStartPara.StartSource=sxs;
			xTaskNotifyGive(_workRunLeftHandle);
		}
		else if(_isWaitSemaphore)
		{
			_isWaitSemaphore=false;
			//���ж��з����ź�������FromISR��β�ĺ������б������ܣ�����������з����ź�����ʹ��xSemaphoreGive��
			xSemaphoreGiveFromISR(_xSemaphoreNext, &xHigherPriorityTaskWoken );  
		}
	}
	else
	{
		//�����ƽ̨���ڹ���(�����������������ϵȴ�)������ƽ̨����׼��״̬(Y1�Ƶ���һ������)
		if(_leftStartPara.WorkTaskState==TaskRunning)
		{
			_rightStartPara.WorkTaskState=YReadyed;//��ƽ̨Y����׼������
			RunYReadyTask(RightPlatform);
			return;
		}
		eTaskState taskState=eTaskGetState(_workRunRightHandle);
		if(taskState==eSuspended) //��Y�����׼��״̬������һƽ̨�������˴���ʱ����״̬���eReady��why??
		{
			if(Parameter.IsWorkpieceCheck && GetInPortState(RightWorkpiecePort))//��������˹�����⣬��û��⵽����
			{
        //WriteLog(msg);
				ShowWorkMsg(msg);
				return;
			}
			_rightStartPara.StartSource=sxs;
			xTaskNotifyGive(_workRunRightHandle);
		}
		else if(_isWaitSemaphore)
		{
			_isWaitSemaphore=false;
			//���ж��з����ź�������FromISR��β�ĺ������б������ܣ�����������з����ź�����ʹ��xSemaphoreGive��
			xSemaphoreGiveFromISR(_xSemaphoreNext, &xHigherPriorityTaskWokenRight);  
		}
	}	
}

//���Ϻ��ش�����
void RepeatWork(void)
{
	if(_isWaitSemaphore)
	{
		ShowWorkMsg("�ش��ź�");
		_isWaitSemaphore=false;
		//���ж��з����ź�������FromISR��β�ĺ������б������ܣ�����������з����ź�����ʹ��xSemaphoreGive��
		//xSemaphoreGiveFromISR(_xSemaphoreRedo, &xHigherPriorityTaskWoken ); 
		xSemaphoreGive(_xSemaphoreRedo);
	}
}

//USBHost����رջ���
void SetUSBHostTaskEnable(bool isEnable)
{
    if(_usbHostHandle==NULL)
        return;
	if(isEnable)
		xTaskResumeFromISR(_usbHostHandle);
	else
		vTaskSuspend(_usbHostHandle);
}

//��������˸ʹ��
void EnableAlarmTwinkle(bool isEnable)
{
	if(isEnable)
		vTaskResume(_alarmHandle);
	else
		vTaskSuspend(_alarmHandle);
}

//ʵʱ������ʾ����رջ���
void SetRealCoordTaskEnable(bool isEnable)
{
    if(_showRealDataHandle==NULL)
        return;
	if(isEnable)
		vTaskResume(_showRealDataHandle);
	else
		vTaskSuspend(_showRealDataHandle);
}

//IO״̬��ʾ����رջ���
void SetIOStateTaskEnable(bool isEnable)
{
    if(_IOStateShowHandle==NULL)
        return;
	if(isEnable)
		vTaskResume(_IOStateShowHandle);
	else
		vTaskSuspend(_IOStateShowHandle);
}

//ʱ����ʾ����رջ���
void SetDateTimeTaskEnable(bool isEnable)
{
    if(_dateTimeShowHandle==NULL)
        return;
    if(isEnable)
		vTaskResume(_dateTimeShowHandle);
	 else 
		vTaskSuspend(_dateTimeShowHandle);
}
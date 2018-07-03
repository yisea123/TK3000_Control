/**
  ******************************************************************************
  * @file    usbh_usr.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file includes the usb host library user callbacks
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "FileOperate.h"
#include "USBFunc.h"
#include "AppTasks.h"

USB_OTG_CORE_HANDLE   USB_OTG_Core;
USBH_HOST             USB_Host;
bool _isInset=false;  //U���Ƿ��Ѿ�����

//USB�ص���������˳��ִ��
USBH_Usr_cb_TypeDef USR_cb =
{
  USBH_USR_Init,							//��ʼ��
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,					//��⵽��USB�豸����
  USBH_USR_ResetDevice,						//�����豸
  USBH_USR_DeviceDisconnected,				//��⵽���豸�Ƴ�
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,				//����豸�ٶȣ�������١�ȫ�١�����
  USBH_USR_Device_DescAvailable,			//�豸��Ϣ
  USBH_USR_DeviceAddressAssigned,	   		//�����ַ�ɹ�
  USBH_USR_Configuration_DescAvailable,	    //��������
  USBH_USR_Manufacturer_String,		  		//������Ϣ���ַ�����
  USBH_USR_Product_String,			  		//��Ʒ��Ϣ���ַ�����
  USBH_USR_SerialNum_String,		   		//�豸����
  USBH_USR_EnumerationDone,			  		//ö�����
  USBH_USR_UserInput,						//�û�����Ӧ�ó���״̬��Ŀ
  USBH_USR_MSC_Application,					//USBӦ�ò���
  USBH_USR_DeviceNotSupported,				//�豸��֧��
  USBH_USR_UnrecoveredError 
};

//���Գ�ʼ��һЩ����
void USBH_USR_Init(void)
{

}

//��⵽U�̲���
void USBH_USR_DeviceAttached(void)
{
	//printf("��⵽USB�豸����!\r\n");
}

//�޷��ָ��Ĵ���!!
void USBH_USR_UnrecoveredError (void)
{
	//printf("�޷��ָ��Ĵ���!!!\r\n\r\n");
}

//��⵽U�̰γ�
void USBH_USR_DeviceDisconnected (void)
{
	//printf("USB�豸�γ�!\r\n");
	SetUSBHostTaskEnable(false);//�ڴ˿��Թر�USBHost���񣬽�ʡ������Դ
	_isInset=0;
}

//��������
void USBH_USR_ResetDevice(void)
{
	/* callback for USB-Reset */
	//printf("USBH_USR_ResetDevice\r\n");
}

//��⵽�ӻ��ٶ�
//DeviceSpeed:�ӻ��ٶ�(0,1,2 / ����)
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
	/*if(DeviceSpeed==HPRT0_PRTSPD_HIGH_SPEED)
	{
		printf("����(HS)USB�豸!\r\n");
 	}  
	else if(DeviceSpeed==HPRT0_PRTSPD_FULL_SPEED)
	{
		printf("ȫ��(FS)USB�豸!\r\n"); 
	}
	else if(DeviceSpeed==HPRT0_PRTSPD_LOW_SPEED)
	{
		printf("����(LS)USB�豸!\r\n");  
	}
	else
	{
		printf("�豸����!\r\n");  
	}  */
}

//��⵽�ӻ���������
//DeviceDesc:�豸������ָ��
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
//	USBH_DevDesc_TypeDef *hs;
//	hs = DeviceDesc;
//	usb_printf("> VID : %04Xh\r\n" , (uint32_t)(*hs).idVendor);
//	usb_printf("> PID : %04Xh\r\n" , (uint32_t)(*hs).idProduct);
}

//�ӻ���ַ����ɹ�
void USBH_USR_DeviceAddressAssigned(void)
{
  //printf("�ӻ���ַ����ɹ�\r\n");
}


//��������������Ч
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
	/*USBH_InterfaceDesc_TypeDef *id; 
	id = itfDesc;   
	if((*id).bInterfaceClass==0x08)
	{
		//printf("���ƶ��洢���豸!\r\n"); 
	}
    else if((*id).bInterfaceClass==0x03)
	{
		//printf("HID �豸!\r\n"); 
	}*/    
}

//���̣��ַ�������Ϣ
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  //printf("Manufacturer : %s\r\n", (char *)ManufacturerString);
}

//��Ʒ��Ϣ���ַ�����
void USBH_USR_Product_String(void *ProductString)
{
	//printf("Product : %s\r\n", (char *)ProductString);
}

//�豸����
void USBH_USR_SerialNum_String(void *SerialNumString)
{
	//printf( "Serial Number : %s\r\n", (char *)SerialNumString); 
}

//�豸USBö�����
void USBH_USR_EnumerationDone(void)
{
//	printf("�豸USBö�����\r\n");	
//	USB_OTG_BSP_mDelay(500);
	//USBH_USR_MSC_Application();
}

//�޷�ʶ���USB�豸
void USBH_USR_DeviceNotSupported(void)
{
	//printf("�޷�ʶ���USB�豸\r\n");
}

//�û�����Ӧ�ó���״̬��Ŀ
USBH_USR_Status USBH_USR_UserInput(void)
{
	return USBH_USR_RESP_OK;
}

//USB�ӿڵ�������
void USBH_USR_OverCurrentDetected (void)
{
	//printf("�˿ڵ�������!!!\r\n");
}

void USBBackReset(u16 keyCode)
{
	SystemReset();
}

//Ӧ�ò���
int USBH_USR_MSC_Application(void)
{
    if(!_isInset)
    {
      _isInset=true;
			PermissionTypeEnum destPer=AdminiPer;//��Ҫ����Ա������Ȩ��
//		if(_currPermission<destPer)
//		{
//			ShowLoginForm(destPer,ShowUSBOprateForm,USBBackReset);
//			return 0;
//		}
		ShowUSBFuncForm();
    }
	return 0;
}

//�û�Ҫ�����³�ʼ���豸
void USBH_USR_DeInit(void)
{
	//USBH_USR_ApplicationState = USH_USR_FS_INIT;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

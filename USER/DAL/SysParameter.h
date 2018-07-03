#ifndef SYSPARAMETER_H_
#define SYSPARAMETER_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f4xx.h"
#include "stdbool.h"
#include "SST25V.h"
	 
extern char _superPass[10];

typedef enum
{
	NoReg=0,        //δע�ᣬָ����ʱ��״̬
	Unlimited,      //��ע�ᣬ�����Ƶ�
	Limit,          //��ע�ᣬ��ʱ�����Ƶ�	
}RegisterStateEnum; //ע��״̬ö��

typedef struct
{
	char DevicePass[10];		//�豸��������,���ܷ���Parameter�У���Ϊ�ָ���������ʱ�Ḵλ
	char RegNum[32];            //ע��������
    char RegDate[20];           //ע�����ڣ�
    char YXDate[20];            //��Ч���ڣ���ע�����еó���
    char DevNum[20];            //������
	RegisterStateEnum RegState; //ע��״̬
    char ParaVer[7];			//�����汾�������Զ������������� 
}SysParameterStruct;            //�̶������ṹ��

//ϵͳ����ʵ��
extern SysParameterStruct SysParameter;
//��ʼ��ϵͳ����
void IniSysParameter(void);
//��ϵͳ����д��洢��
void WriteSysParameter(void);


/********���������ƣ��ͻ������޸ĵ�*******/	
typedef struct
{
	char ProdName[32];                          //��Ʒ����
	char ProdModel[16];                         //��Ʒ�ͺ�
	char SoftVer[32];		                    //�����汾
	char HardWareVer[10];	                    //Ӳ���汾
	char ProdID[16];		                    //�����������к�        
	char ProduDate[16];                   		//��������	
	char Tel[16];								//��ϵ�绰
	char URL[32];								//��˾��ַ
	char Company[32];							//������λ
	char CompanyAddr[80];                       //������λ��ַ	
	bool IsWrited;								//�Ƿ��Ѿ�д���������Ϣ
    char ParaVer[7];							//�����汾�������Զ������������� 
}ControlMPStruct;            //������������Ϣ�ṹ��

//����������ʵ��
extern ControlMPStruct ControlMPMsg;
//��ʼ������������
void IniControlMPMsg(void);
//�ѿ�����������洢��
void WriteControlMPMsg(void);

#ifdef __cplusplus
}
#endif

#endif
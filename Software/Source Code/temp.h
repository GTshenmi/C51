#ifndef __TEMP_H_
#define __TEMP_H_
/****************************预编译****************************/
#include<reg52.h>
#define DIGDISPLAY P1
#define DIGCONTROL P0

#define KEYPORT P3

#define LED P2
/****************************重定义****************************/
#ifndef uchar
#define u8 unsigned char
#endif

#ifndef uint 
#define u16 unsigned int
#endif
/***************************端口声明****************************/
sbit IO_18B20=P2^4;

sbit Key1=P3^0;
sbit Key2=P3^1;
sbit Key3=P3^2;
sbit Key4=P3^3;

sbit Led1=P2^3;
sbit Led2=P2^2;
sbit Led3=P2^1;
sbit Led4=P2^0;
/****************************函数声明(LED)****************************/ 
void LedFlash();
void Led1to4Flash();
/****************************函数声明(温度传感器)****************************/
void DelayX10us(unsigned char t);

bit Get18B20Ack();
void Write18B20(unsigned char dat);
u8 Read18B20();

bit Start18B20();
bit Get18B20Temp(int *temp);
void GetTemp(int i);

void DisplayTemp();
void TempDataProcess(int temp,int i);
void TempCollection();
/****************************函数声明(时钟)****************************/
//void ClockInit(int hour,int min,int sec);
void Time0Init();

void Clock();
void TimeDataProcess();
void DisplayTime();
void CalibrationTime();

void DelayTime(u16 i);
/****************************函数声明(按键)****************************/
void KeyRead();

void KeyScan();

void DisplayChange();
/****************************常量****************************/
u8 tempDisplay[8]={0x00};
u8 keyFlag[4]={1,1,0,0};
u8 code NumberDisplayTable[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
u16 displayData[8]={0x00};
int tempData[5]={0,0,0,0,0};

u8 ledMode=0;
int judge=-1;
u16 ms=0,s=0,count=0,num=0;
bit finishCol=0,model=0,is_Settime=0,startCol=0,startLedFlash=0,is_DisplayTemp=0,ledState0=0,ledState1=0;

u8 Trg=0,Cont=0;
typedef struct
{
 	int sec;
	int min;
	int hour;	
} _calendar_obj;
_calendar_obj calendar;
#endif

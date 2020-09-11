/*******************************************************************************
*	本段程序LED位于P2,Ds18b20位于P2^4,按键位于P3,数码管段选P0,位选P1		   *
*	读取18b20温度时关闭中断使能												   										   *
*******************************************************************************/
#include "reg52.h"
#include "temp.c"
#include "temp.h"
/*******************************************************************************
* 函 数 名         : main()												       *
* 函数功能		   : 主函数									                   *
* 输    入         : 无													       *
* 输    出         : 无														   *
*******************************************************************************/
void main()
{
	Time0Init();						   	 //定时器0初始化
//	ClockInit(23,59,59);
	calendar.sec=59;
	calendar.min=59;
	calendar.hour=23;			
	while(1)								
	{
		if(s>=10&&(!is_DisplayTemp)&&(!is_Settime))      //十秒测温一次(若不是在显示5次温度的模式下)
		{
		    if(Start18B20())						
			GetTemp(0);						   		//读取温度传感器数据
			s=0;							 
		}						
		TempCollection();  					 		//按下K3开始收集温度
		DisplayChange();							 //显示切换
	}
}
/*******************************************************************************
* 函 数 名         : LedFlash()												   *
* 函数功能		   : 完成采集温度时,le1d以0.5s间隔闪烁。				       *
* 输    入         : 无													       *
* 输    出         : 无														   *
*******************************************************************************/
void LedFlash()										
{
		if(ledState0)						//判断是否到达0.5s
		{
			Led1=~Led1;						//转换Led状态,重新计时
			ledState0=0;						
		}
		startLedFlash=0;
}
/*******************************************************************************
* 函 数 名         : Led1to4Flash()											   *
* 函数功能		   : 温度大于27度,led间隔0.5s依次闪烁,小于20度,led全灭		   *
* 输    入         : 无													       *
* 输    出         : 无														   *
*******************************************************************************/
void Led1to4Flash()
{
	 static int times=0;
	 if(times==5)
	 times=0;
	 if(ledMode==1&&(!is_DisplayTemp))	   // 温度大于27度且未在5次采集状态，Led闪烁
	 {
	 	if(ledState1)
		{
	 		 LED=(0x10>>times++);
			 ledState1=0;
		}
	 }
	 if(ledMode==0&&(!is_DisplayTemp))
	 LED=0x00; 								//温度小于20 led全灭
	 if(ledMode==-1&&(!is_DisplayTemp))		//温度在20度~27度,led全亮
	 LED=0xff;
}	
/*******************************************************************************
* 函 数 名         : Time0Init()											   *
* 函数功能		   : 定时器0初始化								               *
* 输    入         : 无													       *
* 输    出         : 无														   *
*******************************************************************************/
void Time0Init()  
{
	TMOD|=0x01;	 
	TH0=0xfc;
	TL0=0x18;	  //定时1ms
	EA=1;
	ET0=1;
	TR0=1;
}
/*******************************************************************************
* 函 数 名         : Time0()											       *
* 函数功能		   : 定时器0中断函数								           *
* 输    入         : 无													       *
* 输    出         : 无														   *
*******************************************************************************/
void Time0() interrupt 1  
{	
	TH0=0xfc;
	TL0=0x18;  						//重载定时器
	if(ms++>=1000)					//计时
	{
		calendar.sec++;s++;ms=0; 						
	}
	Clock();
	if(ms%500==0)			//判断是否到0.5s
	{
		ledState0=1;
		ledState1=1;
	}						
	if(count++>=1000)	  			//定时用
	count=0;
	if(ms%2==0)						//2s扫描一次按键
	KeyScan();
	if(!is_DisplayTemp)
	{  	
		if(tempData[0]>2700) //大于27度		//判断温度范围,控制Led
		ledMode=1;
		else
		{
			if(tempData[0]<2000) //小于20度
			ledMode=0;
			else
			ledMode=-1;
		}
	}
	Led1to4Flash();
	if(is_DisplayTemp)
	{
		if(LED&&ledMode)
		{
			ledMode=0;
			LED=0x00;
		}
		if(finishCol)
		LedFlash();					//完成5次温度采集,led1闪烁

	}
}
///*******************************************************************************
//* 函 数 名         : ClockInit()											     *
//* 函数功能		   : 时钟初始化								         		 *
//* 输    入         : 时hour 分min 秒sec										 *
//* 输    出         : 无														 *
//*******************************************************************************/
//void ClockInit(int hour,int min,int sec)
//{
//   	calendar.sec=sec;
//		calendar.min=min;
//		calendar.hour=hour;
//}
/*******************************************************************************
* 函 数 名         : Clock()											       *
* 函数功能		   : 时钟								         			   *
* 输    入         : 无													       *
* 输    出         : 无														   *
*******************************************************************************/
void Clock() 				
{
	if(calendar.sec>=60)			 //秒计时
	{
		calendar.min++;	calendar.sec=0;
	}
	if(calendar.min>=60)			 //分钟计时
	{
		calendar.hour++;	calendar.min=0;
	}
	if(calendar.hour>=24)	  	 //小时
	calendar.hour=0;
}
/*******************************************************************************
* 函 数 名         : DelayTime(u16 i)										   *
* 函数功能		   : 延时								         			   *
* 输    入         : 延迟时间(ms)										       *
* 输    出         : 无														   *
*******************************************************************************/
void DelayTime(u16 i)
{
	count=0;
	while(1)
	{						 //延时函数单位ms
		if(count==i)
		break;
	}
}
/*******************************************************************************
* 函 数 名         : TimeDataProcess()										   *
* 函数功能		   : 处理时间数据				  					   	   	   *
* 输    入         : 无										      			   *
* 输    出         : 无														   *
*******************************************************************************/
void TimeDataProcess()  	
{
	 displayData[0]=NumberDisplayTable[calendar.hour/10];	    //显示小时
	 displayData[1]=NumberDisplayTable[calendar.hour%10];
	 displayData[2]=0x40;						//显示-
	 displayData[3]=NumberDisplayTable[calendar.min/10];	     //显示分钟
	 displayData[4]=NumberDisplayTable[calendar.min%10];
	 displayData[5]=0x40;						 //显示-
	 displayData[6]=NumberDisplayTable[calendar.sec/10];		 //显示秒
	 displayData[7]=NumberDisplayTable[calendar.sec%10];
}
/*******************************************************************************
* 函 数 名         : DisplayTime()										       *
* 函数功能		   : 展示时间								         	   	   *
* 输    入         : 无										      			   *
* 输    出         : 无														   *
*******************************************************************************/
void DisplayTime()	
{
	if(ms%2==0)
	{
		TimeDataProcess(); 					//处理时间数据
		if(calendar.sec%2==0)
		{									//-间隔1s闪烁
			displayData[2]=0x00;
			displayData[5]=0x00;
		}
		CalibrationTime(); 					//如果按下k2,调时间
		DIGCONTROL=~(0x01<<(num=++num%8));		//位选
		DIGDISPLAY=displayData[num];			 	//断选
		DelayTime(1);						
		DIGDISPLAY=0x00;						    //消隐
	}
}
/*******************************************************************************
* 函 数 名         : GetTemp(int temp,int i)								   *
* 函数功能		   : 获取温度								         	   	   *
* 输    入         : 温度temp 采集次数i										   *
* 输    出         : 无														   *
*******************************************************************************/
void GetTemp(int i)
{
	int temp;
	if(Start18B20())
	{
		Get18B20Temp(&temp);
		if(temp<0)
		{
			tempDisplay[3]=0x40;
			temp=((float)~(temp-1))*0.0625*100+0.5;	 //处理温度传感器传来的数据,转换成10进制
		}
		else
		{
			tempDisplay[3]=0x00;
			temp=((float)temp)*0.0625*100+0.5;
		}
		tempData[i]=temp;
	}
}
/*******************************************************************************
* 函 数 名         : TempDataProcess(int temp,int i)						   *
* 函数功能		   : 处理温度数据 							         	   	   *
* 输    入         : 温度temp 采集次数i										   *
* 输    出         : 无														   *
*******************************************************************************/
void TempDataProcess(int temp,int i)
{
	tempDisplay[0]=0x40;
	tempDisplay[2]=0x40;
	tempDisplay[4]=NumberDisplayTable[temp%10000/1000];
	tempDisplay[5]=NumberDisplayTable[temp%1000/100]|0x80;
	tempDisplay[6]=NumberDisplayTable[temp%100/10];
	tempDisplay[7]=NumberDisplayTable[temp%10];
	if(model)
	tempDisplay[1]=NumberDisplayTable[i+1];
}
/*******************************************************************************
* 函 数 名         : TempCollection()						  				   *
* 函数功能		   : 采集集五次温度 							         	   *
* 输    入         : 无														   *
* 输    出         : 无														   *
*******************************************************************************/
void TempCollection()
{
	if(startCol)	 							//如果startCol为1,开始采集温度
	{
		static int i=0;
		if(judge==-1)
		s=0;
		if(s%3==0&&s!=judge)
		{
			GetTemp(i++);
			judge=s;
		}
		if(i==5)								//采集完成,finishCol置为1
		{
			finishCol=1;
			startCol=0;
			i=0;
			judge=-1;
		}

	}
} 
/*******************************************************************************
* 函 数 名         : DisplayTemp()						  				       *
* 函数功能		   : 展示温度,model=0显示一次,model=1显示5次				   *
* 输    入         : 无										   				   *
* 输    出         : 无														   *
*******************************************************************************/
void DisplayTemp()
{
	u8 j=0;
	static int times=0;
	tempDisplay[1]=0x00;
	if(!model)										  //模式1：按住k1,显示一次温度
	{
		    if(Start18B20())
			GetTemp(0);
			TempDataProcess(tempData[0],0);
			do
			{
				tempDisplay[0]=0x00;
				tempDisplay[2]=0x00;
			  	for(j=0;j<8;j++)
				{
			         DIGCONTROL=~(0x01<<j);
				 	 DIGDISPLAY=tempDisplay[j];
					 DelayTime(1);
					 DIGDISPLAY=0x00;
				}
			 }while(!((Cont==0&Trg==0)&&(!model)));
	 }
	 else										     //模式二 显示五次温度
	 {
				if(s>=1)
				{
					s=0;
					times=++times%5;
				}
				TempDataProcess(tempData[times],times);
				for(j=0;j<8;j++)
				{
	     	 	  	 DIGCONTROL=~(0x01<<j);
		 		   	 DIGDISPLAY=tempDisplay[j];
					 DelayTime(2);
					 DIGDISPLAY=0x00;
				}
	 }
}
/*******************************************************************************
* 函 数 名         : KeyScan()						  				           *
* 函数功能		   : 扫描按键状态,并赋予按键效果				  			   *
* 输    入         : 无														   *
* 输    出         : 无														   *
*******************************************************************************/
void KeyScan()
{
		KeyRead();		 										//读取按键数据
		if(Trg)												
		{	 
			if(Trg&0x01)  						  				//按住k1,显示温度
			{	
			    keyFlag[0]++;
			}												  
			if(Trg&0x02)									    //按下k2调整时间
			{		
			    keyFlag[1]++;
				is_Settime=1;
			}
			if(Trg&0x04)										//按下k3
			{													
				if(is_Settime)									//调时模式按下k3调时间
			    keyFlag[2]++;
			    if(keyFlag[0]%2==1&&!is_Settime)				//按下k3收集温度
				{
					startCol=1;
					is_DisplayTemp=1;
				}
			}
			if(Trg&0x08)				  						 //按下K4
			{
				if(is_Settime)									//调时模式调时间
			   	 keyFlag[3]++;
				 if(keyFlag[0]%2==0)  								//如果在显示温度,按下K4调回时钟界面
				{
					keyFlag[0]=1;
					finishCol=0;
					model=0;					
					is_DisplayTemp=0;
				}
				 if(finishCol&&keyFlag[0]%2==1)					//如果温度采集完毕,按下k4显示温度
				{
					keyFlag[0]=0;
					model=1;
					Led1=0;
					startLedFlash=0;
			    }
			}
		}
		if(Cont==0&Trg==0)										//松开k1
		{
			if(!model)
			keyFlag[0]=1;	 		
		}
}
/*******************************************************************************
* 函 数 名         : KeyRead()					  				         	   *
* 函数功能		   : 读取按键端口数据，消抖							  		   *
* 输    入         : 无														   *
* 输    出         : 无														   *
*******************************************************************************/
void KeyRead()  
{
	u8 ReadData=KEYPORT^0xff;					  			//读取数据
	Trg=ReadData&(ReadData^Cont);							//按住一次
	Cont=ReadData;											//持续按下
}
/*******************************************************************************
* 函 数 名         : DisplayChange()					  				       *
* 函数功能		   : 切换显示状态,keyFlag[0]=0显示温度,keyFlag[0]=1显示温度		   *
* 输    入         : 无														   *
* 输    出         : 无														   *
*******************************************************************************/
void DisplayChange()
{
	keyFlag[0]=keyFlag[0]%2;
	if(!keyFlag[0])	   						//keyFlag[0]为1显示温度
	DisplayTemp();
	else
	DisplayTime();						    //keyFlag[0]为0显示时间
}
/*******************************************************************************
* 函 数 名         : CalibrationTime()					  				       *
* 函数功能		   : 调整时间									   			   *
* 输    入         : 无														   *
* 输    出         : 无														   *
*******************************************************************************/
void CalibrationTime()
{
	if(keyFlag[0]==1&&is_Settime)		     //判断是否在温度界面且已经触发调时
	{
		keyFlag[1]=keyFlag[1]%4;	   	
		if(keyFlag[1]==2) 					 //第一次按下调整小时
		{
			  if(calendar.sec%2==0)				 //1s间隔闪烁
			  {
			  	displayData[0]=0x00;
			  	displayData[1]=0x00;
			  }
			  if(keyFlag[2])				 //按下k3小时加一
			  {
			  	 calendar.hour=++calendar.hour%24;
				 keyFlag[2]=0;
			  }
			  if(keyFlag[3])				 //按下k4小时减一
			  {
			  	calendar.hour=(--calendar.hour+24)%24;
				keyFlag[3]=0;
			  }
			  	
		}
		if(keyFlag[1]==3)					  //第二次按下调整分钟
		{
			  if(calendar.sec%2==0)				  //1s间隔闪烁
			  {
			  	displayData[3]=0x00;
			  	displayData[4]=0x00;
			  }
			  if(keyFlag[2])				   //按下k3分钟加一
			  {
			  	 calendar.min=++calendar.min%60;
				 keyFlag[2]=0;
			  }
			  if(keyFlag[3])				   //按下k4	分钟减一
			  {
			  	calendar.min=(--calendar.min+60)%60;
				keyFlag[3]=0;
			  }
		}
		if(keyFlag[1]==0)						//第三次按下调秒
		{
			  if(calendar.sec%2==0)	   				//1s间隔闪烁
			  {
			  	displayData[6]=0x00;
			  	displayData[7]=0x00;
			  }
			  if(keyFlag[2])						//按下k1秒加一
			  {
			  	 calendar.sec=++calendar.sec%60;	     
				 keyFlag[2]=0;
			  }
			  if(keyFlag[3])						//按下k4秒减一
			  {
			  	calendar.sec=(--calendar.sec+60)%60;
				keyFlag[3]=0;
			  }
		}
		if(keyFlag[1]==1)						   //结束调时间状态
		is_Settime=0;
	}
}

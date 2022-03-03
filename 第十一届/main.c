#include <STC15F2K60S2.H>
#include "intrins.h"
#include "iic.h"
#define u8 unsigned char
#define u16 unsigned int	
#define stat0 0
#define stat1 1
#define stat2 2
#define nokey 0xff

bit vol_flag;
bit count_first=1;
bit count_flag1=0;
bit count_flag2=0;
u8 undefined_key=0;
u8 code duanma[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0xc1,0x8c,0x89};
u8 dis_smg[]={0,11,11,11,11,11,6,7,8};
u8 led_mode;
u8 smg_mode=0;
u8 count;
u8 vol_set=0;
u8 vol;
void Delay5ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 59;
	j = 90;
	do
	{
		while (--j);
	} while (--i);
}

void Timer0Init(void)		//100微秒@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x50;		//设置定时初始值
	TH0 = 0xFB;		//设置定时初始值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时

	ET0=1;
	EA=1;
}

void port(u8 pos,u8 value)
{
	P2=0X00;P0=0X00;P2=0X00;
	P2=0XC0;P0=0x01<<(pos-1);P2=0X00;
	P2=0XE0;P0=value;P2=0X00;
}
void KBD()
{
	static unsigned char stat=stat0;
	u8 key_temp=0;
	u8 key1,key2;
	
	P30=0;P31=0;P32=0;P33=0;P34=1;P35=1;P42=1;P44=1;
	if(P44==0)key1=0x70;
	if(P42==0)key1=0xb0;
	if(P35==0)key1=0xd0;
	if(P34==0)key1=0xe0;
	if((P44==1)&&(P42==1)&&(P35==1)&&(P34==1))key1=0xf0;
	
	P30=1;P31=1;P32=1;P33=1;P34=0;P35=0;P42=0;P44=0;
	if(P33==0)key2=0x07;
	if(P32==0)key2=0x0b;
	if(P31==0)key2=0x0d;
	if(P30==0)key2=0x0e;
	if((P33==1)&&(P32==1)&&(P31==1)&&(P30==1))key2=0x0f;
	
	key_temp=key1|key2;
	
	switch(stat)
	{
		case stat0:
			if(key_temp != nokey)
			{
				stat=stat1;
			}
			break;
		case stat1:
		  if(key_temp == nokey)
			{
				stat=stat0;
			}
			else
			{
				switch(key_temp)
				{
					case 0x77:undefined_key++;break;
					case 0x7b:undefined_key++;break;
					case 0x7d:undefined_key++;break;
					case 0x7e:undefined_key++;break;
					
					case 0xb7:undefined_key++;break;
					case 0xbb:undefined_key++;break;
					case 0xbd:undefined_key++;break;
					case 0xbe:undefined_key++;break;
					
					case 0xd7:
						undefined_key=0;
					if(smg_mode==1)
						write_24(0x00,(u8)vol_set*10);
					if(++smg_mode==3)smg_mode=0;
					break;//S12
					
					case 0xdb:
						undefined_key=0;
						if(smg_mode==2)
						{
							count=0;
						}
						break;//S13
					
					case 0xdd:undefined_key++;break;
					case 0xde:undefined_key++;break;
					
					case 0xe7:
					undefined_key=0;
						if(smg_mode==1)
							{
								if(vol_set>=500)	
									vol_set=0;
								else
									vol_set+=50;
							}
						break;//S16
					case 0xeb:
					undefined_key=0;
						if(smg_mode==1)
							{
								if(vol_set ==0)	
									vol_set=500;
								else
									vol_set-=50;
							}
						break;//S17
					
					case 0xed:undefined_key++;break;
					case 0xee:undefined_key++;break;
				}
				stat=stat2;
			}
			break;
		case stat2:
			if(key_temp == nokey)
			{
				stat=stat0;
			}
		break;				
	}
}


void allinit()
{
	P2=0X00;P0=0X00;P2=0X00;
	P2=0X80;P0=0XFF;P2=0X00;
	P2=0XA0;P0=0X00;P2=0X00;
	P2=0XE0;P0=0XFF;P2=0X00;
}



void main()
{
	Timer0Init();
	allinit();
	write_adc(0x03);
	Delay5ms();
	vol_set=read_24(0x00)/10;
	while(1)
	{
		vol=(u16)read_adc(0x03)*1.96 +0.5;
		vol=(u16)read_adc(0x03)*1.96 +0.5;
		//一开始判断测量值是否小于设置值，方便L1灯的显示;
		if(vol<vol_set)
		{
			vol_flag=1;
		}
		else
		{
			vol_flag=0;
			vol_set=0;
		}
//===========曲线计数===================		
		if(vol>vol_set && count_first)
		{
			//对应第二个的第一个
			count_flag1=0;
			count_flag2=1;
			count_first=0;
		}
		else
		{
			//对应第二个的第二个
			//vol<vol_set
			count_flag1=1;
			count_flag2=0;
			count_first=0;
		}
		
		if(vol>vol_set && count_flag1)
		{
			count++;
			count_flag1=0;
			count_flag2=1;
		}
		else if(count_flag2 &&(vol>vol_set))
		{
			count++;
			count_flag2=0;
			count_flag1=1;
		}
//===========设置数码管数组===========		
		if(smg_mode==0)
		{
			dis_smg[6]=vol /100;
			dis_smg[7]=vol /10%10;
			dis_smg[8]=vol %10;
		}
		else if(smg_mode==1)
		{
			dis_smg[6]=vol /100;
			dis_smg[7]=vol /10%10;
			dis_smg[8]=vol %10;
		}
		else if(smg_mode==2)
		{
			dis_smg[6]=11;
			dis_smg[7]=count/10;
			dis_smg[8]=count %10;
		}
		
		KBD();
		Delay5ms();
	}
}



void timer0() interrupt 1 using 1
{
	static unsigned char count=0,i=0,led_count=0;
	static unsigned char led_temp=0xff;
	count++;
	if(count==1)
	{
		count=0;
//=====控制数码管==========
		if(smg_mode==0&&(i==6 || i==1))
	{
		if(i==6)
		{
			port(i,duanma[dis_smg[i]]&0x7f);
		}
		else if(i==1)
		{
			port(i,duanma[12]);
		}
	}
		else if(smg_mode==1&&(i==6 || i==1))
	 {
		if(i==6)
		{
			port(i,duanma[dis_smg[i]]&0x7f);
		}
		else if(i==1)
		{
			port(i,duanma[13]);
		}
		}
	 
		else if(smg_mode==2 && (i==1))
		{
			port(i,duanma[14]);
		}
		else
		{
			port(i,duanma[dis_smg[i]]);
		}
		i++;
		if(i==9)i=1;
	}
	
//======控制led灯==========
	//L1
	if(vol_flag && (++led_count>50000))
	{
		led_count=0;
		led_temp &= 0xfe;//L1灯亮
	}
	else if(!vol_flag)
	{
		led_temp |= 0x01;//L1灯灭
	}
	
	//L2
	if(count%2)
	{
		led_temp &= 0xfd;//L2灯亮
	}
	else
	{
		led_temp |= 0x02;//L2灯灭
	}

	//L3
	if(undefined_key>=3)
	{
		led_temp &= 0xfb;//L3灯亮
	}
	else
	{
		led_temp |= 0x04;//L3灯灭
	}
	
	P2=0X00;P0=0X00;P2=0X00;
	P2=0X80;P0=led_temp;P2=0X00;
	
}
	

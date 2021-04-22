#include "delay.h"

/*
摘自：https://blog.csdn.net/lintax/article/details/83047326

C语言代码：
for(; nCount != 0; nCount--);
汇编代码：
0x08000206 E000      B        0x0800020A
0x08000208 1E40      SUBS     r0,r0,#1
0x0800020A 2800      CMP      r0,#0x00
0x0800020C D1FC      BNE      0x08000208    

可以看到有4条汇编指令：
B，跳转
SUBS，减
CMP，比较
BNE，根据标志跳转

大体理解，就是这样：进入循环后，先跳转去进行比较。比较后，查看比较结果，若不相等，则跳转去执行减操作。
对于循环次数很多的情况，可以忽略第一次的跳转，所以一般情况下的循环一次，就是执行3条指令：
0x08000208 1E40      SUBS     r0,r0,#1
0x0800020A 2800      CMP      r0,#0x00
0x0800020C D1FC      BNE      0x08000208    

3条指令，5个机器周期，能对应上么？
我上网查了一下，大概的结论是：

stm32 属于ARM ，ARM都是精简指令集，大部分的指令（除STM、LDM、BNE等外）都是单周期指令。
对于跳转指令，需要增加两个指令周期。

按这种方式来估算：
SUBS，1
CMP，1
BNE，3
则 1+1+3=5 

执行一次循环 经历了5个时钟周期 stm32 f427初始化后系统时钟达到最高的180Mhz 所以延时1us 需要循环180/5=36次
*/


void delay_ms(unsigned int ms_count)
{
	unsigned int i=0;
	for(; i<ms_count; i++)
	{
		unsigned short nCount=36000;
		for(; nCount != 0; nCount--);
	}
}

void delay_us(unsigned int us_count)
{
	unsigned int i=0;
	for(; i<us_count; i++)
	{
		unsigned char nCount=36;
		for(; nCount != 0; nCount--);
	}	
}

void delay_28ns(unsigned int nCount) //延时nCount*5个时钟周期，即(250/9)ns =  27.777...ns
{
     for(; nCount != 0; nCount--);
}

/*用于SPI读写数据*/
void delay_200ns(unsigned int time)
{    
   unsigned int i=0;  
   while(time--)
   {
      i=3;
      while(i--);    
   }
}


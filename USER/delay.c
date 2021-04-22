#include "delay.h"

/*
ժ�ԣ�https://blog.csdn.net/lintax/article/details/83047326

C���Դ��룺
for(; nCount != 0; nCount--);
�����룺
0x08000206 E000      B        0x0800020A
0x08000208 1E40      SUBS     r0,r0,#1
0x0800020A 2800      CMP      r0,#0x00
0x0800020C D1FC      BNE      0x08000208    

���Կ�����4�����ָ�
B����ת
SUBS����
CMP���Ƚ�
BNE�����ݱ�־��ת

������⣬��������������ѭ��������תȥ���бȽϡ��ȽϺ󣬲鿴�ȽϽ����������ȣ�����תȥִ�м�������
����ѭ�������ܶ����������Ժ��Ե�һ�ε���ת������һ������µ�ѭ��һ�Σ�����ִ��3��ָ�
0x08000208 1E40      SUBS     r0,r0,#1
0x0800020A 2800      CMP      r0,#0x00
0x0800020C D1FC      BNE      0x08000208    

3��ָ�5���������ڣ��ܶ�Ӧ��ô��
����������һ�£���ŵĽ����ǣ�

stm32 ����ARM ��ARM���Ǿ���ָ����󲿷ֵ�ָ���STM��LDM��BNE���⣩���ǵ�����ָ�
������תָ���Ҫ��������ָ�����ڡ�

�����ַ�ʽ�����㣺
SUBS��1
CMP��1
BNE��3
�� 1+1+3=5 

ִ��һ��ѭ�� ������5��ʱ������ stm32 f427��ʼ����ϵͳʱ�Ӵﵽ��ߵ�180Mhz ������ʱ1us ��Ҫѭ��180/5=36��
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

void delay_28ns(unsigned int nCount) //��ʱnCount*5��ʱ�����ڣ���(250/9)ns =  27.777...ns
{
     for(; nCount != 0; nCount--);
}

/*����SPI��д����*/
void delay_200ns(unsigned int time)
{    
   unsigned int i=0;  
   while(time--)
   {
      i=3;
      while(i--);    
   }
}


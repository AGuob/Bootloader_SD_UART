#include "touch_cs.h"
#include "delay.h"  
#include "lcd.h"
#include <stm32f4xx.h>


u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

	float Kx,Ky;
	uint16_t XL,YL;
void Touch_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;//PB1/PB2 ����Ϊ��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PB0����Ϊ�������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//PC13����Ϊ�������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��	
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PF11�����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��			
		
   
//	TP_Read_XY(&tp_dev.x[0],&tp_dev.y[0]);//��һ�ζ�ȡ��ʼ��	 
											 
}

void TP_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0; 
		delay_us(1);
		TCLK=1;		//��������Ч	        
	}		 			    
}

u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	TCLK=0;		//������ʱ�� 	 
	TDIN=0; 	//����������
	TCS=0; 		//ѡ�д�����IC
	TP_Write_Byte(CMD);//����������
	delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
	TCLK=0; 	     	    
	delay_us(1);    	   
	TCLK=1;		//��1��ʱ�ӣ����BUSY
	delay_us(1);    
	TCLK=0; 	     	    
	for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
	{ 				  
		Num<<=1; 	 
		TCLK=0;	//�½�����Ч  	    	   
		delay_us(1);    
 		TCLK=1;
 		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   	//ֻ�и�12λ��Ч.
	TCS=1;		//�ͷ�Ƭѡ	 
	return(Num);   
}
#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
//��ȡx,y����
//��Сֵ��������100.
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}
//����2�ζ�ȡ������IC,�������ε�ƫ��ܳ���
//ERR_RANGE,��������,����Ϊ������ȷ,�����������.	   
//�ú����ܴ�����׼ȷ��
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
#define ERR_RANGE 50 //��Χ 
u8 TP_Read_XY2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 

void Adjust(void)
{
	uint16_t P_x[4],P_y[4];
	uint16_t xtemp,ytemp;
	int16_t S1,S2,S3,S4;
	
	LCD_Draw_Circle(10,10,5);
	while(PEN);
	TP_Read_XY(&xtemp,&ytemp);
	P_x[0]=xtemp;
	P_y[0]=ytemp;
	while(!PEN);
	delay_ms(300);
	
	POINT_COLOR=0x036f;
	LCD_Draw_Circle(10,10,5);
	POINT_COLOR=RED;
	LCD_Draw_Circle(230,10,5);
	while(PEN);
	TP_Read_XY(&xtemp,&ytemp);
	P_x[1]=xtemp;
	P_y[1]=ytemp;
	while(!PEN);
	delay_ms(300);
	
	POINT_COLOR=0x036f;
	LCD_Draw_Circle(230,10,5);
	POINT_COLOR=RED;
	LCD_Draw_Circle(10,310,5);
	while(PEN);
	TP_Read_XY(&xtemp,&ytemp);
	P_x[2]=xtemp;
	P_y[2]=ytemp;
	while(!PEN);
	delay_ms(300);
	
	POINT_COLOR=0x036f;
	LCD_Draw_Circle(10,310,5);
	POINT_COLOR=RED;
	LCD_Draw_Circle(230,310,5);
	while(PEN);
	TP_Read_XY(&xtemp,&ytemp);
	P_x[3]=xtemp;
	P_y[3]=ytemp;
	while(!PEN);
	delay_ms(300);
	
	POINT_COLOR=0x036f;
	LCD_Draw_Circle(230,310,5);
	POINT_COLOR=RED;
	LCD_Draw_Circle(120,160,5);
	while(PEN);
	TP_Read_XY(&xtemp,&ytemp);
	XL=xtemp;
	YL=ytemp;
	while(!PEN);
	delay_ms(300);
	
	POINT_COLOR=0x036f;
	LCD_Draw_Circle(120,160,5);
	POINT_COLOR=RED;
	
	S1=P_x[1]-P_x[0];
	S2=P_x[3]-P_x[2];
	S3=P_y[2]-P_y[0];
	S4=P_y[3]-P_y[1];
				
	Kx=(S1+S2)/440;
	Ky=(S3+S4)/600;
	
	LCD_ShowNum(80,20,(uint16_t)(Kx*1000),6,16);
	LCD_ShowNum(80,40,(uint16_t)(Ky*1000),6,16);
	
	LCD_ShowNum(80,60,XL,6,16);
	LCD_ShowNum(80,80,YL,6,16);
}

void GetXy(u16 *x,u16 *y)
{
//	u16 xtemp,ytemp;
//	TP_Read_XY2(&xtemp,&ytemp);
//	
////	xtemp=TP_Read_XOY(CMD_RDX);
////	ytemp=TP_Read_XOY(CMD_RDY);
//	*x=(2068-xtemp)/15.16+120;
//	*y=(ytemp-1865)/10.942+160;
	
	u16 xtemp,ytemp;
	TP_Read_XY2(&ytemp,&xtemp);
	
//	xtemp=TP_Read_XOY(CMD_RDX);
//	ytemp=TP_Read_XOY(CMD_RDY);
	*x=(2068-xtemp)/10.942+144;
	*y=(1865-ytemp)/15.16+135;
	
}


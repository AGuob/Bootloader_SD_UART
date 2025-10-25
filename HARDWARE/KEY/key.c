#include "key.h"
#include "delay.h" 
#include "stm32f4xx.h" 
#include "stm32f4xx_tim.h" 


//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

//������ʼ������
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOA,GPIOEʱ��

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3|GPIO_Pin_4; //KEY0 KEY1 KEY2��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//WK_UP��Ӧ����PA0
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA0
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period =10000 - 1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler =168;		//PSC 168000000
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);	

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	
	TIM_Cmd(TIM2,DISABLE);	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	
	NVIC_Init(&NVIC_InitStructure);
	
	
} 
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��WKUP���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(WK_UP==1)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1; 	    
 	return 0;// �ް�������
}
u8 KEY_GetState(void)
{	 
	static u16 keyflag1=0,keyflag2=0;//�������ɿ���־
			
	if(KEY0==0)keyflag1++;
	
	if(keyflag1)keyflag2++;
	if(KEY0==1)
	{	
		if(keyflag2<=200)return 1;
		keyflag2=0;
	}
	if(keyflag1==500)
	{
		keyflag1=0;
		if(keyflag2>=100&&keyflag2<=200)return 1;
		if(keyflag2>=300)return 2;
		if(keyflag2>=450)return 3;
	}
	
 	return 0;// �ް�������
}
#define N_key    0             //�޼�
#define S_key    1             //����
#define D_key    2             //˫��
#define L_key    3             //����

#define key_state_0 0
#define key_state_1 1
#define key_state_2 2   
#define key_state_3 3   

unsigned char key_driver(void)
{
    static unsigned char key_state = key_state_0, key_time = 0;
    unsigned char key_press, key_return = N_key;

    key_press = KEY0;                  // ������I/O��ƽ

    switch (key_state)
    {
      case key_state_0:                            // ������ʼ̬
        if (!key_press) key_state = key_state_1;  //�������£�״̬ת��������������ȷ��״̬
        break;
      
      case key_state_1:                      // ����������ȷ��̬
        if (!key_press)
        {
             key_time = 0;                   //  
             key_state = key_state_2;   // ������Ȼ���ڰ��£�������ɣ�״̬ת�������¼�ʱ�� 
                                                          // �ļ�ʱ״̬�������صĻ����޼��¼�
        }
        else
             key_state = key_state_0;   // ������̧��ת����������ʼ̬���˴���ɺ�ʵ����� 
                                                          // ��������ʵ�����İ��º��ͷŶ��ڴ������ġ�
        break;
      
      case key_state_2:
        if(key_press)
        {
             key_return = S_key;           // ��ʱ�����ͷţ�˵���ǲ���һ�ζ̲���������S_key
             key_state = key_state_0;   // ת����������ʼ̬
        }
        else if (++key_time >= 70)    // �������£���ʱ��10ms��10msΪ������ѭ��ִ�м����
        {
             key_return = L_key;            // ����ʱ��>1000ms���˰���Ϊ�������������س����¼�
             key_state = key_state_3;   // ת�����ȴ������ͷ�״̬
        }
        break;

      case key_state_3:                 // �ȴ������ͷ�״̬����״ֻ̬�����ް����¼�
		  key_return=L_key;
        if (key_press) key_state = key_state_0; //�������ͷţ�ת����������ʼ̬
        break;
    }
    return key_return;
}
  unsigned char key_read(void)
{
    static unsigned char key_m = key_state_0, key_time_1 = 0;
    unsigned char key_return = N_key,key_temp;
     
    key_temp = key_driver();
     
    switch(key_m)
    {
        case key_state_0:
            if (key_temp == S_key )
            {
                 key_time_1 = 0; // ��1�ε����������أ����¸�״̬�жϺ����Ƿ����˫��
                 key_m = key_state_1;
            }
            else
                 key_return = key_temp;        // �����޼�������������ԭ�¼�
            break;

        case key_state_1:
            if (key_temp == S_key)             // ��һ�ε���������϶�<500ms��
            {
                 key_return = D_key;           // ����˫�����¼����س�ʼ״̬
                 key_m = key_state_0;
            }
            else                                
            {                         // ����500ms�ڿ϶������Ķ����޼��¼�����Ϊ 
                                      // ����>1000ms����1sǰ�Ͳ㷵�صĶ����޼�
                 if(++key_time_1 >= 20)
                 {
                     key_return = S_key; //500ms��û���ٴγ��ֵ����¼���������һ�� 
                                         // �ĵ����¼�
                     key_m = key_state_0;   // ���س�ʼ״̬
                 }
             }
             break;
    }
    return key_return;
}   

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
	
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}























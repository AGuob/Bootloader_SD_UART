#include "key.h"
#include "delay.h" 
#include "stm32f4xx.h" 
#include "stm32f4xx_tim.h" 


//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//按键输入驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

//按键初始化函数
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOA,GPIOE时钟

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3|GPIO_Pin_4; //KEY0 KEY1 KEY2对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//WK_UP对应引脚PA0
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA0
	
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
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，WKUP按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(WK_UP==1)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1; 	    
 	return 0;// 无按键按下
}
u8 KEY_GetState(void)
{	 
	static u16 keyflag1=0,keyflag2=0;//按键按松开标志
			
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
	
 	return 0;// 无按键按下
}
#define N_key    0             //无键
#define S_key    1             //单键
#define D_key    2             //双键
#define L_key    3             //长键

#define key_state_0 0
#define key_state_1 1
#define key_state_2 2   
#define key_state_3 3   

unsigned char key_driver(void)
{
    static unsigned char key_state = key_state_0, key_time = 0;
    unsigned char key_press, key_return = N_key;

    key_press = KEY0;                  // 读按键I/O电平

    switch (key_state)
    {
      case key_state_0:                            // 按键初始态
        if (!key_press) key_state = key_state_1;  //键被按下，状态转换到按键消抖和确认状态
        break;
      
      case key_state_1:                      // 按键消抖与确认态
        if (!key_press)
        {
             key_time = 0;                   //  
             key_state = key_state_2;   // 按键仍然处于按下，消抖完成，状态转换到按下键时间 
                                                          // 的计时状态，但返回的还是无键事件
        }
        else
             key_state = key_state_0;   // 按键已抬起，转换到按键初始态。此处完成和实现软件 
                                                          // 消抖，其实按键的按下和释放都在此消抖的。
        break;
      
      case key_state_2:
        if(key_press)
        {
             key_return = S_key;           // 此时按键释放，说明是产生一次短操作，回送S_key
             key_state = key_state_0;   // 转换到按键初始态
        }
        else if (++key_time >= 70)    // 继续按下，计时加10ms（10ms为本函数循环执行间隔）
        {
             key_return = L_key;            // 按下时间>1000ms，此按键为长按操作，返回长键事件
             key_state = key_state_3;   // 转换到等待按键释放状态
        }
        break;

      case key_state_3:                 // 等待按键释放状态，此状态只返回无按键事件
		  key_return=L_key;
        if (key_press) key_state = key_state_0; //按键已释放，转换到按键初始态
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
                 key_time_1 = 0; // 第1次单击，不返回，到下个状态判断后面是否出现双击
                 key_m = key_state_1;
            }
            else
                 key_return = key_temp;        // 对于无键、长键，返回原事件
            break;

        case key_state_1:
            if (key_temp == S_key)             // 又一次单击（间隔肯定<500ms）
            {
                 key_return = D_key;           // 返回双击键事件，回初始状态
                 key_m = key_state_0;
            }
            else                                
            {                         // 这里500ms内肯定读到的都是无键事件，因为 
                                      // 长键>1000ms，在1s前低层返回的都是无键
                 if(++key_time_1 >= 20)
                 {
                     key_return = S_key; //500ms内没有再次出现单键事件，返回上一次 
                                         // 的单键事件
                     key_m = key_state_0;   // 返回初始状态
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























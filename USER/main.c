#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch_cs.h" 
#include "update.h"
#include "ff.h"
#include "iap.h" 



FATFS sdcard_fs;


FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FIL fnew;													/* �ļ����� */
FRESULT res_sd;                /* �ļ�������� */
UINT fnum;            					  /* �ļ��ɹ���д���� */
BYTE ReadBuffer[1024]={0};        /* �������� */
BYTE WriteBuffer[] =              /* д������*/
"sihiogsg dggr\r\n";  



int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);
	delay_init(168);
			
 	LCD_Init();	
	LCD_Fill(0,0,320,240,0x036f);
	KEY_Init(); 		
//	sdcard_update();
	
	LCD_ShowString(4,4,100,25,24,"App0Running...");
while(1)
	{
	if(KEY_Scan(0)==1)
 	{
		if(UPDATE_OK==sdcard_update())
            iap_load_app(0x08010000);
	}
//		LED_RED
//		delay_ms(200);
//		LED_GREEN
//		delay_ms(200);
//		LED_RED
////		delay_ms(500);
////		LED_GREEN
////		delay_ms(500);
//		iap_load_app(0x08010000);
	}
}










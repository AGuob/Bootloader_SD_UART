#include <stm32f4xx.h>
#include "lcd.h"
#include "delay.h"  
#include "Linklist_test.h" 


void Linklist_init(void)
{
	student s1={"zhang",20,Male,NULL};
	student s2={"Li",18,Male,NULL};
	student s3={"wang",21,Female,NULL};
//		.name="wang",
//		.age=21,
//		.next=NULL
//	};//	

	s1.next=&s2;
	s2.next=&s3;
	
	
	
}
void Link_show(student *P)
{
	while(P!=NULL)
	{
		LCD_ShowNum(1,1,P->age,5,16); 
		LCD_ShowString(1,57,120,50,16,P->name);
		P=P->next;
		delay_ms(1000);
	}
	
}
void Link_test(void)
{
	
	
}




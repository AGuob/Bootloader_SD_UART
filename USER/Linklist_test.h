#include <stm32f4xx.h>

typedef enum{
	Male,
	Female	
}gender_type;

typedef struct stu{
	char name[10];
	uint8_t age;
	gender_type gender;
	struct stu *next;
}student;


void Linklist_init(void);
void Link_show(student *P);







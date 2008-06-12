#define GPBCON        (*(volatile unsigned *)0x56000010)
#define GPBDAT        (*(volatile unsigned *)0x56000014)
#define GPBDW         (*(volatile unsigned *)0x56000018)   
#define LED3_ON()    (GPBDAT &= ~(0x1))  
#define LED4_ON()    (GPBDAT &= ~(0x2))   
#define LED3_OFF()     (GPBDAT |= (0x1))
#define LED4_OFF()     (GPBDAT |= (0x2))

extern void delay(int time);

int main()
{
    GPBCON = 0x5;
    GPBDW = 0xffff;
    while(1)
    {
	LED3_ON();
	delay(0xffff);
	LED3_OFF() ;
	delay(0xffff);

	LED4_ON();
	delay(0xffff);
	LED4_OFF();
	delay(0xffff);   
    }
    return 1;
}


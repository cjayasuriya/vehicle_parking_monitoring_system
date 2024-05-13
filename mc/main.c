#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define D4 eS_PORTC4
#define D5 eS_PORTC5
#define D6 eS_PORTC6
#define D7 eS_PORTC7
#define RS eS_PORTB0
#define EN eS_PORTB2

#define F_CPU 8000000UL

//Trigger pins
#define  tri1	PA1
#define  tri2	PA2
#define  tri3   PA3
#define  tri4   PA4

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU/ (USART_BAUDRATE*16UL)))-1)

#include "lcd.h"

void init_lcd(void);
double ultrasonic(int t_pin);

void uart_init(void);
void uart_putstring(char *s);
void uart_putchar(char c);
void exportData(int s1, int s2, int s3, int s4, int avb, int nAvb);

static volatile int pulse = 0;
static volatile int i = 0;

int TimerOverflow = 0;
long count;
int trigPoint = 20;
int inCount = 0;
char buffer[5];

//Slots availability
int s1Avb = 0;
int s2Avb = 0;
int s3Avb = 0;
int s4Avb = 0;



ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;
}

int main(void){

	char string[10];
	double distance1;
	double distance2;
    double distance3;
    double distance4;
	double distance5;
	
	DDRA = 0b11111111;  //Make trigger pins as output
	
	DDRD = 0x00;
	PORTD = 0x00;		/* Turn on Pull-up */
	
	DDRC = 0xFF;

	init_lcd();
    uart_init();
	
	sei();
	TIMSK = (1 << TOIE1);	//Enable Timer1 overflow interrupts
	TCCR1A = 0;
	
    PORTC = 0x00;
	
	while(1) {
		
        //US1
		distance1=ultrasonic(tri1); //PA1
        
		dtostrf(distance1, 2, 2, string);
        Lcd4_Set_Cursor(1,0);
        Lcd4_Write_String(string);
        
        if(distance1<trigPoint){
            inCount++;
			PORTC |=(1<<PC0);
            s1Avb = 1;
        }else{
			PORTC &= ~(1<<PC0);
            s1Avb = 0;
        }
		
		_delay_ms(300);
		
        //US2
        distance2=ultrasonic(tri2); //PA2
        
        dtostrf(distance2, 2, 2, string);
        Lcd4_Set_Cursor(2,0);
        Lcd4_Write_String(string);

        if(distance2<trigPoint){
            inCount++;
			PORTC |=(1<<PC1);
            s2Avb = 1;
        }else{
			PORTC &= ~(1<<PC1);
            s2Avb = 0;
		}

        _delay_ms(300);

       //US3
        distance3=ultrasonic(tri3); //PA3
        
		dtostrf(distance3, 2, 2, string);
        Lcd4_Set_Cursor(1,8);
        Lcd4_Write_String(string);
		
		if(distance3<trigPoint){
          inCount++;
            PORTC |=(1<<PC2);
            s3Avb = 1;
        }else{
			PORTC &= ~(1<<PC2);
            s3Avb = 0;
		}

        _delay_ms(300);

        //US4
        distance4=ultrasonic(tri4); //PA4
        
		dtostrf(distance4, 2, 2, string);
        Lcd4_Set_Cursor(2,8);
        Lcd4_Write_String(string);
        
        if(distance4<trigPoint){
            inCount++;
            PORTC |=(1<<PC3);
            s4Avb = 1;
        }else{
            PORTC &= ~(1<<PC3);
            s4Avb = 0;
        }

        _delay_ms(100);
        
        //No of not avb
		itoa(inCount, buffer, 10);
        Lcd4_Set_Cursor(2,13);
		Lcd4_Write_String(buffer);

        //No of avb
        itoa((4-inCount), buffer, 10);
        Lcd4_Set_Cursor(2,14);
        Lcd4_Write_String(buffer);
        
        exportData(s1Avb,s2Avb,s3Avb,s4Avb,(4-inCount),inCount);
        

        inCount = 0;
        //_delay_ms(100);
		
	}

	return 0;
}


double ultrasonic(int t_pin){
        /* Give 10us trigger pulse on trig. pin to HC-SR04 */
		PORTA |= (1 << t_pin);
		_delay_us(10);
		PORTA &= (~(1 << t_pin));
		
		TCNT1 = 0;	/* Clear Timer counter */
		TCCR1B = 0x41;	/* Capture on rising edge, No prescaler*/
		TIFR = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
		TIFR = 1<<TOV1;	/* Clear Timer Overflow flag */

		/*Calculate width of Echo by Input Capture (ICP) */
		
		while ((TIFR & (1 << ICF1)) == 0 && TimerOverflow<1);/* Wait for rising edge */
		TCNT1 = 0;	/* Clear Timer counter */
		TCCR1B = 0x01;	/* Capture on falling edge, No prescaler */
		TIFR = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
		TIFR = 1<<TOV1;	/* Clear Timer Overflow flag */
		TimerOverflow = 0;/* Clear Timer overflow count */

		while ((TIFR & (1 << ICF1)) == 0 && TimerOverflow<1);/* Wait for falling edge */
		count = ICR1 + (65535 * TimerOverflow);	/* Take count */
		/* 8MHz Timer freq, sound speed =343 m/s */
		return (double)(count / 466.47)/1.472;
}

void init_lcd(void){
	DDRB=0xff;
	PORTB = (1<<PORTB2);
	Lcd4_Init();
	Lcd4_Clear();
	_delay_ms(100);
}

void exportData(int s1, int s2, int s3, int s4, int avb, int nAvb){
    
    itoa(s1, buffer, 10);
    uart_putstring(buffer);
    uart_putstring(",");
    
    itoa(s2, buffer, 10);
    uart_putstring(buffer);
    uart_putstring(",");
    
    itoa(s3, buffer, 10);
    uart_putstring(buffer);
    uart_putstring(",");
    
    itoa(s4, buffer, 10);
    uart_putstring(buffer);
    uart_putstring(",");
    
    itoa(avb, buffer, 10);
    uart_putstring(buffer);
    uart_putstring(",");
    
    itoa(nAvb, buffer, 10);
    uart_putstring(buffer);
    
}

void uart_putchar(char c){
    while((UCSRA & (1<< UDRE)) == 0);
    UDR = c;
}

void uart_putstring(char *s){
    while(*s){
        uart_putchar(*s);
        *s++;
    }
}

void uart_init(void){
    UCSRB |= (1<< RXEN) | (1<< TXEN);
    UCSRC |= (1<< URSEL)| (1<< UCSZ0) | (1<< UCSZ1);
    UBRRH = (BAUD_PRESCALE>>8);
    UBRRL = BAUD_PRESCALE;
}

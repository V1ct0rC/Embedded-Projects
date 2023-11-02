#include <REG51F.H>
#define FrClk 12000000
#define FreqTimer0_emHz 100
#define TH0_Inicial ((65536 - (FrClk / (12*FreqTimer0_emHz)) + 9) >> 8)
#define TL0_Inicial ((65536 - (FrClk / (12*FreqTimer0_emHz)) + 9) & 0xFF)
//Nas defini��es acima, CORRECAO = 9 (quantidade de opera��es durante o contador parado em timer0_int())

//Defini��es necess�rias
int timer_B0; //Vari�vel do timer do bit0
int timer_B1; //Vari�vel do timer do bit1

sbit lwr = P2^0;
sbit upr = P2^1;
typedef enum {
    STATE_0, //Espera transi��o dos bits de P2 (0 para 1)
    STATE_1, //Conta 1 segundo para zerar P1
    STATE_2  //Espera transi��o dos bits de P2 (1 para 0)
} State;

//Configura��o do Timer
void timer0_inicializa(){
	TR0 = 0; //Desliga Timer
	
	//GATE = 0; C/~T = 0;
	TMOD = (TMOD & 0xF0) | 0x01; //Timer 0 no modo 1
	
	//Programa o valor de contagem do Timer0
	TH0 = TH0_Inicial;
	TL0 = TL0_Inicial;
	
	ET0 = 1; //Habilita interrup��es Timer0
	TR0 = 1; //Liga Timer0
}

//Interrup��o do timer
void timer0_int() interrupt 1 using 2{
	timer_B0 ++; //Incrementa vari�vel do timer do bit0
	timer_B1 ++; //Incrementa vari�vel do timer do bit1
	
	TR0 = 0; //Desliga Timer
	
	//Recarrega a contagem do Timer0
	TL0 += TL0_Inicial;
	TH0 += TH0_Inicial + (unsigned char) CY;
	
	TR0 = 1; //Liga Timer0
	//9 estados com o timer desligado entre TR0=0 e TR0=1
}

//Lida com os 4 bits MENOS significativos de P0 e P1 e o bit_0 de P2
void bitZero(){
	static State st = STATE_2; //In�cio no terceiro est�gio
	
	switch (st){
		case STATE_0:
			if(lwr){
				//Copia valores de P0 para P1
				P1 = (P1 & 0xF0) | (P0 & 0x0F);
				st = STATE_1;
				timer_B0 = 0;
			}
			break;
			
		case STATE_1:
			if(timer_B0 >= 100){ //Contando 1 segundo (100 * 10ms) 
				//Zera valores de P1 ap�es 1 segundo
				P1 = P1 = P1 & 0xF0;
				timer_B0 = 0; //Reseta contador
				st = STATE_2;
			}
			break;
			
		case STATE_2:
			if(!lwr){
				st = STATE_0;
			}
			break;
	}
}

//Lida com os 4 bits MAIS significativos de P0 e P1 e o bit_1 de P2
void bitUm(){
	static State st = STATE_2; //In�cio no terceiro est�gio
	
	switch (st){
		case STATE_0:
			if(upr){
				//Copia valores de P0 para P1
				P1 = (P1 & 0x0F) | (P0 & 0xF0);
				st = STATE_1;
				timer_B1 = 0;
			}
			break;
			
		case STATE_1:
			if(timer_B1 >= 100){ //Contando 1 segundo (100 * 10ms) 
				//Zera valores de P1 ap�es 1 segundo
				P1 = P1 = P1 & 0x0F;
				timer_B1 = 0; //Reseta contador
				st = STATE_2;
			}
			break;
			
		case STATE_2:
			if(!upr){
				st = STATE_0;
			}
			break;
	}
}


void main(){
	timer0_inicializa();
	EA = 1; //Habilita o tratamento de interrup��es
	while(1){
		bitZero();
		bitUm();
	}
}
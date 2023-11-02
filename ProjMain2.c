#include <REG51F.H>


//Definições necessárias
sbit lwr = P2^0;
sbit upr = P2^1;
typedef enum {
    STATE_0, //Espera transição dos bits de P2 0 para 1
    STATE_1, //Conta 1 segundo para zerar P1
    STATE_2  //Espera transição dos bits de P2 1 para 0
} State;


//Lida com os 4 bits MENOS significativos de P0 e P1 e o bit_0 de P2
void bitZero(){
	static State st = STATE_2; //Início no terceiro estágio
	static int timer = 0;
	
	switch (st){
		case STATE_0:
			if(lwr){
				//Copia valores de P0 para P1
				P1 = (P1 & 0xF0) | (P0 & 0x0F);
				st = STATE_1;
			}
			break;
			
		case STATE_1:
			timer ++;
			if(timer >= 27027){
				//Zera valores de P1 apóes 1 segundo
				P1 = P1 & 0xF0;
				timer = 0; //Reseta timer
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
	static State st = STATE_2; //Início no terceiro estágio
	static int timer = 0;
	
	switch (st){
		case STATE_0:
			if(upr){
				//Copia valores de P0 para P1
				P1 = (P1 & 0x0F) | (P0 & 0xF0);
				st = STATE_1;
			}
			break;
			
		case STATE_1:
			timer ++;
			if(timer >= 27027){
				//Zera valores de P1 apóes 1 segundo
				P1 = P1 = P1 & 0x0F;
				timer = 0; //Reseta timer
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
	while(1){
		bitZero();
		bitUm();
	}
}
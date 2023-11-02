#include <REG51F.H>
#define FrClk 12000000 
#define Baudrate 1200
#define TH1_Inicial (((Baudrate * 98304) - (1 * FrClk)) / (Baudrate * 384)) // = 230 ou (204 smod=1)


//Mensgaens recebidas e enviadas
unsigned char recebido;
unsigned char enviado;

bit recebeu_caractere = 0; //Flag auxiliar para acompanhar recebimento de dados

//Configuração do Timer 1 no modo 2
void timer1_inicializa(){
	//PCON |= 0x80 (set) PCON &=0x7F (reset) o SMOD
	PCON &= 0x7F; //SMOD = 0
	
	TR1 = 0; //Desliga Timer para fazer a configuração
	
	//GATE = 0; C/~T = 0; M1,M0 = 01
	//TMOD = 0 0 1 0 _ _ _ _ 0x20
	TMOD = (TMOD & 0x0F) | 0x20; //Timer1 no modo 2
	
	//Programa o valor de contagem do Timer1
	TH1 = 230;
	TL1 = 230;

	TR1 = 1; //Liga Timer
}

//Configuração da Serial no modo 1
void serial_inicializa(){
	//SCOM = 0 1 0 1 _ _ _ _ Serial no modo 1
	SM0 = 0;
	SM1 = 1;
	SM2 = 0;
	
	REN = 1; //Habilita a recepção de dados
	ES = 1; //Habilita a interrupções da serial
}

void serial_interrupcao() interrupt 4 using 2{
	if(RI){
		RI = 0;	//Reseta RI
		recebido = SBUF; //Recebe dados do SBUF
		recebeu_caractere = 1; 
	}
	if(TI){ 
		TI = 0; //Reseta TI
	}
}

void transmite_char(char c) {
    SBUF = c; //Carrega o caractere a ser transmitido no registrador SBUF
}

void main(){
	timer1_inicializa();
	serial_inicializa();
	EA = 1; //Hanilita interrupções
	
	while(1){
		//Caso um caractére seja recebido, é transmitido (caractere + 1)
		if(recebeu_caractere){ 
			recebeu_caractere = 0; //Reseta a flag
			enviado = recebido + 1;
			
			transmite_char(enviado);
		}
	}
}
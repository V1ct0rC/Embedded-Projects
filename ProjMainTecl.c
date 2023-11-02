#include <REG517A.H>
#define tamBuffer 16 // tamBuffer = 16 bytes
#define FrClk 12000000
#define FreqTimer0_emHz 100
#define TH0_Inicial ((65536 - (FrClk / (12*FreqTimer0_emHz)) + 9) >> 8)
#define TL0_Inicial ((65536 - (FrClk / (12*FreqTimer0_emHz)) + 9) & 0xFF)
//Nas definições acima, CORRECAO = 9 (quantidade de operações durante o contador parado em timer0_int())

// INICIALIZAÇÕES ===========================================================================================
char teclado_Tecla;
bit teclado_TeclaApertada = 0;
const unsigned char teclado_teclas[4][3] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};
// Usar look-up table para saber a tecla
unsigned int teclado_debouncing = 0;
unsigned int teclado_linha = 0;  // 0111 1011 1101 1110 -> 7 B D E
const unsigned int teclado_mascara[10] = {0xF7, 0xFB, 0xFD, 0xFE};

char txBuffer[tamBuffer];
unsigned char in_tx = 0, out_tx = 0;
bit txOcupado = 0;

// Valores para a varredura
sbit P5_4 = P5^4;
sbit P5_5 = P5^5;
sbit P5_6 = P5^6;

// FUNÇÕES AUXILIARES =======================================================================================
void sendChar(char c){
	txBuffer[in_tx] = c; // Recebe o byte “c” e insere em TxBuffer
	in_tx++;
	in_tx %= tamBuffer;
	if (!txOcupado){
		txOcupado = 1;
		TI0 = 1;           // Ativa a transmissão serial, caso ainda não esteja ativa
	}
}

// CONFIGURAÇÕES ============================================================================================
void timer_inicializa(){
	// GATE = 0; C/T = 0; M1 = 0; M0 = 1 Timer 0 no modo 1 (16 bit_ timer sem refill)
	// TMOD = ____0001
	TMOD = (TMOD & 0xF0) | 0x01;
	TR0 = 0;  // Desliga Timer
	
	//Programa o valor de contagem do Timer 0
	TH0 = TH0_Inicial;
	TL0 = TL0_Inicial;
	
	ET0 = 1; //Habilita interrupções Timer 0
	TR0 = 1; //Liga Timer 0
}

void serial_inicializa(){
	// S0COM = 0 1 0 1 _ _ _ _ Serial 0 no modo 1
	SM0 = 0;
	SM1 = 1;
	SM20 = 0;
	
	REN0 = 1; // Habilita a recepção de dados pela serial 0
	ES0 = 1; // Habilita a interrupções da serial 0
	
	BD = 1; // Usar o gerador de baud rate interno da serial 0
	PCON |= 0x80; // Com SMOD = 1, geramos 9600 bits/s de baud rate
	
	// Devido ao uso de 9600 bits/s, não é necessário inicializar o Timer,
	// esse baud rate ja vem automaticamente ao setar BD = 1 e SMOD = 1
}

// ROTINAS DE INTERRUPÇÃO ===================================================================================
typedef enum {
    STATE_0, // Tecla não apertada (nível lógico 1)
    STATE_1, // Tecla apertada (debouncing)
    STATE_2, // Tecla apertada (nível lógico  0)
		STATE_3  // Tecla não apertada (debouncing)
} State;

void timer_interrupt() interrupt 1 using 2{
	// Fazer a varredura de uma linha do teclado a cada 10ms
	// debouncing de 40ms (durante esse tempo, não veja nenhuma outra tecla)
	static State st = STATE_0;
	
	switch (st){
		// -----------------------------------------------------------
		case STATE_0:
			// Abordagem 1 -------------------------------------------
			
			P5 |= 0x0F;
			P5 &= teclado_mascara[teclado_linha];
		
			// Percorre todas as linhas do teclado para ver se alguma tecla foi apertada
			if(!P5_4){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[teclado_linha][0];
			}
			if(!P5_5){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[teclado_linha][1];
			}
			if(!P5_6){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[teclado_linha][2];
			}
		
			if(teclado_TeclaApertada){
				st = STATE_1;
			}
			else{
				// Incrementado ou resetando o índice da linha
				teclado_linha ++;
				if(teclado_linha > 3) teclado_linha = 0;
			}
		
			// Abordagem 2 -------------------------------------------
			/*
			// Percorre todas as linhas do teclado para ver se alguma tecla foi apertada
			P5 = (P5 | 0x0F) & 0xF7;  // Primeira linha
			if(!P5_4){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[0][0];
			}
			if(!P5_5){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[0][1];
			}
			if(!P5_6){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[0][2];
			}
			if(teclado_TeclaApertada){
				st = STATE_1;
				break;
			}

			P5 = (P5 | 0x0F) & 0xFB;  // Segunda linha
			if(!P5_4){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[1][0];
			}
			if(!P5_5){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[1][1];
			}
			if(!P5_6){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[1][2];
			}
			if(teclado_TeclaApertada){
				st = STATE_1;
				break;
			}

			P5 = (P5 | 0x0F) & 0xFD;  // Terceira linha
			if(!P5_4){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[2][0];
			}
			if(!P5_5){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[2][1];
			}
			if(!P5_6){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[2][2];
			}
			if(teclado_TeclaApertada){
				st = STATE_1;
				break;
			}

			P5 = (P5 | 0x0F) & 0xFE;  // Quarta linha
			if(!P5_4){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[3][0];
			}
			if(!P5_5){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[3][1];
			}
			if(!P5_6){
				teclado_TeclaApertada = 1;
				teclado_Tecla = teclado_teclas[3][2];
			}
			if(teclado_TeclaApertada){
				st = STATE_1;
				break;
			}
			*/
			// -------------------------------------------------------
			break;
		// -----------------------------------------------------------
		case STATE_1:
			teclado_debouncing ++;
			if(teclado_debouncing > 4){
				teclado_debouncing = 0;
				st = STATE_2;
			}
			break;
		// -----------------------------------------------------------
		case STATE_2:
			if(P5_4){
				teclado_TeclaApertada = 0;
			}
			if(P5_5){
				teclado_TeclaApertada = 0;
			}
			if(P5_6){
				teclado_TeclaApertada = 0;
			}
			
			if(!teclado_TeclaApertada){
				st = STATE_3;
			}
			break;
		// -----------------------------------------------------------
		case STATE_3:
			teclado_debouncing ++;
			if(teclado_debouncing > 4){
				teclado_debouncing = 0;
				st = STATE_0;
			}
			break;
	}
}

void serial_interrupt() interrupt 4 using 2{
	if(RI0){
		// Nada será recebido
		RI0 = 0;	// Reseta RI0
	}
	if(TI0){ 
		TI0 = 0; //Reseta TI0
		
		if(in_tx==out_tx){ // Buffer vazio
			txOcupado = 0;
		}
		else{ // Retira dados de txBffer para a serial
			S0BUF = txBuffer[out_tx];
			out_tx++;
			out_tx %= tamBuffer;
		}
	}
}

// LOOP PRINCIPAL ===========================================================================================
void main(){	
	timer_inicializa();
	serial_inicializa();
	EAL = 1;
	
	while(1){
		if(teclado_TeclaApertada){
			teclado_TeclaApertada = 0;
			sendChar(teclado_Tecla);
		}
	}
}
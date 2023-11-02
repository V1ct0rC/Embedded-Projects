#include <REG51F.H>
#define FrClk 12000000 // Frquencia de clock
#define Baudrate 1200  // Taxa de transmissao 1200 bps
#define TH1_Inicial (((Baudrate * 98304) - (1 * FrClk)) / (Baudrate * 384)) // = 230 ou (204 smod=1)
#define tamBuffer 16 // tamBuffer = 16 bytes


// INICIALIZAÇÕES ===========================================================================================
// Buffers circulares de recebimento e transmissão
char rxBuffer[tamBuffer];
char txBuffer[tamBuffer];

// Ponteiros dos buffers circulares
unsigned char in_rx = 0, out_rx = 0;
unsigned char in_tx = 0, out_tx = 0;

// Contador de strings recebidas
unsigned char recebeuString = 0;
bit txOcupado = 0;

// FUNÇÕES ==================================================================================================
char RxBufferVazio(){
	return (in_rx == out_rx) ? 1 : 0; // retorna 1 caso o buffer circular de recepção esteja vazio e 0 em caso contrário
}	

void sendChar(char c){
	txBuffer[in_tx] = c; // recebe o byte “c” e insere em TxBuffer
	in_tx++;
	in_tx %= tamBuffer;
	if (!txOcupado){
		txOcupado = 1;
		TI = 1;           // ativa a transmissão serial, caso ainda não esteja ativa
	}
}

void sendString(char *s){
	while(*s != '$'){
		sendChar(*s); // insere a string apontada por “*s” em TxBuffer usando a função sendChar(c)
		s++;
	}
}

char recieveChar(){
	if(!RxBufferVazio()){
		char recebido_rx = rxBuffer[out_rx]; // retorna um byte de RxBuffer
		out_rx++;
		out_rx %= tamBuffer;
	
		return recebido_rx;
	}
	else return 0;
}
	
void recieveString(char *s){
	char recebido = 0;
	
	while(recebido != '$'){
		recebido = recieveChar();
		*s = recebido; // copia uma string de RxBufffer para o local apontado por “*s”
		s++;
	}
	recebeuString--; // decrementa o valor de recebeuString
}

// ROTINA DE INTERRUPÇÃO ====================================================================================
void serial_interrupcao() interrupt 4 using 2{
	if(RI){
		unsigned char recebido_sbuf;
		RI = 0;	// Reseta RI
		recebido_sbuf = SBUF; // Recebe dados do SBUF
		
		// Escreve dados recebido em rxBuffer
		if(((in_rx+1) % tamBuffer) != out_rx){ // Se não estiver cheio
			rxBuffer[in_rx] = recebido_sbuf;
			in_rx++;
			in_rx %= tamBuffer;
		}
		if(recebido_sbuf == '$'){ // $ respesenta o fim de uma mensagem
			recebeuString++;
		}
	}
	if(TI){ 
		TI = 0; // Reseta TI
		
		if(in_tx==out_tx){ // Buffer vazio
			txOcupado = 0;
		}
		else{ // Retira dados de txBffer para a serial
			SBUF = txBuffer[out_tx];
			out_tx++;
			out_tx %= tamBuffer;
		}
	}
}

// CONFIGURAÇÕES ============================================================================================
// Configuração do Timer 1 no modo 2
void timer1_inicializa(){
	// PCON |= 0x80 (set) PCON &=0x7F (reset) o SMOD
	PCON |= 0x80; // SMOD = 1
	
	TR1 = 0; // Desliga Timer para fazer a configuração
	
	// GATE = 0; C/~T = 0; M1,M0 = 01
	// TMOD = 0 0 1 0 _ _ _ _ 0x20
	TMOD = (TMOD & 0x0F) | 0x20; //Timer1 no modo 2
	
	// Programa o valor de contagem do Timer1
	TH1 = 204;
	TL1 = 204;

	TR1 = 1; // Liga Timer
}

//Configuração da Serial no modo 1
void serial_inicializa(){
	// SCOM = 0 1 0 1 _ _ _ _ Serial no modo 1
	SM0 = 0;
	SM1 = 1;
	SM2 = 0;
	
	REN = 1; // Habilita a recepção de dados
	ES = 1;  // Habilita a interrupções da serial
}

// MAIN =====================================================================================================
void main(){
	char str_recebida[16];
	
	timer1_inicializa();
	serial_inicializa();
	EA = 1; //Habilita interrupções
	
	while(1){
		if(recebeuString > 0){  // Testando implementação
			recieveString(str_recebida);
			sendString(str_recebida);
		}
	}
}
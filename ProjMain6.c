#include <REG517A.H>
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
	return (in_rx == out_rx) ? 1 : 0; // Retorna 1 caso o buffer circular de recepção esteja vazio e 0 em caso contrário
}	

void sendChar(char c){
	txBuffer[in_tx] = c; // Recebe o byte “c” e insere em TxBuffer
	in_tx++;
	in_tx %= tamBuffer;
	if (!txOcupado){
		txOcupado = 1;
		TI0 = 1;           // Ativa a transmissão serial, caso ainda não esteja ativa
	}
}

void sendString(char *s){
	while(*s != '$'){
		sendChar(*s); // Insere a string apontada por “*s” em TxBuffer usando a função sendChar(c)
		s++;
	}
}

char recieveChar(){
	if(!RxBufferVazio()){
		char recebido_rx = rxBuffer[out_rx]; // Retorna um byte de RxBuffer
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
		*s = recebido; // Copia uma string de RxBufffer para o local apontado por “*s”
		s++;
	}
	recebeuString--; // Decrementa o valor de recebeuString
}

// ROTINA DE INTERRUPÇÃO ====================================================================================
void serial_interrupcao() interrupt 4 using 2{
	if(RI0){
		unsigned char recebido_sbuf;
		RI0 = 0;	// Reseta RI0
		recebido_sbuf = S0BUF; // Recebe dados do SBUF
		
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

// CONFIGURAÇÕES ============================================================================================
// Configuração da Serial 0 no modo 1 (8bit UART, baud rate variável)
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

void main(){
	char str_recebida[16];
	serial_inicializa(); // Inicialização da porta seria 0
	EAL = 1; // Habilita interrupções
	
	while(1){
		if(recebeuString > 0){
			recieveString(str_recebida);
			sendString(str_recebida);
		}
	}
}
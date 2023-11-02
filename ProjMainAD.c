#include <REG517A.H>
#define tamBuffer 16 // tamBuffer = 16 bytes

// INICIALIZAÇÕES ===========================================================================================
int timer_ = 0;
char txBuffer[tamBuffer];
unsigned char in_tx = 0, out_tx = 0;
bit txOcupado = 0;

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

void sendString(char *s){
	while(*s != '$'){
		sendChar(*s); // Insere a string apontada por “*s” em TxBuffer usando a função sendChar(c)
		s++;
	}
}

int i = 0;
void floatToString(float num, char* str) {
	// Separando a parte inteira da parte decimal (2 casas)
	int valor_int = (int)num;
	int dacimal = (int)((num - valor_int) * 100);
	
	// Convertendo a parte inteira
	i = 0;
	str[i] = '(';  // Para uma melhor visualização '('x.xx)
	i++;
	
	while (valor_int > 0) {
			str[i] = (valor_int % 10) + '0'; // Convertendo os digitos inteiros para ASCII
			valor_int /= 10;
			i++;
	}

	// Adicionando o ponto (x'.'xx)
	str[i] = '.';
	i++;

	// Convertendo os digitos decmais para ASCII
	str[i] = (dacimal / 10) + '0';
	i++;
	str[i] = (dacimal % 10) + '0';
	i++;
	
	str[i] = ')';  // Para uma melhor visualização (x.xx')'
	i++;
	
	// Eof da função sendString
	str[i] = '$';
	i++;
	str[i] = '\0';
}

// CONFIGURAÇÕES ============================================================================================
void timer_inicializa(){
	// GATE = 0; C/T = 0; M1 = 1; M0 = 0 Timer 0 no modo 2 (8 bit timer com refill)
	// TMOD = ____0010
	TMOD = (TMOD & 0xF0) | 0x02;
	TR0 = 0;  // Desliga Timer
	
	// 1MHz = 1us
	// 255-100 = 0.1ms (interrompe a cada 0.1ms)
	// timer_ precisa contar até 10000 para termos 1 segundo
	TH0 = 155;
	TL0 = 155;
	
	TR0 = 1;  // Liga Timer
	ET0 = 1;  // Habilita interrupções do timer 0
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

void ad_inicializa(){
	// ADEX = 0; ADM = 1; MX3 = 0; MX2 = 0; MX1 = 0; MX0 = 0;  Conversão contínua com leitura na entrada P7.0
	// ADCON0 = __001000; ADCON1 = ____0000
	ADCON0 = (ADCON0 & 0xC8) | 0x08;
	ADCON1 = (ADCON1 & 0xF0);
	
	// DAPR = 0000 As tensões de referência internas correspondem às tensões de referência externas
	ADDATL = 0x00;  // Escrever em ADDATL começa a conversão
}

// ROTINAS DE INTERRUPÇÃO ===================================================================================
void timer_interrupt() interrupt 1 using 2{
	timer_++;
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
	char valor_convertido[16];
	int dec = 0;
	float flo = 0.0;
	unsigned char bin;
	
	float scale = 0.1;
	
	timer_inicializa();
	serial_inicializa();
	ad_inicializa();
	
	P7 = 0;
	EAL = 1;
	while(1){
		if(timer_ >= 10000){
			timer_ = 0;
			// Resultado da conversão de 8 bits em ADDATH
			bin = ADDATH;
			
			dec = (int) bin;
			flo = (dec * scale)/5.1;  // Convertendo de binário pra float_
			floatToString(flo, valor_convertido);
			
			sendString(valor_convertido);
		}
	}
}

#include <REG517A.H>
#define maxPower 100 // Pot�ncia m�xima (100%)

// INICIALIZA��ES ===========================================================================================
unsigned int led_power = 0; 
unsigned int valor_recebido = 0; 
bit recebeu = 0;

const unsigned int pwm_lut[10] = {0xFFFF, 0xFFDF, 0xFFC3, 0xFFA7, 0xFF8B, 0xFF6F, 0xFF53, 0xFF37, 0xFF1B, 0xFF00};

// ROTINA DE INTERRUP��O ====================================================================================
// Apenas coloca o recebido na serial para a vari�vel
void serial_interrupcao() interrupt 4 using 2{
	if(RI0){
		RI0 = 0;	// Reseta RI0
		valor_recebido = S0BUF;  // Recebe dados do SBUF
		valor_recebido -= 48;  // Convers�o de tipos
		recebeu = 1;
	}
	if(TI0){ 
		TI0 = 0; //Reseta TI0
	}
}

// CONFIGURA��ES ============================================================================================
// Configura��o da Serial 0 no modo 1 (8bit UART, baud rate vari�vel)
void serial_inicializa(){
	// S0COM = 0 1 0 1 _ _ _ _ Serial 0 no modo 1
	SM0 = 0;
	SM1 = 1;
	SM20 = 0;
	
	REN0 = 1; // Habilita a recep��o de dados pela serial 0
	ES0 = 1; // Habilita a interrup��es da serial 0
	
	BD = 1; // Usar o gerador de baud rate interno da serial 0
	PCON |= 0x80; // Com SMOD = 1, geramos 9600 bits/s de baud rate
	
	// Devido ao uso de 9600 bits/s, n�o � necess�rio inicializar o Timer,
	// esse baud rate ja vem automaticamente ao setar BD = 1 e SMOD = 1
}

void pwm_inicializa(){
	CTCON &= 0xF8;  // CLK2 CLK1 CLK0 = 0
	CTCON |= 0x00;  // 0 0 0  fosc/2
	
	CMSEL = 0xFF;  // Todos os registradores CMx ir�o trabalhar com o CompareTimer (compare mode 0)
	CMEN = 0xFF;  // Fun��o de compara��o ativada para todos os CMx
	
	// CMx s�o registradores de 16 bits
	// Cada CMx representar� P4.x
	// CMx = 0xFFFF far� P4.x = 0
	CM0 = 0xFFFF; CM1 = 0xFFFF; CM2 = 0xFFFF; CM3 = 0xFFFF;
	CM4 = 0xFFFF; CM5 = 0xFFFF; CM6 = 0xFFFF; CM7 = 0xFFFF;
	
	// Em uma configura��o de CompareTimer/CMx, a sa�da de compara��o � definida para um n�vel alto 
  // constante se o conte�do dos registradores de compara��o for igual ao registrador de recarga.
  // (CTREL)
	
	// CTREL = 0xFF00
	CTRELH = 0xFF;
	CTRELL = 0x00;
}

void define_potencia(){
	CM0 = pwm_lut[valor_recebido];
	CM1 = pwm_lut[valor_recebido];
	CM2 = pwm_lut[valor_recebido];
	CM3 = pwm_lut[valor_recebido];
	CM4 = pwm_lut[valor_recebido];
	CM5 = pwm_lut[valor_recebido];
	CM6 = pwm_lut[valor_recebido];
	CM7 = pwm_lut[valor_recebido];
}

void main(){
	P4 = 0x00;
	serial_inicializa(); // Inicializa��o da porta serial 0 no modo 1
	pwm_inicializa();
	EAL = 1; // Habilita interrup��es
	
	while(1){
		define_potencia();
	}
}
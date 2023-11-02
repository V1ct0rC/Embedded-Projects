#include <REG517A.H>
#define maxPower 100 // Potência máxima (100%)

// INICIALIZAÇÕES ===========================================================================================
unsigned int led_power = 0; 
unsigned int valor_recebido = 0; 
bit recebeu = 0;

const unsigned int pwm_lut[10] = {0xFFFF, 0xFFDF, 0xFFC3, 0xFFA7, 0xFF8B, 0xFF6F, 0xFF53, 0xFF37, 0xFF1B, 0xFF00};

// ROTINA DE INTERRUPÇÃO ====================================================================================
// Apenas coloca o recebido na serial para a variável
void serial_interrupcao() interrupt 4 using 2{
	if(RI0){
		RI0 = 0;	// Reseta RI0
		valor_recebido = S0BUF;  // Recebe dados do SBUF
		valor_recebido -= 48;  // Conversão de tipos
		recebeu = 1;
	}
	if(TI0){ 
		TI0 = 0; //Reseta TI0
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

void pwm_inicializa(){
	CTCON &= 0xF8;  // CLK2 CLK1 CLK0 = 0
	CTCON |= 0x00;  // 0 0 0  fosc/2
	
	CMSEL = 0xFF;  // Todos os registradores CMx irão trabalhar com o CompareTimer (compare mode 0)
	CMEN = 0xFF;  // Função de comparação ativada para todos os CMx
	
	// CMx são registradores de 16 bits
	// Cada CMx representará P4.x
	// CMx = 0xFFFF fará P4.x = 0
	CM0 = 0xFFFF; CM1 = 0xFFFF; CM2 = 0xFFFF; CM3 = 0xFFFF;
	CM4 = 0xFFFF; CM5 = 0xFFFF; CM6 = 0xFFFF; CM7 = 0xFFFF;
	
	// Em uma configuração de CompareTimer/CMx, a saída de comparação é definida para um nível alto 
  // constante se o conteúdo dos registradores de comparação for igual ao registrador de recarga.
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
	serial_inicializa(); // Inicialização da porta serial 0 no modo 1
	pwm_inicializa();
	EAL = 1; // Habilita interrupções
	
	while(1){
		define_potencia();
	}
}
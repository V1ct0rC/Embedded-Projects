#include <REG51F.H>

sbit lwr = P2^0;
sbit upr = P2^1;

void main(){
	while(1){
		// Ultimos 4 bits de P0 para P1
		if(lwr) P1 = (P1 & 0xF0) | (P0 & 0x0F);
		else P1 = P1 & 0xF0;
		
		// Primeiros 4 bits de P0 para P1
		if(upr) P1 = (P1 & 0x0F) | (P0 & 0xF0);
		else P1 = P1 & 0x0F;
	}
}
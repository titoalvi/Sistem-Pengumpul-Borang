
#include "DrvGPIO.h"
#include "DrvSYS.h"
#include "ScanKey.h"
#include "LCD_Driver.h"
#include "DrvADC.h"
#include "UART.h"
#include "PWM.h"
#include "Seven_Segment.h"

//Define gak jelas---------
#define  ONESHOT  0   // counting and interrupt when reach TCMPR number, then stop
#define  PERIODIC 1   // counting and interrupt when reach TCMPR number, then counting from 0 again
#define  TOGGLE   2   // keep counting and interrupt when reach TCMPR number, tout toggled (between 0 and 1)
#define  CONTINUOUS 3 // keep counting and interrupt when reach TCMPR number

//define properti servo
//#define  SERVO_CYCTIME        2000 // 20ms = 50Hz
#define  SERVO_HITIME_MIN       50 // minimum Hi width = 0.5ms
#define  SERVO_HITIME_MAX      250 // maximum Hi width = 2.5ms
#define  STEPTIME               10 // incremental time = 1.0ms
//define gak jelas---------

int key,adc, key2;
void Init();
volatile uint8_t  comRbuf[16];
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";
int hitung=0;
int hitungabsen=0;

//----
void seg_display(int value) //coba-coba gak jelas
{
	  int digit;
		digit = value / 1000;
		close_seven_segment();
		show_seven_segment(3,digit);
		//DrvSYS_Delay(5000);

		value = value - digit * 1000;
		digit = value / 100;
		close_seven_segment();
		show_seven_segment(2,digit);
		//DrvSYS_Delay(5000);

		value = value - digit * 100;
		digit = value / 10;
		close_seven_segment();
		show_seven_segment(1,digit);
		//delay(5000);

		value = value - digit * 10;
		digit = value;
		close_seven_segment();
		show_seven_segment(0,digit);
		//DrvSYS_Delay(5000);
}


//inisialisasi variabel PWM
	//uint8_t i;
    uint8_t hitime;
	//char TEXT0[16],TEXT1[16],TEXT2[16],TEXT3[16];
	uint32_t Clock;
	uint32_t Frequency;
	uint8_t  PreScaler;
	uint8_t  ClockDivider;
	uint8_t  DutyCycle;
	uint16_t CNR, CMR;
	//-------------------------
	void servo_buka(){
			//Assignment variabel PWM
			UNLOCKREG();
			SYSCLK->PWRCON.XTL12M_EN=1;
			DrvSYS_Delay(5000);					// Waiting for 12M Xtal stalble
			SYSCLK->CLKSEL0.HCLK_S=0;
			LOCKREG();

			//init_LCD();
			//clear_LCD();

			// PWM_No = PWM channel number
			// PWM_CLKSRC_SEL   = 0: 12M, 1:32K, 2:HCLK, 3:22M
			// PWM_PreScaler    : PWM clock is divided by (PreScaler + 1)
			// PWM_ClockDivider = 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
			init_PWM(0, 0, 130, 4); // initialize PWM0(GPA12) to output 1MHz square wave
			Clock = 12000000;
			PreScaler = 130;
			ClockDivider = 1;
			Frequency = 50;

			//hitime = SERVO_HITIME_MIN;
			//--------------------------------------

			//----------Kodingan inti microservo
			//hitime=hitime + (SERVO_HITIME_MAX - SERVO_HITIME_MIN)/10;
			hitime = 125;
			if (hitime>SERVO_HITIME_MAX) hitime = SERVO_HITIME_MIN;
			//PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1)
			CNR = Clock / Frequency / (PreScaler + 1) / ClockDivider - 1;
			// Duty Cycle = (CMR0+1) / (CNR0+1)
			CMR = hitime  - 1;
			//CMR = 20;
			PWM_Out(0, CNR, CMR);
		}
	void servo_tutup(){
			//Assignment variabel PWM
			UNLOCKREG();
			SYSCLK->PWRCON.XTL12M_EN=1;
			DrvSYS_Delay(5000);					// Waiting for 12M Xtal stalble
			SYSCLK->CLKSEL0.HCLK_S=0;
			LOCKREG();

			//init_LCD();
			//clear_LCD();

			// PWM_No = PWM channel number
			// PWM_CLKSRC_SEL   = 0: 12M, 1:32K, 2:HCLK, 3:22M
			// PWM_PreScaler    : PWM clock is divided by (PreScaler + 1)
			// PWM_ClockDivider = 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
			init_PWM(0, 0, 60, 4); // initialize PWM0(GPA12) to output 1MHz square wave
			Clock = 12000000;
			PreScaler = 60;
			ClockDivider = 1;
			Frequency = 50;

		//	hitime = SERVO_HITIME_MIN;

			//----------Kodingan inti microservo
			//hitime=hitime + (SERVO_HITIME_MAX - SERVO_HITIME_MIN)/10;
			hitime = 125;
			if (hitime>SERVO_HITIME_MAX) hitime = SERVO_HITIME_MIN;
			//PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1)
			CNR = Clock / Frequency / (PreScaler + 1) / ClockDivider - 1;
			// Duty Cycle = (CMR0+1) / (CNR0+1)
			CMR = hitime  - 1;
			//CMR = 20;
			PWM_Out(0, CNR, CMR);
		}
	//---------------------------


//------------------------------
void bacarfid(){
	uint8_t hasiluart[8];
	STR_UART_T sParam;
				UNLOCKREG();
				DrvSYS_Open(50000000);
				LOCKREG();
				/* Set UART Pin */
					DrvGPIO_InitFunction(E_FUNC_UART0);

					/* UART Setting */
				    sParam.u32BaudRate 		= 9600;
				    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
				    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
				    sParam.u8cParity 		= DRVUART_PARITY_NONE;
				    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
			DrvUART_Open(UART_PORT0,&sParam);


}

void kumpul(){
	clr_all_pannal();
	uint8_t hasiluart[8];
	while(key==1){
       	DrvGPIO_ClrBit(E_GPC,12);
       	DrvSYS_Delay(10000);
       	DrvGPIO_ClrBit(E_GPB,0);
       	print_lcd(0,"Tap Kartu Anda");
       	print_lcd(1,"Pada Reader");
       	print_lcd(2,"Tekan Button");
       	print_lcd(3,"untuk Kembali");
       	if(DrvGPIO_GetBit(E_GPB,15)==0){
       	       		main();}
       	bacarfid();
		DrvUART_Read(UART_PORT0,hasiluart,1);
		if(hasiluart[0]=='B'){
						DrvGPIO_ClrBit(E_GPB,11);
			       		DrvSYS_Delay(100000);
			       		DrvGPIO_SetBit(E_GPB,11);
			       		DrvSYS_Delay(100000);
			       		DrvGPIO_ClrBit(E_GPB,11);
			       		DrvSYS_Delay(100000);
			       		DrvGPIO_SetBit(E_GPB,11);
			       		DrvSYS_Delay(100000);
			       		clr_all_pannal();
			       		goto benar;
		}
		if(hasiluart[0]!='B'){
			SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz Crystal
			       		       							SYSCLK->CLKSEL0.HCLK_S = 0;
			       		       						LOCKREG();
			       		clr_all_pannal();
			       		print_lcd(0,"Maaf :(");
			       		print_lcd(1,"Nama kamu gak");
			       		print_lcd(2,"Ada di Basis Data");
			       		print_lcd(3,"COBA LAGI !!!");
			       		DrvGPIO_ClrBit(E_GPB,11);
			       						DrvGPIO_ClrBit(E_GPB,11);
			       			       		DrvSYS_Delay(500000);
			       			       		DrvGPIO_SetBit(E_GPB,11);
			       			       		DrvSYS_Delay(100000);
			       			       		DrvGPIO_ClrBit(E_GPB,11);
			       			       		DrvSYS_Delay(500000);
			       			       		DrvGPIO_SetBit(E_GPB,11);
			       			       		DrvSYS_Delay(100000);
			       			       		DrvGPIO_ClrBit(E_GPB,11);
			       			       		delay(5000000);
			       		kumpul();
			       	}
       	//key2=Scankey();
       	/*if(key2==2){
       		DrvGPIO_ClrBit(E_GPB,11);
       		DrvSYS_Delay(100000);
       		DrvGPIO_SetBit(E_GPB,11);
       		DrvSYS_Delay(100000);
       		DrvGPIO_ClrBit(E_GPB,11);
       		DrvSYS_Delay(100000);
       		DrvGPIO_SetBit(E_GPB,11);
       		DrvSYS_Delay(100000);
       		clr_all_pannal();
       	}*/
       	//mennu harus diubah klo uart dah jadi
		benar:
       	while(hasiluart[0]=='B'){
       		print_lcd(0,"Silakan Masukkan");
       		print_lcd(1,"Borang Kamu");
       		SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz Crystal
       		       							SYSCLK->CLKSEL0.HCLK_S = 0;
       		       						LOCKREG();
       		       				        servo_buka();
       		       				        DrvSYS_Delay(10);
       		Scankey();
       		if(DrvGPIO_GetBit(E_GPA,0)==0){
       		    clr_all_pannal();
       		    print_lcd(0,"Terima Kasih :)");
       		    print_lcd(1,"Tito Alvi");
       		    hitung++;
       		 DrvGPIO_ClrBit(E_GPB,11);
       		 			       		DrvSYS_Delay(100000);
       		 			       		DrvGPIO_SetBit(E_GPB,11);
       		 			       		DrvSYS_Delay(100000);
       		 			       		DrvGPIO_ClrBit(E_GPB,11);
       		 			       		DrvSYS_Delay(100000);
       		 			       		DrvGPIO_SetBit(E_GPB,11);
       		 			       		DrvSYS_Delay(100000);
       		 			       	DrvGPIO_ClrBit(E_GPB,11);
       		 			       	DrvSYS_Delay(100000);
       		 			       	DrvGPIO_SetBit(E_GPB,11);
       		 			       	DrvSYS_Delay(100000);
       		    print_lcd(0,"Terima Kasih :)");
       			DrvSYS_Delay(2000000);
       			main();
       		       	}
       		while(DrvGPIO_GetBit(E_GPA,0)==0){ //sensor cahaya = GPA0 - SIG
       			       				         			       		main();}
       		}

       	}


       	       	}

       //di notepad


void absen(){
	uint8_t hasiluart[8];
	clr_all_pannal();

		while(key==2){
	       	DrvGPIO_ClrBit(E_GPC,12);
	       	DrvSYS_Delay(10000);
	       	print_lcd(0,"Tap Kartu Anda");
	       	print_lcd(1,"Pada Reader");
	       	bacarfid();
	       	DrvUART_Read(UART_PORT0,hasiluart,1);
	       	if(hasiluart[0]=='B'){
	       	       	clr_all_pannal();
	       	       	       		    print_lcd(0,"Terima Kasih :)");
	       	       	       		    print_lcd(1,"Tito Alvi");
	       	       	       		    hitungabsen++;
	       	       	       		    			DrvGPIO_ClrBit(E_GPB,11);
	       	       	       		        		DrvSYS_Delay(100000);
	       	       	       		        		DrvGPIO_SetBit(E_GPB,11);
	       	       	       		        		DrvSYS_Delay(100000);
	       	       	       		        		DrvGPIO_ClrBit(E_GPB,11);
	       	       	       		        		DrvSYS_Delay(100000);
	       	       	       		        		DrvGPIO_SetBit(E_GPB,11);
	       	       	       		        		DrvSYS_Delay(100000);
	       	       	       		        		DrvGPIO_ClrBit(E_GPB,11);
	       	       	       		        		DrvSYS_Delay(100000);
	       	       	       		        		DrvGPIO_SetBit(E_GPB,11);
	       	       	       		        		DrvSYS_Delay(100000);
	       	       	       		    print_lcd(0,"Terima Kasih :)");
	       	       	       			DrvSYS_Delay(5000000);

	       	       	       			main();
	       	       	}
	       	if(hasiluart[0]!='B'){
	       		clr_all_pannal();
	       		print_lcd(0,"Maaf :(");
	       		print_lcd(1,"Nama kamu gak");
	       		print_lcd(2,"Ada di Basis Data");
	       		print_lcd(3,"COBA LAGI !!!");
	       		DrvGPIO_ClrBit(E_GPB,11);
	       			       		DrvSYS_Delay(500000);
	       			       		DrvGPIO_SetBit(E_GPB,11);
	       			       		DrvSYS_Delay(100000);
	       			       		DrvGPIO_ClrBit(E_GPB,11);
	       			       		DrvSYS_Delay(500000);
	       			       		DrvGPIO_SetBit(E_GPB,11);
	       			       		DrvSYS_Delay(100000);
	       			       	delay(5000000);
	       		DrvSYS_Delay(2000000);
	       		absen();
	       	}



		}
}
int main(void)
{
	DrvUART_Close(UART_PORT0);
	//------------------
    Init();
    Initial_pannel();
    clr_all_pannal();
    while(1)
       {
    	seg_display(hitung);

    	/*UNLOCKREG();
    	SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz Crystal
    	SYSCLK->CLKSEL0.HCLK_S = 0;
    	LOCKREG();*/
    	servo_tutup();
    	//DrvSYS_Delay(10);
    	Initial_pannel();
    	DrvGPIO_ClrBit(E_GPD, 14);

    	print_lcd(0,"Selamat Datang");
    	print_lcd(1,"1. Kumpul Borang");
    	print_lcd(2,"2. Absen Doang");
    	print_lcd(3," > Silakan Tekan:");
    	//DrvSYS_Delay(10000);

    	key=Scankey();
    	while(key==4){
           	       		bacarfid();
           	       	}


       	while(key==1){
      kumpul();
       	}

       	while (key==2){
       		absen();
       print_lcd(2,"3. ADC=");
       	DrvADC_StartConvert();
       	adc=DrvADC_GetConversionData(7);
       	Show_Word(3,5,adc/1000+'0');
       	Show_Word(3,6,adc%1000/100+'0');
       	Show_Word(3,7,adc%100/10+'0');
       	Show_Word(3,8,adc%10+'0');

       	}




       	DrvGPIO_SetBit(E_GPC,12);
       	DrvSYS_Delay(1000);


       }
}

#include <Arduino.h>
#define Hi_ST 15    //Стартерный
#define Hi_WK 5    //Рабочий
#define Hi_M 14    //Общий
#define Lo_ST 4
#define Lo_WK 13
#define Lo_M 12

//#define LedFwd 4  //Светодиод "туда"
//#define LedRew 5  //Светодиод "обратно"
//#define LedStp 13 //Светодиод стоп
//#define CmdFwd 2 //Кнопка "туда"
//#define CmdRew 3 //Кнопка "обратно" Обе вместе - стоп.

//#define ValFwd 4 //Крутилка "Туда"
//#define ValRew 5 //Крутилка "Обратно"

int GetDelayByVal(int val);
void SendNegST(int delays);
void SendPosST(int delays);
void SendNegWK(int delays);
void SendPosWK(int delays);
void SendPulse(int pin, int delays);
void RotateFwd(int delays);

int nn = 0;
byte WkState = 1;

void setup() 
{   
	//Serial.begin(9600);

	//pinMode(CmdFwd, INPUT);
	//pinMode(CmdRew, INPUT);
	
	pinMode(Hi_ST, OUTPUT);
	pinMode(Hi_WK, OUTPUT);
	pinMode(Hi_M, OUTPUT);
	pinMode(Lo_ST, OUTPUT);
	pinMode(Lo_WK, OUTPUT);
	pinMode(Lo_M, OUTPUT);
	
	//pinMode(LedFwd, OUTPUT);
	//pinMode(LedRew, OUTPUT);
	//pinMode(LedStp, OUTPUT);

	//delay(1500);
	//Indicate();
}


void loop() 
{
	//Управление
//	bool cmdF = digitalRead(CmdFwd);
//	bool cmdR = digitalRead(CmdRew);
/*
	if ((cmdF && !cmdR))
	{
		if(WkState != 1 && nn > 0)
		{
			WkState = 1; //Это не Бангладеш. Это необходимость СРАЗУ переключить лампочку, а тока потом тормозить мотор.
			Indicate();
			Brake(); //Тормоз
		}
		WkState = 1; //А это на случай если томозить не надобно. Например после стопа.
		Indicate();
	}

	if (!cmdF && cmdR)
	{
		if(WkState != 2 && nn > 0)
		{
			WkState = 2;
			Indicate();
			Brake(); //Тормоз
		}
		Indicate();
		WkState = 2;
	}
*/
	
	//Исполнение
	if (WkState == 0) //Стоп
	{
		nn = 0;
		delay(50);
		return;
	}

	
	nn ++; //Стартерная задержка (циклы работы стартерной обмотки "на полную")
	if (nn > 30000) nn = 30000; 

	if (WkState == 1) //Вперёд
	{
		int delays = 5000;//GetDelayByVal(1024-analogRead(ValFwd)); //Ну наоборот я распаял. Бывает, чо... Инвертируем
		RotateFwd(delays-400);
	}
}

//Крутим вперёд. Тут оборот
void RotateFwd(int delays)
{
	digitalWrite(Lo_M, HIGH); //Включаем общий минус
	SendPosST(delays);
	SendPosWK(delays);
	digitalWrite(Lo_M, LOW); //Отключаем общий минус. 
	
	delayMicroseconds(200); //Отрицательная полярность
	
	digitalWrite(Hi_M, HIGH);
	SendNegST(delays);
	SendNegWK(delays);
	digitalWrite(Hi_M, LOW);

	delayMicroseconds(120);
}

//Крутим назад. Тут оборот
/*
void RotateRew(int delays)
{
	digitalWrite(Lo_M, HIGH); //Включаем общий минус
	SendPosST(delays);
	digitalWrite(Lo_M, LOW); //Отключаем общий минус. 

	delayMicroseconds(133);
	digitalWrite(Hi_M, HIGH);
	SendNegWK(delays); 	
	SendNegST(delays);
	digitalWrite(Hi_M, LOW);

	delayMicroseconds(133);
	digitalWrite(Lo_M, HIGH); //Включаем общий минус
	SendPosWK(delays);
	digitalWrite(Lo_M, LOW); //Отключаем общий минус.

	delayMicroseconds(50);
}
*/
//Отправка пачки импульсов
void SendPulse(int pin, int delays) 
{
	byte pwrCycle = 0;
	while(delays > 0) //Крутимся в цикле, пока не закончится время на такт
	{
		pwrCycle ++;
		if (delays < 0)
			return;
		if (delays < 300) //Если осталось менее 300мкс, то проще запаузиться.
		{
			delayMicroseconds(delays);
			return;
		}
		
		if (pwrCycle < 5){	digitalWrite(pin, HIGH);}
		delayMicroseconds(min(1200,delays));  //Длина импульса. 1200мкс
		digitalWrite(pin, LOW);
		
		if (delays < 300)//Если осталось менее 300мкс, то проще запаузиться.
		{
			delayMicroseconds(delays);
			return;
		}

		delayMicroseconds(200); 
		delays -= 1400; //Примерное время цикла
	}
}

void SendPosWK(int delays)
{
	SendPulse(Hi_WK,delays);
}
void SendNegWK(int delays)
{
	SendPulse(Lo_WK,delays);
}
void SendPosST(int delays)
{
	if (nn < 100) //Включаем стартерную
	{ SendPulse(Hi_ST,delays); } 
	else
	{ 
		if (delays > 3000) //Если частота мала - уменьшаем импульс до 25% времени
		{
			delayMicroseconds(delays*0.37);
			SendPulse(Hi_ST,delays*0.25);
			delayMicroseconds(delays*0.37);
		}
		else
		{
			delayMicroseconds(delays); //Для коротких интервалов, можно и 100% гнать стартерную - всё равно импульс будет 1-2шт и короткий
		}
	}
}
void SendNegST(int delays)
{
	if (nn < 100) //Включаем стартерную
	{ SendPulse(Lo_ST,delays); } 
	else
	{
		if (delays > 3000)
		{
			delayMicroseconds(delays*0.33);
			SendPulse(Lo_ST,delays*0.33);
			delayMicroseconds(delays*0.33);
		}
		else
		{
			delayMicroseconds(delays);
		}
	}
}
/*
//Тут должен будет жить тормоз. По идее под него надо отдельнй транзистор, подающий постоянку на обмотки через резистор.
void Brake()
{

	AllOff();
	delay(3000);
	return;

	//Serial.println("Brake");

	//Тормозим тупо импульсами одной полярности. 
	//ЗЫ, это убило 3 транзистора. Тогда я понял что так делать не надо. Но тормозит зверски. Мотор аж подпрыгивает.
	digitalWrite(Lo_M, HIGH);
	for (nn = 0; nn < 100; nn ++)
	{
		digitalWrite(Hi_ST, HIGH);
		delay(3);
		digitalWrite(Hi_ST, LOW);
		delayMicroseconds(250);
		digitalWrite(Hi_WK, HIGH);
		delay(1);
		digitalWrite(Hi_WK, LOW);
		delay(3);
	}
	AllOff();
}

void AllOff()
{
	digitalWrite(Hi_ST, LOW);
	digitalWrite(Hi_WK, LOW);
	digitalWrite(Hi_M, LOW);
	digitalWrite(Lo_ST, LOW);
	digitalWrite(Lo_WK, LOW);
	digitalWrite(Lo_M, LOW);
	delayMicroseconds(500);
}

void Indicate()
{
	digitalWrite(LedStp, (WkState == 0 ? 1:0));
	digitalWrite(LedFwd, (WkState == 1 ? 1:0));
	digitalWrite(LedRew, (WkState == 2 ? 1:0));
}
*/
//от 25 до 75гц с шагом 0.25 и 511 (центр пегулятора) = 50гц
int GetDelayByVal(int val)
{
	if (val < 5) return 10000;
	if (val < 10) return 9900;
	if (val < 15) return 9803;
	if (val < 20) return 9708;
	if (val < 25) return 9615;
	if (val < 30) return 9523;
	if (val < 35) return 9433;
	if (val < 40) return 9345;
	if (val < 45) return 9259;
	if (val < 50) return 9174;
	if (val < 55) return 9090;
	if (val < 60) return 9009;
	if (val < 65) return 8928;
	if (val < 70) return 8849;
	if (val < 76) return 8771;
	if (val < 81) return 8695;
	if (val < 86) return 8620;
	if (val < 91) return 8547;
	if (val < 96) return 8474;
	if (val < 101) return 8403;
	if (val < 106) return 8333;
	if (val < 111) return 8264;
	if (val < 116) return 8196;
	if (val < 121) return 8130;
	if (val < 126) return 8064;
	if (val < 131) return 8000;
	if (val < 136) return 7936;
	if (val < 141) return 7874;
	if (val < 147) return 7812;
	if (val < 152) return 7751;
	if (val < 157) return 7692;
	if (val < 162) return 7633;
	if (val < 167) return 7575;
	if (val < 172) return 7518;
	if (val < 177) return 7462;
	if (val < 182) return 7407;
	if (val < 187) return 7352;
	if (val < 192) return 7299;
	if (val < 197) return 7246;
	if (val < 202) return 7194;
	if (val < 207) return 7142;
	if (val < 212) return 7092;
	if (val < 217) return 7042;
	if (val < 223) return 6993;
	if (val < 228) return 6944;
	if (val < 233) return 6896;
	if (val < 238) return 6849;
	if (val < 243) return 6802;
	if (val < 248) return 6756;
	if (val < 253) return 6711;
	if (val < 258) return 6666;
	if (val < 263) return 6622;
	if (val < 268) return 6578;
	if (val < 273) return 6535;
	if (val < 278) return 6493;
	if (val < 283) return 6451;
	if (val < 288) return 6410;
	if (val < 294) return 6369;
	if (val < 299) return 6329;
	if (val < 304) return 6289;
	if (val < 309) return 6250;
	if (val < 314) return 6211;
	if (val < 319) return 6172;
	if (val < 324) return 6134;
	if (val < 329) return 6097;
	if (val < 334) return 6060;
	if (val < 339) return 6024;
	if (val < 344) return 5988;
	if (val < 349) return 5952;
	if (val < 354) return 5917;
	if (val < 359) return 5882;
	if (val < 364) return 5847;
	if (val < 370) return 5813;
	if (val < 375) return 5780;
	if (val < 380) return 5747;
	if (val < 385) return 5714;
	if (val < 390) return 5681;
	if (val < 395) return 5649;
	if (val < 400) return 5617;
	if (val < 405) return 5586;
	if (val < 410) return 5555;
	if (val < 415) return 5524;
	if (val < 420) return 5494;
	if (val < 425) return 5464;
	if (val < 430) return 5434;
	if (val < 435) return 5405;
	if (val < 441) return 5376;
	if (val < 446) return 5347;
	if (val < 451) return 5319;
	if (val < 456) return 5291;
	if (val < 461) return 5263;
	if (val < 466) return 5235;
	if (val < 471) return 5208;
	if (val < 476) return 5181;
	if (val < 481) return 5154;
	if (val < 486) return 5128;
	if (val < 491) return 5102;
	if (val < 496) return 5076;
	if (val < 501) return 5050;
	if (val < 506) return 5025;
	if (val < 512) return 5000;
	if (val < 517) return 4975;
	if (val < 522) return 4950;
	if (val < 527) return 4926;
	if (val < 532) return 4901;
	if (val < 537) return 4878;
	if (val < 542) return 4854;
	if (val < 547) return 4830;
	if (val < 552) return 4807;
	if (val < 558) return 4784;
	if (val < 563) return 4761;
	if (val < 568) return 4739;
	if (val < 573) return 4716;
	if (val < 578) return 4694;
	if (val < 583) return 4672;
	if (val < 588) return 4651;
	if (val < 593) return 4629;
	if (val < 599) return 4608;
	if (val < 604) return 4587;
	if (val < 609) return 4566;
	if (val < 614) return 4545;
	if (val < 619) return 4524;
	if (val < 624) return 4504;
	if (val < 629) return 4484;
	if (val < 634) return 4464;
	if (val < 640) return 4444;
	if (val < 645) return 4424;
	if (val < 650) return 4405;
	if (val < 655) return 4385;
	if (val < 660) return 4366;
	if (val < 665) return 4347;
	if (val < 670) return 4329;
	if (val < 675) return 4310;
	if (val < 680) return 4291;
	if (val < 686) return 4273;
	if (val < 691) return 4255;
	if (val < 696) return 4237;
	if (val < 701) return 4219;
	if (val < 706) return 4201;
	if (val < 711) return 4184;
	if (val < 716) return 4166;
	if (val < 721) return 4149;
	if (val < 727) return 4132;
	if (val < 732) return 4115;
	if (val < 737) return 4098;
	if (val < 742) return 4081;
	if (val < 747) return 4065;
	if (val < 752) return 4048;
	if (val < 757) return 4032;
	if (val < 762) return 4016;
	if (val < 768) return 4000;
	if (val < 773) return 3984;
	if (val < 778) return 3968;
	if (val < 783) return 3952;
	if (val < 788) return 3937;
	if (val < 793) return 3921;
	if (val < 798) return 3906;
	if (val < 803) return 3891;
	if (val < 808) return 3875;
	if (val < 814) return 3861;
	if (val < 819) return 3846;
	if (val < 824) return 3831;
	if (val < 829) return 3816;
	if (val < 834) return 3802;
	if (val < 839) return 3787;
	if (val < 844) return 3773;
	if (val < 849) return 3759;
	if (val < 855) return 3745;
	if (val < 860) return 3731;
	if (val < 865) return 3717;
	if (val < 870) return 3703;
	if (val < 875) return 3690;
	if (val < 880) return 3676;
	if (val < 885) return 3663;
	if (val < 890) return 3649;
	if (val < 896) return 3636;
	if (val < 901) return 3623;
	if (val < 906) return 3610;
	if (val < 911) return 3597;
	if (val < 916) return 3584;
	if (val < 921) return 3571;
	if (val < 926) return 3558;
	if (val < 931) return 3546;
	if (val < 936) return 3533;
	if (val < 942) return 3521;
	if (val < 947) return 3508;
	if (val < 952) return 3496;
	if (val < 957) return 3484;
	if (val < 962) return 3472;
	if (val < 967) return 3460;
	if (val < 972) return 3448;
	if (val < 977) return 3436;
	if (val < 983) return 3424;
	if (val < 988) return 3412;
	if (val < 993) return 3401;
	if (val < 998) return 3389;
	if (val < 1003) return 3378;
	if (val < 1008) return 3367;
	if (val < 1013) return 3355;
	if (val < 1018) return 3344;
	if (val < 1024) return 3333;
}


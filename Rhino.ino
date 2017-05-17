#include <SPI.h>
#include <SD.h>
#include <IcarusXbee.h>
#include <IcarusCelle.h>
#include <IcarusIO.h>
#include <IcarusHWCfg.h>

#define myDebugSerial Serial
#define SerialXbee Serial3
#define PIN_BUZZER 12
#define PIN_CELLA1 A0
#define PIN_CELLA2 A1
#define LED_CHARGE 9
#define LED_FIRE 10
#define LED_TEST 11
#define RELE_CHARGE 5
#define RELE_FIRE 6
#define RELE_SAFE 7
#define BUTTON_DISCHARGE 8

typedef enum {
	Charge,
	safeDischarge,
	Fire,
	countDown,
	testMode,
	Idle
} mode;

IcarusIO IO(&myDebugSerial);
IcarusXbee myXbee(&IO,crocodile,&SerialXbee);
File dataFile;
typeCmdFire CmdFireReceived;
mode Fase;
bool fireEnabled;
bool logEnabled;
int16_t t_countdown;
IcarusCelle cella1(A0, _500kg128);
IcarusCelle cella2(A1, _500kg022);

unsigned long t_lastDataSent,t_lastDataLogged,t_startCmdFire,t_lastLampTest,t_lastLampCharge,t_lastLampFire;

void setup() {
  myDebugSerial.begin(115200);
  if (myDebugSerial)printInfo();
  setupPinIO();
  setupSDCard();
  setupXbee();
  IO.Time = millis();
  t_lastDataSent = IO.Time;
  t_startCmdFire = IO.Time;
  t_lastLampTest= IO.Time;
  Fase = Idle;
  fireEnabled = false;
  logEnabled = false;
}

void loop() {
	IO.Time = millis();
	if (digitalRead(BUTTON_DISCHARGE) == HIGH) {
		digitalWrite(RELE_CHARGE, LOW);
		digitalWrite(RELE_FIRE, LOW);
		digitalWrite(RELE_SAFE, HIGH);
		digitalWrite(LED_FIRE, LOW);
		digitalWrite(LED_TEST, LOW);
		digitalWrite(LED_CHARGE, LOW);
		Fase = safeDischarge;
		t_startCmdFire = IO.Time;
		t_lastLampTest = IO.Time;
		myXbee.SendCmdFire(scaricaSicura);
		dataFile.println(String(String(IO.Time)+" - Safe Discharge button pressed"));
		dataFile.flush();	
	}
	myXbee.Read();
	if (IO.cmdFireAvailable) {
		CmdFireReceived=myXbee.readCmdFire();
		if (CmdFireReceived == stopEmergenza || CmdFireReceived == Fuoco || CmdFireReceived == startLog ||CmdFireReceived == stopLog)
		{
			if (CmdFireReceived == stopEmergenza) {
				digitalWrite(RELE_SAFE, HIGH);
				digitalWrite(LED_TEST, LOW);
				digitalWrite(RELE_FIRE, LOW);
				digitalWrite(LED_FIRE, LOW);
				digitalWrite(RELE_CHARGE, LOW);
				digitalWrite(LED_CHARGE, LOW);
				Fase = safeDischarge;
				t_startCmdFire = IO.Time;
				myXbee.SendCmdFire(stopEmergenza);
				dataFile.println(String(String(IO.Time) + " - Emergency stop command received"));
				myDebugSerial.println("Emergency stop command received");
			}
			else if (CmdFireReceived == Fuoco) {
				if (((IO.Time - t_startCmdFire) >= (t_countdown*1000 - T_TOLL)) &&
					((IO.Time - t_startCmdFire) <= (t_countdown*1000 + T_TOLL))) {
					fireEnabled = true;
					myXbee.SendCmdFire(Fuoco);
					dataFile.println(String(String(IO.Time) + " - Fire command received"));
					myDebugSerial.println("Fire command received");
				}
				else
				{
					fireEnabled = false;
					Fase = Idle;
					myXbee.SendCmdFire(Timeout);
					dataFile.println(String(String(IO.Time) + " - Fire command not received on time"));
					myDebugSerial.println("Fire command not received on time");
				}
			}
			else if (CmdFireReceived == startLog) {
				logEnabled = true;
				myXbee.SendCmdFire(startLog);
				dataFile.println(String(String(IO.Time) + " - Start logging command received"));
				myDebugSerial.println("Start logging command received");
			}
			else if (CmdFireReceived == stopLog) {
				logEnabled = false;
				myXbee.SendCmdFire(stopLog);
				dataFile.println(String(String(IO.Time) + " - Stop logging command received"));
				myDebugSerial.println("Stop logging command received");
			}

		}
		else if (Fase==Idle)
		{
			switch (CmdFireReceived)
			{
			case Carica:
				digitalWrite(RELE_SAFE, LOW);
				digitalWrite(RELE_FIRE, LOW);
				digitalWrite(RELE_CHARGE, HIGH);
				digitalWrite(LED_TEST, LOW);
				digitalWrite(LED_FIRE, LOW);
				digitalWrite(LED_CHARGE, HIGH);
				Fase = Charge;
				t_startCmdFire = IO.Time;
				myXbee.SendCmdFire(Carica);
				dataFile.println(String(String(IO.Time) + " - Charge command received"));
				myDebugSerial.println("Charge command received");
				break;
			case scaricaSicura:
				digitalWrite(RELE_CHARGE, LOW);
				digitalWrite(RELE_FIRE, LOW);
				digitalWrite(RELE_SAFE, HIGH);
				digitalWrite(LED_CHARGE, LOW);
				digitalWrite(LED_FIRE, LOW);
				digitalWrite(LED_TEST, LOW);			
				Fase = safeDischarge;
				t_startCmdFire = IO.Time;
				myXbee.SendCmdFire(scaricaSicura);
				dataFile.println(String(String(IO.Time) + " - Safe Discharge command received"));
				myDebugSerial.println("Safe Discharge command received");
				break;
			case avviaCountdown:
				digitalWrite(RELE_SAFE, LOW);
				digitalWrite(LED_TEST, LOW);
				digitalWrite(RELE_FIRE, LOW);
				digitalWrite(LED_FIRE, LOW);
				digitalWrite(RELE_CHARGE, LOW);
				digitalWrite(LED_CHARGE, LOW);
				Fase = countDown;
				t_startCmdFire = IO.Time;
				myXbee.SendCmdFire(avviaCountdown);
				dataFile.println(String(String(IO.Time) + " - Start countdown command received"));
				myDebugSerial.println("Start countdown command received");
				break;
			case testConnessione:
				digitalWrite(RELE_SAFE, LOW);
				digitalWrite(LED_TEST, HIGH);
				digitalWrite(RELE_FIRE, LOW);
				digitalWrite(LED_FIRE, LOW);
				digitalWrite(RELE_CHARGE, LOW);
				digitalWrite(LED_CHARGE, LOW);
				Fase = testMode;
				t_startCmdFire = IO.Time;
				t_lastLampTest = IO.Time;
				myXbee.SendCmdFire(testConnessione);
				dataFile.println(String(String(IO.Time) + " - Test command received"));
				myDebugSerial.println("Test command received");
				break;
			default: break;
			}
		}
		dataFile.flush();
	}
	switch (Fase) {
	case Charge:
		if (IO.Time - t_startCmdFire >= T_CHARGE)
		{
			Fase = Idle;
			digitalWrite(RELE_CHARGE, LOW);
			digitalWrite(LED_CHARGE, LOW);
			myXbee.SendCmdFire(Carica);
			myDebugSerial.println("Carica completata");
		}
		break;
	case safeDischarge:
		if (IO.Time - t_startCmdFire >= T_SAFE_DISCHARGE)
		{
			Fase = Idle;
			digitalWrite(RELE_SAFE, LOW);
			myXbee.SendCmdFire(scaricaSicura);
			myDebugSerial.println("Scarica sicura completata");
		}
		break;
	case Fire:
		if ((digitalRead(RELE_FIRE)==LOW))
		{
			t_startCmdFire = IO.Time;
			digitalWrite(RELE_FIRE, HIGH);
			digitalWrite(LED_FIRE, HIGH);
			analogWrite(PIN_BUZZER, 20);
			myDebugSerial.println("Fuoco avviato");
			break;
		}
		if (IO.Time - t_startCmdFire >= T_FIRE)
		{
			digitalWrite(RELE_FIRE, LOW);
			digitalWrite(LED_FIRE, LOW);
			digitalWrite(PIN_BUZZER, LOW);
			Fase = Idle;
			t_startCmdFire = IO.Time;
			fireEnabled = false;
			myXbee.SendCmdFire(Fuoco);
			myDebugSerial.println("Fuoco completato");
		}
		break;
	case countDown:
		if (IO.cmdAvailable)
		{
			t_countdown = static_cast<int16_t>(myXbee.getCmd());
			myDebugSerial.print("Tempo di countdown ricevuto ");
			myDebugSerial.println(t_countdown);
			myDebugSerial.println("Countdown avviato");
			logEnabled = true;
		}
		blink(LED_FIRE, &t_lastLampFire, IO.Time, T_LAMP_SLOW);
		beep(IO.Time, 40);
		if (IO.Time - t_startCmdFire >= t_countdown * 1000)
		{
			if (IO.Time - t_startCmdFire >= t_countdown * 1000 + T_TOLL)
			{
				Fase = Idle;
				myDebugSerial.println("Fire command not received on time");
				myXbee.SendCmdFire(MissingCountdown);
			}
			else if (fireEnabled) {
				Fase = Fire;
			}
		}
		break;
	case testMode:
		if (IO.Time - t_startCmdFire >= T_TEST) {
			Fase = Idle;
			digitalWrite(LED_TEST, LOW);
			myDebugSerial.println("Stop Led");
			break;
		}
		blink(LED_TEST, &t_lastLampTest, IO.Time, T_LAMP_FAST);
		break;
	case Idle:
	default: break;
	}
	if (logEnabled) {
		if (IO.Time - t_lastDataSent >= T_SDLOG) {
			IO.Load[0] = cella1.measureForce();
			IO.Load[1] = cella2.measureForce();
			dataFile.println(String(String(IO.Load[0]) + ";"+ String(IO.Load[0]) + ";"));
			dataFile.flush();
			t_lastDataLogged = IO.Time;
			if (IO.Time - t_lastDataSent >= T_XBEELOG) {
				myXbee.addToPayload(Celle);
				myXbee.Send();
				t_lastDataSent = IO.Time;
			}
		}
	}

	}


void setupSDCard() {
	const int chipSelect = 4;
	myDebugSerial.print("Initializing SD card...");
	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		myDebugSerial.println("Card failed, or not present");
		// don't do anything more:
		while (1){
			blink(LED_CHARGE,&t_lastLampCharge,millis(), T_LAMP_FAST);
			blink(LED_TEST, &t_lastLampTest, millis(), T_LAMP_FAST);
			blink(LED_FIRE, &t_lastLampFire, millis(), T_LAMP_FAST);
		}
	}
	myDebugSerial.println("SDcard initialized.");

	int numfile = 0;
	String nomefile = "RHINO_" + String(numfile) + ".txt";
	while (SD.exists(nomefile)) {
		numfile++;
		nomefile = "RHINO_" + String(numfile) + ".txt";
	}
	dataFile = SD.open(nomefile, FILE_WRITE);
	delay(100);
	myDebugSerial.print("Opened ");
	myDebugSerial.println(nomefile);
	delay(1000);

}

void setupXbee() {
	SerialXbee.begin(115200);
	if (!SerialXbee)
	{
		myDebugSerial.println("Xbee not ready!");
		while (1) {
			blink(LED_CHARGE, &t_lastLampCharge, millis(), T_LAMP_FAST);
			blink(LED_TEST, &t_lastLampTest, millis(), T_LAMP_FAST);
			blink(LED_FIRE, &t_lastLampFire, millis(), T_LAMP_FAST);
		}
	}
	if (SerialXbee) myDebugSerial.println("Communications initialized");
}

void setupPinIO() {
	pinMode(LED_CHARGE, OUTPUT);
	pinMode(LED_FIRE, OUTPUT);
	pinMode(LED_TEST, OUTPUT);
	pinMode(RELE_CHARGE, OUTPUT);
	pinMode(RELE_FIRE, OUTPUT);
	pinMode(RELE_SAFE, OUTPUT);
	pinMode(PIN_BUZZER, OUTPUT);
	pinMode(BUTTON_DISCHARGE, INPUT);
	digitalWrite(LED_CHARGE, LOW);
	digitalWrite(LED_FIRE, LOW);
	digitalWrite(LED_TEST, LOW);
	digitalWrite(RELE_CHARGE, LOW);
	digitalWrite(RELE_FIRE, LOW);
	digitalWrite(RELE_SAFE, LOW);
	digitalWrite(PIN_BUZZER, LOW);
}

void blink(uint8_t led, unsigned long int* ultimoLamp, unsigned long int adesso, uint16_t semiperiodo) {
	if (adesso - *ultimoLamp >= semiperiodo)
	{
		digitalWrite(led, !digitalRead(led));
		*ultimoLamp = adesso;
	}
}

void beep(unsigned long int adesso, uint16_t periodoON) {
	static bool stato = false;
	static long int ultimoBeepStart = adesso;
	static long int BeepStart = adesso;

	if (adesso - ultimoBeepStart >= T_LAMP_SLOW)
	{
		analogWrite(PIN_BUZZER, 80);
		ultimoBeepStart = adesso;
		stato = true;
	}
	if (stato && (adesso - ultimoBeepStart >= periodoON)) {
		digitalWrite(PIN_BUZZER, LOW);
		stato = false;
	}

}

void printInfo() {
	myDebugSerial.println("Coded by Dave");
}

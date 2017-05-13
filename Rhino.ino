#include <SPI.h>
#include <SD.h>
#include <IcarusXbee.h>
#include <IcarusCelle.h>
#include <IcarusIO.h>

#define myDebugSerial Serial
#define SerialXbee Serial3
#define PIN_BUZZER 12
#define LED_CHARGE 9
#define LED_FIRE 10
#define LED_TEST 11
#define RELE_CHARGE 5
#define RELE_FIRE 6
#define RELE_SAFE 7
#define BUTTON_DISCHARGE 8
#define T_SAFE_DISCHARGE 10000
#define T_CHARGE 30000
#define T_FIRE 3000
#define T_TOLL 2000
#define T_LAMP 1000


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
bool countdownStarted;
uint16_t t_countdown;

unsigned long t_lastDataSent,t_startCmdFire, t_lastLamp;

void setup() {
  myDebugSerial.begin(115200);
  setupSDCard();
  setupXbee();
  setupPinIO();
  IO.Time = millis();
  t_lastDataSent = IO.Time;
  t_startCmdFire = IO.Time;
  t_lastLamp= IO.Time;
  Fase = Idle;
  fireEnabled = false;
}

void loop() {

	IO.Time = millis();

	// make a string for assembling the data to log:
	String dataString = "";
	// read three sensors and append to the string:
	if (digitalRead(BUTTON_DISCHARGE) == HIGH) {
		digitalWrite(RELE_CHARGE, LOW);
		digitalWrite(RELE_FIRE, LOW);
		digitalWrite(RELE_SAFE, HIGH);
		digitalWrite(LED_FIRE, LOW);
		digitalWrite(LED_TEST, LOW);
		digitalWrite(LED_CHARGE, LOW);
		Fase = safeDischarge;
		t_startCmdFire = IO.Time;
		myXbee.SendCmdFire(scaricaSicura);
		dataFile.println(String(String(IO.Time)+" - Safe Discharge button pressed"));
		dataFile.flush();	
	}
	myXbee.Read();
	if (IO.cmdFireAvailable) {
		CmdFireReceived=myXbee.readCmdFire();
		if (CmdFireReceived == stopEmergenza || CmdFireReceived == Fuoco)
		{
			if (CmdFireReceived == stopEmergenza) {
				digitalWrite(RELE_SAFE, LOW);
				digitalWrite(LED_TEST, LOW);
				digitalWrite(RELE_FIRE, LOW);
				digitalWrite(LED_FIRE, LOW);
				digitalWrite(RELE_CHARGE, LOW);
				digitalWrite(LED_CHARGE, LOW);
				Fase = Idle;
				t_startCmdFire = IO.Time;
				myXbee.SendCmdFire(stopEmergenza);
				dataFile.println(String(String(IO.Time) + " - Emergency stop command received"));
				myDebugSerial.println("Emergency stop command received");
			}
			else if (CmdFireReceived == Fuoco) {
				if (((IO.Time - t_startCmdFire) >= (t_countdown - T_TOLL)) &&
					((IO.Time - t_startCmdFire) <= (t_countdown + T_TOLL))) {
					Fase = Fire;
					fireEnabled = true;
					myXbee.SendCmdFire(Fuoco);
					dataFile.println(String(String(IO.Time) + " - Fire command received"));
					myDebugSerial.println("Fire command received");
				}
				else
				{
					Fase = Idle;
					myXbee.SendCmdFire(Timeout);
					dataFile.println(String(String(IO.Time) + " - Fire command not received on time"));
					myDebugSerial.println("Fire command not received on time");
				}
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
		if (IO.Time - t_startCmdFire >= T_FIRE)
		{
			digitalWrite(RELE_FIRE, LOW);
			digitalWrite(LED_FIRE, LOW);
			Fase = Idle;
			t_startCmdFire = IO.Time;
			fireEnabled = false;
			countdownStarted = false;
			myXbee.SendCmdFire(Fuoco);
			myDebugSerial.println("Fuoco completato");
		}
		break;
	case countDown:
		if (IO.cmdAvailable)
		{
			t_countdown = myXbee.getCmd();
			myXbee.addToPayload(Comando);
			myXbee.Send();
			countdownStarted = true;
			myDebugSerial.println("Tempo di countdown ricevuto");
		}
		if (IO.Time - t_startCmdFire >= t_countdown * 1000)
		{
			if (fireEnabled) {
				digitalWrite(RELE_FIRE, HIGH);
				digitalWrite(LED_FIRE, HIGH);
				Fase = Fire;
				t_startCmdFire = IO.Time;
				myXbee.SendCmdFire(avviaCountdown);
				myDebugSerial.println("Countdown avviato");
			}
			else
			{
				Fase = Idle;
				myXbee.SendCmdFire(MissingCountdown);

			}
		}
		break;
	case testMode:
		if (IO.Time - t_startCmdFire >= T_LAMP) {
			Fase = Idle;
			digitalWrite(LED_TEST, LOW);
			myDebugSerial.println("Stop Led");
			break;
		}
		if (IO.Time - t_lastLamp > 200)
		{
			digitalWrite(LED_TEST, !digitalRead(LED_TEST));
			t_lastLamp = IO.Time;
			myDebugSerial.println("Switch");
		}
		break;
	case Idle:
	default: break;
	}
	}


void setupSDCard() {
	const int chipSelect = 4;
	myDebugSerial.print("Initializing SD card...");
	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		myDebugSerial.println("Card failed, or not present");
		// don't do anything more:
		return;
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
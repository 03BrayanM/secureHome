#include "StateMachineLib.h"
// include the library code:
#include <LiquidCrystal.h>
//Include the keypad code
#include <Keypad.h>
#include "AsyncTaskLib.h"
#include "DHT.h"

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//Configuration for the Keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {22, 24, 26, 28}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 32, 34, 36}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//Configuration for the leds
#define  LED_RED 10
#define  LED_GREEN 9
#define  LED_BLUE 8 
//Configuration for the sensors
//Light
const int pinLight = A0;
int valueLight=0;
//Hall
const int pinHall = A1;
int valueHall=0;
//DHT
#define DHTTYPE 11
#define DHTPIN 38
DHT dht(DHTPIN,DHTTYPE);
float h = 0;
float t = 0;
//Configuration for the security
const char password[6] = {'1','2','3','4','5','#'};
char buffer[6];
int counter = -1;
char tryCounter=0;


// State Alias
enum State
{
	Inicio = 0,
	ConfigMenu = 1,
	MonitorAmbiental = 2,
	Bloqueo = 3,
  Alarma = 4,
  MonitorEventos  = 5
};

// Input Alias
enum Input
{
	time = 0,
	claveCorrecta = 1,
	systemBlock = 2,
  btnPress = 3,
  hallExceeded = 4,
  tempLightExceeded = 5,
	unknown = 6,
};

// Create new StateMachine
StateMachine stateMachine(6, 12);

// Stores last user input
Input input = Input::unknown;


// Setup the State Machine
void setupStateMachine()
{
	// Add transitions
	stateMachine.AddTransition(Inicio, ConfigMenu, []() { return input == claveCorrecta; });
  stateMachine.AddTransition(Inicio, Bloqueo, []() { return input == systemBlock; });

	stateMachine.AddTransition(ConfigMenu, MonitorAmbiental, []() { return input == btnPress; });

	stateMachine.AddTransition(MonitorAmbiental, ConfigMenu, []() { return input == btnPress; });
	stateMachine.AddTransition(MonitorAmbiental, MonitorEventos, []() { return input == time; });
	stateMachine.AddTransition(MonitorAmbiental, Alarma, []() { return input == tempLightExceeded; });

  stateMachine.AddTransition(MonitorEventos, ConfigMenu, []() { return input == btnPress; });
  stateMachine.AddTransition(MonitorEventos, MonitorAmbiental, []() { return input == time; });
  stateMachine.AddTransition(MonitorEventos, Alarma, []() { return input == hallExceeded; });  

  stateMachine.AddTransition(Alarma, MonitorAmbiental, []() { return input == time; });
  stateMachine.AddTransition(Alarma, Inicio, []() { return input == btnPress; });  

   stateMachine.AddTransition(Bloqueo, Inicio, []() { return input == time; });

	// Add actions
	stateMachine.SetOnEntering(Inicio, outputInicio);
	stateMachine.SetOnEntering(ConfigMenu, outputMenu);
	stateMachine.SetOnEntering(MonitorAmbiental, outputMAmbiental);
	stateMachine.SetOnEntering(Bloqueo, outputBloqueo);
  stateMachine.SetOnEntering(Alarma, outputAlarma);
  stateMachine.SetOnEntering(MonitorEventos, outputMEventos);

  void leavingAmbiental(void);
	stateMachine.SetOnLeaving(Inicio, []() {Serial.println("Leaving Inicio"); });
	stateMachine.SetOnLeaving(ConfigMenu, []() {Serial.println("Leaving Menu"); });
	stateMachine.SetOnLeaving(MonitorAmbiental, leavingAmbiental);
	stateMachine.SetOnLeaving(Bloqueo, []() {Serial.println("Leaving Bloqueo"); });
  stateMachine.SetOnLeaving(Alarma, []() {Serial.println("Leaving Alarma"); });
  stateMachine.SetOnLeaving(MonitorEventos, []() {Serial.println("Leaving Eventos"); });
}
//Tasks
  void readLight(void);
  void readTemp(void);
  void readHum(void);
  void readTime(void);
  void printSensorsLcd(void);
  AsyncTask taskLight(1000,true,readLight);
  AsyncTask taskTemp(1000,true,readTemp);
  AsyncTask taskHum(1000,true,readHum);
  AsyncTask taskTime(10000,true,readTime);
  AsyncTask taskPrintLcd(1000,true,printSensorsLcd);

void setup() 
{
	Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  //About leds
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  //Turn off the leds
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW); 

	Serial.println("Starting State Machine...");
	setupStateMachine();	
	Serial.println("Start Machine Started");

	// Initial state
	stateMachine.SetState(Inicio, false, true);
}

void loop() 
{
	taskHum.Update();
  taskLight.Update();
	taskPrintLcd.Update();
  taskTemp.Update();
    


	// Update State Machine
	stateMachine.Update();
  input = Input::unknown;
}

// Auxiliar function that reads the user input
int readInput()
{
	Input currentInput = Input::unknown;
	if (Serial.available())
	{
		char incomingChar = Serial.read();

		switch (incomingChar)
		{
			case 'T': currentInput = Input::time; break;
			case 'C': currentInput = Input::claveCorrecta; break;
			case 'S': currentInput = Input::systemBlock; break;
      case 'B': currentInput = Input::btnPress; break;
			case 'H': currentInput = Input::hallExceeded; break;			
      case 'L': currentInput = Input::tempLightExceeded; break;
      default: break;
		}
	}

	return currentInput;
}

// Auxiliar output functions that show the state debug
void outputInicio()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("  X                                                   ");
	Serial.println();
  //tryCounter = 0;
  //counter = -1;
  //seguridad();
}

void outputMenu()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("          X                                           ");
	Serial.println();
  //claCorrecta();
}

void outputMAmbiental()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                    X                                 ");
	Serial.println();
  taskLight.Start();
  taskTemp.Start();
  taskHum.Start();
  taskPrintLcd.Start();
  taskTime.SetIntervalMillis(7000);
  taskTime.Start();
}
void leavingAmbiental(){
  taskLight.Stop();
  taskTemp.Stop();
  taskHum.Stop();
} 

void outputBloqueo()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                               X                      ");
	Serial.println();
  sisBloqueado();
  taskTime.SetIntervalMillis(10000);
  taskTime.Start();
}
void outputAlarma()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                                         X            ");
	Serial.println();

  taskTime.SetIntervalMillis(7000);
  taskTime.Start();
}
void outputMEventos()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                                                  X");
	Serial.println();
}

void seguridad(){
  while(tryCounter < 3){
    if(counter==-1){
    lcd.clear();
    lcd.print("Clave:");
    counter++;
  }
  char key = keypad.getKey();    
  if (key){     
      Serial.println(key);
      lcd.setCursor(counter,2);
      lcd.print("*");
      if(counter<6){
        buffer[counter] = key;
      }
      counter++;
      if(key == '#'){
        if(comparar(password,buffer,6)==true){
          input = Input::claveCorrecta;
          return;
        }
        else{
          tryCounter++;
          digitalWrite(LED_BLUE,HIGH);
          lcd.clear();
          lcd.print("Clave incorrecta");
          delay(3000);
          digitalWrite(LED_BLUE,LOW);
          counter=-1;
          lcd.clear();
        }
      }
    }  
  }
  input = Input::systemBlock;
}
bool comparar(char vector1[], char vector2[], int longitud) {
  for (int i = 0; i < longitud; i++) {
    if (vector1[i] != vector2[i]) {
      return false;
    }
  }
  return true;
}
void sisBloqueado(){
  Serial.println("SystemBlock");
  lcd.print("SystemBlock");
  digitalWrite(LED_RED,HIGH);
  delay(5000);
  lcd.clear();
  Input::time;
}
void claCorrecta(){
  lcd.clear();
  Serial.println("Clave correcta");
  lcd.print("Clave correcta");
  digitalWrite(LED_GREEN,HIGH);
}

void readLight(){
  valueLight = analogRead(pinLight);
}
void readTemp(){
  t = dht.readTemperature();
  if(isnan(t)){
    Serial.println("Failed to read temperature");
  }
}
void readHum(){
  h = dht.readHumidity();
  if(isnan(h)){
    Serial.println("Failed to read temperature");
  }
}
void readTime(void){
  input = Input::time;
}
void printSensorsLcd(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TEM:");
  lcd.print(t);
  //Print humidity  
  lcd.setCursor(8,0);
  lcd.print("HUM:");
  lcd.print(h);
  //Print light
  lcd.setCursor(0,1);
  lcd.print("LUZ:");
  lcd.print(valueLight);
  //Print Mag
  lcd.setCursor(8,1);
  lcd.print("MAG:");
  lcd.print(valueHall);
}
#include "StateMachineLib.h"
// include the library code:
#include <LiquidCrystal.h>
//Include the keypad code
#include <Keypad.h>
#include <AsyncTaskLib.h>

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

const char password[6] = {'1','2','3','4','5','#'};
char buffer[6];
int counter = -1;
char tryCounter=0;
// definity task
void alarma(void);
AsyncTask taskAlarm(4000, true, alarma);

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

	stateMachine.SetOnLeaving(Inicio, []() {Serial.println("Leaving Inicio"); });
	stateMachine.SetOnLeaving(ConfigMenu, []() {Serial.println("Leaving Menu"); });
	stateMachine.SetOnLeaving(MonitorAmbiental, []() {Serial.println("Leaving Ambiental"); });
	stateMachine.SetOnLeaving(Bloqueo, []() {Serial.println("Leaving Bloqueo"); });
    stateMachine.SetOnLeaving(Alarma, []() {Serial.println("Leaving Alarma"); });
    stateMachine.SetOnLeaving(MonitorEventos, []() {Serial.println("Leaving Eventos"); });
}

void setup() 
{
	Serial.begin(115200);
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
	// Read user input
	input = static_cast<Input>(readInput());

	// Update State Machine
	stateMachine.Update();
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
}

void outputBloqueo()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                               X                      ");
	Serial.println();
  //sisBloqueado();
}
void outputAlarma()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                                         X            ");
	Serial.println();
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
void alarma(){
  
}
#include "StateMachineLib.h"
// include the library code:
#include <LiquidCrystal.h>
//Include the keypad code
#include <LiquidMenu.h>
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
// configuration for the alarm state melody 
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_A5  880
#define NOTE_G5  784
#define NOTE_FS5 740
 int melody[] = {
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-4, NOTE_E5,8, NOTE_FS5,4,
  };
int tempo = 85;// change this to make the song slower or faster
int buzzer = 11; // change this to whichever pin you want to use
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;
//Configuration for the sensors
//Light
const int pinLight = A0;
int valueLight=0;
int lightLimit = 20;
//Hall
const int pinHall = A1;
int valueHall=0;
int hallLimit = -1;
//DHT
#define DHTTYPE 22
#define DHTPIN 38
DHT dht(DHTPIN,DHTTYPE);
float h = 0;
float t = 0;
float humLimit = 0;
float tempLimit = 0;
//Configuration for the security
const char password[6] = {'1','2','3','4','5','#'};
char buffer[6];
int counter = -1;
char tryCounter=0;
// configuration for configMenu State
const byte pwmPin = 6;
byte pwmLevel = 0;
const byte ledPin = LED_BUILTIN;
bool ledState = LOW;
char ledState_text[4];
char string_on[] = "ON";
char string_off[] = "OFF";
const byte analogPin = A5;
unsigned short analogValue = 0;

LiquidLine line1(0, 0, "TH_TEMP_HIGH ", LIQUIDMENU_VERSION);
LiquidLine line2(0, 1, "TH_TEMP_LOW");
LiquidScreen screen1(line1, line2);

LiquidLine line3(0, 0, "TH_TEMP_LOW", analogValue);
LiquidLine line4(0, 1, "TH_LUZ_HIGH", ledState_text);
LiquidScreen screen2(line3, line4);

LiquidLine line5(0, 0, "TH_LUZ_HIGH");
LiquidLine line6(0, 1, "TH_LUZ_LOW");
LiquidScreen screen3(line5, line6);

LiquidLine line7(0, 0, "TH_LUZ_LOW");
LiquidLine line8(0, 1, "TH_HALL");
LiquidScreen screen4(line7, line8);

LiquidLine line9(0, 0, "TH_HALL");
LiquidLine line10(0, 1, "RESET");
LiquidScreen screen5(line9, line10);

LiquidLine pwm_line(0, 0, "PWM level: ", pwmLevel);
LiquidScreen pwm_screen(pwm_line);

LiquidMenu menu(lcd);
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
  void leavingEventos(void);
  void leavingAlarma(void);
	stateMachine.SetOnLeaving(Inicio, []() {Serial.println("Leaving Inicio"); });
	stateMachine.SetOnLeaving(ConfigMenu, []() {Serial.println("Leaving Menu"); });
	stateMachine.SetOnLeaving(MonitorAmbiental, leavingAmbiental);
	stateMachine.SetOnLeaving(Bloqueo, []() {Serial.println("Leaving Bloqueo"); });
  stateMachine.SetOnLeaving(Alarma, leavingAlarma);
  stateMachine.SetOnLeaving(MonitorEventos, leavingEventos);
}
//Tasks
  void readLight(void);
  void readTemp(void);
  void readHum(void);
  void readTime(void);
  void readHall(void);
  void printSensorsLcd(void);
  void printHallLcd(void);
  void verifyTempLightLimits(void);
  void verifyHallLimit(void);
  void readBluelight (void);
  void melodyExecutable(void);
  void Menu(void);
  AsyncTask taskReadLight(1000,true,readLight);
  AsyncTask taskReadTemp(1000,true,readTemp);
  AsyncTask taskReadHum(1000,true,readHum);
  AsyncTask taskReadHall(1000,true,readHall);
  AsyncTask taskSetTime(10000,true,readTime);
  AsyncTask taskPrintLcd(1000,true,printSensorsLcd);
  AsyncTask taskPrintHallLcd(1000,true,printHallLcd);
  AsyncTask taskTempLightLimits(1000,true,verifyTempLightLimits);
  AsyncTask taskHallLimits(1000,true,verifyHallLimit);
  AsyncTask taskMenu(3000, true, Menu);
  AsyncTask taskBlueLight(800,true,readBluelight);
  AsyncTask taskMelody(800,true,melodyExecutable);
 

// fuction for menu to the configMneu state
void pwm_up() {
	if (pwmLevel < 225) {
		pwmLevel += 25;
	} else {
		pwmLevel = 250;
	}
	analogWrite(pwmPin, pwmLevel);
}

// Function to be attached to the pwm_line object.
void pwm_down() {
	if (pwmLevel > 25) {
		pwmLevel -= 25;
	} else {
		pwmLevel = 0;
	}
	analogWrite(pwmPin, pwmLevel);
}
void setup() 
{
	Serial.begin(115200);
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

  // Function to attach functions to LiquidLine objects.
	pwm_line.attach_function(1, pwm_up);
	pwm_line.attach_function(2, pwm_down);

	Serial.println("Starting State Machine...");
	setupStateMachine();	
	Serial.println("Start Machine Started");

	// Initial state
	stateMachine.SetState(Inicio, false, true);
}

void loop() 
{
  input = static_cast<Input>(readInput());
  taskSetTime.Update();
  //Updating tasks from Ambiental State
	taskReadLight.Update();
  taskReadTemp.Update();
  taskReadHum.Update();
  taskPrintLcd.Update();
  //Updating tasks from Eventos State
  taskReadHall.Update();
  taskPrintHallLcd.Update();
  taskHallLimits.Update(); 
  taskMenu.Update(); 
// updating task for Alarma State
  taskBlueLight.Update();

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
  taskMenu.Start();
}

void outputMAmbiental()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                    X                                 ");
	Serial.println();
  taskReadLight.Start();
  taskReadTemp.Start();
  taskReadHum.Start();
  taskPrintLcd.Start();
  taskTempLightLimits.Start();
  taskSetTime.SetIntervalMillis(7000);
  taskSetTime.Start();
}
void leavingAmbiental(){
  taskReadLight.Stop();
  taskReadTemp.Stop();
  taskReadHum.Stop();
  taskPrintLcd.Stop();
  taskTempLightLimits.Stop();
  taskSetTime.Stop();
}
void outputBloqueo()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                               X                      ");
	Serial.println();
  sisBloqueado();
  taskSetTime.SetIntervalMillis(10000);
  taskSetTime.Start();
}
void outputAlarma()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                                         X            ");
	Serial.println();
  taskMelody.Start();
  taskBlueLight.Start();
  taskSetTime.SetIntervalMillis(4000);
  taskSetTime.Start();
}
void leavingAlarma(){
  lcd.clear();
  taskMelody.Stop();
  taskBlueLight.Stop();
}

void outputMEventos()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                                                  X");
	Serial.println();
  taskReadHall.Start();
  taskPrintHallLcd.Start();
  taskHallLimits.Start();
  taskSetTime.SetIntervalMillis(3000);
  taskSetTime.Start();
}
void leavingEventos(){
  taskReadHall.Stop();
  taskPrintHallLcd.Stop();
  taskHallLimits.Stop();
  taskSetTime.Stop();
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
void readHall(){
  valueHall = analogRead(pinHall);
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
  //Print temperature
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
void printHallLcd(){
  //Print hall
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MAG:");
  lcd.print(valueHall);
}
void verifyTempLightLimits(){
  if(t > tempLimit && h > humLimit){
    input = Input::tempLightExceeded;
  }
}
void verifyHallLimit(){
  if(valueHall > hallLimit){
    input = Input::hallExceeded;
  }}
  void readBluelight(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BLUE:");
    digitalWrite(LED_BLUE,HIGH);
  
  }

  void melodyExecutable() {
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
   divider = melody[thisNote + 1];// calculates the duration of each note
    if (divider > 0) {
      noteDuration = (wholenote) / divider; // regular note, just proceed
    } else if (divider < 0) {
      noteDuration = (wholenote) / abs(divider);// dotted notes are represented with negative durations!!
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
    tone(buzzer, melody[thisNote], noteDuration * 0.9);// we only play the note for 90% of the duration, leaving 10% as a pause
     delay(noteDuration);// Wait for the specief duration before playing the next note.
     noTone(buzzer);// stop the waveform generation before the next note.
  }
}
void setupMenu() {

	menu.add_screen(screen1);
	menu.add_screen(screen2);
	menu.add_screen(screen3);
  menu.add_screen(screen4);
  menu.add_screen(screen5);
  menu.add_screen( pwm_screen);

	strncpy(ledState_text, string_off, sizeof(string_off));

	menu.update();
}
void Menu (){
  static bool initialized = false;
  if (!initialized) {
    setupMenu();
    initialized = true;
  }
boolean aux=true;
while (aux){
char key = keypad.getKey();
  
  if (key) {
    Serial.println(key);
    if (key == 'A') {
      Serial.println(F("RIGHT button pressed"));
      menu.next_screen();
    }
    if (key == 'B') {
      Serial.println(F("LEFT button pressed"));
      menu.previous_screen();
    }
    if (key == 'C') {
      Serial.println(F("UP button pressed"));
      menu.call_function(1);
    }
    if (key == 'D') {
      Serial.println(F("DOWN button pressed"));
      menu.call_function(2);
    }
    if (key == '*') {
      Serial.println(F("ENTER button pressed"));
      menu.switch_focus();
    }
    if(key == '#') {
    aux=false;
    }

    static unsigned long lastMillis = 0;
    static unsigned int period = 2000;
    if (millis() - lastMillis > period) {
      lastMillis = millis();

      Serial.print("LED turned ");
      if (ledState == LOW) {
        ledState = HIGH;
        strncpy(ledState_text, string_on, sizeof(string_on));
        Serial.println(ledState_text);
        menu.update();
      } else {
        ledState = LOW;
        strncpy(ledState_text, string_off, sizeof(string_off));
        Serial.println(ledState_text);
        menu.update();
      }
      digitalWrite(ledPin, ledState);

      analogValue = analogRead(analogPin);
      static unsigned short lastAnalogValue = 0;
      if (analogValue != lastAnalogValue) {
        lastAnalogValue = analogValue;
        menu.update();
      }
    }
  }}

}

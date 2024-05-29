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
#define  LED_RED 13
#define  LED_GREEN 9
#define  LED_BLUE 8 
#define BUTTON_PIN 6

// configuration for the alarm state melody 
#define NOTE_G6 1568
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_C7 2093
#define NOTE_E7 2637
#define NOTE_G7 3136
#define DHTTYPE DHT22 
#define DHTPIN 7

int buzzer = 7; // El pin del buzzer activo
int BloqueoMelody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0

};
int correctDurations[] = {
  300, 300, 300, 300,
  300, 300, 300, 300,
  300, 300, 300, 300
};
int correctMelodyLength = sizeof(BloqueoMelody) / sizeof(BloqueoMelody[0]);

int AlarmMelody[] = {

   NOTE_F3, NOTE_A3,

};
int incorrectDurations[] = {
  300, 300, 300,  300
};
int incorrectMelodyLength = sizeof(AlarmMelody) / sizeof(AlarmMelody[0]);

int blockedMelody[] = {
  NOTE_C4, NOTE_G3, NOTE_E3, NOTE_A3, NOTE_B3,
  NOTE_A3, NOTE_GS3, NOTE_AS3, NOTE_GS3, NOTE_G3,
  NOTE_F3, NOTE_G3
};
int blockedDurations[] = {
  500, 500, 500, 500, 300, 150, 500, 325, 175, 500, 325, 675
};
int blockedMelodyLength = sizeof(blockedMelody) / sizeof(blockedMelody[0]);


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
int h = 0;
int t = 0;

int tempHigh = 30;
int tempLow = 15;
int luzHigh = 800;
int luzLow = 200;
int humHigh = 70;
int humLow = 30;
int hallHigh = 5;
int hallLow = 1;
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
  void leavingEventos(void);
  void leavingAlarma(void);
  void leavingMenu(void);
	stateMachine.SetOnLeaving(Inicio, []() {Serial.println("Leaving Inicio"); });
	stateMachine.SetOnLeaving(ConfigMenu, leavingMenu );
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
  void readButton(void);
  AsyncTask taskReadLight(1000,true,readLight);
  AsyncTask taskReadTemp(1000,true,readTemp);
  AsyncTask taskReadHum(1000,true,readHum);
  AsyncTask taskReadHall(1000,true,readHall);
  AsyncTask taskSetTime(10000,true,readTime);
  AsyncTask taskPrintLcd(1000,true,printSensorsLcd);
  AsyncTask taskPrintHallLcd(1000,true,printHallLcd);
  AsyncTask taskTempLightLimits(500,true,verifyTempLightLimits);
  AsyncTask taskHallLimits(1000,true,verifyHallLimit);
  AsyncTask taskMenu(3000, true, Menu);
  AsyncTask taskBlueLight(400,true,readBluelight);
  AsyncTask taskMelody(800,true,melodyExecutable);
  AsyncTask taskReadButton(100,true,readButton);

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


LiquidLine line1(0, 0, "TH_TEMP_HIGH ", tempHigh);
LiquidLine line2(0, 1, "TH_TEMP_LOW ", tempLow);
LiquidScreen screen1(line1, line2);

LiquidLine line3(0, 0, "TH_TEMP_LOW ", tempLow);
LiquidLine line4(0, 1, "TH_LUZ_HIGH ", luzHigh);
LiquidScreen screen2(line3, line4);

LiquidLine line5(0, 0, "TH_LUZ_HIGH ", luzHigh);
LiquidLine line6(0, 1, "TH_LUZ_LOW ", luzLow);
LiquidScreen screen3(line5, line6);

LiquidLine line7(0, 0, "TH_LUZ_LOW ", luzLow);
LiquidLine line8(0, 1, "TH_HUM_HIGH ", humHigh);
LiquidScreen screen4(line7, line8);

LiquidLine line9(0, 0, "TH_HUM_LOW ", humLow);
LiquidLine line10(0, 1, "TH_HALL_HIGH ", hallHigh);
LiquidScreen screen5(line9, line10);

LiquidLine line11(0, 0, "TH_HALL_LOW ", hallLow);
LiquidLine line12(0, 1, "RESET");
LiquidScreen screen6(line11, line12);

LiquidMenu menu(lcd);



// Funciones para modificar los valores
void increase_tempHigh() {
    tempHigh += 5;
    menu.update();
}

void decrease_tempHigh() {
    tempHigh -= 5;
    menu.update();
}

void increase_tempLow() {
    tempLow += 5;
    menu.update();
}

void decrease_tempLow() {
    tempLow -= 5;
    menu.update();
}

void increase_luzHigh() {
    luzHigh += 10;
    menu.update();
}

void decrease_luzHigh() {
    luzHigh -= 10;
    menu.update();
}

void increase_luzLow() {
    luzLow += 10;
    menu.update();
}

void decrease_luzLow() {
    luzLow -= 10;
    menu.update();
}

void increase_humHigh() {
    humHigh += 5;
    menu.update();
}

void decrease_humHigh() {
    humHigh -= 1;
    menu.update();
}

void increase_humLow() {
    humLow += 1;
    menu.update();
}

void decrease_humLow() {
    humLow -= 1;
    menu.update();
}

void increase_hallHigh() {
    hallHigh += 1;
    menu.update();
}

void decrease_hallHigh() {
    hallHigh -= 1;
    menu.update();
}

void increase_hallLow() {
    hallLow += 1;
    menu.update();
}

void decrease_hallLow() {
    hallLow -= 1;
    menu.update();
}

void reset_values() {
    tempHigh = 30;
    tempLow = 15;
    luzHigh = 800;
    luzLow = 200;
    humHigh = 70;
    humLow = 30;
    hallHigh = 5;
    hallLow = 1;
    menu.update();
}
// fuction for menu to the configMneu state

void setup() 
{
	Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  dht.begin();
  //About leds
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  //Turn off the leds
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW); 

  // Function to attach functions to LiquidLine objects.
   
    line1.attach_function(1, increase_tempHigh);
    line1.attach_function(2, decrease_tempHigh);
    line2.attach_function(1, increase_tempLow);
    line2.attach_function(2, decrease_tempLow);
    line3.attach_function(1, increase_tempLow);
    line3.attach_function(2, decrease_tempLow);
    line4.attach_function(1, increase_luzHigh);
    line4.attach_function(2, decrease_luzHigh);
    line5.attach_function(1, increase_luzHigh);
    line5.attach_function(2, decrease_luzHigh);
    line6.attach_function(1, increase_luzLow);
    line6.attach_function(2, decrease_luzLow);
    line7.attach_function(1, increase_luzLow);
    line7.attach_function(2, decrease_luzLow);
    line8.attach_function(1, increase_humHigh);
    line8.attach_function(2, decrease_humHigh);
    line9.attach_function(1, increase_humLow);
    line9.attach_function(2, decrease_humLow);
    line10.attach_function(1, increase_hallHigh);
    line10.attach_function(2, decrease_hallHigh);
    line11.attach_function(1, increase_hallLow);
    line11.attach_function(2, decrease_hallLow);
    line12.attach_function(1, reset_values);

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
  taskReadButton.Update();
  //Updating tasks from Ambiental State
	taskReadLight.Update();
  taskTempLightLimits.Update();
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
  taskMelody.Update();
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
  taskReadButton.Start();
}
void leavingMenu(){
  taskMenu.Stop();
  taskReadButton.Stop();
}
void outputMAmbiental()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                    X                                 ");
	Serial.println();
  taskReadButton.Start();
  taskReadLight.Start();
  taskReadTemp.Start();
  taskReadHum.Start();
  taskPrintLcd.Start();
  taskTempLightLimits.Start();
  taskSetTime.SetIntervalMillis(7000);
  taskSetTime.Start();
}
void leavingAmbiental(){
  taskReadButton.Stop();
  taskReadLight.Stop();
  taskReadTemp.Stop();
  taskReadHum.Stop();
  taskPrintLcd.Stop();
  taskTempLightLimits.Stop();
  taskSetTime.Stop();
  lcd.clear();
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
  digitalWrite(LED_BLUE, LOW);
}

void outputMEventos()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("                                                  X");
	Serial.println();
  taskReadButton.Start();
  taskReadHall.Start();
  taskPrintHallLcd.Start();
  taskHallLimits.Start();
  taskSetTime.SetIntervalMillis(3000);
  taskSetTime.Start();
}
void leavingEventos(){
  taskReadButton.Stop();
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
  
}
void printHallLcd(){
  //Print hall
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MAG:");
  lcd.print(valueHall);
}
void verifyTempLightLimits(){
  if(t > tempHigh && h > humHigh){
    input = Input::tempLightExceeded;
  }
}
void verifyHallLimit(){
  if(valueHall > hallHigh){
    input = Input::hallExceeded;
  }}
  void readBluelight(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BLUE:");
    digitalWrite(LED_BLUE,HIGH);
  }
  void melodyExecutable(){

   int  *melody=AlarmMelody;
   int  *durations=correctDurations;
   int length=correctMelodyLength;
    for (int i = 0; i < length; i++) {
    tone(buzzer, melody[i], durations[i]);
    delay(durations[i] * 1.20); // Delay entre notas
    noTone(buzzer); // Asegurar que el buzzer esté apagado
  }
  }

void setupMenu() {
   menu.add_screen(screen1);
    menu.add_screen(screen2);
    menu.add_screen(screen3);
    menu.add_screen(screen4);
    menu.add_screen(screen5);
    menu.add_screen(screen6);

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
    input = Input::btnPress;
    }

    static unsigned long lastMillis = 0;
    static unsigned int period = 3500;
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
void readButton(){
  if(digitalRead(BUTTON_PIN)==LOW){
    input = Input::btnPress;
  }
}
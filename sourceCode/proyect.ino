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
#define BUTTON_PIN 6


//Sec Notes
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_DB3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_EB3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST     0

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
#define DHTTYPE 11 //Simultor 22
#define DHTPIN 38
DHT dht(DHTPIN,DHTTYPE);
int hum = 0;
int temp = 0;

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
  void leavingInicio(void);
	stateMachine.SetOnLeaving(Inicio, leavingInicio);
	stateMachine.SetOnLeaving(ConfigMenu, leavingMenu );
	stateMachine.SetOnLeaving(MonitorAmbiental, leavingAmbiental);
	stateMachine.SetOnLeaving(Bloqueo, leavingBloqueo);
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
  void seguridad(void);
  void failMelody(void);
  void successMelody(void);
  void sisBloqueado(void);
  AsyncTask taskReadLight(1000,true,readLight);
  AsyncTask taskReadTemp(1000,true,readTemp);
  AsyncTask taskReadHum(1000,true,readHum);
  AsyncTask taskReadHall(1000,true,readHall);
  AsyncTask taskSetTime(10000,true,readTime);
  AsyncTask taskPrintLcd(1000,true,printSensorsLcd);
  AsyncTask taskPrintHallLcd(1000,true,printHallLcd);
  AsyncTask taskTempLightLimits(500,true,verifyTempLightLimits);
  AsyncTask taskHallLimits(1000,true,verifyHallLimit);
  AsyncTask taskMenu(100, true, Menu);
  AsyncTask taskBlueLight(400,true,readBluelight);
  AsyncTask taskMelody(800,true,melodyExecutable);
  AsyncTask taskReadButton(100,true,readButton);
  AsyncTask taskSecurity(1000,false,seguridad);
  AsyncTask taskMelodyFail(800,false,failMelody);
  AsyncTask taskMelodySuccess(800,false,successMelody);
  AsyncTask taskBloqueo(500,true,sisBloqueado);


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
	Serial.begin(9600); //Simulator 115200
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
  //Updating security stuff
  taskSecurity.Update();
  taskMelodyFail.Update();
  taskMelodySuccess.Update();
  taskBloqueo.Update();
// updating task for Alarma State
  taskBlueLight.Update();
  taskMelody.Update();
	// Update State Machine
	stateMachine.Update();
  input = Input::unknown;
}

// Auxiliar output functions that show the state debug
void outputInicio()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("  X                                                   ");
	Serial.println();
  tryCounter = 0;
  counter = -1;
  taskSecurity.Start();
}
void leavingInicio()
{
  taskSecurity.Stop();
  taskMelodySuccess.Stop();
}
void outputMenu()
{
	Serial.println("Inicio   Menu   Ambiental   Bloqueo   Alarma   Eventos");
	Serial.println("          X                                           ");
	Serial.println();
     menu.add_screen(screen1);
    menu.add_screen(screen2);
    menu.add_screen(screen3);
    menu.add_screen(screen4);
    menu.add_screen(screen5);
    menu.add_screen(screen6);

	strncpy(ledState_text, string_off, sizeof(string_off));

	menu.update();
  taskMenu.Start();
  taskReadButton.Start();
}
void leavingMenu(){
  taskMenu.Stop();
  taskReadButton.Stop();
  lcd.clear();
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
  taskSetTime.SetIntervalMillis(10000);
  taskSetTime.Start();
  taskBloqueo.Start();
  taskMelodyFail.Start();
}
void leavingBloqueo(){
  lcd.clear();
  taskMelodyFail.Stop();
  taskBloqueo.Stop();
  digitalWrite(LED_RED,LOW);
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
   taskReadButton.Start();
   lcd.clear();
}
void leavingAlarma(){
  lcd.clear();
  taskMelody.Stop();
  taskBlueLight.Stop();
  digitalWrite(LED_BLUE, LOW);
    taskSetTime.Stop();
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
          //claCorrecta();
          successMelody();
          input = Input::claveCorrecta;
          return;
        }
        else{
          tryCounter++;
          digitalWrite(LED_BLUE,HIGH);
          lcd.clear();
          lcd.print("Clave incorrecta");
          delay(1000);
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
  Input::time;
}
void claCorrecta(){
  lcd.clear();
  taskMelodySuccess.Start();
  Serial.println("Clave correcta");
  lcd.print("Clave correcta");
  digitalWrite(LED_GREEN,HIGH);
  delay(3000);
  taskMelodySuccess.Stop();
  digitalWrite(LED_GREEN,LOW);
}

void readLight(){
  valueLight = analogRead(pinLight);
}
void readHall(){
  valueHall = analogRead(pinHall);
}
void readTemp(){
  temp = dht.readTemperature();
  if(isnan(temp)){
    Serial.println("Failed to read temperature");
  }
}
void readHum(){
  hum = dht.readHumidity();
  if(isnan(hum)){
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
  lcd.print(temp);
  //Print humidity  
  lcd.setCursor(8,0);
  lcd.print("HUM:");
  lcd.print(hum);
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
  if(temp > tempHigh && hum > humHigh){
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
    lcd.print("ALARMA ACTIVADA:");
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

void Menu (){
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
   }

}
void readButton(){
  if(digitalRead(BUTTON_PIN)==HIGH){
    input = Input::btnPress;
  }
}

void failMelody(){
  int melody[] = {
    NOTE_AS4, NOTE_AS4, NOTE_AS4,
    NOTE_F5, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,

  };

  int durations[] = {
    8, 8, 8,
    2, 2,
    8, 8, 8, 2, 4,
  };
  int size = sizeof(durations) / sizeof(int);

  for (int note = 0; note < size; note++) {
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int duration = 1000 / durations[note];
    tone(buzzer, melody[note], duration);

    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);

    //stop the tone playing:
    noTone(buzzer);
  }
}

void successMelody(){
  int  *melody=blockedMelody;
  int  *durations=blockedDurations;
  int length=blockedMelodyLength;
  for (int i = 0; i < length; i++) {
    tone(buzzer, melody[i], durations[i]);
    delay(durations[i] * 1.20); // Delay entre notas
    noTone(buzzer); // Asegurar que el buzzer esté apagado
  }
}

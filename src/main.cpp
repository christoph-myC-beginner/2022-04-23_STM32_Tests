
//erste Tests STM32 Christoph Schumachers
//
//

#include <Arduino.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


#include <STM32RTC.h>
/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

#include <stdio.h>
#include <time.h>
time_t nowRTC;                  // this is the epoch
//tm tm;                          // the structure tm holds time information in a more convient way

#include "stm32f1xx.h"

#include "max7219.h"
max7219sevSeg lc7(PA15);

#include "OneButton.h"
OneButton button1(A2, true);

#include "DrehEnco.h"
//Setup a new encoder
DrehEnco DrehEncoEins(A0, A1);

#include "lmenu.h"

//Menue wichtige Eintraege 
menu a1,a2,a3,a4,a5,a1a1,a1a2,a1a3,a1a4,a1a5,a2a1,a2a2,a2a3,a2a4,a2a5,a2a6,a2a7; //Menues entsprechend des struct definieren

int vorZurueckB; //Variable die Mittels Knopf verändert wird; muss manuell zurück gesetzt werden

int setValFlag;  //Variable für Eingabe einer Variablen

//Menue-Eintraege hier vornehmen
static char* a1text= "RGB LED";
static char* a2text= "Uhrzeit & Datum";
static char* a3text= "A3 text";
static char* a4text= "A4 text";
static char* a5text= "A5 text";
static char* a1a1text= "LED 13 Blink";
static char* a1a2text= "LED Gruen";
static char* a1a3text= "LED Blau 2";
static char* a1a4text= "LED Rot";
static char* a1a5text= "LED Blau";
static char* a2a1text= "set second";
static char* a2a2text= "set minute";
static char* a2a3text= "set hour";
static char* a2a4text= "set weekDay";
static char* a2a5text= "set day";
static char* a2a6text= "set month";
static char* a2a7text= "set year";

//ENDE Menue wichtige Eintraege

int toggleInterval = 250;
int multiplier = 5;
char* dutyUnit = " ms";

int m_hours;
int multiplierHours = 1;
char* hoursUnit = " h";
int m_minutes;
int multiplierMinutes = 1;
char* minutesUnit = "min";
int m_seconds;
int multiplierSeconds = 10;
char* secondUnit = "s";
int m_weekDay;
int multiplierWeekDay = 1;
char* weekDayUnit = "-";
int m_day;
int multiplierDay = 1;
char* dayUnit = "-";
int m_month;
int multiplierMonth = 1;
char* monthUnit = "-";
int m_year = 2022;
int multiplierYear = 1;
char* yearUnit = "-";

int keepFlag = 0;
unsigned long keepMillis;
unsigned long lastUsed;
unsigned long displayMillis;
const unsigned long displayRefreshRate = 500;
unsigned long testEpoch = 1617900991;

unsigned long delaytime=250;

int16_t cntVal = 0;

//declaration of functions
//void serialMsg(int msgZeit); 
//bool keepAnyFlagHighForTime(int keepItTime, bool anyFlag);
int keepFlagForTime(int keepTime, int flag);

void noDelayBlink(byte pin, int blinkZeit );

void showTimeDate(void);
void showTimeDateEpoch(void);
/*
void max7219_Init(uint8_t intensivity);

void max7219_SetIntensivity(uint8_t intensivity);
void max7219_Clean(void);
void max7219_SendData(uint8_t addr, uint8_t data);
void max7219_Turn_On(void);
void max7219_Turn_Off(void);
void max7219_Decode_On(void);
void max7219_Decode_Off(void);
void max7219_PrintDigit(MAX7219_Digits position, MAX7219_Numeric numeric, bool point);
*/
void delay_cus(uint32_t);

void click1();
void doubleclick1();
//void longPressStart1();
void longPress1();
//void longPressStop1();
/*
void click2();
void doubleclick2();
void longPress2();
void click3();
void doubleclick3();
void click4();
void doubleclick4();
void click5();
void doubleclick5();
void click6();
*/
void a1action();
void a2action();
void a3action();
void a4action();
void a5action();
void a1a1action();
void a1a2action();
void a1a3action();
void a1a4action();
void a1a5action();
void a2a1action();
void a2a2action();
void a2a3action();
void a2a4action();
void a2a5action();
void a2a6action();
void a2a7action();
void link_menu();
//END declaration of functions

void setup()
{
  Serial.begin(115200);
  pinMode(19, OUTPUT);
  pinMode(PC13, OUTPUT);
  pinMode(PB11, OUTPUT);
  pinMode(PA15, OUTPUT);

  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC

  //rtc.setEpoch(testEpoch);
  // link the doubleclick function to be called on a doubleclick event.
  // link the button 1 functions.
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  //button1.attachLongPressStart(longPressStart1);
  //button1.attachLongPressStop(longPressStop1);
  //button1.attachDuringLongPress(longPress1);
  button1.setDebounceTicks(20);

  
  //setup of TIM3 for encoder Mode
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

  TIM3->ARR = 0xFFFF;

  TIM3->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
  TIM3->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
  TIM3->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;
  TIM3->CR1 |= TIM_CR1_CEN;

  lcd.init();                      // initialize the lcd 
  //lcd.init();
  //delay(100);
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print(" Testing myC");
  lcd.setCursor(2,1);
  lcd.print(" STM32F103C6T8 ");
  lcd.setCursor(2,2);
  lcd.print(" Last change: ");
  lcd.setCursor(0,3);
  lcd.print(__DATE__);
  lcd.setCursor(12,3);
  lcd.print(__TIME__);
  
  //Setup of 7 Segment LED
  lc7.max7219_Init(8);
  //max7219_PrintDigit(DIGIT_3, LETTER_E, false);

  //Setup Menue
  link_menu();
  menu_init(&a1, &setValFlag);
  menu_lcdInit(&lcd);
  //menu_ssd1306_init(&display);
  //menu_ssd1306Print();
  //menu_lcdPrint();
  menu_print3();
  //ENDE Setup Menue
}

void loop()
{
  noDelayBlink(PC13, toggleInterval);
  button1.tick();
  DrehEncoEins.check();
  
  cntVal = TIM3->CNT;

  int aufAbB = DrehEncoEins.get4Step();
  workMenu(aufAbB, vorZurueckB);
  vorZurueckB = 0;
  
  switch (keepFlag) {
    case 0:
      
    break;
    case 1:
      if (millis() - displayMillis > displayRefreshRate) {
        displayMillis = millis();
        showTimeDateEpoch();
      }
      keepFlag = keepFlagForTime(20000, keepFlag);
    break;
  }
}

void noDelayBlink(byte pin, int blinkZeit ) {
  static unsigned long blinkMillis;
  if (millis() - blinkMillis > blinkZeit) {
    blinkMillis = millis();
    digitalWrite(pin, !digitalRead(pin)); //toggeln eines Digitalausgangs
  }
}

int keepFlagForTime(int keepTime, int flag) {
  if (keepTime == 0) {
    return flag;
  }
  else {
    if (millis()-keepMillis < keepTime) {
      return flag;
    }
    else {
      
      return 0;
    }
  }
}

// This function will be called when the button1 was pressed 1 time.
void click1() {
  Serial.println("Button 1 click.");
  vorZurueckB = 1;
} // click1


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("Button 1 doubleclick.");
  vorZurueckB = -1;
} // doubleclick1

//Menue Funktionen bei Aufruf
void a2action(){ 
  //Serial.println("A2 action!\n");
  
}
void a3action(){
  //Serial.println("A3 action!\n");
  rtc.setTime(m_hours, m_minutes, m_seconds);
  //rtc.setWeekDay(m_weekDay);
  rtc.setDay(m_day);
  rtc.setMonth(m_month);
  rtc.setYear(m_year-2000);
  

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(m_hours);
  lcd.print(":");
  lcd.print(m_minutes);
  lcd.print(":");
  lcd.print(m_seconds);
  lcd.setCursor(0,1);
  lcd.print(m_weekDay);
  lcd.print(", ");
  lcd.print(m_day);
  lcd.print("/");
  lcd.print(m_month);
  lcd.print("/");
  lcd.print(m_year);
  
  lcd.setCursor(0,2);
  if (rtc.getHours() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getHours());
  lcd.print(":");
  if (rtc.getMinutes() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getMinutes());
  lcd.print(":");
  if (rtc.getSeconds() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getSeconds());
  lcd.setCursor(0,3);
  if (rtc.getYear() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getYear());
  lcd.print("-");
  if (rtc.getMonth() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getMonth());
  lcd.print("-");
  if (rtc.getDay() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getDay());
  delay(5000);
  
}
void a4action(){ 
  //Serial.println("A4 action!\n");
  
  keepFlag = 1;
  keepMillis = millis(); 
  
}

void a5action(){ 
  //Serial.println("A5 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplier, &toggleInterval, dutyUnit);
    drawVarSet(toggleInterval);
    drawVarSet_lcd(toggleInterval);
  }
}

void a1a1action(){ 
  //Serial.println("A1a1 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplier, &toggleInterval, dutyUnit);
    drawVarSet(toggleInterval);
    drawVarSet_lcd(toggleInterval);
  }
}

void a1a2action(){ 
  //Serial.println("A1a2 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;

  }
}

void a1a3action(){ 
  //Serial.println("A1a3 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;

  }
}

void a1a4action(){ 
  //Serial.println("A1a4 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;

  }
}

void a1a5action(){ 
  //Serial.println("A1a5 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;

  }
}

void a2a1action(){ 
  //Serial.println("A2a1 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierSeconds, &m_seconds, secondUnit); 
    drawVarSet(m_seconds);
    drawVarSet_lcd(m_seconds);
  }
}

void a2a2action(){ 
  //Serial.println("A2a2 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierMinutes, &m_minutes, minutesUnit); 
    drawVarSet(m_minutes);
    drawVarSet_lcd(m_minutes);
  }
}

void a2a3action(){ 
  //Serial.println("A2a3 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierHours, &m_hours, hoursUnit); 
    drawVarSet(m_hours);
    drawVarSet_lcd(m_hours);
  }
}

void a2a4action(){ 
  //Serial.println("A2a4 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierWeekDay, &m_weekDay, weekDayUnit); 
    drawVarSet(m_weekDay);
    drawVarSet_lcd(m_weekDay);
  }
}

void a2a5action(){ 
  //Serial.println("A2a5 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierDay, &m_day, dayUnit); 
    drawVarSet(m_day);
    drawVarSet_lcd(m_day);
  }
}

void a2a6action(){ 
  //Serial.println("A2a6 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierMonth, &m_month, monthUnit); 
    drawVarSet(m_month);
    drawVarSet_lcd(m_month);
  }
}

void a2a7action(){ 
  //Serial.println("A2a7 action!\n");
  if (setValFlag == 0) {
    setValFlag = 1;
    ptr_init(&multiplierYear, &m_year, yearUnit); 
    drawVarSet(m_year);
    drawVarSet_lcd(m_year);
  }
}

//LINK Menue
void link_menu(){
//Main Menu
  a1.text = a1text;
  a1.up = &a5;
  a1.down = &a2;
  a1.ok = &a1a1;
  a1.back = &a1;

  a2.text = a2text;
  a2.up = &a1;
  a2.down = &a3;
  a2.ok = &a2a1;
  a2.back = &a2;
  a2.function = a2action;

  a3.text = a3text;
  a3.up = &a2;
  a3.down = &a4;
  a3.ok = NULL;
  a3.back = &a3;
  a3.function = a3action;

  a4.text = a4text;
  a4.up = &a3;
  a4.down = &a5;
  a4.ok = NULL;
  a4.back = &a4;
  a4.function = a4action;

  a5.text = a5text;
  a5.up = &a4;
  a5.down = &a1;
  a5.ok = NULL;
  a5.back = &a5;
  a5.function = a5action;

//Submenu
  a1a1.text = a1a1text;
  a1a1.up = &a1a3;
  a1a1.down = &a1a2;
  a1a1.ok = NULL;
  a1a1.back = &a1;
  a1a1.function = a1a1action;

  a1a2.text = a1a2text;
  a1a2.up = &a1a1;
  a1a2.down = &a1a3;
  a1a2.ok = NULL;
  a1a2.back = &a1;
  a1a2.function = a1a2action;

  a1a3.text = a1a3text;
  a1a3.up = &a1a2;
  a1a3.down = &a1a4;
  a1a3.ok = NULL;
  a1a3.back = &a1;
  a1a3.function = a1a3action;

  a1a4.text = a1a4text;
  a1a4.up = &a1a3;
  a1a4.down = &a1a5;
  a1a4.ok = NULL;
  a1a4.back = &a1;
  a1a4.function = a1a4action;

  a1a5.text = a1a5text;
  a1a5.up = &a1a4;
  a1a5.down = &a1a1;
  a1a5.ok = NULL;
  a1a5.back = &a1;
  a1a5.function = a1a5action;

  a2a1.text = a2a1text;
  a2a1.up = &a2a3;
  a2a1.down = &a2a2;
  a2a1.ok = NULL;
  a2a1.back = &a2;
  a2a1.function = a2a1action;

  a2a2.text = a2a2text;
  a2a2.up = &a2a1;
  a2a2.down = &a2a3;
  a2a2.ok = NULL;
  a2a2.back = &a2;
  a2a2.function = a2a2action;

  a2a3.text = a2a3text;
  a2a3.up = &a2a2;
  a2a3.down = &a2a4;
  a2a3.ok = NULL;
  a2a3.back = &a2;
  a2a3.function = a2a3action;

  a2a4.text = a2a4text;
  a2a4.up = &a2a3;
  a2a4.down = &a2a5;
  a2a4.ok = NULL;
  a2a4.back = &a2;
  a2a4.function = a2a4action;

  a2a5.text = a2a5text;
  a2a5.up = &a2a4;
  a2a5.down = &a2a6;
  a2a5.ok = NULL;
  a2a5.back = &a2;
  a2a5.function = a2a5action;

  a2a6.text = a2a6text;
  a2a6.up = &a2a5;
  a2a6.down = &a2a7;
  a2a6.ok = NULL;
  a2a6.back = &a2;
  a2a6.function = a2a6action;

  a2a7.text = a2a7text;
  a2a7.up = &a2a6;
  a2a7.down = &a2a1;
  a2a7.ok = NULL;
  a2a7.back = &a2;
  a2a7.function = a2a7action;
}


/*
 * ToDo
 * Begrenzung Variable
 * Darstellung Uhrzeit und Datum
 * Setzen von Uhrzeit und Datum auf RTC;speichern
 * Menue fuer Alarmzeit
 * Eingabe von "Namen"
 * Darstellung PID Parameter
 * PID Bib
 * PWM Output
 * Analog Input
 */

void showTimeDate(void) {

  lcd.clear();
  lcd.setCursor(0,0);
  if (rtc.getHours() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getHours());
  lcd.print(":");
  if (rtc.getMinutes() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getMinutes());
  lcd.print(":");
  if (rtc.getSeconds() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getSeconds());
  lcd.setCursor(0,1);
  if (rtc.getYear() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getYear());
  lcd.print("-");
  if (rtc.getMonth() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getMonth());
  lcd.print("-");
  if (rtc.getDay() < 10) {
    lcd.print("0");
  }
  lcd.print(rtc.getDay());
}

void showTimeDateEpoch(void) {

  nowRTC = rtc.getEpoch();
  //localtime_r(&nowRTC, &tm);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(nowRTC);
  
  lcd.setCursor(0,1);
  lcd.print(rtc.getMinutes());
  //lcd.print("----");
  //lcd.print(tm.tm_hour);
  
  lcd.setCursor(0,2);
  
  lcd.print(rtc.getYear());

  lcd.setCursor(8,3);
  lcd.print(cntVal);
  /*
  lcd.print("-");
  lcd.print(month(nowRTC));
  lcd.print("-");
  lcd.print(day(nowRTC));
  */
  
}

// delay loop for 8 MHz CPU clock with optimizer enabled
void delay_cus(uint32_t msec)
{
    for (uint32_t j=0; j < 2000UL * msec; j++)
    {
        __NOP();
    }
}
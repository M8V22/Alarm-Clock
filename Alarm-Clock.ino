#include <RTClib.h>
#include <EasyButton.h>

//Multiplexing frequency
#define Frequency 4

//Time to show the date (ms)
#define DateShowTime 2000
//Time to ring the alarm, before it stops automatically (ms) (1h -> 3600000, 2h -> 7200000)
#define AlarmRingTime 7200000
//Time to snooze the alarm (ms) (1 min -> 60000, 2 min -> 120000, 5 min -> 300000, 10 min -> 600000, 15 min -> 900000)
#define SnoozeTime 300000

//Use a piezo buzzer with oscillator or without
#define BuzzerOscillator false
//Alarm beep frequency
#define AlarmFrequency 1976

//Alarm LED
#define AlarmLED 13
//Piezo buzzer for alarm
#define AlarmSPK 8

//Segments A, B, C, D, E, F, G, DP
//https://en.wikipedia.org/wiki/Seven-segment_display#Characters
const int Segment[8] = {0, 1, 2, 3, 4, 5, 6, 7};
//Digits 1, 2, 3, 4
const int Digit[4] = {A0, A1, A2, A3};

//Button name () -> Pin on which the button is connected
//Connect the button between the pin and GND
//Button to show the date, set the date, time and alarms
EasyButton EnterButton(12);
//Buttons + -
EasyButton UpButton(9);
EasyButton DownButton(10);

RTC_DS1307 RTC;

int Hour = 0, Minute = 0, Day = 1, Month = 1, Year = 2021, AlarmHour = 0, AlarmMinute = 0;
bool ShowDate = 0, AlarmState = 0;
unsigned long Millis = 0, Millis2 = 0, AlarmMillis = 0;

void setup() {
  //Define segments as outputs
  for(int i = 0; i < 8; i++) {
    pinMode(Segment[i], OUTPUT);
    digitalWrite(Segment[i], LOW);
  }
  //Define digits as outputs
  for(int i = 0; i < 4; i++) {
    pinMode(Digit[i], OUTPUT);
    digitalWrite(Digit[i], LOW);
  }
  //Define alarm LED and the piezzo buzzer as output
  pinMode(AlarmLED, OUTPUT);
  pinMode(AlarmSPK, OUTPUT);
  
  //Start I2C and test successful connection to RTC
  RTC.begin();
  //Initialize the buttons
  EnterButton.begin();
  UpButton.begin();
  DownButton.begin();
  EnterButton.onPressed(SetShowDate);
  EnterButton.onPressedFor(3000, ClockSet);
  //If RTC is not set, set it
  if(!RTC.isrunning()) {
    ClockSet();
    SaveAlarm();
  } else {
    AlarmState = RTC.readnvram(00);
    AlarmHour = RTC.readnvram(01);
    AlarmMinute = RTC.readnvram(02);
    digitalWrite(AlarmLED, AlarmState);
  }
}

void loop() {
  //Read the RTC
  DateTime now = RTC.now();
  //Poll the buttons
  PollButtons();
  if(EnterButton.pressedFor(1000) && (UpButton.pressedFor(1000) || DownButton.pressedFor(1000))) AlarmSet();
  if(AlarmState == true && AlarmHour == now.hour() && AlarmMinute == now.minute() && now.second() <= 5) Alarm();
  //Show the date or the time 
  if(ShowDate == true && millis() - Millis2 <= DateShowTime) {
    DisplayDate();
  } else {
    DisplayTime();
    ShowDate = false;
  }
}

void Alarm() {
  AlarmMillis = millis();
  while(!(UpButton.pressedFor(1000) && DownButton.pressedFor(1000)) && millis() - AlarmMillis <= AlarmRingTime) {
    PollButtons();
    if(EnterButton.wasPressed() || UpButton.wasPressed() || DownButton.wasPressed()) {
      AlarmMillis = millis();
      if (BuzzerOscillator == true) noTone(AlarmSPK);
        else digitalWrite(AlarmSPK, LOW);
      while(!(UpButton.pressedFor(1000) && DownButton.pressedFor(1000)) && millis() - AlarmMillis <= SnoozeTime) {
        DisplayTime();
        PollButtons();
        if(millis() - AlarmMillis > 1500) digitalWrite(AlarmLED, ToggleState(1000));
      }
      digitalWrite(AlarmLED, AlarmState);
    }
    if(ToggleState(500) == true) {
      DisplayTime();
      if (BuzzerOscillator == true) tone(AlarmSPK, AlarmFrequency);
        else digitalWrite(AlarmSPK, HIGH);
    } else {
      Display('O', 'O', 'O', 'O');
      if (BuzzerOscillator == true) noTone(AlarmSPK);
        else digitalWrite(AlarmSPK, LOW);
    }
  }
  if (BuzzerOscillator == true) noTone(AlarmSPK);
    else digitalWrite(AlarmSPK, LOW);
}

void ClockSet() {
  //Turn OFF the colon in case it was left on
  digitalWrite(Segment[7], LOW);
  if(RTC.isrunning()) {
    DateTime now = RTC.now();
    Hour = now.hour();
    Minute = now.minute();
    Day = now.day();
    Month = now.month();
    Year = now.year();
  }
  TimeSet();
  DateSet();
  RTC.adjust(DateTime(Year, Month, Day, Hour, Minute, 0));
}

void AlarmSet() {
  //Turn OFF the colon in case it was left on
  digitalWrite(Segment[7], LOW);
  while(!EnterButton.wasPressed()) {
    PollButtons();
    if(UpButton.wasPressed() || DownButton.wasPressed()) AlarmState = !AlarmState;
    if(ToggleState(500) == true)
      if(AlarmState == true) Display('O', 0, 'n', 'O');
        else Display('O', 0, 'F', 'F');
    else Display('O', 'O', 'O', 'O');
    digitalWrite(AlarmLED, AlarmState);
  }
  PollButtons();
  Hour = AlarmHour;
  Minute = AlarmMinute;
  TimeSet();
  AlarmHour = Hour;
  AlarmMinute = Minute;
  SaveAlarm();
  digitalWrite(AlarmLED, AlarmState);
}

void PollButtons() {
  EnterButton.read();
  UpButton.read();
  DownButton.read();
}

void SaveAlarm() {
  RTC.writenvram(00, AlarmState);
  RTC.writenvram(01, AlarmHour);
  RTC.writenvram(02, AlarmMinute);
}

void SetShowDate() {
  ShowDate = !ShowDate;
  Millis2 = millis();
}

void TimeSet() {
  //Set the hours
  while(!(UpButton.wasPressed() || DownButton.wasPressed() || EnterButton.wasPressed())) {
    PollButtons();
    if(ToggleState(500) == true)
      Display(Hour/10, Hour%10, Minute/10, Minute%10);
    else Display('O', 'O', Minute/10, Minute%10);
  }
  while(!EnterButton.wasPressed()) {
    PollButtons();
    if(UpButton.wasPressed()) Hour++;
    if(DownButton.wasPressed()) Hour--;
    if(UpButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Hour++;
      Millis2 = millis();
    }
    if(DownButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Hour--;
      Millis2 = millis();
    }
    if(Hour > 23) Hour = 0;
    if(Hour < 0) Hour = 23;
    Display(Hour/10, Hour%10, Minute/10, Minute%10);
  }
  PollButtons();
  //Set the minutes
  while(!(UpButton.wasPressed() || DownButton.wasPressed() || EnterButton.wasPressed())) {
    PollButtons();
    if(ToggleState(500) == true)
      Display(Hour/10, Hour%10, Minute/10, Minute%10);
    else Display(Hour/10, Hour%10, 'O', 'O');
  }
  while(!EnterButton.wasPressed()) {
    PollButtons();
    if(UpButton.wasPressed()) Minute++;
    if(DownButton.wasPressed()) Minute--;
    if(UpButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Minute++;
      Millis2 = millis();
    }
    if(DownButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Minute--;
      Millis2 = millis();
    }
    if(Minute > 59) Minute = 0;
    if(Minute < 0) Minute = 59;
    Display(Hour/10, Hour%10, Minute/10, Minute%10);
  }
  PollButtons();
}

void DateSet() {
  //Set the day
  while(!(UpButton.wasPressed() || DownButton.wasPressed() || EnterButton.wasPressed())) {
    PollButtons();
    if(ToggleState(500) == true)
      Display(Day/10, Day%10, Month/10, Month%10);
    else Display('O', 'O', Month/10, Month%10);
  }
  while(!EnterButton.wasPressed()) {
    PollButtons();
    if(UpButton.wasPressed()) Day++;
    if(DownButton.wasPressed()) Day--;
    if(UpButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Day++;
      Millis2 = millis();
    }
    if(DownButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Day--;
      Millis2 = millis();
    }
    if(Day > 31) Day = 1;
    if(Day < 1) Day = 31;
    Display(Day/10, Day%10, Month/10, Month%10);
  }
  PollButtons();
  //Set the month
  while(!(UpButton.wasPressed() || DownButton.wasPressed() || EnterButton.wasPressed())) {
    PollButtons();
    if(ToggleState(500) == true)
      Display(Day/10, Day%10, Month/10, Month%10);
    else Display(Day/10, Day%10, 'O', 'O');
  }
  while(!EnterButton.wasPressed()) {
    PollButtons();
    if(UpButton.wasPressed()) Month++;
    if(DownButton.wasPressed()) Month--;
    if(UpButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Month++;
      Millis2 = millis();
    }
    if(DownButton.pressedFor(1000) && (millis() - Millis2 >= 75)) {
      Month--;
      Millis2 = millis();
    }
    if(Month > 12) Month = 1;
    if(Month < 1) Month = 12;
    Display(Day/10, Day%10, Month/10, Month%10);
  }
  PollButtons();
  //Set the year
  while(!EnterButton.wasPressed()) {
    PollButtons();
    if(UpButton.wasPressed()) Year++;
    if(DownButton.wasPressed()) Year--;
    Display((Year/1000)%10, (Year/100)%10, (Year/10)%10, Year%10);
  }
}

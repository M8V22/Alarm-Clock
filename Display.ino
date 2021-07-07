//Convert Time to numbers, display them and blink the colon
void DisplayTime() {
  //Read the RTC
  DateTime now = RTC.now();
  Display(now.hour()/10, now.hour()%10, now.minute()/10, now.minute()%10);
  //Blink the colon
  digitalWrite(Segment[7], ToggleState(1000));
}

//Convert Date to numbers and display them
void DisplayDate() {
  //Read the RTC
  DateTime now = RTC.now();
  //Turn OFF the colon in case it was left on
  digitalWrite(Segment[7], LOW);
  Display(now.day()/10, now.day()%10, now.month()/10, now.month()%10);
}

void Display(char Number1, char Number2, char Number3, char Number4) {
  //Select each digit and light it with it's number
  Segments(Number1);
  digitalWrite(Digit[0], LOW);
  delay(Frequency);
  digitalWrite(Digit[0], HIGH);
  Segments(Number2);
  digitalWrite(Digit[1], LOW);
  delay(Frequency);
  digitalWrite(Digit[1], HIGH);
  Segments(Number3);
  digitalWrite(Digit[2], LOW);
  delay(Frequency);
  digitalWrite(Digit[2], HIGH);
  Segments(Number4);
  digitalWrite(Digit[3], LOW);
  delay(Frequency);
  digitalWrite(Digit[3], HIGH);
}

//Convert number to segments
void Segments(char Number) {
  //Define numbers 0 - 9, letters 'F' and 'n' and display OFF (which segments need to be ON and which OFF)
  //https://en.wikipedia.org/wiki/Seven-segment_display#Hexadecimal
  const byte Numbers[13] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x71, 0x54, 0x00};
  switch(Number) {
    case 'F':
      Number = 10;
      break;

    case 'n':
      Number = 11;
      break;

    case 'O':
      Number = 12;
      break;
  }
  bool SegmentState;
  for(int i = 0; i < 7; i++) {
    SegmentState = bitRead(Numbers[Number], i);
    digitalWrite(Segment[i], SegmentState);
  }
}

//Change returned state every interval
bool State;
int ToggleState(int Interval) {
  if(millis() - Millis >= Interval) {
    Millis = millis();
    State = !State;
  }
  return State;
}

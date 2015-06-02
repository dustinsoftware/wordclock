// Word clock logic modified from https://github.com/wouterdevinck/wordclock

#include <stdint.h>
#include <TFT.h>
#include <Time.h>
#include <TouchScreen.h>

#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3
#define TS_MINX 140
#define TS_MAXX 900
#define TS_MINY 120
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Tasks
const int wait = 10;
const int noTasks = 2;
typedef struct Tasks {
   long unsigned int previous;
   int interval;
   void (*function)();
} Task;
Task tasks[noTasks];

// Buffer
boolean prevframe[16][16];

// Format: { line index, start position index, length }

const int w_it[3] =        { 0,  0,  2 };
const int w_is[3] =        { 0,  3,  2 };
const int w_half[3] =      { 7,  0,  4 }; 
const int w_to[3] =        { 7,  14, 2 };
const int w_past[3] =      { 8,  0,  4 };
const int w_oclock[3] =    { 11, 10, 6 };
const int w_in[3] =        { 12, 0,  2 };
const int w_the[3] =       { 12, 3,  3 };
const int w_afternoon[3] = { 12, 7,  9 };
const int w_noon[3] =      { 12, 12, 4 }; // part of "afternoon"
const int w_midnight[3] =  { 4,  8,  8 };
const int w_morning[3] =   { 13, 0,  7 };
const int w_at[3] =        { 13, 8,  2 };
const int w_night[3] =     { 13, 11, 5 };
const int w_evening[3] =   { 14, 0,  7 };
const int w_and[3] =       { 14, 8,  3 };
const int w_cold[3] =      { 14, 12, 4 };
const int w_cool[3] =      { 15, 0,  4 };
const int w_warm[3] =      { 15, 6,  4 };
const int w_hot[3] =       { 15, 12, 3 };
const int w_el[3] =        { 9,  2,  2 };

const int w_minutes[20][3] = {
  { 0,  13, 3 }, // one
  { 1,  0,  3 }, // two
  { 3,  0,  5 }, // three
  { 2,  12, 4 }, // four
  { 2,  0,  4 }, // five
  { 5,  0,  3 }, // six
  { 6,  0,  5 }, // seven
  { 5,  8,  5 }, // eight
  { 3,  6,  4 }, // nine
  { 1,  4,  3 }, // ten
  { 2,  5,  6 }, // eleven
  { 6,  10, 6 }, // twelve
  { 1,  8,  8 }, // thirteen
  { 4,  0,  8 }, // fourteen
  { 7,  6,  7 }, // quarter
  { 5,  0,  7 }, // sixteen
  { 6,  0,  9 }, // seventeen
  { 5,  8,  8 }, // eighteen
  { 3,  6,  8 }, // nineteen
  { 0,  6,  6 }  // twenty
};

const int w_hours[12][3] = {
  { 8,  5,  3 }, // one
  { 8,  9,  3 }, // two
  { 11, 4,  5 }, // three
  { 9,  7,  4 }, // four
  { 9,  12, 4 }, // five
  { 8,  13, 3 }, // six
  { 10, 0,  5 }, // seven
  { 10, 6,  5 }, // eight
  { 10, 12, 4 }, // nine
  { 11, 0,  3 }, // ten
  { 10, 1,  4 }, // "even"
  { 9,  0,  6 }  // twelve
};

char const gridLine1[] PROGMEM = "ITLISOTWENTYRONE";
char const gridLine2[] PROGMEM = "TWOETENMTHIRTEEN";
char const gridLine3[] PROGMEM = "FIVEMELEVENIFOUR";
char const gridLine4[] PROGMEM = "THREEPNINETEENSU";
char const gridLine5[] PROGMEM = "FOURTEENMIDNIGHT";
char const gridLine6[] PROGMEM = "SIXTEENDEIGHTEEN";
char const gridLine7[] PROGMEM = "SEVENTEENOTWELVE";
char const gridLine8[] PROGMEM = "HALFELQUARTEROTO";
char const gridLine9[] PROGMEM = "PASTRONESTWOISIX";
char const gridLine10[] PROGMEM = "TWELVETFOURAFIVE";
char const gridLine11[] PROGMEM = "SEVENMEIGHTENINE";
char const gridLine12[] PROGMEM = "TENTTHREECOCLOCK";
char const gridLine13[] PROGMEM = "INOTHENAFTERNOON";
char const gridLine14[] PROGMEM = "MORNINGSATENIGHT";
char const gridLine15[] PROGMEM = "EVENINGCANDTCOLD";
char const gridLine16[] PROGMEM = "COOLETWARMURHOTA";
const char* const PROGMEM characterGrid[16] = {
 gridLine1,
 gridLine2,
 gridLine3,
 gridLine4,
 gridLine5,
 gridLine6,
 gridLine7,
 gridLine8,
 gridLine9,
 gridLine10,
 gridLine11,
 gridLine12,
 gridLine13,
 gridLine14,
 gridLine15,
 gridLine16
};

void setup()
{
  loadTasks();
  Tft.init();  //init TFT library  
  Tft.setDisplayDirect(RIGHT2LEFT);
 // Tft.drawString("Testing.",80,0,1,WHITE);
  bool emptyFrame[16][16] = {0};
  updateDisplay(emptyFrame, emptyFrame, 1);  
  setTime(9,9,00,31,05,2015);
}


void showTime() {
  
  // Get the time
  
  int h = hour() % 24;
  int h2 = h;
  int m = minute();
  
  // DEBUG
  /*Serial.print("[DEBUG] ");
  Serial.print(h, DEC);
  Serial.print(':');
  Serial.println(m, DEC);*/
  
  // The frame
  boolean frame[16][16];
  for(int r = 0; r < 16; r++) {
    for(int c = 0; c < 16; c++) {
      frame[r][c] = false;
    }
  }
 
  // Show "IT IS"
  addWordToFrame(w_it, frame);
  addWordToFrame(w_is, frame);
  
  // Minutes
  if (m == 0) {
    
    if (h == 0) {
      addWordToFrame(w_midnight, frame);
    } else if (h == 12) {
      addWordToFrame(w_noon, frame);
    } else {
      addWordToFrame(w_oclock, frame);
    }

  } else {
  
    if (m <= 20) {
      addWordToFrame(w_minutes[m - 1], frame);
    } else if (m < 30) {
      addWordToFrame(w_minutes[19], frame); // twenty
      addWordToFrame(w_minutes[m - 21], frame);
    } else if (m == 30) {
      addWordToFrame(w_half, frame);
    } else if (m < 40) {
      addWordToFrame(w_minutes[19], frame); // twenty
      addWordToFrame(w_minutes[60 - m - 21], frame);
    } else {
      addWordToFrame(w_minutes[60 - m - 1], frame);
    }
 
    if(m <= 30) {
      addWordToFrame(w_past, frame);
    } else {
      addWordToFrame(w_to, frame);
      ++h2;
    }
    
  } 
  
  if(!(m ==0 && (h == 0 || h == 12))) {
  
    // Hours
    if(h2 == 0) {
      addWordToFrame(w_hours[11], frame);
    } else if (h2 <= 12) {
      addWordToFrame(w_hours[h2 - 1], frame);
    } else {
      addWordToFrame(w_hours[h2 - 13], frame);
    }
    if(h2 == 11 || h2 == 23) {
      addWordToFrame(w_el, frame);
    }
  
    // Time of day
    if(h < 12) {
      addWordToFrame(w_in, frame);
      addWordToFrame(w_the, frame);
      addWordToFrame(w_morning, frame);
    } else if(h < 17) {
      addWordToFrame(w_in, frame);
      addWordToFrame(w_the, frame);
      addWordToFrame(w_afternoon, frame);
    } else if(h < 20) {
      addWordToFrame(w_in, frame);
      addWordToFrame(w_the, frame);
      addWordToFrame(w_evening, frame);
    } else {
      addWordToFrame(w_at, frame);
      addWordToFrame(w_night, frame);
    }
  }

  // Update display
  updateDisplay(prevframe, frame, 0);
}

void updateDisplay(boolean previousframe[16][16], boolean frame[16][16], boolean initialDraw) {
  // todo implement for lcd 
  for (int y = 0; y < 15; y++) {
    for (int x = 0; x < 16; x++) {
      if (previousframe[y][x] == frame[y][x] && !initialDraw) {
         continue;
      }     
      
      char currentLineBuffer[17];
      strcpy_P(currentLineBuffer, (char*)pgm_read_word(&characterGrid[y]));
      
      previousframe[y][x] = frame[y][x];
      Tft.drawChar(currentLineBuffer[x], 239 - x * 15, 319 - y * 21, 2, frame[y][x] ? YELLOW : GRAY2);
    }
  }
}

void addWordToFrame(const int theword[3], boolean frame[16][16]){
  for(int i = 0; i < theword[2]; ++i) {
    frame[theword[0]][theword[1] + i] = true;
  }
}

void readInput() {
  TSPoint p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
  p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
  if (p.z < ts.pressureThreshhold)
    return;

  if (p.x < 120) {
    adjustTime(p.y < 240 ? 60 : -60);
  } else {
    adjustTime(p.y < 240 ? 3600 : -3600);
  }

  showTime();
}

void loadTasks() {
  
  // Show time
  tasks[0].previous = 0;
  tasks[0].interval = 1000;
  tasks[0].function = showTime;
  
  tasks[1].previous = 0;
  tasks[1].interval = 0;
  tasks[1].function = readInput;
}

void loop() {
  unsigned long time = millis();
  for(int i = 0; i < noTasks; i++) {
    Task task = tasks[i];
    if (time - task.previous > task.interval) {
      tasks[i].previous = time;
      task.function();
    }
  }  
  delay(wait);
}


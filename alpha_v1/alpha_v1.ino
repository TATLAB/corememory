#include <Keypad.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer =
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// the number of rows and columns in the quilt
// (some intersections might not be used)
const byte ROWS = 9; // 9 rows
const byte COLS = 7; // 7 columns

// This array defines a numerical label for each of the intersections.
// The value 0 is used for NO_KEY, so we have to pick 70 values greater than 0.

char keys[ROWS * COLS] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
  60, 61, 62, 63
};

#define NUM_TRACKS 36

// initialize array of audio track names
// array of the 36 quilt patches that trigger audio
// displayed by row

//// int triggers[NUM_TRACKS] = {
////  49, 48, 46, 45,
////  36, 42, 40, 38, 37,
////  29, 34, 33, 32, 30,
////  28, 27, 25, 24,
////  15, 20, 19, 18, 16,
////  8, 14, 12, 10, 9,
////  7, 6, 4, 3
////  };
//
//int triggers[NUM_TRACKS] = {
//  49, 48, 46, 45,
//  36, 42, 40, 38, 37,
//  29, 34, 33, 32, 30,
//  28, 27, 25, 24,
//  15, 20, 19, 18, 16,
//  8, 14, 12, 10, 9,
//  1, 2, 3, 4
//};

// this part needs to select which intersections are active
int triggers[NUM_TRACKS] = {
   1, 2, 3, 4,
   5, 6, 7, 8, 9,
  10, 11, 12, 13, 14,
  15, 16, 17, 18,
  19, 20, 21, 22, 23,
  24, 25, 26, 27, 28,
  29, 30, 31, 32
};

int trigger;

// character array of audio track names
// [number of audio tracks][Number of characters in the track name (12) + 1 newline character (= 13)]
char audiotracks[NUM_TRACKS][13] = {
  "track001.mp3", "track002.mp3", "track003.mp3", "track004.mp3",
  "track005.mp3", "track006.mp3", "track007.mp3", "track008.mp3",
  "track009.mp3", "track010.mp3", "track011.mp3", "track012.mp3",
  "track013.mp3", "track014.mp3", "track015.mp3", "track016.mp3",
  "track017.mp3", "track018.mp3", "track019.mp3", "track020.mp3",
  "track021.mp3", "track022.mp3", "track023.mp3", "track024.mp3",
  "track025.mp3", "track026.mp3", "track027.mp3", "track028.mp3",
  "track029.mp3", "track030.mp3", "track031.mp3", "track032.mp3",
  "track033.mp3", "track034.mp3", "track035.mp3", "track036.mp3"
};

// Pick 14 pins of the Arduino MEGA 2560.  They don't have to be in order, or consecutive.
//byte rowPins[ROWS] = {22, 24, 26, 28, 30, 32, 34, 36, 38}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {13, 14, 15, 16, 17, 18, 19, 20, 21}; //connect to the column pinouts of the keypad (13 and 14 are non functional but used to keep array out of bounds)
byte rowPins[ROWS] = {22, 24, 26, 28, 30, 32, 34, 36, 38}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {15, 16, 17, 18, 19, 20, 21}; //connect to the column pinouts of the keypad (13 and 14 are non functional but used to keep array out of bounds)

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// this will be assigned a track name according the given key value.
char currentTrack = "";

void setup() {
  Serial.begin(9600);
  Serial.println("This board is running: alpha_v1.ino");
  Serial.println("Started OK.");

  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 audio player chip found."));

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed, or is not present."));
    while (1);  // don't do anything more
  }
  Serial.println("SD card is detected.");
  Serial.println("Ready for input.");
  Serial.write(10);
  
  // not neccesary, but here if you need it
  // printDirectory(SD.open("/"), 0);

  // 20 quiet. 5 louder.
  musicPlayer.setVolume(5, 5);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
}

void loop() {
  // read in key value from the quilt
  char key = keypad.getKey();
//  if (newKey != NULL ) {
//    key = newKey;
//  }

  if (key != NO_KEY) {
    Serial.print("Detected trigger number: ");
    Serial.println(int(key));

    // check if key is in the array of triggers
    trigger = findKeyIndex(int(key));

    // if it is, set current track to audiotrack at that trigger index
    if (trigger != -1) {

      Serial.print("Playing file: ");
      Serial.println(audiotracks[trigger]);
      Serial.write(10);
      musicPlayer.stopPlaying();
      musicPlayer.currentTrack.close();
      musicPlayer.startPlayingFile(audiotracks[trigger]);
    }

  }

  // in case we need to do something while a file is playing
  //while (musicPlayer.playingMusic) {
  //  }

}

// utility function to find index of key in the array of triggers
// if key is not in the array, return -1.
int findKeyIndex(int key) {
  int index = -1;
  for (int i = 0; i < NUM_TRACKS; i++) {
    if (key == triggers[i]) {
      index = i;
    }
  }
  return index;
}




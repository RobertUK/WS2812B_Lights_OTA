/*
  OTAUpdate.ino, Example for the AutoConnect library.
  Copyright (c) 2020, Hieromon Ikasamo
  https://github.com/Hieromon/AutoConnect
  This example is an implementation of a lightweight update feature
  that updates the ESP8266's firmware from your web browser.
  You need a compiled sketch binary file to the actual update and can
  retrieve it using Arduino-IDE menu: [Sketch] -> [Export compiled binary].
  Then you will find the .bin file in your sketch folder. Select the.bin
  file on the update UI page to update the firmware.

  Notes:
  If you receive a following error, you need reset the module before sketch running.
  Update error: ERROR[11]: Invalid bootstrapping state, reset ESP8266 before updating.
  Refer to https://hieromon.github.io/AutoConnect/faq.html#hang-up-after-reset for details.

  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/
#define USE_LITTLEFS
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>

#include <ArduinoOTA.h>

typedef ESP8266WebServer WiFiWebServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
typedef WebServer WiFiWebServer;
#endif
#include <AutoConnect.h>

#ifdef INCLUDE_FALLBACK_INDEX_HTM
#include "extras/index_htm.h"
#endif

#if defined USE_SPIFFS
#include <FS.h>
const char *fsName = "SPIFFS";
FS *fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#elif defined USE_LITTLEFS
#include <LittleFS.h>
const char *fsName = "LittleFS";
FS *fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
#elif defined USE_SDFS
#include <SDFS.h>
const char *fsName = "SDFS";
FS *fileSystem = &SDFS;
SDFSConfig fileSystemConfig = SDFSConfig();
// fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif

const char *host = "Lights";
#define DBG_OUTPUT_PORT Serial

WiFiWebServer server;
AutoConnect portal(server);
AutoConnectConfig config;

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

#define AUTOCONNECT_APID "lights_setup"
#define AUTOCONNECT_PSK "12345678"

#define CONTROLUI_APID "Light_Control"

static bool fsOK;
String unsupportedFiles = String();

File uploadFile;
bool runAnimation = true;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";
static const char INVALID_COMMAND[] PROGMEM = "InvalidCommand";

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define RUN_DEMO 1

#include <MD_MAX72xx.h>
#include <SPI.h>

#if RUN_DEMO
#define DEMO_DELAY 10 // time to show each demo element in seconds
#else
#include <MD_UISwitch.h>
#endif

#define DEBUG 0 // Enable or disable (default) debugging output

#if DEBUG
#define PRINT(s, v)     \
  {                     \
    Serial.print(F(s)); \
    Serial.print(v);    \
  } // Print a string followed by a value (decimal)
#define PRINTX(s, v)      \
  {                       \
    Serial.print(F(s));   \
    Serial.print(v, HEX); \
  } // Print a string followed by a value (hex)
#define PRINTB(s, v)      \
  {                       \
    Serial.print(F(s));   \
    Serial.print(v, BIN); \
  } // Print a string followed by a value (binary)
#define PRINTC(s, v)       \
  {                        \
    Serial.print(F(s));    \
    Serial.print((char)v); \
  } // Print a string followed by a value (char)
#define PRINTS(s)       \
  {                     \
    Serial.print(F(s)); \
  } // Print a string
#else
#define PRINT(s, v)  // Print a string followed by a value (decimal)
#define PRINTX(s, v) // Print a string followed by a value (hex)
#define PRINTB(s, v) // Print a string followed by a value (binary)
#define PRINTC(s, v) // Print a string followed by a value (char)
#define PRINTS(s)    // Print a string
#endif

// #define CLK_PIN   4
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 40
#define FRAMES_PER_SECOND 100 // 120

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN 14  // 14  // or SCK
#define DATA_PIN 13 // 13  ```````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````// or MOSI
#define CS_PIN 12   // or SS

#define DATA_PIN_STRIP 2

// MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);                      // SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins

#if !RUN_DEMO
// --------------------
// Mode keyswitch parameters and object
//
#define MODE_SWITCH 9 // Digital Pin

MD_UISwitch_Digital ks = MD_UISwitch_Digital(MODE_SWITCH, LOW);
#endif

// --------------------
// Constant parameters
//
// Various delays in milliseconds
#define UNIT_DELAY 25 //  25
#define SCROLL_DELAY (4 * UNIT_DELAY)
#define MIDLINE_DELAY (6 * UNIT_DELAY)
#define SCANNER_DELAY (2 * UNIT_DELAY)
#define RANDOM_DELAY (6 * UNIT_DELAY)
#define FADE_DELAY (8 * UNIT_DELAY)
#define SPECTRUM_DELAY (4 * UNIT_DELAY)
#define HEARTBEAT_DELAY (1 * UNIT_DELAY)
#define HEARTS_DELAY (28 * UNIT_DELAY)
#define EYES_DELAY (20 * UNIT_DELAY)
#define WIPER_DELAY (1 * UNIT_DELAY)
#define ARROWS_DELAY (3 * UNIT_DELAY)
#define ARROWR_DELAY (8 * UNIT_DELAY)
#define INVADER_DELAY (6 * UNIT_DELAY)
#define PACMAN_DELAY (4 * UNIT_DELAY)
#define SINE_DELAY (2 * UNIT_DELAY)

#define CHAR_SPACING 1 // pixels between characters
#define BUF_SIZE 75    // character buffer size

// ========== General Variables ===========
//
uint32_t prevTimeAnim = 0; // Used for remembering the millis() value in animations
#if RUN_DEMO
uint32_t prevTimeDemo = 0;     //  Used for remembering the millis() time in demo loop
uint8_t timeDemo = DEMO_DELAY; // number of seconds left in this demo loop
#endif

// ========== Text routines ===========
//
// Text Message Table
// To change messages simply reorder, add to, or delete from, this table
const char *msgTab[] =
    {
        "Maynard Drive",
        // "Sorry for missing Christmas Lunch",
        // "I'm not very good at Cristmas",
        "WiFi: " CONTROLUI_APID,
};

// ========== Control routines ===========
//
void resetMatrix(void)
{
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
  prevTimeAnim = 0;
}

bool scrollText(bool bInit, const char *pmsg)
// Callback function for data that is required for scrolling into the display
{
  static char curMessage[BUF_SIZE];
  static char *p = curMessage;
  static uint8_t state = 0;
  static uint8_t curLen, showLen;
  static uint8_t cBuf[8];
  uint8_t colData;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Initializing ScrollText");
    resetMatrix();
    strcpy(curMessage, pmsg);
    state = 0;
    p = curMessage;
    bInit = false;
  }

  // Is it time to scroll the text?
  if (millis() - prevTimeAnim < SCROLL_DELAY)
    return (bInit);

  // scroll the display
  mx.transform(MD_MAX72XX::TSL); // scroll along
  prevTimeAnim = millis();       // starting point for next time

  // now run the finite state machine to control what we do
  PRINT("\nScroll FSM S:", state);
  switch (state)
  {
  case 0: // Load the next character from the font table
    PRINTC("\nLoading ", *p);
    showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
    curLen = 0;
    state = 1;

    // !! deliberately fall through to next state to start displaying

  case 1: // display the next part of the character
    colData = cBuf[curLen++];
    mx.setColumn(0, colData);
    if (curLen == showLen)
    {
      showLen = ((*p != '\0') ? CHAR_SPACING : mx.getColumnCount() - 1);
      curLen = 0;
      state = 2;
    }
    break;

  case 2: // display inter-character spacing (blank column) or scroll off the display
    mx.setColumn(0, 0);
    if (++curLen == showLen)
    {
      state = 0;
      bInit = (*p == '\0');
    }
    break;

  default:
    state = 0;
  }

  return (bInit);
}

// ========== Graphic routines ===========
//
bool graphicMidline1(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Midline1 init");
    resetMatrix();
    bInit = false;
  }
  else
  {
    for (uint8_t j = 0; j < MAX_DEVICES; j++)
    {
      mx.setRow(j, 3, 0xff);
      mx.setRow(j, 4, 0xff);
    }
  }

  return (bInit);
}

bool graphicMidline2(bool bInit)
{
  static uint8_t idx = 0;   // position
  static int8_t idOffs = 1; // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Midline2 init");
    resetMatrix();
    idx = 0;
    idOffs = 1;
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < MIDLINE_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  PRINT("\nML2 R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // turn off the old lines
  for (uint8_t j = 0; j < MAX_DEVICES; j++)
  {
    mx.setRow(j, idx, 0x00);
    mx.setRow(j, ROW_SIZE - 1 - idx, 0x00);
  }

  idx += idOffs;
  if ((idx == 0) || (idx == ROW_SIZE - 1))
    idOffs = -idOffs;

  // turn on the new lines
  for (uint8_t j = 0; j < MAX_DEVICES; j++)
  {
    mx.setRow(j, idx, 0xff);
    mx.setRow(j, ROW_SIZE - 1 - idx, 0xff);
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicScanner(bool bInit)
{
  const uint8_t width = 3;  // scanning bar width
  static uint8_t idx = 0;   // position
  static int8_t idOffs = 1; // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Scanner init");
    resetMatrix();
    idx = 0;
    idOffs = 1;
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SCANNER_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  PRINT("\nS R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // turn off the old lines
  for (uint8_t i = 0; i < width; i++)
    mx.setColumn(idx + i, 0);

  idx += idOffs;
  if ((idx == 0) || (idx + width == mx.getColumnCount()))
    idOffs = -idOffs;

  // turn on the new lines
  for (uint8_t i = 0; i < width; i++)
    mx.setColumn(idx + i, 0xff);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicRandom(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Random init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < RANDOM_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t i = 0; i < mx.getColumnCount(); i++)
    mx.setColumn(i, (uint8_t)random(255));
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicScroller(bool bInit)
{
  const uint8_t width = 3; // width of the scroll bar
  const uint8_t offset = mx.getColumnCount() / 3;
  static uint8_t idx = 0; // counter

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Scroller init");
    resetMatrix();
    idx = 0;
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SCANNER_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  PRINT("\nS I:", idx);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  mx.transform(MD_MAX72XX::TSL);

  mx.setColumn(0, idx >= 0 && idx < width ? 0xff : 0);
  if (++idx == offset)
    idx = 0;

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicSpectrum1(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Spectrum1 init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SPECTRUM_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t i = 0; i < MAX_DEVICES; i++)
  {
    uint8_t r = random(ROW_SIZE);
    uint8_t cd = 0;

    for (uint8_t j = 0; j < r; j++)
      cd |= 1 << j;
    for (uint8_t j = 1; j < COL_SIZE - 1; j++)
      mx.setColumn(i, j, ~cd);
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicSpectrum2(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Spectrum2init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SPECTRUM_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t i = 0; i < mx.getColumnCount(); i++)
  {
    uint8_t r = random(ROW_SIZE);
    uint8_t cd = 0;

    for (uint8_t j = 0; j < r; j++)
      cd |= 1 << j;

    mx.setColumn(i, ~cd);
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicHeartbeat(bool bInit)
{
#define BASELINE_ROW 4

  static uint8_t state;
  static uint8_t r, c;
  static bool bPoint;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Heartbeat init");
    resetMatrix();
    state = 0;
    r = BASELINE_ROW;
    c = mx.getColumnCount() - 1;
    bPoint = true;
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < HEARTBEAT_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  PRINT("\nHB S:", state);
  PRINT(" R: ", r);
  PRINT(" C: ", c);
  PRINT(" P: ", bPoint);
  mx.setPoint(r, c, bPoint);

  switch (state)
  {
  case 0: // straight line from the right side
    if (c == mx.getColumnCount() / 2 + COL_SIZE)
      state = 1;
    c--;
    break;

  case 1: // first stroke
    if (r != 0)
    {
      r--;
      c--;
    }
    else
      state = 2;
    break;

  case 2: // down stroke
    if (r != ROW_SIZE - 1)
    {
      r++;
      c--;
    }
    else
      state = 3;
    break;

  case 3: // second up stroke
    if (r != BASELINE_ROW)
    {
      r--;
      c--;
    }
    else
      state = 4;
    break;

  case 4: // straight line to the left
    if (c == 0)
    {
      c = mx.getColumnCount() - 1;
      bPoint = !bPoint;
      state = 0;
    }
    else
      c--;
    break;

  default:
    state = 0;
  }

  return (bInit);
}

bool graphicFade(bool bInit)
{
  static uint8_t intensity = 0;
  static int8_t iOffs = 1;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Fade init");
    resetMatrix();
    mx.control(MD_MAX72XX::INTENSITY, intensity);

    // Set all LEDS on
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
    for (uint8_t i = 0; i < mx.getColumnCount(); i++)
      mx.setColumn(i, 0xff);
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < FADE_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  intensity += iOffs;
  PRINT("\nF I:", intensity);
  PRINT(" D:", iOffs);
  if ((intensity == 0) || (intensity == MAX_INTENSITY))
    iOffs = -iOffs;
  mx.control(MD_MAX72XX::INTENSITY, intensity);

  return (bInit);
}

bool graphicHearts(bool bInit)
{
#define NUM_HEARTS ((MAX_DEVICES / 2) + 1)
  const uint8_t heartFull[] = {0x1c, 0x3e, 0x7e, 0xfc};
  const uint8_t heartEmpty[] = {0x1c, 0x22, 0x42, 0x84};
  const uint8_t offset = mx.getColumnCount() / (NUM_HEARTS + 1);
  const uint8_t dataSize = (sizeof(heartFull) / sizeof(heartFull[0]));

  static bool bEmpty;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Hearts init");
    resetMatrix();
    bEmpty = true;
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < HEARTS_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  PRINT("\nH E:", bEmpty);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t h = 1; h <= NUM_HEARTS; h++)
  {
    for (uint8_t i = 0; i < dataSize; i++)
    {
      mx.setColumn((h * offset) - dataSize + i, bEmpty ? heartEmpty[i] : heartFull[i]);
      mx.setColumn((h * offset) + dataSize - i - 1, bEmpty ? heartEmpty[i] : heartFull[i]);
    }
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  bEmpty = !bEmpty;

  return (bInit);
}

bool graphicEyes(bool bInit)
{
#define NUM_EYES 2
  const uint8_t eyeOpen[] = {0x18, 0x3c, 0x66, 0x66};
  const uint8_t eyeClose[] = {0x18, 0x3c, 0x3c, 0x3c};
  const uint8_t offset = mx.getColumnCount() / (NUM_EYES + 1);
  const uint8_t dataSize = (sizeof(eyeOpen) / sizeof(eyeOpen[0]));

  bool bOpen;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Eyes init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < EYES_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  bOpen = (random(1000) > 100);
  PRINT("\nH E:", bOpen);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t e = 1; e <= NUM_EYES; e++)
  {
    for (uint8_t i = 0; i < dataSize; i++)
    {
      mx.setColumn((e * offset) - dataSize + i, bOpen ? eyeOpen[i] : eyeClose[i]);
      mx.setColumn((e * offset) + dataSize - i - 1, bOpen ? eyeOpen[i] : eyeClose[i]);
    }
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicBounceBall(bool bInit)
{
  static uint8_t idx = 0;   // position
  static int8_t idOffs = 1; // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- BounceBall init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SCANNER_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  PRINT("\nBB R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // turn off the old ball
  mx.setColumn(idx, 0);
  mx.setColumn(idx + 1, 0);

  idx += idOffs;
  if ((idx == 0) || (idx == mx.getColumnCount() - 2))
    idOffs = -idOffs;

  // turn on the new lines
  mx.setColumn(idx, 0x18);
  mx.setColumn(idx + 1, 0x18);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicArrowScroll(bool bInit)
{
  const uint8_t arrow[] = {0x3c, 0x66, 0xc3, 0x99};
  const uint8_t dataSize = (sizeof(arrow) / sizeof(arrow[0]));

  static uint8_t idx = 0;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- ArrowScroll init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < ARROWS_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  PRINT("\nAR I:", idx);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.transform(MD_MAX72XX::TSL);
  mx.setColumn(0, arrow[idx++]);
  if (idx == dataSize)
    idx = 0;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicWiper(bool bInit)
{
  static uint8_t idx = 0;   // position
  static int8_t idOffs = 1; // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Wiper init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < WIPER_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  PRINT("\nW R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.setColumn(idx, idOffs == 1 ? 0xff : 0);
  idx += idOffs;
  if ((idx == 0) || (idx == mx.getColumnCount()))
    idOffs = -idOffs;

  return (bInit);
}

bool graphicInvader(bool bInit)
{
  const uint8_t invader1[] = {0x0e, 0x98, 0x7d, 0x36, 0x3c};
  const uint8_t invader2[] = {0x70, 0x18, 0x7d, 0xb6, 0x3c};
  const uint8_t dataSize = (sizeof(invader1) / sizeof(invader1[0]));

  static int8_t idx;
  static bool iType;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Invader init");
    resetMatrix();
    bInit = false;
    idx = -dataSize;
    iType = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < INVADER_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  // now run the animation
  PRINT("\nINV I:", idx);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  for (uint8_t i = 0; i < dataSize; i++)
  {
    mx.setColumn(idx - dataSize + i, iType ? invader1[i] : invader2[i]);
    mx.setColumn(idx + dataSize - i - 1, iType ? invader1[i] : invader2[i]);
  }
  idx++;
  if (idx == mx.getColumnCount() + (dataSize * 2))
    bInit = true;
  iType = !iType;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicPacman(bool bInit)
{
#define MAX_FRAMES 4 // number of animation frames
#define PM_DATA_WIDTH 18
  const uint8_t pacman[MAX_FRAMES][PM_DATA_WIDTH] = // ghost pursued by a pacman
      {
          {0x3c, 0x7e, 0x7e, 0xff, 0xe7, 0xc3, 0x81, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe},
          {0x3c, 0x7e, 0xff, 0xff, 0xe7, 0xe7, 0x42, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe},
          {0x3c, 0x7e, 0xff, 0xff, 0xff, 0xe7, 0x66, 0x24, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe},
          {0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe},
      };

  static int16_t idx;        // display index (column)
  static uint8_t frame;      // current animation frame
  static uint8_t deltaFrame; // the animation frame offset for the next frame

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Pacman init");
    resetMatrix();
    bInit = false;
    idx = -1; // DATA_WIDTH;
    frame = 0;
    deltaFrame = 1;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < PACMAN_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  PRINT("\nPAC I:", idx);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();

  // clear old graphic
  for (uint8_t i = 0; i < PM_DATA_WIDTH; i++)
    mx.setColumn(idx - PM_DATA_WIDTH + i, 0);
  // move reference column and draw new graphic
  idx++;
  for (uint8_t i = 0; i < PM_DATA_WIDTH; i++)
    mx.setColumn(idx - PM_DATA_WIDTH + i, pacman[frame][i]);

  // advance the animation frame
  frame += deltaFrame;
  if (frame == 0 || frame == MAX_FRAMES - 1)
    deltaFrame = -deltaFrame;

  // check if we are completed and set initialize for next time around
  if (idx == mx.getColumnCount() + PM_DATA_WIDTH)
    bInit = true;

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return (bInit);
}

bool graphicArrowRotate(bool bInit)
{
  static uint16_t idx; // transformation index

  uint8_t arrow[COL_SIZE] =
      {
          0b00000000,
          0b00011000,
          0b00111100,
          0b01111110,
          0b00011000,
          0b00011000,
          0b00011000,
          0b00000000};

  MD_MAX72XX::transformType_t t[] =
      {
          MD_MAX72XX::TRC,
          MD_MAX72XX::TRC,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TSR,
          MD_MAX72XX::TRC,
          MD_MAX72XX::TRC,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TSL,
          MD_MAX72XX::TRC,
      };

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- ArrowRotate init");
    resetMatrix();
    bInit = false;
    idx = 0;

    // use the arrow bitmap
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
    for (uint8_t j = 0; j < mx.getDeviceCount(); j++)
      mx.setBuffer(((j + 1) * COL_SIZE) - 1, COL_SIZE, arrow);
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < ARROWR_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);
  mx.transform(t[idx++]);
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::OFF);

  // check if we are completed and set initialize for next time around
  if (idx == (sizeof(t) / sizeof(t[0])))
    bInit = true;

  return (bInit);
}

bool graphicSinewave(bool bInit)
{
  static uint8_t curWave = 0;
  static uint8_t idx;

#define SW_DATA_WIDTH 11 // valid data count followed by up to 10 data points
  const uint8_t waves[][SW_DATA_WIDTH] =
      {
          {9, 8, 6, 1, 6, 24, 96, 128, 96, 16, 0},
          {6, 12, 2, 12, 48, 64, 48, 0, 0, 0, 0},
          {10, 12, 2, 1, 2, 12, 48, 64, 128, 64, 48},

      };
  const uint8_t WAVE_COUNT = sizeof(waves) / (SW_DATA_WIDTH * sizeof(uint8_t));

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Sinewave init");
    resetMatrix();
    bInit = false;
    idx = 1;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SINE_DELAY)
    return (bInit);
  prevTimeAnim = millis(); // starting point for next time

  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);
  mx.transform(MD_MAX72XX::TSL);
  mx.setColumn(0, waves[curWave][idx++]);
  if (idx > waves[curWave][0])
  {
    curWave = random(WAVE_COUNT);
    idx = 1;
  }
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::OFF);

  return (bInit);
}

void runMatrixAnimation(void)
// Schedule the animations, switching to the next one when the
// the mode switch is pressed.
{
  static uint8_t state = 0;
  static uint8_t mesg = 0;
  static boolean bRestart = true;
  static boolean bInMessages = false;
  boolean changeState = false;

#if RUN_DEMO
  // check if one second has passed and then count down the demo timer. Once this
  // gets to zero, change the state.
  if (millis() - prevTimeDemo >= 1000)
  {
    prevTimeDemo = millis();
    if (--timeDemo == 0)
    {
      timeDemo = DEMO_DELAY;
      changeState = true;
    }
  }
#else
  // check if the switch is pressed and handle that first
  changeState = (ks.read() == MD_UISwitch::KEY_PRESS);
#endif
  if (changeState)
  {
    if (bInMessages) // the message display state
    {
      mesg++;
      if (mesg >= sizeof(msgTab) / sizeof(msgTab[0]))
      {
        mesg = 0;
        bInMessages = false;
        state++;
      }
    }
    else
      state++;

    bRestart = true;
  };

  // now do whatever we do in the current state
  switch (state)
  {
  case 0:
    bInMessages = true;
    bRestart = scrollText(bRestart, msgTab[mesg]);
    break;
  case 1:
    bRestart = graphicMidline1(bRestart);
    break;
  case 2:
    bRestart = graphicMidline2(bRestart);
    break;
  case 3:
    bRestart = graphicScanner(bRestart);
    break;
  case 4:
    bRestart = graphicRandom(bRestart);
    break;
  case 5:
    bRestart = graphicFade(bRestart);
    break;
  case 6:
    bRestart = graphicSpectrum1(bRestart);
    break;
  case 7:
    bRestart = graphicHeartbeat(bRestart);
    break;
  case 8:
    bRestart = graphicHearts(bRestart);
    break;
  case 9:
    bRestart = graphicEyes(bRestart);
    break;
  case 10:
    bRestart = graphicBounceBall(bRestart);
    break;
  case 11:
    bRestart = graphicArrowScroll(bRestart);
    break;
  case 12:
    bRestart = graphicScroller(bRestart);
    break;
  case 13:
    bRestart = graphicWiper(bRestart);
    break;
  case 14:
    bRestart = graphicInvader(bRestart);
    break;
  case 15:
    bRestart = graphicPacman(bRestart);
    break;
  case 16:
    bRestart = graphicArrowRotate(bRestart);
    break;
  case 17:
    bRestart = graphicSpectrum2(bRestart);
    break;
  case 18:
    bRestart = graphicSinewave(bRestart);
    break;

  default:
    state = 0;
  }
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK()
{
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg)
{
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg)
{
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg)
{
  DBG_OUTPUT_PORT.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg)
{
  DBG_OUTPUT_PORT.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

#ifdef USE_SPIFFS
/*
   Checks filename for character combinations that are not supported by FSBrowser (alhtough valid on SPIFFS).
   Returns an empty String if supported, or detail of error(s) if unsupported
*/
String checkForUnsupportedPath(String filename)
{
  String error = String();
  if (!filename.startsWith("/"))
  {
    error += F("!NO_LEADING_SLASH! ");
  }
  if (filename.indexOf("//") != -1)
  {
    error += F("!DOUBLE_SLASH! ");
  }
  if (filename.endsWith("/"))
  {
    error += F("!TRAILING_SLASH! ");
  }
  return error;
}
#endif

////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/
void handleStatus()
{
  DBG_OUTPUT_PORT.println("handleStatus");
  FSInfo fs_info;
  String json;
  json.reserve(128);

  json = "{\"type\":\"";
  json += fsName;
  json += "\", \"isOk\":";
  if (fsOK)
  {
    fileSystem->info(fs_info);
    json += F("\"true\", \"totalBytes\":\"");
    json += fs_info.totalBytes;
    json += F("\", \"usedBytes\":\"");
    json += fs_info.usedBytes;
    json += "\"";
  }
  else
  {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  server.send(200, "application/json", json);
}

/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chuncked responses.
*/
void handleFileList()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (!server.hasArg("dir"))
  {
    return replyBadRequest(F("DIR ARG MISSING"));
  }

  String path = server.arg("dir");
  if (path != "/" && !fileSystem->exists(path))
  {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
  Dir dir = fileSystem->openDir(path);
  path.clear();

  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!server.chunkedResponseModeStart(200, "text/json"))
  {
    server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }

  // use the same string for every line
  String output;
  output.reserve(64);
  while (dir.next())
  {
#ifdef USE_SPIFFS
    String error = checkForUnsupportedPath(dir.fileName());
    if (error.length() > 0)
    {
      DBG_OUTPUT_PORT.println(String("Ignoring ") + error + dir.fileName());
      continue;
    }
#endif
    if (output.length())
    {
      // send string from previous iteration
      // as an HTTP chunk
      server.sendContent(output);
      output = ',';
    }
    else
    {
      output = '[';
    }

    output += "{\"type\":\"";
    if (dir.isDirectory())
    {
      output += "dir";
    }
    else
    {
      output += F("file\",\"size\":\"");
      output += dir.fileSize();
    }

    output += F("\",\"name\":\"");
    // Always return names without leading "/"
    if (dir.fileName()[0] == '/')
    {
      output += &(dir.fileName()[1]);
    }
    else
    {
      output += dir.fileName();
    }

    output += "\"}";
  }

  // send last string
  output += "]";
  server.sendContent(output);
  server.chunkedResponseFinalize();
}

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path)
{
  DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);
  if (!fsOK)
  {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/"))
  {
    path += "index.htm";
  }

  String contentType;
  if (server.hasArg("download"))
  {
    contentType = F("application/octet-stream");
  }
  else
  {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path))
  {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path))
  {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size())
    {
      DBG_OUTPUT_PORT.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}

/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent(String path)
{
  while (!path.isEmpty() && !fileSystem->exists(path))
  {
    if (path.lastIndexOf('/') > 0)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    else
    {
      path = String(); // No slash => the top folder does not exist
    }
  }
  DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
  return path;
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg("path");
  if (path.isEmpty())
  {
    return replyBadRequest(F("PATH ARG MISSING"));
  }

#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0)
  {
    return replyServerError(F("INVALID FILENAME"));
  }
#endif

  if (path == "/")
  {
    return replyBadRequest("BAD PATH");
  }
  if (fileSystem->exists(path))
  {
    return replyBadRequest(F("PATH FILE EXISTS"));
  }

  String src = server.arg("src");
  if (src.isEmpty())
  {
    // No source specified: creation
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith("/"))
    {
      // Create a folder
      path.remove(path.length() - 1);
      if (!fileSystem->mkdir(path))
      {
        return replyServerError(F("MKDIR FAILED"));
      }
    }
    else
    {
      // Create a file
      File file = fileSystem->open(path, "w");
      if (file)
      {
        file.write((const char *)0);
        file.close();
      }
      else
      {
        return replyServerError(F("CREATE FAILED"));
      }
    }
    if (path.lastIndexOf('/') > -1)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    replyOKWithMsg(path);
  }
  else
  {
    // Source specified: rename
    if (src == "/")
    {
      return replyBadRequest("BAD SRC");
    }
    if (!fileSystem->exists(src))
    {
      return replyBadRequest(F("SRC FILE NOT FOUND"));
    }

    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith("/"))
    {
      path.remove(path.length() - 1);
    }
    if (src.endsWith("/"))
    {
      src.remove(src.length() - 1);
    }
    if (!fileSystem->rename(src, path))
    {
      return replyServerError(F("RENAME FAILED"));
    }
    replyOKWithMsg(lastExistingParent(src));
  }
}

/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path)
{
  File file = fileSystem->open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir)
  {
    fileSystem->remove(path);
    return;
  }

  // Otherwise delete its contents first
  Dir dir = fileSystem->openDir(path);

  while (dir.next())
  {
    deleteRecursive(path + '/' + dir.fileName());
  }

  // Then delete the folder itself
  fileSystem->rmdir(path);
}

/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg(0);
  if (path.isEmpty() || path == "/")
  {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
  if (!fileSystem->exists(path))
  {
    return replyNotFound(FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path);

  replyOKWithMsg(lastExistingParent(path));
}

/*
   Handle a file upload request
*/
void handleFileUpload()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  if (server.uri() != "/edit")
  {
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
    uploadFile = fileSystem->open(filename, "w");
    if (!uploadFile)
    {
      return replyServerError(F("CREATE FAILED"));
    }
    DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (uploadFile)
    {
      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize)
      {
        return replyServerError(F("WRITE FAILED"));
      }
    }
    DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile)
    {
      uploadFile.close();
    }
    DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
  }
}

/*
   The "Not Found" handler catches all URI not explicitely declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  if (handleFileRead(uri))
  {
    return;
  }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += F(" NAME:");
    message += server.argName(i);
    message += F("\n VALUE:");
    message += server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += server.arg("path");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);

  return replyNotFound(message);
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit()
{
  if (handleFileRead(F("/edit/index.htm")))
  {
    return;
  }

#ifdef INCLUDE_FALLBACK_INDEX_HTM
  server.sendHeader(F("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
#else
  replyNotFound(FPSTR(FILE_NOT_FOUND));
#endif
}
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void handleLightsOff()
{
  char str[10];
  sprintf(str, "%u", FastLED.getBrightness());

  if (FastLED.getBrightness() == 0)
  {
    return replyServerError(str);
  }

  DBG_OUTPUT_PORT.println(String("handleLightsOff"));

  FastLED.clear(true);
   //FastLED.show();
  runAnimation = false;
  replyOK();
}

void handleLightsOn()
{

  DBG_OUTPUT_PORT.println(String("handleLightsOn"));
  fill_solid(leds, NUM_LEDS, CRGB::White);

   FastLED.show();
  //runAnimation = true;
  replyOK();
}

void handleSetBrightness()
{

  DBG_OUTPUT_PORT.println(String("handleSetBrightness"));
  String message;
  message.reserve(100);
  message += "brightness=";
  message += server.arg("brightness");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);

  uint16_t brightness = server.arg("brightness").toInt();

  if (brightness < 0 || brightness > 255)
  {
    return replyServerError(FPSTR(INVALID_COMMAND));
  }

  FastLED.setBrightness(brightness);

  replyOK();
}



void handleSetColour()
{

  DBG_OUTPUT_PORT.println(String("handleSetColour"));
  String message;
  message.reserve(100);
  message += "colour=";
  message += server.arg("colour");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);

  fill_solid(leds, NUM_LEDS, CRGB::White);

  uint32_t colour = server.arg("colour").toInt()
  ;

  CRGB colourPreset; 

  switch (colour)
  {
  case 16777215:
    colourPreset = CRGB::White;
    break;

  case 16776960:
    colourPreset = CRGB::Yellow;
    break;

  case 16711680:
    colourPreset = CRGB::Red;
    break;

  case 255:
    colourPreset = CRGB::Blue;
    break;

  case 32768:
    colourPreset = CRGB::Green;
    break;

  case 16761035:
    colourPreset = CRGB::Pink;
    break;

  case 10824234:
    colourPreset = CRGB::Brown;
    break;
  
  default:
    colourPreset = CRGB::White;
    break;
  }

  fill_solid(leds, NUM_LEDS,colourPreset);


  runAnimation = false;

  replyOK();
}

void handleResumeAnimation()
{

  DBG_OUTPUT_PORT.println(String("handleResumeAnimation"));

  // FastLED.setBrightness(255);

  runAnimation = true;

  replyOK();
}

void handlePauseAnimation()
{

  DBG_OUTPUT_PORT.println(String("handlePauseAnimation"));

  // FastLED.setBrightness(255);

  runAnimation = false;

  replyOK();
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void addGlitter(fract8 chanceOfGlitter)
{
  FastLED.show(BRIGHTNESS / 2);
  if (random8() < chanceOfGlitter)
  {
    // FastLED.setBrightness(BRIGHTNESS);
    // FastLED.show(BRIGHTNESS / 2)
    leds[random16(NUM_LEDS)] += CRGB::White;
    // leds[random16(NUM_LEDS)] +=
  }
}
void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter

  rainbow();
  addGlitter(80);
}
void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++)
  { // 9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};
SimplePatternList gPatterns = {rainbowWithGlitter, sinelon};
void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void exitOTAStart()
{
  Serial.println("OTA started");
}

void exitOTAProgress(unsigned int amount, unsigned int sz)
{
  Serial.printf("OTA in progress: received %d bytes, total %d bytes\n", sz, amount);
}

void exitOTAEnd()
{
  Serial.println("OTA ended");
}

void exitOTAError(uint8_t err)
{
  Serial.printf("OTA error occurred %d\n", err);
}

static const char PAGE_AUTH[] PROGMEM = R"(
{
  "uri": "/tt",
  "title": "Auth",
  "menu": true,
  "auth": "basic"
}
)";

void setup()
{
  ////////////////////////////////
  // SERIAL INIT
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print('\n');

  ////////////////////////////////
  // FILESYSTEM INIT

  fileSystemConfig.setAutoFormat(true);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  DBG_OUTPUT_PORT.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));

  ArduinoOTA.setHostname((config.hostName + "-ota").c_str());

  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Responder of root page and apply page handled directly from WebServer class.
  server.on("/o", []()
            {
    String content = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
Place the root page with the sketch application.&ensp;
__AC_LINK__
</body>
</html>
    )";
    content.replace("__AC_LINK__", String(AUTOCONNECT_LINK(COG_16)));
    server.send(200, "text/html", content); });

  ////////////////////////////////
  // WEB SERVER INIT

  // Filesystem status
  server.on("/status", HTTP_GET, handleStatus);

  // List directory
  server.on("/list", HTTP_GET, handleFileList);

  // Load editor
  server.on("/edit", HTTP_GET, handleGetEdit);

  // Create file
  server.on("/edit", HTTP_PUT, handleFileCreate);

  // Delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);

  // Upload file
  // - first callback is called after the request has ended with all parsed arguments
  // - second callback handles file upload at that location
  server.on("/edit", HTTP_POST, replyOK, handleFileUpload);

  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  // To make AutoConnect recognize the 404 handler, replace it with:
  // server.onNotFound(handleNotFound);
  portal.onNotFound(handleNotFound);

  server.on("/setcolour", HTTP_GET, handleSetColour);
  server.on("/setbrightness", HTTP_GET, handleSetBrightness);
  server.on("/resumeanimation", HTTP_GET, handleResumeAnimation);
  server.on("/pauseanimation", HTTP_GET, handlePauseAnimation);
  server.on("/lightson", HTTP_GET, handleLightsOn);
  server.on("/lightsoff", HTTP_GET, handleLightsOff);

  // Using AutoConnect does not require the HTTP server to be started
  // intentionally. It is launched inside AutoConnect.begin.
  // Start server
  // server.begin();
  // DBG_OUTPUT_PORT.println("HTTP server started");

  // Start AutoConnect
  config.title = "FSBrowser";
  config.hostName = "lights";
  config.username = "user";
  config.password = "password";
  config.auth = AC_AUTH_BASIC;
  config.authScope = AC_AUTHSCOPE_PARTIAL;

  portal.config(config);
  portal.append("/edit", "Edit");
  portal.append("/list?dir=\"/\"", "List");
  portal.load(FPSTR(PAGE_AUTH));
  if (portal.begin())
  {
    DBG_OUTPUT_PORT.print(F("Connected! IP address: "));
    DBG_OUTPUT_PORT.println(WiFi.localIP());
  }
  else
  {
    DBG_OUTPUT_PORT.print(F("Problem connecting to WiFi most likely, starting SoftAP"));
  }

  DBG_OUTPUT_PORT.println("HTTP server started");

  // With applying AutoConnect, the MDNS service must be started after
  // establishing a WiFi connection.
  // MDNS INIT
  // if (MDNS.begin(host)) {
  //   MDNS.addService("http", "tcp", 80);
  //   DBG_OUTPUT_PORT.print(F("Open http://"));
  //   DBG_OUTPUT_PORT.print(host);
  //   DBG_OUTPUT_PORT.println(F(".local/edit to open the FileSystem Browser"));
  //   DBG_OUTPUT_PORT.print(F("Open http://"));
  //   DBG_OUTPUT_PORT.print(host);
  //   DBG_OUTPUT_PORT.println(F(".local/_ac to AutoConnect statistics"));
  // }

  config.ota = AC_OTA_BUILTIN;
  portal.config(config);
  portal.onOTAStart(exitOTAStart);
  portal.onOTAEnd(exitOTAEnd);
  portal.onOTAProgress(exitOTAProgress);
  portal.onOTAError(exitOTAError);
  portal.begin();

  DBG_OUTPUT_PORT.println(F("Setup FastLED"));
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN_STRIP, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  DBG_OUTPUT_PORT.println(F("Connected! IP address: "));

  DBG_OUTPUT_PORT.println(F("Starting AP for lights settings captive portal page"));

  Serial.println("Configuring access point...");
  WiFi.softAPConfig(apIP, apIP, netMsk);

  WiFi.softAP(CONTROLUI_APID);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  DBG_OUTPUT_PORT.println(F("Starting AP for lights settings captive portal page"));

  DBG_OUTPUT_PORT.print(F("Connected! AP IP: "));
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  // Setup MDNS responder
  if (!MDNS.begin(host))
  {
    Serial.println("Error setting up MDNS responder!");
  }
  else
  {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    DBG_OUTPUT_PORT.print(F("Open http://"));
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(F(".local/edit to open the FileSystem Browser"));
    DBG_OUTPUT_PORT.print(F("Open http://"));
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(F(".local/_ac to AutoConnect statistics"));
  }

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  mx.begin();
  prevTimeAnim = millis();
#if RUN_DEMO
  prevTimeDemo = millis();
#else
  ks.begin();
#endif
#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_MAX72XX DaftPunk]");
}

void loop()
{
  dnsServer.processNextRequest();

  MDNS.update();

  portal.handleClient();

  MDNS.update();
  ArduinoOTA.handle();

      FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

  if (runAnimation)
  {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS(20) { gHue++; }   // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(10) { nextPattern(); } // change patterns periodically

    runMatrixAnimation();
  }
}

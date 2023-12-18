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

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
typedef ESP8266WebServer  WiFiWebServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
typedef WebServer WiFiWebServer;
#endif
#include <AutoConnect.h>

WiFiWebServer server;
AutoConnect portal(server);
AutoConnectConfig config;

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define DATA_PIN    D7
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120




// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))



void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}



void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
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
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


  SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };


void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}




void exitOTAStart() {
  Serial.println("OTA started");
}

void exitOTAProgress(unsigned int amount, unsigned int sz) {
  Serial.printf("OTA in progress: received %d bytes, total %d bytes\n", sz, amount);
}

void exitOTAEnd() {
  Serial.println("OTA ended");
}

void exitOTAError(uint8_t err) {
  Serial.printf("OTA error occurred %d\n", err);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  // Responder of root page and apply page handled directly from WebServer class.
  server.on("/", []() {
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
    server.send(200, "text/html", content);
  });

  config.ota = AC_OTA_BUILTIN;
  portal.config(config);
  portal.onOTAStart(exitOTAStart);
  portal.onOTAEnd(exitOTAEnd);
  portal.onOTAProgress(exitOTAProgress);
  portal.onOTAError(exitOTAError);
  portal.begin();

  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  portal.handleClient();
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}
// /// @file    Pacifica.ino
// /// @brief   Gentle, blue-green ocean wave animation
// /// @example Pacifica.ino

// //
// //  "Pacifica"
// //  Gentle, blue-green ocean waves.
// //  December 2019, Mark Kriegsman and Mary Corey March.
// //  For Dan.
// //

// #define FASTLED_ALLOW_INTERRUPTS 0
// #include <FastLED.h>
// FASTLED_USING_NAMESPACE

// #define DATA_PIN            22
// #define NUM_LEDS            60
// #define MAX_POWER_MILLIAMPS 500
// #define LED_TYPE            WS2812B
// #define COLOR_ORDER         GRB

// //////////////////////////////////////////////////////////////////////////

// CRGB leds[NUM_LEDS];



// //////////////////////////////////////////////////////////////////////////
// //
// // The code for this animation is more complicated than other examples, and 
// // while it is "ready to run", and documented in general, it is probably not 
// // the best starting point for learning.  Nevertheless, it does illustrate some
// // useful techniques.
// //
// //////////////////////////////////////////////////////////////////////////
// //
// // In this animation, there are four "layers" of waves of light.  
// //
// // Each layer moves independently, and each is scaled separately.
// //
// // All four wave layers are added together on top of each other, and then 
// // another filter is applied that adds "whitecaps" of brightness where the 
// // waves line up with each other more.  Finally, another pass is taken
// // over the led array to 'deepen' (dim) the blues and greens.
// //
// // The speed and scale and motion each layer varies slowly within independent 
// // hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// // with a lot of oddly specific numeric ranges.
// //
// // These three custom blue-green color palettes were inspired by the colors found in
// // the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
// //
// CRGBPalette16 pacifica_palette_1 = 
//     { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
//       0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
// CRGBPalette16 pacifica_palette_2 = 
//     { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
//       0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
// CRGBPalette16 pacifica_palette_3 = 
//     { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
//       0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };



// // Add one layer of waves into the led array
// void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
// {
//   uint16_t ci = cistart;
//   uint16_t waveangle = ioff;
//   uint16_t wavescale_half = (wavescale / 2) + 20;
//   for( uint16_t i = 0; i < NUM_LEDS; i++) {
//     waveangle += 250;
//     uint16_t s16 = sin16( waveangle ) + 32768;
//     uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
//     ci += cs;
//     uint16_t sindex16 = sin16( ci) + 32768;
//     uint8_t sindex8 = scale16( sindex16, 240);
//     CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
//     leds[i] += c;
//   }
// }

// // Add extra 'white' to areas where the four layers of light have lined up brightly
// void pacifica_add_whitecaps()
// {
//   uint8_t basethreshold = beatsin8( 9, 55, 65);
//   uint8_t wave = beat8( 7 );
  
//   for( uint16_t i = 0; i < NUM_LEDS; i++) {
//     uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
//     wave += 7;
//     uint8_t l = leds[i].getAverageLight();
//     if( l > threshold) {
//       uint8_t overage = l - threshold;
//       uint8_t overage2 = qadd8( overage, overage);
//       leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
//     }
//   }
// }

// // Deepen the blues and greens
// void pacifica_deepen_colors()
// {
//   for( uint16_t i = 0; i < NUM_LEDS; i++) {
//     leds[i].blue = scale8( leds[i].blue,  145); 
//     leds[i].green= scale8( leds[i].green, 200); 
//     leds[i] |= CRGB( 2, 5, 7);
//   }
// }


// void pacifica_loop()
// {
//   // Increment the four "color index start" counters, one for each wave layer.
//   // Each is incremented at a different speed, and the speeds vary over time.
//   static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
//   static uint32_t sLastms = 0;
//   uint32_t ms = GET_MILLIS();
//   uint32_t deltams = ms - sLastms;
//   sLastms = ms;
//   uint16_t speedfactor1 = beatsin16(3, 179, 269);
//   uint16_t speedfactor2 = beatsin16(4, 179, 269);
//   uint32_t deltams1 = (deltams * speedfactor1) / 256;
//   uint32_t deltams2 = (deltams * speedfactor2) / 256;
//   uint32_t deltams21 = (deltams1 + deltams2) / 2;
//   sCIStart1 += (deltams1 * beatsin88(1011,10,13));
//   sCIStart2 -= (deltams21 * beatsin88(777,8,11));
//   sCIStart3 -= (deltams1 * beatsin88(501,5,7));
//   sCIStart4 -= (deltams2 * beatsin88(257,4,6));

//   // Clear out the LED array to a dim background blue-green
//   fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

//   // Render each of four layers, with different scales and speeds, that vary over time
//   pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
//   pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
//   pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
//   pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

//   // Add brighter 'whitecaps' where the waves lines up more
//   pacifica_add_whitecaps();

//   // Deepen the blues and greens a bit
//   pacifica_deepen_colors();
// }void setup() {
//   delay( 3000); // 3 second delay for boot recovery, and a moment of silence
//   FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
//         .setCorrection( TypicalLEDStrip );
//   FastLED.setMaxPowerInVoltsAndMilliamps( 5, MAX_POWER_MILLIAMPS);
// }

// void loop()
// {
//   EVERY_N_MILLISECONDS( 20) {
//     pacifica_loop();
//     FastLED.show();
//   }
// }
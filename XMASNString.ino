
#include "Adafruit_WS2801.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma
#ifdef __AVR_ATtiny85__
 #include <avr/power.h>
#endif

//  #define debug for test mode
#undef debug

uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 3;    // Green wire on Adafruit Pixels

const int numpins = 50;
const int patternsize = 32;

#ifdef debug
  const int repcount = 2; // 10 for testing, 100 changes often, 1000 slow changes
#else
  const int repcount = 20; // 10 for testing, 100 changes often, 1000 slow changes
#endif

int pattern[patternsize];
int ledtolight[8];
int twinkletime[numpins];

const int waitpd = 20;

Adafruit_WS2801 strip = Adafruit_WS2801(numpins, dataPin, clockPin);

int choice=0;

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  for(int i = 0; i<16; i++)
  {
    pattern[i] = map(i,0,15,0,255);
  }
  for(int i = 16; i<32; i++)
  {
    pattern[i] = map(i,16,31,255,0);
  }
  for(int i = 0; i<numpins; i++)
  {
    twinkletime[i] = 0;
  }

  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();
  randomSeed(analogRead(0));  
  
  // Randomize order of ledtolight

  for(int i = 0; i <8; i++)
    ledtolight[i] = i;
  
  for(int i = 0; i <32; i++)
  {
    int x = random(8);
    int y;
    do
    {  
      y = random(8);
    } while(y == x);
    
    int t = ledtolight[x];
    ledtolight[x] = ledtolight[y];
    ledtolight[y] = t;
  }
  
}

// lednum - the LED to illuminate (0-49)
// cyslenum - point in cycle. Currently cysle is 0-127 over and over

uint32_t cf_rg(int lednum, int cyclenum)
{
  int n = (lednum % 2);
  int p = ((n*16) + cyclenum) % 32; 
  int r = pattern[p];
  if(n == 0)
    return Color(r,0,0);
  else  
    return Color(0,r,0);
}

uint32_t cf_rgb(int lednum, int cyclenum)
{
  int n = (lednum % 4);
  int p = ((n*8) + cyclenum) % 32; 
  int r;
  if((n == 2) && (p > 0)) // use dimmmer value for n=2, since 2 colours lit
    p -= 1;
  r = pattern[p];
  
  switch(n){
    case 0: return Color(r,0,0);
    case 1: return Color(0,r,0);
    case 2: return Color(r ,r , 0);
    default: return Color(0,0,r);
  }
}

uint32_t cf_rgchase(int lednum, int cyclenum)
{
  int chaseoffset = cyclenum / 8;
  int colourindex = ((chaseoffset + lednum) % 4) / 2;
  
  if(colourindex == 0)
    return 0xff0000; //  Color(255,0,0);
  else  
    return 0x00ff00; // Color(0,255,0);
}

uint32_t cf_rgchaseleft(int lednum, int cyclenum)
{
 // int chaseoffset = ((4*32) - cyclenum) / 32;
  int cn = 3 - (cyclenum / 8); // [0,3]
  int colourindex = ((lednum + cn) % 4) / 2;
  
  if(colourindex == 0)
    return 0xff0000; //  Color(255,0,0);
  else  
    return 0x00ff00; // Color(0,255,0);
}

uint32_t cf_rgbchase(int lednum, int cyclenum)
{
  int chaseoffset = cyclenum / 8;
  int colourindex = ((chaseoffset + lednum) % 6) / 2;
  
  switch(colourindex){
    case 0: return 0xff0000;
    case 1: return 0x00ff00;
    default: return 0x0000ff;
  }
}


uint32_t cf_whitetwinkle(int lednum, int cyclenum)
{
  
  // pairs are offset in phase (32/2 = 16)
  
  int r = pattern[(((lednum % 2)*16) + cyclenum) % 32];
  return Color(r,r,r);
}

uint32_t cf_rgtwinkle(int lednum, int cyclenum)
{
  int nmod = lednum % 2;
  int r = pattern[((nmod*16) + cyclenum) % 32];
  if(nmod == 0)
    return Color(r,0,0);
  else  
    return Color(0,r,0);
}


uint32_t cf_colourwheel(int lednum, int cyclenum)
{
// // int cn = cyclenum / 8;
  int ledindex = (lednum + (cyclenum >> 3)) % 8;
  return Wheel((ledindex * 256) / (numpins / 4));
  
  
}

uint32_t cf_twinklerandomrgb(int lednum, int cyclenum)
{
  if(lednum < 2)
  {
    int tpin = random(numpins);
    if(twinkletime[tpin] <= 0)
      twinkletime[tpin] = 16;
  }
    
  int t = twinkletime[lednum]; 
  if(t > 0)
  {
    twinkletime[lednum] = t-1;
    switch(lednum % 3){
      case 0: return 0xff0000;
      case 1: return 0x00ff00;
      default: return 0x0000ff;
    }
  }
  else
  {
    return(0);
  }
}

uint32_t cf_twinklerandomwhite(int lednum, int cyclenum)
{
  if(lednum < 2)
  {
    int tpin = random(numpins);
    if(twinkletime[tpin] <= 0)
      twinkletime[tpin] = 16;
  }
    
  int t = twinkletime[lednum]; 
  if(t > 0)
  {
    twinkletime[lednum] = t-1;
    return 0xffffff;
  }
  else
  {
    return(0);
  }
}

// FLASHING - VEY ANNOYING
//uint32_t cf_flkr1(int lednum, int cyclenum)
//{
// // int cn = cyclenum / 8;
//  int ledindex = lednum % 8;
//  int currentindextolight = ledtolight[cyclenum % 8];
//
//  if(ledindex == currentindextolight)  
//    return 0xffffff;
//  else  
//    return 0;
//}
//uint32_t cyclecolourwhite(int n, int k)
//{
//  int r = pattern[k];
//  return Color(r,r,r);
//}
//
//uint32_t cyclecolourgreen(int n, int k)
//{
//  int r = pattern[k];
//  return Color(0,r,0);
//}
//
//uint32_t cyclecolourred(int n, int k)
//{
//  int r = pattern[k];
//  return Color(r,0,0);
//}
//
//uint32_t cyclecolourrgbg(int n, int k)
//{
//  int r = pattern[k];
//  
// switch (n % 3) {
//    case 0:
//      return Color(r,0,0);
//      break;
//    case 1:
//      return Color(0,r,0);
//      break;
//    default: 
//      return Color(0,0,r);
//  }
//}
//

void doacycle(uint32_t (* functionptr)(int, int))
{
  for(int rep = 0; rep <repcount; rep++)
  {
    
    for(int cyclenum = 0; cyclenum<(32*4); cyclenum++)
    {
      for(int lednum = 0; lednum < numpins; lednum++)
      {
        strip.setPixelColor(lednum, functionptr(lednum, cyclenum));
      }  
      strip.show();
      delay(waitpd);  
    }
  }
}

void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 256; j++) {     // 3 cycles of all 256 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 255));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, 7);
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


const int numchoices = 10;

void loop() {
  switch (choice) {
    case 0:
      doacycle(cf_twinklerandomrgb); 
      break;
    case 1:
      doacycle(cf_rgb); 
      break;
    case 2:
      doacycle(cf_rgchase); 
      break;
    case 3:
      doacycle(cf_rgbchase); 
      break;
    case 4:
      doacycle(cf_twinklerandomwhite); 
      break;
    case 5:
      doacycle(cf_colourwheel); 
      break;
    case 6:
      doacycle(cf_rgchaseleft); 
      break;

    case 7:
      doacycle(cf_whitetwinkle); 
      break;

    case 8:
      doacycle(cf_rgtwinkle); 
      break;
      
    default:
      doacycle(cf_rg); 
      break;
  }
  
  #ifdef debug
    choice = (choice + 1) % numchoices;
  #else
    choice = random(numchoices); 
  #endif
  
//  switch (random(10)) {
//    case 0:
//      doacycle(cyclecolourrrggsteady);  // steady
//      break;
//    case 1:
//      doacycle(cyclecolourrgsteady);  // steady
//      break;
//    case 2:
//      doacycle(cyclecolourrgbgsteady);  // steady
//      break;
//    case 3:
//      doacycle(cyclecolourrrgg);
//      break;
//    case 4:
//      doacycle(cyclecolourred);
//      break;
//    case 5:
//      doacycle(cyclecolourrg);
//      break;
//    case 6:
//      doacycle(cyclecolourgreen);
//      break;
//    case 7:
//      doacycle(cyclecolourwheel);  // steady
//      break;
//    case 8:
//      doacycle(cyclecolourwhite);
//      break;
//    default:  // 9
//      doacycle(cyclecolourrgbg);
//  }

  
}


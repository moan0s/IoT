// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    32

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 28

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  Serial.begin(115200);
  Serial.println("Setup");

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)
  Serial.println("Setup ended");
}


// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  Serial.println("Loop");
  // Fill along the length of the strip in various colors...
  colorWipe(strip.Color(255,   0,   0), 50); // Red
  colorWipe(strip.Color(  0, 255,   0), 50); // Green
  colorWipe(strip.Color(  0,   0, 255), 50); // Blue

  for (int i=0; i<24; i++) {
    light_for_scale(i/2+16, 16.0, 24.0, 8, 0);
    delay(500);
  }
  for (int i=0; i<24; i++) {
    light_for_scale(i/2+16, 16.0, 24.0, 8, 8);
    delay(500);
  }
  for (int i=0; i<24; i++) {
    light_for_scale(i/2+16, 16.0, 24.0, 12, 16);
    delay(500);
  }

}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void light_for_scale(float val, float lower_scale_val, float higher_scale_val, int number_leds, int offset) {
  //Takes a value and illuminates a NeoPixel ring to display this value.
  // Values in the lower third are displayed red, middle orange, and upper third green
  
  float val_range = higher_scale_val-lower_scale_val;

  float fraction = (val-lower_scale_val)/val_range;
  if (fraction > 1) {
    fraction = 1;
  }

  int red = (int) 255-(fraction*255.0);
  int green = (int) round(fraction*255.0);
  int blue = 0;

  int number_of_leds_to_turn_on = round(fraction*number_leds);
  

  for (int i = 0; i<number_of_leds_to_turn_on; i++) {
    strip.setPixelColor(i+offset, strip.Color(red, green, blue));         //  Set pixel's color (in RAM)
  }
  strip.show();
  
}

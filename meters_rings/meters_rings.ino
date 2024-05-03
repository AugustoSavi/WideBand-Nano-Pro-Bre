/*
 An example showing 'ring' analogue meters on a 2.2" ILI9341 TFT 240x320
 colour screen
 
 The meter graphic function works best with slowly changing values without
 large rapid changes such as environmental parameters (temperature etc)

 This example uses the hardware SPI and a board based on the ATmega328
 such as an UNO.  If using a Mega or outher micocontroller then comment out
 the following line:
 #define F_AS_T
 in  the "Adafruit_FAST.h" found within the Adafruit_AS
 library folder. Change pins for other Arduinos to the hardware SPI pins.

 Needs Fonts 2, and 4 (also Font 6 if using a large size meter)

 Alan Senior 18/3/2015
 */

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#define DARK_GREY 0x2104 // Dark grey 16 bit colour
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0xCCCCCC

uint32_t runTime = -99999;       // time for next update

int reading = 0; // Value to be displayed
int d = 0; // Variable used for the sinewave test waveform

SoftwareSerial bluetooth(0, 1); // RX, TX

void setup(void) {
  Serial.begin(9600); // Inicia a comunicação serial com o computador
  bluetooth.begin(9600); // Inicia a comunicação serial com o módulo Bluetooth
  uint16_t ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9486; // write-only shield
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
}


void loop() {
  if (bluetooth.available()) { // Verifica se há dados disponíveis para leitura
    char receivedChar = bluetooth.read(); // Lê o caractere recebido do Bluetooth
    Serial.print(receivedChar); // Imprime o caractere recebido no monitor serial
  }

    // reading = 1;
  // if (millis() - runTime >= 50L) { // Execute every 1s
  //   runTime = millis();

    // Test with a slowly changing value from a Sine function
    d += 5; if (d >= 360) d = 0;

    // Set the the position, gap between meters, and inner radius of the meters
    int xpos = 15, ypos = 15, gap = 30, radius = 65;

    // Draw meter and get back x position of next meter
    // Test with Sine wave function, normally reading will be from a sensor
    reading = 250 + 250 * sineWave(d+0);
    xpos = gap + ringMeter(reading, 0, 500, xpos, ypos, radius, "mA", GREEN2RED); // Draw analogue meter

    reading = 20 + 30 * sineWave(d+60);
    xpos = gap + ringMeter(reading, -10, 50, xpos, ypos, radius, "degC", BLUE2RED); // Draw analogue meter

    reading = 50 + 50 * sineWave(d + 120);
    ringMeter(reading, 0, 100, xpos, ypos, radius, "%RH", BLUE2BLUE); // Draw analogue meter

    // Draw two more larger meters
    xpos = 30, ypos = 145, gap = 60, radius = 90;

    reading = 100 + 150 * sineWave(d + 90);
    xpos = gap + ringMeter(reading, 0, 1000, xpos, ypos, radius, "mb", BLUE2RED); // Draw analogue meter

    reading = 15 + 15 * sineWave(d + 150);
    xpos = gap + ringMeter(reading, 0, 30, xpos, ypos, radius, "Volts", GREEN2GREEN); // Draw analogue meter

    // Draw a large meter
    // xpos = 40, ypos = 5, gap = 15, radius = 120;
    // reading = 175;
    // Comment out above meters, then uncomment the next line to show large meter
    //ringMeter(reading,0,200, xpos,ypos,radius," Watts",GREEN2RED); // Draw analogue meter

  // }
}


// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 4;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 5; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = RED; break; // Fixed colour
      case 1: colour = GREEN; break; // Fixed colour
      case 2: colour = BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  byte len = 0; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);

  // Set the text colour to default
  // tft.setTextColor(WHITE, BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  // tft.setTextColor(text_colour, BLACK);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 85) showmsgXY(buf, x, y - 20, r, 5); // Value in middle
  else showmsgXY(buf, x, y - 20, r, 4); // Value in middle

  // Print units, if the meter is large then use big font 4, othewise use 2
  // tft.setTextColor(WHITE, BLACK);
  if (r > 85) showmsgXY(units, x, y + 30, r, 3); // Units display
  else showmsgXY(units, x, y + 20, r, 2); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}


void showmsgXY(const char *msg, int x, int y, int r, int sz)
{
  tft.setTextSize(sz);

  // Calcula o comprimento da mensagem
  int msgLength = strlen(msg);

  // Calcula a largura total do texto
  int textWidth = msgLength * 6 * sz; // Assumindo que cada caractere tem aproximadamente 6 pixels de largura
  
  // Calcula a posição central do texto com base no raio do anel
  int centerX = x - textWidth / 2;

  // Se a posição central do texto ultrapassar os limites do anel, ajusta para que fique dentro do anel
  if (centerX < x - r)
    centerX = x - r;
  else if (centerX + textWidth > x + r)
    centerX = x + r - textWidth;

  // Define a cor do texto
  tft.setTextColor(WHITE, BLACK);

  // Define a posição do cursor para o centro do texto
  tft.setCursor(centerX, y);

  // Imprime a mensagem
  tft.print(msg);
}


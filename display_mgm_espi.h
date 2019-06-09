// v.00 first version

// Display routines

// SPI library, built into IDE
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip v5
#include <SPI.h>
/*#define FS_NO_GLOBALS
#include <FS.h>
// JPEG decoder library
#include <JPEGDecoder.h>*/

// Invoke TFT library
TFT_eSPI tft = TFT_eSPI();

#define dim_testo 0

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5
#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN         0xBFF7
#define LTCYAN    0xC7FF
#define LTRED           0xFD34
#define LTMAGENTA       0xFD5F
#define LTYELLOW        0xFFF8
#define LTORANGE        0xFE73
#define LTPINK          0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY          0xE71C
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define DKBLUE        0x000D
#define DKTEAL    0x020C
#define DKGREEN       0x03E0
#define DKCYAN        0x03EF
#define DKRED         0x6000
#define DKMAGENTA       0x8008
#define DKYELLOW        0x8400
#define DKORANGE        0x8200
#define DKPINK          0x9009
#define DKPURPLE      0x4010
#define DKGREY        0x4A49
#define ORANGE        0xFD20
#define PINK          0xF81F
#define PURPLE    0x801F

#define TFT_GREY 0x5AEB
#define ILI9341_GREY 0x2104 // Dark grey 16 bit colour
#define ILI9341_RED RED
#define ILI9341_GREEN GREEN
#define ILI9341_BLUE BLUE
#define ILI9341_WHITE WHITE
#define ILI9341_BLACK BLACK

uint32_t runTime = -99999;       // time for next update
uint32_t runTime1 = -99999;

boolean graph_1 = true;
boolean graph_2 = true;
boolean graph_3 = true;
boolean graph_4 = true;
boolean graph_5 = true;
boolean graph_6 = true;
boolean graph_7 = true;
boolean redraw1 = true;
double ox , oy ;
double ox2 , oy2 ;
double x, y;
double a1, b1, c1, d1, r2, r1, vo, tempC, tempF, tempK;
byte p_len =4 ;

int reading = 0; // Value to be displayed - ring meter
int scan = 1;

// --------------------------------------------------------
// SETUP ROUTINES FOR DISPLAY
// --------------------------------------------------------
// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)

  // converts the dBm to a range between 0 and 100%
  int8_t getWifiQuality() 
  {
    int32_t dbm = WiFi.RSSI();
    if(dbm <= -100) 
    {
        return 0;
    } else if(dbm >= -50) 
    {
        return 100;
    } else 
    {
        return 2 * (dbm + 100);
    }
  }

  String twoDigits(int digits)
  {
    if(digits < 10) 
    {
      String i = '0'+String(digits);
      return i;
    }
    else 
    {
      return String(digits);
    }
  }
  
  void drawWifiQuality() 
  {
    tft.fillRect(265,0,320,8,BLACK);
    
    int8_t quality = getWifiQuality();
    tft.setTextSize(0); 
    tft.setCursor(280,0,1); 
    tft.setTextColor(TFT_WHITE);
    tft.print(String(quality) + "%"); //  wifi signal quality
    
    tft.setTextColor(TFT_WHITE);
    #ifdef MQTT_Y
    if (collegato_MQTT == 1)
     { 
      tft.setCursor(265,0);
      tft.print("Q");
     }
     else
     {
      tft.setCursor(265,0);
      tft.print("X");
     }
    #endif
    for (int8_t i = 0; i < 4; i++) 
    {
      for (int8_t j = 0; j < 2 * (i + 1); j++) 
      {
        if (quality > i * 25 || j == 0) {
          tft.drawPixel(305 + 2 * i, 6 - j, TFT_WHITE);
        }
      }
    }
    tft.setTextSize(dim_testo);
  }

  void scrivi_ora()
  {
     tft.setTextSize(0);
     tft.setCursor(128,0,1); //128,0
     tft.setTextColor(TFT_WHITE);
     tft.print(String(day(local))+" "+String(dayShortStr(local))+" "+String(monthShortStr(local))
       +" "+twoDigits(hour(local)) + ":" + twoDigits(minute(local)));
     tft.print(twoDigits(hour(local)) + ":" + twoDigits(minute(local)));
     tft.setTextSize(dim_testo);
  }

  void spegni_backlight()
  {
    tft.fillScreen(TFT_BLACK);
    pinMode(D8, OUTPUT);    //PWM on A0 for backlight
    digitalWrite(D8,0);
  }
  
  void accendi_backlight()
  {
    pinMode(D8, OUTPUT);    //PWM on A0 for backlight
    digitalWrite(D8,1);
  }

  
  void scrivi_display_riga_colore (String line1, int riga, int colore) // 8, 16, 24
  {  /* ST7735_RED, ST7735_BLUE, ST7735_GREEN, ST7735_WHITE */
     tft.setTextSize(1);
     tft.setTextColor(colore);
     tft.setCursor(0,riga,4); // This is the upper most left pixel on the OLED shield in Hackerbox #023
     tft.print(line1);
     tft.setTextSize(dim_testo);
  }
  
  void cancella_display()
  {
    tft.setTextWrap(true); // Allow text to run off right edge
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 0);
    tft.setTextColor(TFT_WHITE);
    drawWifiQuality();
    tft.setTextSize(dim_testo);
    //scrivi_ora();
  }
  
  void setup_display()
   {
    //tft.initR(INITR_BLACKTAB);
    tft.init();
    tft.begin();
    
    tft.setTextWrap(false); // Allow text to run off right edge
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(dim_testo);
    tft.setTextColor(TFT_WHITE);
    tft.setTextFont(2);
    tft.setRotation(1); //tft.getRotation()+3)
    //drawJpeg("/trenordlogo160.jpg", 0, 10);
    //drawJpeg("/fnm-treno.jpg", 0, 50);
   }
  
  void scrivi_display(String line1, String line2, String line3)
  {
     tft.setTextColor(TFT_WHITE); //BLACK se arcobaleno
     //tft.setTextSize(2);
     // Text to display on the screen.
     tft.setCursor(0,15,4); // This is the upper most left pixel on the OLED shield in Hackerbox #023
     tft.println(line1);
     tft.setCursor(0,43,4); // This starts the 2nd line of text.  Remember, spacing on OLED screens are by pixels and not by characters.
     tft.println(line2);
     tft.setCursor(0,71,4); // This starts the 2nd line of text.  Remember, spacing on OLED screens are by pixels and not by characters.
     tft.println(line3);
     tft.setTextSize(dim_testo);
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

String Format(double val, int dec, int dig ) {
  int addpad = 0;
  char sbuf[20];
  String condata = (dtostrf(val, dec, dig, sbuf));
  int slen = condata.length();
  for ( addpad = 1; addpad <= dec + dig - slen; addpad++) {
    condata = " " + condata;
  }
  return (condata);
}


// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(long int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme,int sp, int segm, int wid)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  x += r; y += r;   // Calculate coords of centre of ring
  int w = r / wid;    // Width of outer ring is 1/4 of radius
  sp = sp/2;
  int angle = sp;  // Half the sweep angle of meter (300 degrees)
  int text_colour = 0; // To hold the text colour
  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v
  byte seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 5*segm; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {
    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = ILI9341_RED; break; // Fixed colour
      case 1: colour = ILI9341_GREEN; break; // Fixed colour
      case 2: colour = ILI9341_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = ILI9341_BLUE; break; // Fixed colour
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
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);
  //itoa(value, buf, len);
  tft.setTextColor(text_colour, ILI9341_BLACK);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 84)
  {  /* drawCentreString*/
    tft.setTextColor(text_colour, ILI9341_BLACK);
    tft.drawCentreString(buf, x - 5, y - 20, 4);
  }
  else
  {
    tft.fillRect(x-12, y-10, 32, 13, TFT_BLACK); //delete previous number
    tft.setTextColor(text_colour, TFT_BLACK);
    tft.drawCentreString(buf,x, y - 10, 2);
  } // Value in middle
  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  if (r > 84) tft.drawCentreString(units, x, y + 30, 2); // Units display
  else tft.drawCentreString(units, x, y + 8, 1); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

void DrawBarChartV(TFT_eSPI & d, long int x , long int y , long int w, long int h , long int loval , long int hival , long int inc , long int curval ,  int dig , int dec, unsigned int barcolor, unsigned int voidcolor, unsigned int bordercolor, unsigned int textcolor, unsigned int backcolor, String label, boolean & redraw)
{
  double stepval, range;
  double my, level;
  double i, data;
  // draw the border, scale, and label once
  // avoid doing this on every update to minimize flicker
  if (redraw == true) 
  {
    redraw = false;
    d.drawRect(x - 1, y - h - 1, w + 2, h + 2, bordercolor);
    d.setTextColor(textcolor, backcolor);
    //d.setTextSize(2);
    //d.setCursor(x , y + 10);
    d.drawCentreString(label,x , y + 10,1);
    // step val basically scales the hival and low val to the height
    // deducting a small value to eliminate round off errors
    // this val may need to be adjusted
    stepval = ( inc) * (double (h) / (double (hival - loval))) - .001;
    for (i = 0; i <= h; i += stepval) 
    {
      my =  y - h + i;
      d.drawFastHLine(x + w + 1, my,  5, textcolor);
      // draw lables
      //  d.setTextSize(1);
      d.setTextColor(textcolor, backcolor);
      //d.setCursor(x + w + 12, my - 3 );
      data = hival - ( i * (inc / stepval));
      d.drawCentreString(Format(data, dig, dec), x + w + 12, my - 3, 1);
    }
  }
  // compute level of bar graph that is scaled to the  height and the hi and low vals
  // this is needed to accompdate for +/- range
  level = (h * (((curval - loval) / (hival - loval))));
  // draw the bar graph
  // write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
  d.fillRect(x, y - h, w, h - level,  voidcolor);
  d.fillRect(x, y - level, w,  level, barcolor);
  // write the current value
  d.setTextColor(textcolor, backcolor);
  //d.setTextSize(2);
  //d.setCursor(x , y - h - 15); //23
  d.drawCentreString(Format(curval, dig, dec), x , y - h - 15, 1);
}

/*
  This method will draw a horizontal bar graph for single input
  it has a rather large arguement list and is as follows

  &d = display object name
  x = position of bar graph (upper left of bar)
  y = position of bar (upper left of bar (add some vale to leave room for label)
  w = width of bar graph (does not need to be the same as the max scale)
  h =  height of bar graph
  loval = lower value of the scale (can be negative)
  hival = upper value of the scale
  inc = scale division between loval and hival
  curval = date to graph (must be between loval and hival)
  dig = format control to set number of digits to display (not includeing the decimal)
  dec = format control to set number of decimals to display (not includeing the decimal)
  barcolor = color of bar graph
  voidcolor = color of bar graph background
  bordercolor = color of the border of the graph
  textcolor = color of the text
  back color = color of the bar graph's background
  label = bottom lable text for the graph
  redraw = flag to redraw display only on first pass (to reduce flickering)
*/
void DrawBarChartH(TFT_eSPI & d, double x , double y , double w, double h , double loval , double hival , double inc , double curval ,  int dig , int dec, unsigned int barcolor, unsigned int voidcolor, unsigned int bordercolor, unsigned int textcolor, unsigned int backcolor, String label, boolean & redraw)
{
  double stepval, range;
  double mx, level;
  double i, data;

  // draw the border, scale, and label once
  // avoid doing this on every update to minimize flicker
  // draw the border and scale
  if (redraw == true) {
    redraw = false;
    d.drawRect(x , y , w, h, bordercolor);
    d.setTextColor(textcolor, backcolor);
   // d.setFontScale(1);
    //d.setCursor(x , y - 20);
    d.drawCentreString(label, x , y - 20, 1);
    // step val basically scales the hival and low val to the width
    stepval =  inc * (double (w) / (double (hival - loval))) - .00001;
    // draw the text
    for (i = 0; i <= w; i += stepval) {
      d.drawFastVLine(i + x , y + h + 1,  5, textcolor);
      // draw lables
     // d.setFontScale(0);
      d.setTextColor(textcolor, backcolor);
      //d.setCursor(i + x , y + h + 10);
      // addling a small value to eliminate round off errors
      // this val may need to be adjusted
      data =  ( i * (inc / stepval)) + loval + 0.00001;
      d.drawCentreString(Format(data, dig, dec), i + x , y + h + 10, 1);
    }
  }
  // compute level of bar graph that is scaled to the width and the hi and low vals
  // this is needed to accompdate for +/- range capability
  // draw the bar graph
  // write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
  level = (w * (((curval - loval) / (hival - loval))));
  d.fillRect(x + level + 1, y + 1, w - level - 2, h - 2,  voidcolor);
  d.fillRect(x + 1, y + 1 , level - 1,  h - 2, barcolor);
  // write the current value
  d.setTextColor(textcolor, backcolor);
 // d.setFontScale(1);
  //d.setCursor(x + w + 10 , y + 5);
  d.drawCentreString(Format(curval, dig, dec), x + w + 20 , y + 5, 1);
  //print ms
  d.drawCentreString("ms", x + w + 23 , y + 15, 1);
}

/*
  function to draw a cartesian coordinate system and plot whatever data you want
  just pass x and y and the graph will be drawn

  huge arguement list
  &d name of your display object
  x = x data point
  y = y datapont
  gx = x graph location (lower left)
  gy = y graph location (lower left)
  w = width of graph
  h = height of graph
  xlo = lower bound of x axis
  xhi = upper bound of x asis
  xinc = division of x axis (distance not count)
  ylo = lower bound of y axis
  yhi = upper bound of y asis
  yinc = division of y axis (distance not count)
  title = title of graph
  xlabel = x asis label
  ylabel = y asis label
  &redraw = flag to redraw graph on first call only
  color = plotted trace colour
*/
void Graph(TFT_eSPI &d, long int x, long int y, int gx, int gy, int w, int h, int xlo, long int xhi, int xinc, int ylo, long int yhi, int yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor, boolean &redraw) 
{
//if (redraw1==true){redraw=true;}
  double ydiv, xdiv;
  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  //static double ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  //static double oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  int i;
  double temp;
  int rot, newrot;
  //ymax draw text
  d.setTextSize(0); //label
  d.setTextColor(pcolor, bcolor);
  d.setCursor(gx, gy-h-15);
  d.println("Max download "+String(yhi)+" kb");

  if (redraw == true) {
    redraw = false;
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    ox = ox + 2;//shift axis x so graph is not over the other graph
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
    // draw y scale
    /*for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
      if (i == 0) {
        d.drawLine(gx, temp, gx + w, temp, acolor);
      }
      else {
        d.drawLine(gx, temp, gx + w, temp, gcolor);
      }
      d.setTextSize(0);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(gx - 40, temp);
      // precision is default Arduino--this could really use some format control
      int strip =i;
      d.println(strip);
    }
    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {
      // compute the transform
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        d.drawLine(temp, gy, temp, gy - h, acolor);
      }
      else {
        d.drawLine(temp, gy, temp, gy - h, gcolor);
      }
      d.setTextSize(0);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(temp, gy + 10);
      // precision is default Arduino--this could really use some format control
     //int strip =(i/.66666666666666666666);
     int strip=(i/2)/3600;
      d.println(strip);
    }
    /*
    d.setTextSize(0);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(gx , gy - h - 30);
    d.println(title);
    d.setTextSize(0);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx , gy + 20);
    d.println(xlabel);
    d.setTextSize(0);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx - 30, gy - h - 10);
    d.println(ylabel);*/
  }
  //graph drawn now plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized as static above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  d.drawLine(ox, oy, x, y, pcolor);
  d.drawLine(ox, oy + 1, x, y + 1, pcolor);
  d.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;
}

void Graph2(TFT_eSPI &d, long int x, long int y, int gx, int gy, int w, int h, int xlo, long int xhi, int xinc, int ylo, long int yhi, int yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor, boolean &redraw) 
{
//if (redraw1==true){redraw=true;}
  double ydiv, xdiv;
  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  //static double ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  //static double oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  int i;
  double temp;
  int rot, newrot;
  //ymax draw text
  d.setTextSize(0); //label
  d.setTextColor(pcolor, bcolor);
  d.setCursor(gx, gy+5);
  d.println("Max upload " + String(yhi)+" kb");

  if (redraw == true) {
    redraw = false;
    ox2 = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy2 = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
    // draw y scale
    /*for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
      if (i == 0) {
        d.drawLine(gx, temp, gx + w, temp, acolor);
      }
      else {
        d.drawLine(gx, temp, gx + w, temp, gcolor);
      }
      d.setTextSize(0);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(gx - 40, temp);
      // precision is default Arduino--this could really use some format control
      int strip =i;
      d.println(strip);
    }
    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {
      // compute the transform
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        d.drawLine(temp, gy, temp, gy - h, acolor);
      }
      else {
        d.drawLine(temp, gy, temp, gy - h, gcolor);
      }
      d.setTextSize(0);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(temp, gy + 10);
      // precision is default Arduino--this could really use some format control
     //int strip =(i/.66666666666666666666);
     int strip=(i/2)/3600;
      d.println(strip);
    }
    //now draw the labels
    d.setTextSize(0);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(gx , gy - h - 30);
    d.println(title);
    d.setTextSize(0);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx , gy + 20);
    d.println(xlabel);
    d.setTextSize(0);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx - 30, gy - h - 10);
    d.println(ylabel);*/
  }
  //graph drawn now plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized as static above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  d.drawLine(ox2, oy2, x, y, pcolor);
  d.drawLine(ox2, oy2 + 1, x, y + 1, pcolor);
  d.drawLine(ox2, oy2 - 1, x, y - 1, pcolor);
  ox2 = x;
  oy2 = y;
}

//procedure to paint screen indicators ======================================================================================
void disegna(long int received_speed, long int sent_speed, int avg_time_ms)
{
  scan++; // write point for x on graph
  x=scan; //stepping function for graphs X axis 
  double Tinc = 2, Time = 20;
  Time =(Time *2) ;//timedelay compensation 
  double gx = 1, gy = 185, w = 210, h = 170;

  if (millis() - runTime >= (Time*5000))
  {
    runTime=millis();
    //tft.fillRect(0,gy-h,gx+w,gy,BLACK);
    tft.fillRect(0,0,320,240,BLACK);
    scan=1; ///resets X axis
    x=scan;
    max_speed_r=300; max_speed_s = 100;
    graph_3=true;  /// add graphs affected by resetting screen
    graph_1=true;  // radial meters are un affected 
    graph_2=true;
    graph_4=true;
  }
  drawWifiQuality();

  //void Graph(TFT_eSPI &d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, 
  //  double xinc, double ylo, double yhi, double yinc, String title, String xlabel, String ylabel, 
  //  unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor, 
  //  boolean &redraw) 
  // ----------------------------- download graph
  y = received_speed; /// Y axis
  Graph(tft, x, y, gx, gy, w, h, 0, Time, Tinc, 0, max_speed_r, 1, "kbytes", "sec", "Down (Y) / Up (B)", BLACK, RED, YELLOW, WHITE, BLACK, graph_3);
  y = sent_speed; /// Y axis
  // ----------------------------- upload graph
  Graph2(tft, x, y, gx, gy, w, h, 0, Time, Tinc, 0, max_speed_s, 1, "kbytes", "sec", "", BLACK, RED, BLUE, WHITE, BLACK, graph_4);
  
  //RADIAL METERS FOR DOWNLOAD AND UPLOAD =============================================================================================
  int xpos, ypos, gap, radius, sweep, segm, wid;
  xpos = 226, ypos = 20, gap = 1, radius = 44, sweep = 260, segm = 2, wid =3 ; 
  ringMeter(received_speed, 0, max_download_rate, xpos, ypos, radius, "kb DOWN", GREEN2RED, sweep, segm, wid); // Draw analogue meter
  ringMeter(sent_speed, 0, max_upload_rate, xpos, ypos+88, radius, "kb UP", GREEN2RED, sweep, segm, wid); // Draw analogue meter

  /*////////////horizontal bar///////////////==========================================================================================*/
  if (avg_time_ms>100) avg_time_ms = 99;
  if (avg_time_ms<0) avg_time_ms = 0;
  DrawBarChartH(tft, 10, 200, 270, 15, 0, 100, 10, avg_time_ms, 3, 0, RED, BLACK, WHITE, WHITE, BLACK, "", graph_2);

}
  

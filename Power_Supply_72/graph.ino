////////////////////////////////////////////////////////////////////////////////
// 320(y) * 480(x)


#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49


// these are the only external variables used by the graph function
// it's a flag to draw the coordinate system only on the first call to the Graph() function
// and will mimize flicker
// also create some variables to store the old x and y, if you draw 2 graphs on the same display
// you will need to store ox and oy per each display


double ox = -999, oy = -999; // Force them to be off screen


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

#define SHIFT_X -40

void Trace(TFT_HX8357 &tft, double x,  double y,  byte dp,
           double gx, double gy,
           double w, double h,
           double xlo, double xhi, double xinc,
           double ylo, double yhi, double yinc,
           boolean &update1, unsigned int color, unsigned int shift_y)
{
  double ydiv, xdiv;
  double i;
  double temp;
  int rot, newrot;

  unsigned int gcolor = DKBLUE;   // gcolor = graph grid color
  unsigned int acolor = RED;        // acolor = main axes and label color
  unsigned int pcolor = color;      // pcolor = color of your plotted data
  unsigned int tcolor = WHITE;      // tcolor = text color
  unsigned int bcolor = BLACK;      // bcolor = background color

  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  if (update1) {
    update1 = false;

    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    if ((ox < gx) || (ox > gx + w)) {
      update1 = true;
      return;
    }
    if ((oy < gy - h) || (oy > gy)) {
      update1 = true;
      return;
    }


    tft.setTextDatum(MR_DATUM);
    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      //if (i == 0) {
      // tft.setTextColor(acolor, bcolor);
      // tft.drawString(xlabel, (int)(gx + w) , (int)temp, 2);     // label satırı yoooook
      //tft.drawLine(gx+SHIFT_X, temp+shift_y, gx+SHIFT_X + w, temp+shift_y, acolor);
      // }
      tft.drawLine(gx + SHIFT_X, temp + shift_y, gx + w + SHIFT_X, temp + shift_y, gcolor);
      // draw the axis labels
      tft.setTextColor(tcolor, bcolor);
      // precision is default Arduino--this could really use some format control
      // tft.drawNumber(i, gx-4+SHIFT_X, temp+shift_y, 1);
      tft.drawFloat(i, dp, gx - 4 + SHIFT_X, temp + shift_y, 1);
    }
    //tft.setTextDatum(TC_DATUM);
    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {

      // compute the transform
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      //      if (i == 0) {
      //        tft.setTextColor(acolor, bcolor);         // label satırı yoooook
      //        tft.setTextDatum(BC_DATUM);
      //        tft.drawString(ylabel, (int)temp, (int)(gy - h - 8) , 2);
      //      }
      tft.drawLine(temp + SHIFT_X, gy + shift_y, temp + SHIFT_X, gy - h + shift_y, gcolor);
      // draw the axis labels
      tft.setTextColor(tcolor, bcolor);
      // precision is default Arduino--this could really use some format control
      tft.drawNumber(i, temp + SHIFT_X, gy + 7 + shift_y, 1);
      //tft.drawFloat(i, dp, temp+SHIFT_X, gy + 7 + shift_y, 1);
    }

    //now draw the graph labels
    //tft.setTextColor(tcolor, bcolor);
    // tft.drawString(title, (int)(gx + w / 2) , (int)(gy - h - 30), 4);
  }

  // the coordinates are now drawn, plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

  if ((x < gx) || (x > gx + w)) {
    update1 = true;
    return;
  }
  if ((y < gy - h) || (y > gy)) {
    update1 = true;
    return;
  }


  tft.drawLine(ox + SHIFT_X, oy + shift_y, x + SHIFT_X, y + shift_y, pcolor);
  // it's up to you but drawing 2 more lines to give the graph some thickness
  //tft.drawLine(ox, oy + 1, x, y + 1, pcolor);
  //tft.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;

}

void Trace2(double x, double y,  byte dp,
            double ylo, double yhi, double yinc,
            boolean &update1, unsigned int color, unsigned int shift_y, double &ox, double &oy)
{
  double i;
  int temp;
  
  if (update1)
  {
    update1 = false;

    ox = x * 440 / 90 + 80;
    oy = (y - ylo) * (-90) / (yhi - ylo) + 80;

    tft.setTextDatum(MR_DATUM);
    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (- 90) / (yhi - ylo) + 80;

      tft.drawLine(80 + SHIFT_X, temp + shift_y, 520 + SHIFT_X, temp + shift_y, DKBLUE);
      // draw the axis labels
      tft.setTextColor(WHITE, BLACK);
      // precision is default Arduino--this could really use some format control
      // tft.drawNumber(i, gx-4+SHIFT_X, temp+shift_y, 1);
      tft.drawFloat(i, dp, 76 + SHIFT_X, temp + shift_y, 1);
    }

    // draw x scale
    for (i = 0; i <= 54; i += 3) {

      // compute the transform
      temp =  i * 440 / 54 + 80;
      //      if (i == 0) {
      //        tft.setTextColor(acolor, bcolor);         // label satırı yoooook
      //        tft.setTextDatum(BC_DATUM);
      //        tft.drawString(ylabel, (int)temp, (int)(gy - h - 8) , 2);
      //      }
      tft.drawLine(temp + SHIFT_X, 80 + shift_y, temp + SHIFT_X, shift_y -10, DKBLUE);
      tft.drawNumber(i, temp + SHIFT_X, 87 + shift_y, 1);
      //tft.drawFloat(i, dp, temp+SHIFT_X, gy + 7 + shift_y, 1);
    }    
  }

  x =  x * 440 / 90 + 80;
  y =  (y - ylo) * (-90) / (yhi - ylo) + 80;

  tft.drawLine(ox + SHIFT_X, oy + shift_y, x + SHIFT_X, y + shift_y, color);

  ox = x;
  oy = y;

}



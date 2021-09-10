#ifndef _CLAPPY_AVATOR_H_
#define _CLAPPY_AVATOR_H_

#include <M5Stack.h>

#define PRIMARY_COLOR WHITE
//#define SECONDARY_COLOR 0x6140
#define SECONDARY_COLOR BLACK

/*
#define BLACK    0x0000
#define NAVY      0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN    0x03EF
#define MAROON    0x7800
#define PURPLE    0x780F
#define OLIVE   0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE      0x001F
#define GREEN   0x07E0
#define CYAN      0x07FF
#define RED     0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE   0xFFFF
#define ORANGE    0xFD20
#define GREENYELLOW 0xAFE5
#define PINK      0xF81F
https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
*/

class Mouth
{
private:
  int x;
  int y;
  int minWidth;
  int maxWidth;
  int minHeight;
  int maxHeight;
  int lastX;
  int lastY;
  int lastY2;
  int lastW;
  int lastH;
  uint32_t primaryColor;
  uint32_t secondaryColor;
  void clear(void);
  void draw(int x1, int y1, int x2, int y2, int percent);
public:
  // constructor
  Mouth();
  Mouth(int x, int y,
  int minWidth, int maxWidth,
  int minHeight, int maxHeight,
  uint32_t primaryColor, uint32_t secondaryColor);

  void open(int percent);
};

class Eye
{
private:
  int x;
  int y;
  int r;
  int lastX;
  int lastY;
  int lastR;
  uint32_t primaryColor;
  uint32_t secondaryColor;
  void clear(void);
  void drawCircle(int x, int y, int r);
  void drawRect(int x, int y, int w, int h);
public:
  // constructor
  Eye();
  Eye(int x, int y, int r, uint32_t primaryColor, uint32_t secondaryColor);

  void open(boolean isOpen);
};

class Avator
{
private:
  Mouth mouth;
  Eye eyeR;
  Eye eyeL;
public:
  // constructor
  Avator(void);

  void openMouth(int percent);
  void openEye(boolean isOpen);
  void smile();
  void init();
};

#endif

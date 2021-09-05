#ifndef _CLAPPY_AVATOR_H_
#define _CLAPPY_AVATOR_H_

#include <M5Stack.h>

#define PRIMARY_COLOR WHITE
#define SECONDARY_COLOR RED

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

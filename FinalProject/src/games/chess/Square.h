#ifndef SQUARE_H
#define SQUARE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

class Square {
    private:
        int _x;
        int _y;

    public:
        Square(){
            _x = 0;
            _y = 0;
        }
        Square(int x, int y) {
            _x = x;
            _y = y;
        }

        int getX() { return _x; }
        int getY() { return _y; }
};


#endif
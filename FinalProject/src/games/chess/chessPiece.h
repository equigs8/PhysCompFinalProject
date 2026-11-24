#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include <Arduino.h>
#include "Square.h"
#include "PieceType.h"
//Class for chess pieces
class chessPiece {
    private:
    Square _location;
    int _color;
    PieceType _pieceType;
    const uint16_t* _sprite;

    public:
    chessPiece() {
        _sprite = nullptr;
    }
    chessPiece(Square location, int color, const uint16_t* sprite) {
        _location = location;
        _color = color;
        //_pieceType = pieceType;
        _sprite = sprite;
    }


    const uint16_t* getSprite(){
        return _sprite;
    }
};



#endif
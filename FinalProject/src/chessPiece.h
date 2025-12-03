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

    public:
    chessPiece(Square location, int color, PieceType pieceType) {
        _location = location;
        _color = color;
        _pieceType = pieceType;
    }
};



#endif
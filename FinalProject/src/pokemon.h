#ifndef POKEMON_H
#define POKEMON_H

#include <Arduino.h>
#include "move.h"


class Pokemon{
    private:
    const char * _name;
    int _health;
    int _maxHealth;
    int _level;
    Move _moveSet[4];


    public:
    Pokemon(){

    }
    Pokemon(const char * name, int maxHealth, int level, Move moves[]){
        _name = name;
        _maxHealth = maxHealth;
        _health = _maxHealth;
        _level = level;

        for (int i = 0; i < 4; i++) {
                _moveSet[i] = moves[i];
        }
    }

    const char * getName(){
        return _name;
    }

    int getHealth(){
        return _health;
    }

    int getLevel(){
        return _level;
    }

    int getMaxHealth(){
        return _maxHealth;
    }

    void takeDamage(int damage){
        _health -= damage;
    }
    
    void heal(int amount){
        _health += amount;
    }

    Move getMove(int index){
        if (index >= 0 && index < 4) {
                return _moveSet[index];
        }
        return Move();
    }
    Move getMoveByName(const char * name){
        for (int i = 0; i < 4; i++){
            if (strcmp(_moveSet[i].getName(), name) == 0){
                return _moveSet[i];
            }
        }
        return Move(0, "None", 0);
    }
};



#endif // POKEMON_H

#ifndef MOVE_H
#define MOVE_H


// A base class for pokemon moves
class Move {
    private:
        int _damage; // if negative it heals the pokemon that uses it
        const char *_name;
        int _pp; // Number of times the move can be used

    public:
    Move() {
        _damage = 0;
        _name = "None";
        _pp = 0;
    }

    Move(int damage, const char * name, int pp) {
        _damage = damage;
        _name = name;
        _pp = pp;
    }

    int getDamage() {
        return _damage;
    }

    const char * getName() {
        return _name;
    }

    int getPP() {
        return _pp;
    }
    
};



#endif
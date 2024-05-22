#include "types.h"

v2 operator+(v2 lhs, v2 rhs) {
    v2 res = { 0 };
    
    res.x = lhs.x + rhs.x;    
    res.x = lhs.x + rhs.x;
    
    return res;
}

v2 operator-(v2 lhs, v2 rhs) {
    v2 res = { 0 };
    
    res.x = lhs.x - rhs.x;    
    res.x = lhs.x - rhs.x;
    
    return res;
}

v2 operator+=(v2 lhs, v2 rhs) {    
    lhs.x += rhs.x;    
    lhs.x += rhs.x;
    
    return lhs;
}

v2 operator-=(v2 lhs, v2 rhs) {    
    lhs.x -= rhs.x;    
    lhs.x -= rhs.x;
    
    return lhs;
}
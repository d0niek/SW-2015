//
// Created by d0niek on 11/24/15.
//

#ifndef LICZNIK_VALUE_H
#define LICZNIK_VALUE_H

#include "pre_emptive_os/api/general.h"

struct Value
{
    tS32 current;
    tS32 last;

    Value() :
        current(0), last(0)
    {
    }

    void setLast()
    {
        this->last = this->current;
    }
};

#endif //LICZNIK_VALUE_H

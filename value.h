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
};

void setLast(struct Value *v);

#endif //LICZNIK_VALUE_H

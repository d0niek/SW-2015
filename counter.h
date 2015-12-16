//
// Created by d0niek on 11/25/15.
//

#ifndef LICZNIK_COUNTER_H
#define LICZNIK_COUNTER_H

#include "value.h"

#define GATE_A (1 << 17)
#define GATE_B (1 << 11)

void counter(struct Value *enters, struct Value *exits);

#endif //LICZNIK_COUNTER_H

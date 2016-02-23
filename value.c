//
// Created by d0niek on 12/02/15.
//

#include "value.h"

/*****************************************************************************
 *
 * Description:
 *    Set last value to current
 *
 * Params:
 *    [in] v
 *
 ****************************************************************************/
void setLast(struct Value *v)
{
	v->last = v->current;
}

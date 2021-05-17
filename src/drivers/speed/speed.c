/** @file
 *  @brief Source file for a (dummy) Speedometer driver.
 */

#include "speed.h"

#include <stdio.h>

static float curSpeed = 0.0;

void set_speed(float speed)
{
    curSpeed = speed;
}

/** @brief Returns the current speed
 *  @param[out] current speed
 */
float get_speed()
{
    printf("Speed: %f\n", curSpeed);
    return curSpeed;
}

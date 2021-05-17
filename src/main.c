/** @file
 *  @brief Main file.
 *  @description Just contains some example code. Adapt it in the way you like.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "drivers/adc_driver/adc_driver.h"
#include "drivers/error_led/error_led.h"
#include "drivers/speed/speed.h"

/** @brief Initializes slopes and constants of the straight lines defining the torques at various pedal angles for speeds 0 and 50km/hr.
 *  @param[in] t_0 Critical torque values for speed = 0
 *  @param[in] t_50 Critical torque values for speed = 50
 *  @param[in] angle_perc Percentage of pedal angle for which torque values are unique
 *  @param[out] m_0 Solpes of straight lines defining torque for speed = 0
 *  @param[out] m_50 Solpes of straight lines defining torque for speed = 50
 *  @param[out] c_0 Constants of straight lines defining torque for speed = 0
 *  @param[out] c_50 Constants of straight lines defining torque for speed = 50
 */
void init_torque_param(int size, int t_0[], int t_50[], int angle_perc[], float m_0[], float m_50[], float c_0[], float c_50[]);


/** @brief Calculates the percentage of pedal angle pressed
 *  @param[out] pedal_angle_perc Percentage of pedal angle pressed
 */
float get_pedal_angle_perc();


/** @brief Fetches the current speed
 *  @param[out] speed Current speed
 */
float get_cur_speed();

/** @brief Calculates the required slope based on pedal angle percentage and current speed; Speeds lesser than 50 km/hr are treated as 0km/hr while determining the torque; Speeds above and equal to 50km/hr are treated equal to 50km/hr
 *  @param[in] speed Current speed
 *  @param[in] pedal_angle_perc Percentage of pedal angle pressed
 *  @param[out] m_0 Solpes of straight lines defining torque for speed = 0
 *  @param[out] m_50 Solpes of straight lines defining torque for speed = 50
 *  @param[out] c_0 Constants of straight lines defining torque for speed = 0
 *  @param[out] c_50 Constants of straight lines defining torque for speed = 50
 */
float calc_torque(float pedal_angle_perc, float speed, float m_0[], float m_50[], float c_0[], float c_50[]);


int main(int argc, char *argv[]) {
    int size = 8;
    int t_0[] = {0,18,35,50,62,82,103,120};
    int t_50[] = {-30,-10,10,30,45,72,95,120};
    int angle_perc[] = {0,10,20,30,40,60,80,100};

    float m_0[] = {0,0,0,0,0,0,0};
    float m_50[] = {0,0,0,0,0,0,0};
    float c_0[] = {0,0,0,0,0,0,0};
    float c_50[] = {0,0,0,0,0,0,0};

    float pedal_angle_perc = 0.0;
    float speed = 0.0;
    float torque = 0.0;

    adc_return_t adc0_status = ADC_RET_NOK;
    adc_return_t adc1_status = ADC_RET_NOK;

//    Initialization
    init_torque_param(size, t_0, t_50, angle_perc, m_0, m_50, c_0, c_50);
    error_led_init();

    adc0_status = adc_init(ADC_CHANNEL0);
    adc1_status = adc_init(ADC_CHANNEL1);

    //  Turn on Error LED if status of any of the adc is not ok
    if(adc0_status == ADC_RET_NOK || adc1_status == ADC_RET_NOK)
    {
        printf("\tFailure in ADC Init!!!\n");
        error_led_set(true);
        torque = 0.0;
    }
    else
    {
        pedal_angle_perc = get_pedal_angle_perc();
        speed = get_cur_speed();
        torque = calc_torque(pedal_angle_perc, speed, m_0, m_50, c_0, c_50);
    }

    printf("Required Torque: %f\n", torque);
    return torque;
}

void init_torque_param(int size, int t_0[], int t_50[], int angle_perc[], float m_0[], float m_50[], float c_0[], float c_50[])
{
//  printf("\nt_0[], t_50[], angle_perc[], m_0[], m_50[], c_0[], c_50[]:      \n");
    for(int i = 0; i < (size-1); i++)
    {
        m_0[i] = (float)(t_0[i+1] - t_0[i])/ (float)(angle_perc[i+1] - angle_perc[i]);
        m_50[i] = (float)(t_50[i+1] - t_50[i])/ (float)(angle_perc[i+1] - angle_perc[i]);
        c_0[i] = t_0[i] - (m_0[i]*angle_perc[i]);
        c_50[i] = t_50[i] - (m_50[i]*angle_perc[i]);
//      printf("%d, %d, %d, %f, %f, %f, %f\n", t_0[i], t_50[i], angle_perc[i], m_0[i], m_50[i], c_0[i], c_50[i]);
    }
}

float get_pedal_angle_perc()
{
    adc_value_t adc0 = 0;
    adc_value_t adc1 = 0;
    adc_return_t adc0_status = ADC_RET_NOK;
    adc_return_t adc1_status = ADC_RET_NOK;

    float adc0_voltage = 0;
    float adc1_voltage = 0;
    float angle0 = 0.0;
    float angle1 = 0.0;
    float angle = 0.0;
    float pedal_angle_perc = 0.0;

//  Read from adc channels 1 & 2
    printf("\n");
    adc_read_set_output(ADC_CHANNEL0, 103, ADC_RET_OK);
    adc_read_set_output(ADC_CHANNEL1, 205, ADC_RET_OK);
    adc0_status = adc_read(ADC_CHANNEL0, &adc0);
    adc1_status = adc_read(ADC_CHANNEL1, &adc1);

//  Turn on Error LED if status of any of the adc is not ok
    if(adc0_status == ADC_RET_NOK || adc1_status == ADC_RET_NOK)
    {
        printf("\tFailure in ADC Read!!!\n");
        error_led_set(true);
        pedal_angle_perc = 0; // Set pedal_angle_perc to 0 in case of error
    }
    else
    {
//      Calculate equivalent Voltages value from adc values
        adc0_voltage = ((float)adc0 * 5) /1023;
        adc1_voltage = ((float)adc1 * 5) /1023;

//      Calculate equivalent Pedal angles based on voltages
        angle0 = (adc0_voltage - 0.5)/(0.1);
        angle1 = (adc1_voltage - 1.0)/(0.08);
    //  Turn on error LEDs if:
    //  1. angle values are out of valid range: [0, 30]
    //  2. difference between angles calculated from 2 adc channels is more than 5 degrees.
    // (Condition. 2 is not given in problem statement;
    //  However it seems to be necessary considering the critical nature of task)
        if(angle0<0 || angle1<0 || angle0>30 || angle1>30 || fabs((angle0-angle1))>5)
        {
            printf("\t[ADC Value of Range] OR [Difference in angles > 5 Degrees] !!!\n");
            error_led_set(true);
            pedal_angle_perc = 0; // Set pedal_angle_perc to 0 in case of error
        }
        else
        {
            angle = (angle0 + angle1)/2;
            pedal_angle_perc = (angle * 100) / 30;
        }
    }
    printf("adc0_voltage: %f\n", adc0_voltage);
    printf("adc1_voltage: %f\n", adc1_voltage);
    printf("angle0: %f\n", angle0);
    printf("angle1: %f\n", angle1);
    printf("angle: %f\n", angle);
    printf("angle percentage: %f\n", pedal_angle_perc);

    return pedal_angle_perc;
}

float get_cur_speed()
{
    float speed = 0.0;

    printf("\n");
    set_speed(100);
    speed = get_speed();
//  Turn on error LEDs if speed is not in range[0, 120]
    if(speed<0 || speed>120)
    {
       printf("\tSpeed out of range!!!\n");
       error_led_set(true);
       speed = 0;
    }
    return speed;
}

float calc_torque(float pedal_angle_perc, float speed, float m_0[], float m_50[], float c_0[], float c_50[])
{
    float torque = 0.0;
    float* m = m_0;
    float* c = c_0;
    int i = 0;


    printf("\n");
    if(speed >= 50)
    {
        m = m_50;
        c = c_50;
    }

//  Iterator to access correct values from the arrays with slopes and constants
    i = pedal_angle_perc / 10;
    switch(i)
    {
        case 5:
            i = 4;
            break;
        case 6:
        case 7:
            i = 5;
            break;
        case 8:
        case 9:
        case 10:
            i = 6;
            break;
    }

    torque = (m[i] * pedal_angle_perc) + c[i];
    printf("Required Torque: %f\n", torque);
    return torque;
}


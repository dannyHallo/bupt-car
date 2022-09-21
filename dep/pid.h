#pragma once

float Kp = 1,Ki = 0.1,Kd = 0.2;
float P = 0,I = 0,D = 0,PID_value = 0,error = 0;
float previous_error = 0,previous_I = 0;

float getPID(float error) {
    P = error;
    I = I+previous_I;
    D = error-previous_error;
    PID_value = (Kp*P)+(Ki*I)+(Kd*D);
    previous_I = I;
    previous_error = error;
    return PID_value;
}
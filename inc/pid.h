#ifndef PID_H_
#define PID_H_

void pid_setup_constants(double Kp_, double Ki_, double Kd_);
void pid_update_reference(double reference);
double pid_control(double saida_medida);

#endif

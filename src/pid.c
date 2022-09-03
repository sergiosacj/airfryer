#include <stdio.h>

#include "pid.h"

static double control_signal;
static double reference = 0.0;
static double Kp = 0.0;  // Ganho Proporcional
static double Ki = 0.0;  // Ganho Integral
static double Kd = 0.0;  // Ganho Derivativo
static int T = 1.0;      // Período de Amostragem (ms)
static double total_error, last_error = 0.0;
static int control_signal_MAX = 100.0;
static int control_signal_MIN = -100.0;

void pid_setup_constants(double Kp_, double Ki_, double Kd_){
  Kp = Kp_;
  Ki = Ki_;
  Kd = Kd_;
}

void pid_update_reference(double reference){
  reference = reference;
}

double pid_control(double measure){
  double error = reference - measure;

  // Acumula o erro (Termo Integral)
  total_error += error;

  if (total_error >= control_signal_MAX) {
    total_error = control_signal_MAX;
  } else if (total_error <= control_signal_MIN) {
    total_error = control_signal_MIN;
  }

  // Diferença entre os erros (Termo Derivativo)
  double delta_error = error - last_error;

  // PID calcula sinal de controle
  control_signal = Kp*error + (Ki*T)*total_error + (Kd/T)*delta_error;

  if (control_signal >= control_signal_MAX) {
    control_signal = control_signal_MAX;
  } else if (control_signal <= control_signal_MIN) {
    control_signal = control_signal_MIN;
  }

  last_error = error;
  return control_signal;
}

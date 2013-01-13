/*  Proportional–integral–derivative controller (PID controller) is a control loop feedback mechanism.

    PID controller calculates an "error" value as the difference between a measured process variable and a desired setpoint. 
    The controller attempts to minimize the error by adjusting the process control inputs.
    
    Input = Kinematics or sensor input
    Output = PID output
    SetPoint = input value
    windupGuard = maximum and minimum constrain on I term
*/
class PID {
    public:
        PID(double* Input, double* Output, double* Setpoint, double kp, double ki, double kd, double windupGuard) {
            previous_error = 0;
            integral = 0;
            
            PID_input = Input;
            PID_output = Output;
            PID_setpoint = Setpoint;
            
            Kp = kp;
            Ki = ki;
            Kd = kd;
        }
        
        void Compute() {            
            unsigned long now = micros();
            double delta_time = (now - last_time) / 1000000.0;
            double error = *PID_setpoint - *PID_input;
            integral = constrain(integral + error * delta_time, -windupGuard, windupGuard);
            double derivative = (*PID_input - previous_error) / delta_time;
            
            *PID_output = Kp * error + Ki * integral + Kd * derivative;
            
            previous_error = *PID_input;
            last_time = now;
        };
    
    private:
        double *PID_input;
        double *PID_output;
        double *PID_setpoint;
        
        double Kp, Ki, Kd;
        double windupGuard;
        
        double previous_error;
        double integral;
        unsigned long last_time;
};    
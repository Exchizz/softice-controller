#include "../include/MotorStateMachine.h"

#define ST_COMPRESSOR_INIT 0
#define ST_COMPRESSOR_IDLE 1
#define ST_COMPRESSOR_OFF 3
#define ST_COMPRESSOR_ON 4
#define ST_COMPRESSOR_RUNNING 5

class CompressorStateMachine : public MotorStateMachine
{
private:
    bool flag_ready = false;

public:
    CompressorStateMachine(int motor_pin) : MotorStateMachine(motor_pin)
    {
    }
    void set_ready_flag(){
        flag_ready = true;
    }

    void clear_ready_flag(){
        flag_ready = false;
    }

    void update();
};
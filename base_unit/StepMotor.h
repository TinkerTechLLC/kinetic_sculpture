#ifndef _STEPMOTOR_h
#define _STEPMOTOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class StepMotor{
    
    public:

    // Constructors
    StepMotor();
    StepMotor(int p_mot_steps, int p_step_pin, int p_dir_pin);

    // Public functions
    void ms(int p_ms);
    int ms();

    void speed(int p_spd);
    int speed();

    void dir(bool p_fwd);
    bool dir();

    void select();
    bool isSelected();

    void stepPin(int p_pin);
    int stepPin();

    void dirPin(int p_pin);
    int dirPin();

    bool updateRequired();

    private:
    
    //Basic properties
    int m_ms;
    int m_spd;
    bool m_dir;

    // Physical properties
    int m_mot_steps;
    int m_dir_pin;
    int m_step_pin;
    int m_ms_pin1;
    int m_ms_pin2;
    int m_ms_pin3;

    // Derived properties
    long m_step_delay;
    long m_last_step_time;
    bool m_update_required;
    bool m_selected;
    float m_steps_per_sec;
    byte m_on;
    byte m_off;

    // Consts
    static const int g_SEC_PER_MIN = 60;
    static const long g_MICROS_PER_SEC = 1e6;

    // Callback pointers
    void (*m_StepCallback)();

    // Private functions
    void step();
    void updateStepDelay();
}
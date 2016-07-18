# include "StepMotor.h"

StepMotor::StepMotor(){}

StepMotor::StepMotor(int p_mot_steps, int p_step_pin, int p_dir_pin, int p_ms_pin_1, int p_ms_pin_2, p_ms_pin_3){
    m_mot_steps = p_mot_steps;
    m_step_pin = p_step_pin;
    m_dir_pin = p_step_pin;
    m_ms_pin1 = p_ms_pin_1;
    m_ms_pin2 = p_ms_pin_2;
    m_ms_pin3 = p_ms_pin_3;
}

void StepMotor::stepPin(int p_pin){
    m_step_pin = p_pin;

    // Set port to use and correct bit to toggle
    if(m_step_pin >= 0 && m_step_pin <= 7){
        m_StepCallback = &PortBStep;
        m_on = 1 << m_step_pin;
    }
    else if(m_step_pin >= 8 && m_step_pin <= 13){
        m_StepCallback = &PortDStep;
        m_on = 1 << (m_step_pin - 8);
    }
}
int StepMotor::stepPin(){
    return m_step_pin;
}

void StepMotor::dirPin(int p_pin){
    m_dir_pin = p_pin;
}
int StepMotor::dirPin(){
    return m_dir_pin;
}

void StepMotor::ms(int p_ms){
    // Don't update anything if the setting didn't change
    if(p_ms == m_ms)
        return;

    m_ms = p_ms;
    switch(m_ms){
        case 1:
        digitalWrite(m_ms_pin1, LOW);
        digitalWrite(m_ms_pin2, LOW);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 2:
        digitalWrite(m_ms_pin1, HIGH);
        digitalWrite(m_ms_pin2, LOW);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 4:
        digitalWrite(m_ms_pin1, LOW);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 8:
        digitalWrite(m_ms_pin1, LOW);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, HIGH);
        break;

        case 16:
        digitalWrite(m_ms_pin1, HIGH);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, HIGH);
        break;
    }
    updateStepDelay();
}
int StepMotor::ms(){
    return m_ms;
}

void StepMotor::RPM(float p_rpm){
    float steps_per_sec = p_rpm / (g_SEC_PER_MIN * m_mot_steps * m_ms);
    stepPerSec(steps_per_sec);
}
float StepMotor::RPM(){
    return float RPM = (m_mot_steps * m_ms / m_step_per_sec) * g_SEC_PER_MIN;
}
void StepMotor::stepPerSec(float p_step_per_sec){
    m_steps_per_sec = p_steps_per_sec;
    // Update the direction pin
    dir(m_step_per_sec >= 0 ? true : false);
    updateStepDelay();
}
float StepMotor::stepsPerSec(){
    return m_steps_per_sec;
}
void StepMotor::updateStepDelay(){
    m_step_delay = round((float)g_MICROS_PER_SEC / m_steps_per_sec);
    m_update_required = true;
}

void StepMotor::dir(bool p_fwd){
    m_dir = p_fwd;
    digitalWrite(m_dir, p_fwd);
}
bool StepMotor::dir(){
    return m_dir;
}

void StepMotor::select(p_selected){
    m_selected = p_selected;
}
bool StepMotor::isSelected(){
    return m_selected;
}

bool StepMotor::updateRequired(){
    bool ret = m_update_required;
    updateRequired = false;
    return ret;
}

void StepMotor::step(){
    m_StepCallback();
}

void StepMotor::PortDStep(){
    PORTD |= on[1];
    delayMicroseconds(1);
    PORTD &= off[1];
}

void StepMotor::PortBStep(){
    PORTB |= on[1];
    delayMicroseconds(1);
    PORTB &= off[1];
}

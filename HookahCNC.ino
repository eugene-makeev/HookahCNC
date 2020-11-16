#include "AccelStepper.h"
#include "SerialCommand.h"
#include "button.h"
#include "Config.h"
#include "Cmd.h"

//-----------------------------------------------------------------------------------------------------------
long microstepsPerRevolution = MICROSTEPS_PER_REVOLUTION;
int rotAxisSpeed = ROT_AXIS_MAX_SPEED;
int rotAxisAccel = ROT_AXIS_ACCELERATION;

void cycleStart(void);

// Buttons
Button startButtonPush(START_BUTTON_PIN, cycleStart);

// stepper motor
AccelStepper rotMotor(1, ROT_AXIS_STEP_PIN, ROT_AXIS_DIR_PIN);

void setup()
{
#if (ENABLE_SERIAL_INTERFACE == 1)
    // enable eeprom wait in avr/eeprom.h functions
    SPMCSR &= ~SELFPRGEN;
    // TODO: load configuration values
    
    Serial.begin(115200);
    makeComInterface();
#endif

    // motors
    rotMotor.setEnablePin(ROT_AXIS_EN_PIN);
    rotMotor.setPinsInverted(!ROT_AXIS_DIR_ACTIVE, !ROT_AXIS_STEP_ACTIVE, !ROT_AXIS_EN_ACTIVE);
    rotMotor.disableOutputs();
    //rotMotor.setMinPulseWidth(50);
    rotMotor.setMaxSpeed(rotAxisSpeed);
    rotMotor.setAcceleration(rotAxisAccel);
    rotMotor.setSpeed(rotAxisSpeed);

    // servo and loopback pins setup
    pinMode(SERVO0_PIN, OUTPUT);
    pinMode(SERVO1_PIN, OUTPUT);
#if (LOOPBACK_PIN_ANLOG == 1)
    analogReference(EXTERNAL);
#else
    pinMode(LOOPBACK_PIN, INPUT_PULLUP);
#endif

    // divisor selector pins setup
    pinMode(SELECTOR_BIT0_PIN, INPUT_PULLUP);
    pinMode(SELECTOR_BIT1_PIN, INPUT_PULLUP);
    pinMode(SELECTOR_BIT2_PIN, INPUT_PULLUP);
    pinMode(SELECTOR_BIT3_PIN, INPUT_PULLUP);

    // safe power on: wait till start cycle button released
    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    while(!digitalRead(START_BUTTON_PIN));
}

int getDivisor()
{
    int divisor = 0;

    divisor = digitalRead(SELECTOR_BIT0_PIN);
    divisor |= digitalRead(SELECTOR_BIT1_PIN) << 1;
    divisor |= digitalRead(SELECTOR_BIT2_PIN) << 2;
    divisor |= digitalRead(SELECTOR_BIT3_PIN) << 3;

    // inverted output
    divisor ^= 0xF;

    return divisor ? divisor : 1;
}

bool punchCycle()
{
    // pulse to start servo
    digitalWrite(SERVO0_PIN, SERVO0_ACTIVE);
    delay(SERVO0_PULSE_WIDTH);
    digitalWrite(SERVO0_PIN, !SERVO0_ACTIVE);

    // wait for loopback signal (active low)
    unsigned long timeout = millis() + LOOPBACK_TIMEOUT; 
    
#if (LOOPBACK_PIN_ANALOG == 1)
    while ((analogRead(LOOPBACK_PIN) * ADC_RESOLUTION) > LOOPBACK_ACTIVE_THRESHOLD)
#else
    while (digitalRead(LOOPBACK_PIN) != LOOPBACK_ACTIVE)
#endif
    {
        if (millis() >= timeout)
        {
            // timeout
            return false;
        }
    }

    return true;
}

void cycleStart()
{
    int cycle = 0, divisor = getDivisor();
    long position = 0;

    // reset current position
    rotMotor.setCurrentPosition(position);
    // enable motor
    rotMotor.enableOutputs();

    for (int cycle = 0; cycle < divisor; cycle ++)
    {
        position = (1.0 * microstepsPerRevolution / divisor) * cycle;
        rotMotor.runToNewPosition(position);

//        rotMotor.moveTo(position);
//        while(rotMotor.distanceToGo())
//        {
//            rotMotor.run();
//        }

        // emergency stop
        if (digitalRead(START_BUTTON_PIN))
        {
            // disable motor
            rotMotor.disableOutputs();
            return;
        }
        
        if (!punchCycle())
        {
            // disable motor
            rotMotor.disableOutputs();
            return;
        }
    }

    digitalWrite(SERVO1_PIN, SERVO1_ACTIVE);
    delay(SERVO1_PULSE_WIDTH);
    punchCycle();
    digitalWrite(SERVO1_PIN, !SERVO1_ACTIVE);
    
    // disable motor
    rotMotor.disableOutputs();
}

void loop()
{
#if (ENABLE_SERIAL_INTERFACE == 1)
    SCmd.readSerial();
#endif

    startButtonPush.check();  
}

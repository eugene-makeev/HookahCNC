#include "Config.h"
#if (USE_FAST_ACCEL_STEPPER_LIB == 1)
#include "FastAccelStepper.h"
#else
#include "AccelStepper.h"
#endif

#include "SerialCommand.h"
#include "Cmd.h"
#include "button.h"
//-----------------------------------------------------------------------------------------------------------
long microstepsPerRevolution = MICROSTEPS_PER_REVOLUTION;
int rotAxisSpeed = ROT_AXIS_MAX_SPEED;
int rotAxisAccel = ROT_AXIS_ACCELERATION;

void cycleStart(void);

// Buttons
Button startButtonPush(START_BUTTON_PIN, cycleStart);

#if (USE_FAST_ACCEL_STEPPER_LIB == 1)
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;
#else
// stepper motor
AccelStepper rotMotor(1, ROT_AXIS_STEP_PIN, ROT_AXIS_DIR_PIN);
AccelStepper *stepper = &rotMotor;
#endif

void setup()
{
#if (ENABLE_SERIAL_INTERFACE == 1)
    // enable eeprom wait in avr/eeprom.h functions
    SPMCSR &= ~SELFPRGEN;
    // TODO: load configuration values
    
    Serial.begin(115200);
    makeComInterface();
#endif

    // configure motors
#if (USE_FAST_ACCEL_STEPPER_LIB == 1)
    engine.init();
    stepper = engine.stepperConnectToPin(ROT_AXIS_STEP_PIN);
    if (stepper) {
       stepper->setDirectionPin(ROT_AXIS_DIR_PIN);
       stepper->setEnablePin(ROT_AXIS_EN_PIN);
       stepper->setAutoEnable(false);
  
       stepper->setSpeed(1000);       // the parameter is us/step !!!??
       stepper->setAcceleration(100);
    }
#else
    stepper->setEnablePin(ROT_AXIS_EN_PIN);
    stepper->setPinsInverted(!ROT_AXIS_DIR_ACTIVE, !ROT_AXIS_STEP_ACTIVE, !ROT_AXIS_EN_ACTIVE);
    stepper->disableOutputs();
    //stepper->setMinPulseWidth(50);
    stepper->setMaxSpeed(rotAxisSpeed);
    stepper->setAcceleration(rotAxisAccel);
    stepper->setSpeed(rotAxisSpeed);
#endif

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

    return divisor;
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
    stepper->setCurrentPosition(position);
    // enable motor
    stepper->enableOutputs();

    bool sts = punchCycle();

    if (sts && divisor)
    {
        digitalWrite(SERVO1_PIN, SERVO1_ACTIVE);
        delay(SERVO1_PULSE_WIDTH);
        for (int cycle = 0; cycle < divisor; cycle ++)
        {
            position = (1.0 * microstepsPerRevolution / divisor) * cycle;

//          stepper->runToNewPosition(position);

            stepper->moveTo(position);
            while(stepper->isRunning())
            {
#if (USE_FAST_ACCEL_STEPPER_LIB == 0)
                stepper->run();
#endif
            }

            // emergency stop
            if (digitalRead(START_BUTTON_PIN))
            {
                // disable motor
                stepper->disableOutputs();
                return;
            }
            
            if (!punchCycle())
            {
                break;
            }
        }
            
        digitalWrite(SERVO1_PIN, !SERVO1_ACTIVE);
    }
    
    // disable motor
    stepper->disableOutputs();
}

void loop()
{
#if (ENABLE_SERIAL_INTERFACE == 1)
    SCmd.readSerial();
#endif

    startButtonPush.check();  
}

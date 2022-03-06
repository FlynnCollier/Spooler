#ifndef SPOOLER_H
#define SPOOLER_H

#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

#define MOTOR_STEPS 200
#define MOTOR_C_RPM 60
#define MOTOR_Z_RPM 60

#define ENABLE_C 27
#define DIR_C 25
#define STEP_C 26

#define ENABLE_Z 33
#define DIR_Z 14
#define STEP_Z 32

#define WIRE_TENSION_PIN 22
#define Z_STOP_PIN 23

#define MICROSTEPS 16

#define FIND_SPEED 1

class Spooler {
    private:
        bool _isProcessing;
        int _degrees;
        int _gauge;

        BasicStepperDriver* _stepperC;
        BasicStepperDriver* _stepperZ;
        SyncDriver* _controller;

        void Align() {
            FindEnd();
            _degrees = FindHome();
        }

        int FindHome() {
            int degreesToHome = 0;

            while (digitalRead(Z_STOP_PIN) == LOW) {
                degreesToHome++;
                Jog(-FIND_SPEED);
            }
            Jog(FIND_SPEED);

            return degreesToHome;
        }

        int FindEnd() {
            int degreesToEnd = 0;

            while (digitalRead(Z_STOP_PIN) == LOW) {
                degreesToEnd++;
                Jog(FIND_SPEED);
            }
            Jog(-FIND_SPEED);

            return degreesToEnd;
        }

    public:
        Spooler() {
            _isProcessing = false;
            _stepperC = new BasicStepperDriver(MOTOR_STEPS, DIR_C, STEP_C);
            _stepperZ = new BasicStepperDriver(MOTOR_STEPS, DIR_Z, STEP_Z);
            _controller = new SyncDriver(*_stepperZ, *_stepperC);
        }

        void Init() {
            _stepperC->begin(MOTOR_C_RPM, MICROSTEPS);
            _stepperZ->begin(MOTOR_Z_RPM, MICROSTEPS);

            digitalWrite(ENABLE_C, LOW);
            digitalWrite(ENABLE_Z, LOW);
        }

        void Jog(int degrees) {
            _stepperZ->rotate(degrees);
        }

        void Start(int gauge) {
            Align();
            _gauge = gauge;
            _isProcessing = true;
        }

        void Stop() {
            _isProcessing = false;
        }

        void Process() {
            if (!_isProcessing) 
                return;

            for (int r = 0; r < _degrees; r++) {
                _controller->rotate(1, _gauge);
                if (!_isProcessing) break;
            }
            _isProcessing = false;
        }
};

#endif
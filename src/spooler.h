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

enum SpoolerState {
    idle,
    processing,
    aligning
};

class Spooler {
    private:
        SpoolerState _state;
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

            Serial.println("find home...");
            while (digitalRead(Z_STOP_PIN) == LOW) {
                degreesToHome++;
                Jog(-FIND_SPEED);
            }

            Serial.println("jogging back...");
            while(digitalRead(Z_STOP_PIN) == HIGH) {
                degreesToHome--;
                Jog(FIND_SPEED);
            }

            Serial.print("degrees to home: ");
            Serial.println(degreesToHome);
            return degreesToHome;
        }

        int FindEnd() {
            int degreesToEnd = 0;

            Serial.println("find end...");
            while (digitalRead(Z_STOP_PIN) == LOW) {
                degreesToEnd++;
                Jog(FIND_SPEED);
            }

            Serial.println("jogging back...");
            while(digitalRead(Z_STOP_PIN) == HIGH) {
                degreesToEnd--;
                Jog(-FIND_SPEED);
            }

            Serial.print("degrees to end: ");
            Serial.println(degreesToEnd);
            return degreesToEnd;
        }

    public:
        Spooler() {
            _state = idle;
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
            Serial.print("jogging: ");
            Serial.println(degrees);
            _stepperZ->rotate(degrees);
        }

        void Start(int gauge) {
            _gauge = gauge;
            _state = aligning;
        }

        void Stop() {
            _state = idle;
        }

        void Process() {
            if (_state == aligning) {
                Serial.println("aligning...");
                Align();
                _state = processing;
            }

            if (_state == processing) {
                Serial.println("processing");

                for (int r = 0; r < _degrees; r++) {
                    _controller->rotate(1, _gauge);
                    if (_state != processing) break;
                }
                _state = idle;

                Serial.println("complete");
            }
        }
};

#endif
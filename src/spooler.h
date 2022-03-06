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
        int _widthIndegrees;
        int _windings;
        int _gauge;

        BasicStepperDriver* _stepperC;
        BasicStepperDriver* _stepperZ;
        SyncDriver* _controller;

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

        void Start(int windings, int gauge) {
            _windings = windings;
            _gauge = gauge;
            _state = processing;
        }

        void Stop() {
            _state = idle;
        }

        void Align() {
            FindEnd();
            _widthIndegrees = FindHome();
        }

        void Process() {
            if (_state == aligning) {
                Serial.println("aligning...");
                Align();
                _state = idle;
            }

            if (_state == processing) {
                Serial.println("processing");
                
                int cOverZ = _gauge;
                int position = 0;
                int direction = 1;
                for (int w = 0; w < _windings * 360; w++) {
                    position += direction;
                    _controller->rotate(direction, cOverZ);

                    //if reached home or end, turn around
                    if (position >= _widthIndegrees) {
                        direction = 1;
                    } else if (position <= 0) {
                        direction = -1;
                    }
                }

                _state = idle;
                Serial.println("complete");
            }
        }
};

#endif
#include <Servo.h>
#include <TinyGPS++.h>

const int MOTOR_L = 3; // Motor L PWM pin
const int MOTOR_R = 5; // Motor R PWM pin

Servo motor_l;
Servo motor_r;

void setupMotors() {
  motor_l.attach(MOTOR_L, 1000, 2000);
  motor_r.attach(MOTOR_R, 1000, 2000);
  motor_l.write(0);
  motor_r.write(0);
}

void move() {
    // Calculate movement
    int distance = TinyGPSPlus::distanceBetween(predictedLat(), predictedLng(), bleLat(), bleLng()); // in meters
    int targetCourse = TinyGPSPlus::courseTo(predictedLat(), predictedLng(), bleLat(), bleLng());
  
    // Figure out distance off in IMU
    int diff = targetCourse - predictedCourse();

    // Move
    float motor_l_pow = 0;
    float motor_r_pow = 0;
    
    if (abs(diff) > 45) {
      // Turn in place if that far off
      if (diff < 0) {
          motor_l_pow = 1;
      } else {
          motor_r_pow = 1;
      }
    } else {
      // Adjust motor powers to be max when 80m off
      motor_l_pow = min(distance, 80.0)/100.0;
      motor_r_pow = motor_l_pow;

      // Turn slightly based on error
      motor_l_pow -= 0.2*(diff/45.0);
      motor_r_pow += 0.2*(diff/45.0);
    }
    
    motor_l.write(motor_l_pow*180);
    motor_r.write(motor_r_pow*180);
}

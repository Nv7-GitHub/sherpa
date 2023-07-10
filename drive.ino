#include <Servo.h>
#include <TinyGPS++.h>

const int MOTOR_L = 7; // Motor L PWM pin
const int MOTOR_R = 8; // Motor R PWM pin

Servo motor_l;
Servo motor_r;

void setupMotors() {
  motor_l.attach(MOTOR_L, 1000, 2000);
  motor_r.attach(MOTOR_R, 1000, 2000);
  motor_l.write(0);
  motor_r.write(0);
}

int targetYaw;
int distance;
bool needRefresh = true;

void refreshTargets() {
  // Calculate distances
  distance = (int)(TinyGPSPlus::distanceBetween(lat(), lng(), bleLat(), bleLng()));
  int targetCourse = TinyGPSPlus::courseTo(lat(), lng(), bleLat(), bleLng());

  // Figure out distance off in IMU
  int diff = targetCourse - course();
  targetYaw = yaw() + diff;
}

void move() {
    // Refresh
    if (needRefresh) {
      refreshTargets();
      needRefresh = false;
    }
    if (distance < 10) {
      motor_l.write(0);
      motor_r.write(0);
      return;
    }

    // Move
    float motor_l_pow = 0;
    float motor_r_pow = 0;
    
    int yawDist = yaw() - targetYaw;
    if (abs(yawDist) > 45) {
      // Turn in place if that far off
      if (yawDist < 0) {
          motor_l_pow = 1;
      } else {
          motor_r_pow = 1;
      }
    } else {
      // Adjust motor powers to be max when 80m off
      motor_l_pow = min(distance, 80.0)/100.0;
      motor_r_pow = motor_l_pow;

      // Turn slightly based on error
      motor_l_pow -= 0.2*(yawDist/45.0);
      motor_r_pow += 0.2*(yawDist/45.0);
    }
    
    motor_l.write(motor_l_pow*180);
    motor_r.write(motor_r_pow*180);
}

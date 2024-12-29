
#include <iostream>
#include <fstream>
#include "double_link.h"
//
// Uncemment the desirable observer
//
//#include "../lib/momentum_observer.h"
//#include "../lib/disturbance_observer.h"
#include "../lib/sliding_mode_observer.h"
//#include "../lib/disturbance_kalman_filter.h"
//#include "../lib/filtered_dyn_observer.h"
//#include "../lib/filtered_range_observer.h"
//#include "../lib/disturbance_kalman_filter_exp.h"

#define OMEGA1 1.3
#define OMEGA2 0.8
#define FNAME "force.csv"
#define TSTEP 0.01

// use it to emulate external torque
#define SET_TORQUE

int main(int argc, char** argv)
{
  DoubleLink robot;
  VectorJ q(2), qd(2), q2d(2), tau(2), ext(2);

#ifdef MOMENTUM_OBSERVER_H
  Vector k(2);
  k << 30,30;
  MomentumObserver m_observer(&robot,k);
#endif
#ifdef DISTURBANCE_OBSERVER_H
  double sigma = 21, xeta = 18, beta = 50;
  DisturbanceObserver d_observer(&robot,sigma,xeta,beta);
#endif
#ifdef SLIDING_MODE_OBSERVER_H
  VectorJ T1(2), S1(2), T2(2), S2(2);
  S1 << 15,20;
  T1(0) = 2*sqrt(S1(0)); T1(1) = 2*sqrt(S1(1));
  S2 << 10,10; // set 0 to exclude linear part
  T2(0) = 2*sqrt(S2(0)); T2(1) = 2*sqrt(S2(1));
  SlidingModeObserver sm_observer(&robot,T1,S1,T2,S2);
#endif
#ifdef DISTURBANCE_KALMAN_FILTER_H
  Matrix S = Matrix::Zero(2,2);
  Matrix H = Matrix::Identity(2,2);
  Matrix Q = Matrix::Identity(4,4);
  Matrix R = Matrix::Identity(2,2);
  Q(0,0) = 0.002; Q(1,1) = 0.002; Q(2,2) = 0.3; Q(3,3) = 0.3;
  R *= 0.05;
  DKalmanObserver dkm_observer(&robot,S,H,Q,R);
#endif
#ifdef DISTURBANCE_KALMAN_FILTER_EXP_H
  Matrix S = Matrix::Zero(2,2);
  Matrix H = Matrix::Identity(2,2);
  Matrix Q = Matrix::Identity(4,4);
  Matrix R = Matrix::Identity(2,2);
  Q(0,0) = 0.2; Q(1,1) = 0.2; Q(2,2) = 30; Q(3,3) = 30;
  R *= 0.0005;
  DKalmanObserverExp dkme_observer(&robot,S,H,Q,R);
#endif
#ifdef FILTERED_DYNAMIC_OBSERVER_H
  FDynObserver fd_observer(&robot, 8, TSTEP);
#endif
#ifdef FILTERED_RANGE_OBSERVER_H
  FRangeObserver fr_observer(&robot, 8, TSTEP, -0.1);
#endif


  // save to file
  std::ofstream file;
  file.open(FNAME);
  // second file with the robot state
  std::ofstream file2;
  file2.open("input.csv");

  double dt = TSTEP;
  for(double t = 0; t < 3; t += dt) {
    double c1 = cos(OMEGA1*t), c2 = cos(OMEGA2*t);
    double s1 = sin(OMEGA1*t), s2 = sin(OMEGA2*t);
    // state
    q(0) = s1; q(1) = s2;
    qd(0) = OMEGA1*c1; qd(1) = OMEGA2*c2;
    q2d(0) = -OMEGA1*OMEGA1*s1; q2d(1) = -OMEGA2*OMEGA2*s2;

    // torque
    tau = robot.getM(q)*q2d + robot.getC(q,qd)*qd + robot.getG(q);
#ifdef SET_TORQUE
    if(t > 1 && t < 2) {
      tau(0) -= 0.5;
      tau(1) -= 0.5;
    }
#endif // SET_TORQUE
    // estimate torque and save

#ifdef MOMENTUM_OBSERVER_H
    ext = m_observer.getExternalTorque(q,qd,tau,dt);
#endif
#ifdef DISTURBANCE_OBSERVER_H
    ext = d_observer.getExternalTorque(q,qd,tau,dt);
#endif
#ifdef SLIDING_MODE_OBSERVER_H
    ext = sm_observer.getExternalTorque(q,qd,tau,dt);
#endif
#ifdef DISTURBANCE_KALMAN_FILTER_H
    ext = dkm_observer.getExternalTorque(q,qd,tau,dt);
#endif
#ifdef DISTURBANCE_KALMAN_FILTER_EXP_H
    ext = dkme_observer.getExternalTorque(q,qd,tau,dt);
#endif
#ifdef FILTERED_DYNAMIC_OBSERVER_H
    ext = fd_observer.getExternalTorque(q,qd,tau,dt);
#endif
#ifdef FILTERED_RANGE_OBSERVER_H
    ext = fr_observer.getExternalTorque(q,qd,tau,dt);
#endif

    file << t << "," << ext(0) << "," << ext(1) << std::endl;
    // robot state
    file2 << t << "," << q(0) << "," << q(1) << ","
                      << qd(0) << "," << qd(1) << ","
                      << tau(0) << "," << tau(1) << std::endl;
  }

  file.close();
  file2.close();

  return 0;
}


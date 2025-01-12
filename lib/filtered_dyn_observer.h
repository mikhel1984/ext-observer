// Copyright 2020-2024 Stanislav Mikhel

#ifndef FILTERED_DYNAMIC_OBSERVER_H
#define FILTERED_DYNAMIC_OBSERVER_H

#include "external_observer.h"
#include "iir_filter.h"

#define ID_FDynObserver 2

class FDynObserver final : public ExternalObserver {
public:
  FDynObserver(RobotDynamics *rd, double cutOffHz, double sampHz);

  VectorJ getExternalTorque(VectorJ& q, VectorJ& qd, VectorJ& tau, double dt) override;

  void settings(double cutOffHz, double sampHz);

private:
  FilterF1 f1;
  FilterF2 f2;
  VectorJ p, res;
};  // FDynObserver


FDynObserver::FDynObserver(RobotDynamics *rd, double cutOffHz, double sampHz)
  : ExternalObserver(rd, ID_FDynObserver)
  , f1(FilterF1(cutOffHz, sampHz, jointNo))
  , f2(FilterF2(cutOffHz, sampHz, jointNo))
  , p(VectorJ(jointNo))
  , res(VectorJ(jointNo))
{
}

void FDynObserver::settings(double cutOffHz, double sampHz)
{
  f1.update(cutOffHz, sampHz);
  f2.update(cutOffHz, sampHz);
}

VectorJ FDynObserver::getExternalTorque(VectorJ& q, VectorJ& qd, VectorJ& tau, double dt)
{
  p = dyn->getM(q) * qd;

  if(isRun) {
    res = f2.filt(p, dt) + f2.getOmega() * p ;
    p = dyn->getFriction(qd) + dyn->getG(q) - dyn->getC(q, qd).transpose() * qd;  // reuse
    p -= tau;
    res += f1.filt(p, dt);
  } else {
    f2.set(p);
    p = dyn->getFriction(qd) + dyn->getG(q) - dyn->getC(q, qd).transpose() * qd;  // reuse
    p -= tau;
    f1.set(p);
    res.setZero();
    isRun = true;
  }

  return res;
}

#endif  // FILTERED_DYNAMIC_OBSERVER_H

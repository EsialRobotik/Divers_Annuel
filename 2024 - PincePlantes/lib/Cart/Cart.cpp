#include "Cart.hpp"

Cart::Cart(unsigned long commandTimeout)
  : lastCommand(0.)
  , commandTimeout(commandTimeout)
{
  Cart::mainInstance = this;
}

void Cart::init() {
  pinMode(PIN_ODO_CODEUR_A, INPUT);
  pinMode(PIN_ODO_CODEUR_B, INPUT);
  pinMode(PIN_PID_MONTER, OUTPUT);
  pinMode(PIN_PID_DESCENDRE, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_ODO_CODEUR_A), odoComptageTicks, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ODO_CODEUR_B), odoComptageTicks, CHANGE);

  chariotPID.setOutputLimits((double) -PID_MAX_MOTOR_POWER, (double) PID_MAX_MOTOR_POWER);
  chariotPID.begin(&pidAltitudeCourante, &pidCommandeMoteur, &pidAltitudeCible, pidP, pidI, pidD);
}

void Cart::odoComptageTicks() {
  Cart::mainInstance->odoCurrA = digitalRead(PIN_ODO_CODEUR_A);
  Cart::mainInstance->odoCurrB = digitalRead(PIN_ODO_CODEUR_B);

  if (Cart::mainInstance->odoCurrA) {
    if (Cart::mainInstance->odoCurrB) {
      Cart::mainInstance->odoTicks += Cart::mainInstance->odoPrevA && !Cart::mainInstance->odoPrevB ? -1 : 1;
    } else {
      Cart::mainInstance->odoTicks += Cart::mainInstance->odoPrevA && Cart::mainInstance->odoPrevB ? 1 : -1;
    }
  } else {
    if (Cart::mainInstance->odoCurrB) {
      Cart::mainInstance->odoTicks += Cart::mainInstance->odoPrevA && Cart::mainInstance->odoPrevB ? -1 : 1;
    } else {
      Cart::mainInstance->odoTicks += Cart::mainInstance->odoPrevA && !Cart::mainInstance->odoPrevB ? 1 : -1;
    }
  }
  Cart::mainInstance->odoPrevA = Cart::mainInstance->odoCurrA;
  Cart::mainInstance->odoPrevB = Cart::mainInstance->odoCurrB;
}

void Cart::commander(double commande) {
  lastCommand = commande;
  analogWrite(PIN_PID_MONTER, commande > 0 ? (int) commande : 0);
  analogWrite(PIN_PID_DESCENDRE, commande < 0 ? (int) -commande : 0);
}

void Cart::zero(bool altMax) {
  pidEnabled = false;
  commander((double) (altMax ? 1 : -1) * PID_MAX_MOTOR_POWER_ZERO);
  unsigned long timeout = millis() + ODO_ZERO_TIMEOUT_MS;
  unsigned int prevTicks = 0;
  while (true) {
    prevTicks = odoTicks; 
    delay(250);
    Serial.print(" "); // Pour une raison obscure, ce print permet d'obtenir une valeur actualisée d'odoTicks, sinon elle ne varie jamais
    /*
     * Avant de pondre le fix ci-dessus, les autres tentatives infructueuses ont été :
     *  Utiliser une fonction d'accès à odoTicks => unsigned int getOdoTicks() { return odoTicks; }
     *  Utiliser une boucle pour l'attente => while (millis() < timetarget);
     *  Ecrire/lire sur une I/O => digitalWrite() + digitalRead() 
     */
    if (prevTicks > odoTicks) {
      if (prevTicks - odoTicks < ODO_SEUIL_TICKS_DEPLACEMENT_NUL) {
        break;
      }
    } else {
      if (odoTicks - prevTicks < ODO_SEUIL_TICKS_DEPLACEMENT_NUL) {
        break;
      }
    }
    if (millis() > timeout) {
      Serial.print("err: home timeout reached in ");
      Serial.print(ODO_ZERO_TIMEOUT_MS);
      Serial.println("ms");
      commander(0.);
      return;
    }
  }
  commander(0.);
  if (altMax) {
    pidAltitudeCourante = ((double) (odoTicks - ODO_TICKS_OFFSET)) / ((double) ODO_TICKS_PAR_MILLIMETRE);
    hauteurMaxMm = (int) pidAltitudeCourante;
  } else {
    odoTicks = ODO_TICKS_OFFSET;
    odoHomeSet = true;
    altMax = -1;
  }
  Serial.println("ok");
}

void Cart::setPosition(int position) {
  if (!odoHomeSet) {
    Serial.println("err: home not set");
  }
  if (position < 0) {
    Serial.println("err: position < 0");
    return;
  }
  if (!pidEnabled) {
    pidEnabled = true;
  }
  lastCommandStart = millis();
  pidAltitudeCible = (double) position;
  pidTargetReached = false;
}

void Cart::emergencyStop() {
    Serial.println("ok");
    pidEnabled = false;
    commander(0.);
}

void Cart::handlePID() {
  if (pidEnabled) {
    pidAltitudeCourante = ((double) (odoTicks - ODO_TICKS_OFFSET)) / ((double) ODO_TICKS_PAR_MILLIMETRE);
    chariotPID.compute();
    if (abs(pidAltitudeCourante - pidAltitudeCible) < ODO_MARGE_COMMANDE || (lastCommandStart + commandTimeout) < millis()) {
      commander(0.);
      if (!pidTargetReached) {
        pidTargetReached = true;
        Serial.println("ok");
      }
    } else {
      // Si on est en descente et qu'on est en dessous de 10mm, on y va molo sinon on perd l'odométrie quand on tape la butée
      if (pidCommandeMoteur < 0 && pidAltitudeCourante < ARRIVAL_BOTTOM_THRESHOLD_MM) {
        commander(pidCommandeMoteur * ARRIVAL_BOTTOM_POWER_RATIO);
      } else {
        commander(pidCommandeMoteur);
      }
    }
  }
}

double Cart::getDistanceMax() {
  return hauteurMaxMm;
}

double Cart::getPositionCourante() {
  return pidAltitudeCourante;
}

bool Cart::isHomeSet() {
  return odoHomeSet;
}

void Cart::heartBeat() {
  handlePID();
}

bool Cart::isMoving() {
  return lastCommand == 0.;
}

Cart* Cart::mainInstance = NULL;

double Cart::getPidP() {
  return pidP;
}

double Cart::getPidI() {
  return pidI;
}

double Cart::getPidD() {
  return pidD;
}

void Cart::setPidP(double p) {
  pidP = p;
  chariotPID.setCoefficients(pidP, pidI, pidD);
}

void Cart::setPidI(double i) {
  pidI = i;
  chariotPID.setCoefficients(pidP, pidI, pidD);
}

void Cart::setPidD(double d) {
  pidD = d;
  chariotPID.setCoefficients(pidP, pidI, pidD);
}
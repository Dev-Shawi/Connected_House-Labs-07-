#include "PorteAutomatique.h"
#include <Arduino.h>  // Pour les fonctions Arduino

PorteAutomatique::PorteAutomatique(int p1, int p2, int p3, int p4, float& distancePtr) 
  : _stepper(AccelStepper::FULL4WIRE, p1, p2, p3, p4), _distance(distancePtr) {
  _stepper.setMaxSpeed(1000);
  _stepper.setAcceleration(500);
}

void PorteAutomatique::update() {
  _currentTime = millis();
  
  switch (_etat) {
    case FERMEE: _fermeState(); break;
    case OUVERTE: _ouvertState(); break;
    case EN_OUVERTURE: _ouvertureState(); break;
    case EN_FERMETURE: _fermetureState(); break;
  }
  
  _stepper.run();
  _mettreAJourEtat();
}

void PorteAutomatique::setAngleOuvert(float angle) {
  _angleOuvert = angle;
}

void PorteAutomatique::setAngleFerme(float angle) {
  _angleFerme = angle;
}

void PorteAutomatique::setPasParTour(int steps) {
  _stepsPerRev = steps;
}

void PorteAutomatique::setDistanceOuverture(float distance) {
  _distanceOuverture = distance;
}

void PorteAutomatique::setDistanceFermeture(float distance) {
  _distanceFermeture = distance;
}

const char* PorteAutomatique::getEtatTexte() const {
  switch (_etat) {
    case FERMEE: return "Fermee";
    case OUVERTE: return "Ouverte";
    case EN_OUVERTURE: return "Ouverture";
    case EN_FERMETURE: return "Fermeture";
    default: return "Inconnu";
  }
}

float PorteAutomatique::getAngle() const {
  return _stepper.currentPosition() / (_stepsPerRev / 360.0);
}

void PorteAutomatique::_fermeState() {
  if (_distance < _distanceOuverture) {  // Si objet trop proche
    _ouvrir();
  }
}

void PorteAutomatique::_ouvertState() {
  if (_distance > _distanceFermeture) {  // Si objet s'éloigne
    _fermer();
  }
}

void PorteAutomatique::_ouvertureState() {
  if (_stepper.distanceToGo() == 0) {
    _etat = OUVERTE;
    _stepper.disableOutputs();
  }
}

void PorteAutomatique::_fermetureState() {
  if (_stepper.distanceToGo() == 0) {
    _etat = FERMEE;
    _stepper.disableOutputs();
  }
}

void PorteAutomatique::_ouvrir() {
  _etat = EN_OUVERTURE;
  _stepper.enableOutputs();
  _stepper.moveTo(_angleEnSteps(_angleOuvert));
}

void PorteAutomatique::_fermer() {
  _etat = EN_FERMETURE;
  _stepper.enableOutputs();
  _stepper.moveTo(_angleEnSteps(_angleFerme));
}

void PorteAutomatique::_mettreAJourEtat() {
  // Transition automatique entre les états
}

long PorteAutomatique::_angleEnSteps(float angle) const {
  return angle * (_stepsPerRev / 360.0);
}
#include "Alarm.h"
#include <Arduino.h>  // Pour millis(), tone(), etc.

Alarm::Alarm(int rPin, int gPin, int bPin, int buzzerPin, float* distancePtr) 
  : _rPin(rPin), _gPin(gPin), _bPin(bPin), _buzzerPin(buzzerPin), _distance(distancePtr) {
  pinMode(_rPin, OUTPUT);
  pinMode(_gPin, OUTPUT);
  pinMode(_bPin, OUTPUT);
  pinMode(_buzzerPin, OUTPUT);
  
  _setRGB(0, 0, 0);
  noTone(_buzzerPin);
}

void Alarm::update() {
  _currentTime = millis();
  
  switch (_state) {
    case OFF: _offState(); break;
    case WATCHING: _watchState(); break;
    case ON: _onState(); break;
    case TESTING: _testingState(); break;
  }
  
  if (_turnOnFlag) {
    _state = ON;
    _turnOnFlag = false;
    tone(_buzzerPin, 1000);
    _lastUpdate = _currentTime;
    _currentColor = false;
    _setRGB(_colA[0], _colA[1], _colA[2]);
  }
  
  if (_turnOffFlag) {
    _state = OFF;
    _turnOffFlag = false;
    _turnOff();
  }
}

void Alarm::_offState() {
  if (*_distance <= _distanceTrigger) {
    _state = WATCHING;
    _lastDetectedTime = _currentTime;
  }
}

void Alarm::_watchState() {
  if (*_distance > _distanceTrigger) {
    if (_currentTime - _lastDetectedTime >= _timeoutDelay) {
      _state = OFF;
    }
  } else {
    _lastDetectedTime = _currentTime;
    _state = ON;
    _turnOnFlag = true;
  }
}

void Alarm::_onState() {
  if (*_distance > _distanceTrigger) {
    _state = WATCHING;
    _lastDetectedTime = _currentTime;
  } else if (_currentTime - _lastUpdate >= _variationRate) {
    _lastUpdate = _currentTime;
    _currentColor = !_currentColor;
    if (_currentColor) {
      _setRGB(_colB[0], _colB[1], _colB[2]);
    } else {
      _setRGB(_colA[0], _colA[1], _colA[2]);
    }
  }
}

void Alarm::_testingState() {
  if (_currentTime - _testStartTime >= 3000) {
    _state = OFF;
    _turnOff();
  } else if (_currentTime - _lastUpdate >= _variationRate) {
    _lastUpdate = _currentTime;
    _currentColor = !_currentColor;
    if (_currentColor) {
      _setRGB(_colB[0], _colB[1], _colB[2]);
    } else {
      _setRGB(_colA[0], _colA[1], _colA[2]);
    }
  }
}

void Alarm::setColourA(int r, int g, int b) {
  _colA[0] = r; _colA[1] = g; _colA[2] = b;
}

void Alarm::setColourB(int r, int g, int b) {
  _colB[0] = r; _colB[1] = g; _colB[2] = b;
}

void Alarm::setVariationTiming(unsigned long ms) {
  _variationRate = ms;
}

void Alarm::setDistance(float d) {
  _distanceTrigger = d;
}

void Alarm::setTimeout(unsigned long ms) {
  _timeoutDelay = ms;
}

void Alarm::turnOff() {
  if (_state != TESTING) {
    _turnOffFlag = true;
  }
}

void Alarm::turnOn() {
  if (_state == OFF) {
    _turnOnFlag = true;
  }
}

void Alarm::test() {
  _state = TESTING;
  _testStartTime = _currentTime;
  tone(_buzzerPin, 1000);
  _currentColor = false;
  _setRGB(_colA[0], _colA[1], _colA[2]);
}

AlarmState Alarm::getState() const {
  return _state;
}

void Alarm::_setRGB(int r, int g, int b) {
  analogWrite(_rPin, r);
  analogWrite(_gPin, g);
  analogWrite(_bPin, b);
}

void Alarm::_turnOff() {
  _setRGB(0, 0, 0);
  noTone(_buzzerPin);
}
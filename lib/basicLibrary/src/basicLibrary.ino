#include <functional>
#include <vector>
typedef std::function<void()> VoidFunc;
typedef std::vector<VoidFunc> FuncVector;
FuncVector updates;
class LED
{
public:
  bool val = false;
  pin_t pin;
  void initialize(pin_t PIN)
  {
    updates.push_back(std::bind(&LED::update, this));
    pin = PIN;
    pinMode(pin, OUTPUT);
  }
  void update()
  {
    digitalWrite(pin, val);
  }
};
class Button
{
public:
  bool bDown = false;
  bool val = false;
  bool clicked = false;
  int timesClicked = 0;
  pin_t pin;
  void initialize(pin_t PIN)
  {
    updates.push_back(std::bind(&Button::update, this));
    pin = PIN;
    pinMode(pin, INPUT);
  }
  bool once = true;
  void update()
  {
    clicked = false;
    pinMode(pin, INPUT);
    if (digitalRead(pin) == HIGH && !bDown)
    {
      bDown = true;
      if (once)
      {
        timesClicked++;
        clicked = true;
        once = false;
      }
      val = !val;
    }
    if (digitalRead(pin) == LOW)
    {
      bDown = false;
      once = true;
    }
  }
};
class Potentiometer
{
public:
  pin_t pin;
  void initialize(pin_t PIN)
  {
    pin = PIN;
    pinMode(pin, INPUT);
    updates.push_back(std::bind(&Potentiometer::update, this));
  }
  int val = analogRead(pin);
  void update()
  {
    val = analogRead(pin);
  }
};

class LEDGroup
{
public:
  LED *lightsInGroup = {};
  pin_t *pins = {};
  LEDGroup(LED *array)
  {
    lightsInGroup = array;
  }
  void set(LED setLights[sizeof(lightsInGroup) / sizeof(LED) + 2])
  {
    lightsInGroup = setLights;
  }
  void oneOn(int index)
  {
    lightsInGroup[index].val = true;
  }
  void oneOff(int index)
  {
    lightsInGroup[index].val = false;
  }
  void initialize(pin_t *PINS)
  {
    pins = PINS;
    for (uint i = 0; i < sizeof(lightsInGroup) / sizeof(LED) + 2; i++)
    {
      lightsInGroup[i].initialize(pins[i]);
    }
  }
  void allOff()
  {
    for (uint i = 0; i < sizeof(lightsInGroup) / sizeof(LED) + 2; i++)
    {
      lightsInGroup[i].val = false;
    }
  }

  void allOn()
  {
    for (uint i = 0; i < sizeof(lightsInGroup) / sizeof(LED) + 1; i++)
    {
      lightsInGroup[i].val = true;
    }
  }

  void onTo(int index)
  {
    for (int i = 0; i < index; i++)
    {
      lightsInGroup[i].val = true;
    }
  }

  void offTo(int index)
  {
    for (int i = 0; i < index; i++)
    {
      lightsInGroup[i].val = false;
    }
  }
};

class Temperature
{
public:
  pin_t pin;
  int val;
  int celcius;
  int fahrenheit;
  void initialize(pin_t PIN)
  {
    pin = PIN;
    pinMode(pin, INPUT);
    updates.push_back(std::bind(&Temperature::update, this));
  }
  void update()
  {
    val = analogRead(pin);
    celcius = (((val * 3.3) / 4095.0) - 0.5) * 100;
    fahrenheit = celcius * 1.8 + 32;
  }
};

void
update()
{
  for (uint i = 0; i < updates.size(); i++)
  {
    updates[i]();
  }
}
#include "basicLibrary.ino"
#include "oled-wing-adafruit.h"
#include "sparkfun_VCNL4040_arduino_library.h"
#include "env.h"
#include "blynk.h"

SYSTEM_THREAD(ENABLED);

#define lowLightPin D6
#define onTargetLightPin D7
#define highLightPin D8

#define modeSwitchingButton D5
#define potentiometerPin A4
#define tempMonitorPin A3

#define lightLevelPin V0
#define lowRange V1
#define highRange V2

#define tempFahrenheitPin V4
#define tempCelciusPin V3
#define setRanges V5

#define LightIsLow "light_low"
#define LightIsStable "light_stable"
#define LightIsHigh "light_high"

OledWingAdafruit display;

VCNL4040 lightLevelSensor;

Temperature tempMonitor;

String notifications[3] = {LightIsLow, LightIsStable, LightIsHigh};

LED ledGroupSize[3];
pin_t pinsForLEDs[3] = {lowLightPin, onTargetLightPin, highLightPin};
LEDGroup lowGoodHigh(ledGroupSize);

volatile bool blinkIsReady = true;

Potentiometer lowHighRangeSetter;

bool notifyOnce = true;

volatile int lowHighRanges[2];
int lightToBlink;
int lightLevel;

volatile int blynkButtonVal;

Timer ledBlinkTimer(500, blink, true);
bool blinkOn = false;

Button modeSwitcher;

int lightChanged;

int counter = 0;

bool changed[3];
bool previousChanged[3];
bool equations[3];

void setup()
{
  Serial.begin(9600);

  Blynk.begin(BLYNK_AUTH_TOKEN);

  tempMonitor.initialize(tempMonitorPin);

  lowGoodHigh.initialize(pinsForLEDs);
  modeSwitcher.initialize(modeSwitchingButton);
  lowHighRangeSetter.initialize(potentiometerPin);

  if (!lightLevelSensor.begin())
  {
    Serial.println("Not Connected");
  }
  lightLevelSensor.powerOnAmbient();
  lightLevelSensor.powerOffProximity();

  display.setup();
  display.clearDisplay();
  display.display();
}
void loop()
{
  update();
  Blynk.run();
  display.loop();
  display.clearDisplay();
  display.setCursor(0, 0);
  lowGoodHigh.allOff();
  lightLevel = map(lightLevelSensor.getAmbient(), 0, 1000, 0, 100);

  Blynk.virtualWrite(tempFahrenheitPin, tempMonitor.fahrenheit);
  Blynk.virtualWrite(tempCelciusPin, tempMonitor.celcius);

  if (modeSwitcher.timesClicked < 2)
  {
    lowHighRanges[modeSwitcher.timesClicked] = map(lowHighRangeSetter.val, 0, 4095, 0, 100);
    lightToBlink = modeSwitcher.timesClicked * 2;
    lowGoodHigh.lightsInGroup[lightToBlink].val = blinkOn;
    printToOled("Range:");
    printToOled((String)lowHighRanges[modeSwitcher.timesClicked]);
    if (blinkIsReady)
    {
      ledBlinkTimer.start();
      blinkOn = !blinkOn;
      blinkIsReady = false;
    }
  }
  else
  {
    if (display.pressedA())
    {
      modeSwitcher.timesClicked = 0;
    }
    Blynk.virtualWrite(lightLevelPin, lightLevel);
    Blynk.virtualWrite(lowRange, lowHighRanges[0]);
    Blynk.virtualWrite(highRange, lowHighRanges[1]);

    equations[0] = lightLevel < lowHighRanges[0];
    equations[2] = lightLevel > lowHighRanges[1];
    equations[1] = !equations[0] && !equations[2];

    for (int i = 0; i < 3; i++)
    {
      changed[i] = previousChanged[i];
      lowGoodHigh.lightsInGroup[i].val = equations[i];
      previousChanged[i] = lowGoodHigh.lightsInGroup[i].val;
      if (changed[i] != lowGoodHigh.lightsInGroup[i].val && lowGoodHigh.lightsInGroup[i].val)
      {
        notifyOnce = true;
        lightChanged = i;
      }
    }

    if (notifyOnce)
    {
      Blynk.logEvent(notifications[lightChanged], "Light Level Changed");
      notifyOnce = false;
    }

    printToOled("Fahrenheit:");
    printToOled((String)tempMonitor.fahrenheit);
    printToOled("Celcius:");
    printToOled((String)tempMonitor.celcius);
  }
  display.display();
}

void printToOled(String msg)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(msg);
}

void blink()
{
  blinkIsReady = true;
}

BLYNK_WRITE(setRanges)
{
  blynkButtonVal = param.asInt();
  if (blynkButtonVal)
  {
    lowHighRanges[0] = lightLevel - 5;
    lowHighRanges[1] = lightLevel + 5;
  }
}
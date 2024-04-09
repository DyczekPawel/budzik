#include <SPI.h>
#include <TFT_eSPI.h>
#include <utility>

TFT_eSPI tft = TFT_eSPI();
// https://github.com/PaulStoffregen/XPT2046_Touchscreen
// jak będą problemy z wykrywaniem dotyku

/*
 touchQueuedAction[0] - type of action: //TODO (potencjalnie zastąpić event group albo task notify z rtos)
    0 - longPress
    1 - swipe
    3 - press
 touchQueuedAction[1] - x coordinates or deviation in case of swipe
 touchQueuedAction[2] - y coordinates or deviation in case of swipe
*/
short touchQueuedAction[3];

void menuScreen()
{
  tft.fillScreen(0);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(20, 20, 4);
  tft.print("MENU");
  tft.setTextSize(1);

  tft.setCursor(68, 141, 4);
  tft.print("Opcja 1");

  tft.setCursor(329, 141, 4);
  tft.print("Opcja 2");

  tft.drawRect(90, 190, 40, 40, TFT_RED);
  tft.drawRect(220, 190, 40, 40, TFT_GREEN);
  tft.drawRect(350, 190, 40, 40, TFT_BLUE);
}

// TODO Potencjalnie zaimplementować obiekt z detalami o elementach interaktywnych scen.
TaskHandle_t updateDisplay_t;
void updateDisplay(void *params)
{
  menuScreen();
  for (;;)
  {
  }
}

// TODO zaimplementować jako task z przerwaniami. Przerwania: dotyk, zmiana sceny.
void handleTouch(short *touchQueuedAction)
{
  if (touchQueuedAction[0] == 0)
  {
    menuScreen();
  }
  else if (touchQueuedAction[0] == 1)
  {
    tft.fillScreen(0);
  }
}

TaskHandle_t detectTouch_t;
// Task for touch detection, upon detecting touch calls handleTouch function with touch type and coordinates or deviation as arguments.
void detectTouch(void *params)
{
  u_int16_t x, y, vx, vy, x2, y2, x3, y3, x4, y4;
  std::pair<short, short> touchBufor[43];
  short i = 0;      // touchBufor index
  short devX, devY; // axis deviation in swipe
  for (;;)
  {
    // Check if screen is being touched and no long press detected
    if ((analogRead(8) < 100) && i != 43)
    {
      // Touch threshold
      if (tft.getTouchRawZ() > 200)
      {
        // Check for raw data errors
        tft.getTouchRaw(&x, &y);
        tft.getTouchRaw(&x2, &y2);
        tft.getTouchRaw(&x3, &y3);
        tft.getTouchRaw(&x4, &y4);
        vx = x - x2 + x3 - x4;
        vy = y - y2 + y3 - y4;
        if (abs(vx) < 250 && abs(vy) < 250)
        {
          // Add deviation to raw data
          x += vx / 2;
          y += vy / 2;
          tft.convertRawXY(&x, &y);
          // Throw warring for conversion error //TODO add proper warn/error handling
          if (x + y > 1000)
          {
            Serial.println("Coordinates conversion error");
          }
          else
          {
            touchBufor[i] = {x, y};
            i++;
          }
        }
      }
      vTaskDelay(7);
    }
    // If screen is no longer touched or touched for max amount (touchBufor[size] * vTaskDelay(ms)) of time
    else if (i > 0)
    {
      // Detect long press
      if (i >= 35)
      {
        touchQueuedAction[0] = 0;
        touchQueuedAction[1] = touchBufor[42].first;
        touchQueuedAction[2] = touchBufor[42].second;
      }
      else
      {
        for (short j = 0; j < 35; j++)
        {

          if (touchBufor[j].first == '\0')
          {
            devX = touchBufor[0].first - touchBufor[j - 1].first;
            devY = touchBufor[0].second - touchBufor[j - 1].second;
            // Detect swipe
            if (abs(devX) > 60 || abs(devY) > 30 || abs(devX) + abs(devY) > 67)
            {
              touchQueuedAction[0] = 1;
              touchQueuedAction[1] = devX;
              touchQueuedAction[2] = devY;
            }

            // Detect press
            else if (i <= 10)
            {
              touchQueuedAction[0] = 3;
              touchQueuedAction[1] = touchBufor[0].first;
              touchQueuedAction[2] = touchBufor[0].second;
            }
            break;
          }
        }
      }
      // TODO Wywołać przerwanie i podać argumenty do taska
      handleTouch(touchQueuedAction);
      // Clear touchBufor and reset its index
      std::fill(std::begin(touchBufor), std::end(touchBufor), std::pair<short, short>('\0', '\0'));
      i = 0;
      vTaskDelay(500);
    }
    // Screen was not touched
    else
    {
      vTaskDelay(150);
    }
  }
}

void setup(void)
{
  delay(2500);
  Serial.begin(115200);
  Serial.println("\n\nStarting...");

  tft.init();
  tft.setRotation(1);
  analogWrite(TFT_BL, 100 * 2.55);

  uint16_t calData[5] = {225, 3765, 200, 3765, 7};
  tft.setTouch(calData);

  tft.fillScreen(0);

  xTaskCreate(detectTouch, "touch", 2048, NULL, 1, &detectTouch_t);
  menuScreen();

  Serial.println("Running...");
}

void loop()
{
}

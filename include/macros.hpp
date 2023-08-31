#pragma once

#define DREAD(x) digitalRead(x)
#define DHIGH(x) digitalWrite(x, HIGH)
#define DLOW(x) digitalWrite(x, LOW)
#define DWRITE(x, y) digitalWrite(x, y)
#define DELAY(x) vTaskDelay(x / portTICK_PERIOD_MS);

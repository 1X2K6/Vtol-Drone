/*
  ESP32 24GHz mmWave (HLK-LD2410C) sweeping radar + 2.4" ILI9341 TFT
  Servo sweeps 0-180 deg, radar reading is plotted on a half-circle radar screen.

  Libraries (Library Manager):
    - Adafruit GFX Library
    - Adafruit ILI9341
    - ESP32Servo
    - ld2410  (by ncmreynolds)

  WIRING
  LD2410C : VCC->5V  GND->GND  TX->GPIO16 (ESP32 RX2)  RX->GPIO17 (ESP32 TX2)
  Servo   : Signal->GPIO13, power from 5V (use a separate 5V supply if it jitters,
            share GND with the ESP32)
  TFT 2.4": VCC->3V3  GND->GND  CS->GPIO5  RESET->GPIO26  DC->GPIO27
            MOSI->GPIO23  SCK->GPIO18  LED->3V3  MISO->GPIO19
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <ESP32Servo.h>
#include <ld2410.h>

// ---------- Pins ----------
#define TFT_CS    5
#define TFT_RST   26
#define TFT_DC    27
#define RADAR_RX  16   // ESP32 RX  <- LD2410 TX
#define RADAR_TX  17   // ESP32 TX  -> LD2410 RX
#define SERVO_PIN 13

// ---------- Settings ----------
#define MAX_RANGE_CM   500     // radar plot range (LD2410 max is ~600 cm)
#define STEP_DEG       4       // sweep step
#define SERVO_SETTLE   70      // ms to let servo reach position
#define SAMPLE_MS      110     // ms to collect radar data at each angle
#define SERVO_MIN_US   500
#define SERVO_MAX_US   2400

// ---------- Colors ----------
#define C_BLACK  0x0000
#define C_GREEN  0x07E0
#define C_DGREEN 0x0320
#define C_RED    0xF800
#define C_YELLOW 0xFFE0
#define C_WHITE  0xFFFF
#define C_GREY   0x7BEF

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
Servo servo;
ld2410 radar;

// Radar geometry on a 320x240 landscape screen
const int CX = 160;        // sweep origin x
const int CY = 215;        // sweep origin y
const int R_PX = 205;      // radius in pixels for MAX_RANGE_CM

const int NUM_POS = 180 / STEP_DEG + 1;
uint16_t movingDist[NUM_POS];      // cm, 0 = nothing
uint16_t statDist[NUM_POS];
uint8_t  movingSig[NUM_POS];

int angleDeg = 0;
int dirStep  = STEP_DEG;
int posIdx   = 0;

// ---------- Helpers ----------
void polar(int deg, float rPx, int &x, int &y) {
  float a = deg * DEG_TO_RAD;
  x = CX - (int)(rPx * cos(a));   // 0 deg = left, 180 deg = right
  y = CY - (int)(rPx * sin(a));
}

void drawGrid() {
  // range rings every 100 cm
  for (int cm = 100; cm <= MAX_RANGE_CM; cm += 100) {
    float r = (float)cm / MAX_RANGE_CM * R_PX;
    int px, py, lx, ly;
    polar(0, r, lx, ly);
    for (int d = 3; d <= 180; d += 3) {
      polar(d, r, px, py);
      tft.drawLine(lx, ly, px, py, C_DGREEN);
      lx = px; ly = py;
    }
    tft.setTextColor(C_GREY);
    tft.setTextSize(1);
    tft.setCursor(CX + 2, CY - (int)r - 8);
    tft.print(cm / 100);
    tft.print("m");
  }
  // angle spokes every 30 deg
  for (int d = 0; d <= 180; d += 30) {
    int x, y;
    polar(d, R_PX, x, y);
    tft.drawLine(CX, CY, x, y, C_DGREEN);
  }
  tft.drawFastHLine(0, CY, 320, C_DGREEN);
}

void drawBlips() {
  for (int i = 0; i < NUM_POS; i++) {
    int deg = i * STEP_DEG;
    if (statDist[i] > 0 && statDist[i] <= MAX_RANGE_CM) {
      int x, y;
      polar(deg, (float)statDist[i] / MAX_RANGE_CM * R_PX, x, y);
      tft.fillCircle(x, y, 3, C_YELLOW);     // stationary (still person)
    }
    if (movingDist[i] > 0 && movingDist[i] <= MAX_RANGE_CM) {
      int x, y;
      polar(deg, (float)movingDist[i] / MAX_RANGE_CM * R_PX, x, y);
      tft.fillCircle(x, y, 4, C_RED);        // moving target
    }
  }
}

void drawSweepLine(int deg, uint16_t color) {
  int x, y;
  polar(deg, R_PX, x, y);
  tft.drawLine(CX, CY, x, y, color);
}

void drawStatus(bool present, uint16_t mDist, uint16_t sDist) {
  tft.fillRect(0, 0, 320, 14, C_BLACK);
  tft.setTextSize(1);
  tft.setCursor(2, 3);
  if (present) {
    tft.setTextColor(C_RED);
    tft.print("HUMAN ");
  } else {
    tft.setTextColor(C_GREEN);
    tft.print("CLEAR ");
  }
  tft.setTextColor(C_WHITE);
  tft.printf("ANG:%3d M:%3dcm S:%3dcm", angleDeg, mDist, sDist);
}

void drawLegend() {
  tft.fillCircle(8, 228, 3, C_RED);
  tft.setTextColor(C_WHITE);
  tft.setCursor(14, 225);
  tft.print("Moving");
  tft.fillCircle(8, 214, 3, C_YELLOW);
  tft.setCursor(14, 211);
  tft.print("Still");
}

void redrawScene() {
  tft.fillRect(0, 14, 320, 226, C_BLACK);
  drawGrid();
  drawBlips();
  drawLegend();
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);             // landscape 320x240
  tft.fillScreen(C_BLACK);
  tft.setTextColor(C_GREEN);
  tft.setTextSize(2);
  tft.setCursor(40, 100);
  tft.print("RADAR STARTING");

  Serial2.begin(256000, SERIAL_8N1, RADAR_RX, RADAR_TX);
  delay(500);
  if (radar.begin(Serial2)) {
    Serial.println("LD2410 connected");
  } else {
    Serial.println("LD2410 NOT detected - check wiring");
    tft.setCursor(40, 130);
    tft.setTextColor(C_RED);
    tft.print("RADAR NOT FOUND");
    delay(1500);
  }

  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  servo.write(0);
  delay(600);

  memset(movingDist, 0, sizeof(movingDist));
  memset(statDist, 0, sizeof(statDist));
  memset(movingSig, 0, sizeof(movingSig));

  tft.fillScreen(C_BLACK);
  redrawScene();
}

// ---------- Loop ----------
void loop() {
  // move servo to next angle
  servo.write(angleDeg);
  delay(SERVO_SETTLE);

  // collect radar data for SAMPLE_MS
  uint16_t bestM = 0, bestS = 0;
  uint8_t bestSig = 0;
  bool present = false;
  uint32_t t0 = millis();
  while (millis() - t0 < SAMPLE_MS) {
    radar.read();
    if (radar.isConnected()) {
      if (radar.movingTargetDetected() && radar.movingTargetSignal() >= bestSig) {
        bestM = radar.movingTargetDistance();
        bestSig = radar.movingTargetSignal();
        present = true;
      }
      if (radar.stationaryTargetDetected()) {
        bestS = radar.stationaryTargetDistance();
        present = true;
      }
    }
  }

  posIdx = angleDeg / STEP_DEG;
  movingDist[posIdx] = bestM;
  movingSig[posIdx]  = bestSig;
  statDist[posIdx]   = bestS;

  if (present) {
    Serial.printf("ang=%d moving=%dcm(sig %d) still=%dcm\n", angleDeg, bestM, bestSig, bestS);
  }

  // redraw scene + new sweep line (avoids leftover sweep trails)
  redrawScene();
  drawSweepLine(angleDeg, C_GREEN);
  drawStatus(present, bestM, bestS);

  // advance and bounce at ends
  angleDeg += dirStep;
  if (angleDeg >= 180) { angleDeg = 180; dirStep = -STEP_DEG; }
  if (angleDeg <= 0)   { angleDeg = 0;   dirStep =  STEP_DEG; }
}

#include <LEDFader.h>
#include <MSGEQ7.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Potentiometer.h>

#define MSGEQ7_READINGS 10



const int pinAnalogLeft = A0;
const int pinAnalogRight = A0;
const int pinReset = 3;
const int pinStrobe = 2;


Potentiometer bassPot = Potentiometer(1);
Potentiometer midPot = Potentiometer(2);
Potentiometer highPot = Potentiometer(3);

byte level0[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111};
byte level1[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};
byte level2[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111};
byte level3[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111};
byte level4[8] = {0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
byte level5[8] = {0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
byte level6[8] = {0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
byte level7[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};


LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

const int fadeSpeed = 100;
const int readTimeout = 25;

const bool DEBUG = true;

const int redPin = 9;
const int greenPin = 11;
const int bluePin = 10;



LEDFader redFader = LEDFader(redPin);
LEDFader greenFader = LEDFader(greenPin);
LEDFader blueFader = LEDFader(bluePin);

void setup() {

    MSGEQ7.begin(pinReset, pinStrobe, pinAnalogLeft);

    lcd.begin(16, 2);


    if (DEBUG) {
        Serial.begin(115200);
        Serial.println("Startup");
    }


    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    TCCR2B = TCCR2B & 0b11111000 | 0x01;


    lcd.backlight();
    lcd.clear();
    lcd.createChar(0, level0);
    lcd.createChar(1, level1);
    lcd.createChar(2, level2);
    lcd.createChar(3, level3);
    lcd.createChar(4, level4);
    lcd.createChar(5, level5);
    lcd.createChar(6, level6);
    lcd.createChar(7, level7);

    lcd.setCursor(8, 1);
    lcd.write('B');

    lcd.setCursor(9, 1);
    lcd.write('M');

    lcd.setCursor(10, 1);
    lcd.write('H');


}

void displayColor(uint8_t red, uint8_t green, uint8_t blue) {
    redFader.fade(red, fadeSpeed);
    greenFader.fade(green, fadeSpeed);
    blueFader.fade(blue, fadeSpeed);
}

int prepareSignal(int signal) {
    return constrain(signal, 0, 255);
}


void loop() {
    redFader.update();
    greenFader.update();
    blueFader.update();
    musicEQ();
}


void displaySpecrum() {
    for (int band = 0; band < 7; band++) {
        int currentLeft = map(MSGEQ7.get(band, MSGEQ7_LEFT), 0, 255, 0, 8);
        lcd.setCursor(band, 0);
        lcd.write(currentLeft);
    }
    lcd.setCursor(8, 0);
    lcd.write(map(1024 - bassPot.getValue(), 0, 1023, 0, 7));

    lcd.setCursor(9, 0);
    lcd.write(map(1024 - midPot.getValue(), 0, 1023, 0, 7));

    lcd.setCursor(10, 0);
    lcd.write(map(1024 - highPot.getValue(), 0, 1023, 0, 7));
}

void musicEQ() {
    static long prevMillis = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - prevMillis > readTimeout) {
        prevMillis = currentMillis;
        // analyze
        MSGEQ7.read();
        uint8_t bass = MSGEQ7.get(MSGEQ7_BASS, MSGEQ7_LEFT);
        uint8_t mid = MSGEQ7.get(MSGEQ7_MID, MSGEQ7_LEFT);
        uint8_t high = MSGEQ7.get(MSGEQ7_HIGH, MSGEQ7_LEFT);

        int minusBass = map(bassPot.getValue(), 0, 1023, 0, 255);
        int minusMid = map(midPot.getValue(), 0, 1023, 0, 255);
        int minusHigh = map(highPot.getValue(), 0, 1023, 0, 255);

        int r = prepareSignal(bass - minusBass);
        int g = prepareSignal(mid - minusMid);
        int b = prepareSignal(high - minusHigh);

        displayColor(r, g, b);
        displaySpecrum();
        if (DEBUG) {
            Serial.printf("basPot: %i midPot: %i highPot: %i\n", minusBass, minusMid, minusHigh);
            Serial.printf("bas: %i(%i) mid: %i(%i) high: %i(%i)\n", bass, r, mid, g, high, b);
        }

    }
}


#include <LEDFader.h>
#include <MSGEQ7.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Potentiometer.h>
#include <Bounce2.h>

#define MSGEQ7_READINGS 10

#define SETTING_LOW 0
#define SETTING_MID 1
#define SETTING_HIGH 2
#define SETTING_MAX 3

const uint8_t pinAnalogLeft = A0;
const uint8_t pinReset = 3;
const uint8_t pinStrobe = 2;
const uint8_t buttonPin = 8;

Potentiometer universalPot = Potentiometer(1, 255);

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
const int blinkInterval = 500;

const bool DEBUG = true;

const int redPin = 9;
const int greenPin = 11;
const int bluePin = 10;

uint8_t minusBass = 0;
uint8_t minusMid = 0;
uint8_t minusHigh = 0;
uint8_t minusMax = 0;

LEDFader redFader = LEDFader(redPin);
LEDFader greenFader = LEDFader(greenPin);
LEDFader blueFader = LEDFader(bluePin);

const int max_setting_value = 3;
int currentSetting = 0;


Bounce debouncer = Bounce();

void setup() {
    pinMode(buttonPin, INPUT_PULLUP);
    debouncer.attach(buttonPin);
    debouncer.interval(20); // interval in ms

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

    lcd.setCursor(11, 1);
    lcd.write('M');
}

void checkButtonState() {
    static int prevButtonState = HIGH;
    int buttonState = debouncer.read();
    if (prevButtonState != buttonState) {
        if (buttonState == LOW) {
            currentSetting++;
        }
        prevButtonState = buttonState;
    }

    if (currentSetting > max_setting_value) {
        currentSetting = 0;
    }

    Serial.println(currentSetting);
}

void displayColor(uint8_t red, uint8_t green, uint8_t blue) {
    redFader.fade(red, fadeSpeed);
    greenFader.fade(green, fadeSpeed);
    blueFader.fade(blue, fadeSpeed);
}

int prepareSignal(int signal) {
    return constrain(signal, 0, 255);
}

void readPotValues() {
    switch (currentSetting) {
        case SETTING_LOW:
            minusBass = 255 - universalPot.getSector();
            break;
        case SETTING_MID:
            minusMid = 255 - universalPot.getSector();
            break;
        case SETTING_HIGH:
            minusHigh = 255 - universalPot.getSector();
            break;
        case SETTING_MAX:
            minusMax = 255 - universalPot.getSector();
            break;
    }
}

void loop() {
    debouncer.update();
    redFader.update();
    greenFader.update();
    blueFader.update();
    checkButtonState();
    readPotValues();
    musicEQ();

}


void displaySpecrum() {
    static long prevMillis = 0;
    static bool blink = true;
    unsigned long currentMillis = millis();
    if (currentMillis - prevMillis > blinkInterval) {
        prevMillis = currentMillis;
        blink = !blink;
    }

    for (int band = 0; band < 7; band++) {
        int currentLeft = map(MSGEQ7.get(band, MSGEQ7_LEFT), 0, 255, 0, 8);
        lcd.setCursor(band, 0);
        lcd.write(currentLeft);
    }

    lcd.setCursor(8, 0);
    int bass = map(255 - minusBass, 0, 255, 0, 7);
    if (currentSetting == SETTING_LOW) {
        if (blink) {
            lcd.write(bass);
        } else {
            lcd.write(' ');
        }
    } else {
        lcd.write(bass);
    }

    lcd.setCursor(9, 0);
    int mid = map(255 - minusMid, 0, 255, 0, 7);
    if (currentSetting == SETTING_MID) {
        if (blink) {
            lcd.write(mid);
        } else {
            lcd.write(' ');
        }
    } else {
        lcd.write(mid);
    }


    lcd.setCursor(10, 0);
    int high = map(255 - minusHigh, 0, 255, 0, 7);
    if (currentSetting == SETTING_HIGH) {
        if (blink) {
            lcd.write(high);
        } else {
            lcd.write(' ');
        }
    } else {
        lcd.write(high);
    }


    int max = map(255 - minusMax, 0, 255, 0, 7);
    lcd.setCursor(11, 0);
    if (currentSetting == SETTING_MAX) {
        if (blink) {
            lcd.write(max);
        } else {
            lcd.write(' ');
        }
    } else {
        lcd.write(max);
    }
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


        uint8_t r = prepareSignal(bass - minusBass - minusMax);
        uint8_t g = prepareSignal(mid - minusMid - minusMax);
        uint8_t b = prepareSignal(high - minusHigh - minusMax);

        displayColor(r, g, b);
        displaySpecrum();
//        if (DEBUG) {
//            Serial.printf("basPot: %i midPot: %i highPot: %i\n", minusBass, minusMid, minusHigh);
//            Serial.printf("bas: %i(%i) mid: %i(%i) high: %i(%i)\n", bass, r, mid, g, high, b);
//        }

    }
}


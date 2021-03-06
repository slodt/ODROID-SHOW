#include <SPI.h>
#include <Wire.h>
#include "TimerOne.h"
#include <Adafruit_GFX.h>
#include <ODROID_Si1132.h>
#include <ODROID_Si70xx.h>
#include <Adafruit_ILI9340.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_Sensor.h>

// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8

const char version[] = "v1.3";

uint8_t ledPin = 5;
uint8_t pwm = 255;
uint8_t textSize = 2;
uint8_t rotation = 1;

float battery = 0;
uint8_t batState = 0;

uint16_t foregroundColor, backgroundColor;

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
ODROID_Si70xx si7020;
ODROID_Si1132 si1132;

float BMP180Temperature = 0;
float BMP180Pressure = 0;
float BMP180Altitude = 0;

float Si7020Temperature = 0;
float Si7020Humidity = 0;

float Si1132UVIndex = 0;
uint32_t Si1132Visible = 0;
uint32_t Si1132IR = 0;

void setup()
{
        Serial.begin(500000);
        Serial.println("Welcome to the WEATHER-BOARD");

        // initialize the sensors
        si1132.begin();
        bmp.begin();
        tft.begin();
        
        sensor_t sensor;
        bmp.getSensor(&sensor);

        // initialize the digital pins
	initPins();

	analogReference(INTERNAL);

        //Timer one setting
        Timer1.initialize(200000);
        Timer1.attachInterrupt(timerCallback);

        tft.setRotation(rotation);
        tft.setTextSize(textSize);
        tft.setCursor(50, 50);
        tft.print("Hello WEATHER-BOARD!");
        tft.setCursor(250, 200);
        tft.print(version);
        delay(1000);
        tft.fillScreen(backgroundColor);

	displayLabel();
}

void displayLabel()
{
        tft.setCharCursor(0, 1);
        tft.setTextColor(ILI9340_GREEN, backgroundColor);
        tft.println("Si7020");
        tft.setTextColor(ILI9340_MAGENTA, backgroundColor);
        tft.println("Temp : ");
        tft.println("Humidity :");
        tft.setCharCursor(0, 5);
        tft.setTextColor(ILI9340_GREEN, backgroundColor);
        tft.println("Si1132");
        tft.setTextColor(ILI9340_MAGENTA, backgroundColor);
        tft.println("UV Index : ");
        tft.println("Visible :");
        tft.println("IR :");
        tft.setCharCursor(0, 10);
        tft.setTextColor(ILI9340_GREEN, backgroundColor);
        tft.println("BMP180");
        tft.setTextColor(ILI9340_MAGENTA, backgroundColor);
        tft.println("Temp : ");
        tft.println("Pressure :");
        tft.println("Altitude :");
        tft.drawRect(240, 10, 70, 30, 870);
	tft.fillRect(244, 13, 14, 24, 10000);
	tft.fillRect(260, 13, 14, 24, 10000);
	tft.fillRect(276, 13, 14, 24, 10000);
	tft.fillRect(292, 13, 14, 24, 10000);
}

void initPins()
{
	pinMode(ledPin, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(7, INPUT);
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);

	analogWrite(ledPin, pwm);
}

void timerCallback()
{
	readBtn();
}

unsigned char btn0Presses = 0;
unsigned char btn0Releases = 0;
unsigned char btn1Presses = 0;
unsigned char btn1Releases = 0;
unsigned char btn2Presses = 0;
unsigned char btn2Releases = 0;

unsigned char btn0Pushed = 0;
unsigned char btn1Pushed = 0;
unsigned char btn2Pushed = 0;

void readBtn()
{
        if (!digitalRead(A1) && (btn2Presses == 0)) {
                btn2Presses = 1;
                btn2Releases = 0;
                btn2Pushed = 1;
                digitalWrite(6, LOW);
        }

        if (digitalRead(A1) && (btn2Releases == 0)) {
                btn2Releases = 1;
                btn2Presses = 0;
                btn2Pushed = 0;
                digitalWrite(6, HIGH);
        }

        if (!digitalRead(7) && (btn0Presses == 0)) {
                btn0Presses = 1;
                btn0Releases = 0;
                btn0Pushed = 1;
                if (pwm > 225)
                        pwm = 255;
                else
                        pwm += 30;
                analogWrite(ledPin, pwm);
                digitalWrite(3, LOW);
        }

        if (digitalRead(7) && (btn0Releases == 0)) {
                btn0Releases = 1;
                btn0Presses = 0;
                btn0Pushed = 0;
                digitalWrite(3, HIGH);
        }

        if (!digitalRead(A0) && (btn1Presses == 0)) {
                btn1Presses = 1;
                btn1Releases = 0;
                btn1Pushed = 1;
                if (pwm < 30)
                        pwm = 0;
                else
                        pwm -= 30;
                analogWrite(ledPin, pwm);
                digitalWrite(4, LOW);
        }

        if (digitalRead(A0) && (btn1Releases == 0)) {
                btn1Releases = 1;
                btn1Presses = 0;
                btn1Pushed = 0;
                digitalWrite(4, HIGH);
        }
}

void sendToHost()
{
        Serial.print("w0");
        Serial.print(BMP180Temperature);
        Serial.print("\ew1");
        Serial.print(BMP180Pressure);
        Serial.print("\ew2");
        Serial.print(BMP180Altitude);
        Serial.print("\ew3");
        Serial.print(Si7020Temperature);
        Serial.print("\ew4");
        Serial.print(Si7020Humidity);
        Serial.print("\ew5");
        Serial.print(Si1132UVIndex);
        Serial.print("\ew6");
        Serial.print(Si1132Visible);
        Serial.print("\ew7");
        Serial.print(Si1132IR);
        Serial.print("\ew8");
	Serial.print(battery);
	Serial.print("\e");
}

void getBMP180()
{
        sensors_event_t event;
        bmp.getEvent(&event);
        
        if (event.pressure) {
                bmp.getTemperature(&BMP180Temperature);
                BMP180Pressure = event.pressure;
                BMP180Altitude = bmp.pressureToAltitude(1025, event.pressure);
        }
}

void getSi1132()
{
        Si1132UVIndex = si1132.readUV()/100.0;
        Si1132Visible = si1132.readVisible();
        Si1132IR = si1132.readIR();
}

void getSi7020()
{
        Si7020Temperature = si7020.readTemperature();
        Si7020Humidity = si7020.readHumidity();
}

void displayBMP180()
{
        tft.setTextColor(ILI9340_CYAN, backgroundColor);
	tft.setCharCursor(7, 11);
        tft.print(BMP180Temperature);
        tft.println(" *C  ");
        delay(20);

	tft.setCharCursor(11, 12);
        tft.print(BMP180Pressure);
        tft.println(" Pa  ");
        delay(20);

	tft.setCharCursor(11, 13);
        tft.print(BMP180Altitude);
        tft.println(" meters   ");
}

void displaySi7020()
{
        tft.setTextColor(ILI9340_YELLOW, backgroundColor);
	tft.setCharCursor(7, 2);
        tft.print(Si7020Temperature);
        tft.println(" *C  ");
        delay(20);

	tft.setCharCursor(11, 3);
        tft.print(Si7020Humidity);
        tft.println(" %  \n");
        delay(20);
}

void displaySi1132()
{
	tft.setCharCursor(11, 6);
        tft.print(Si1132UVIndex);
        tft.println("  ");
        delay(20);

	tft.setCharCursor(10, 7);
        tft.print(Si1132Visible);
        tft.println(" Lux  ");
        delay(20);

	tft.setCharCursor(5, 8);
        tft.print(Si1132IR);
        tft.println(" Lux  \n");
        delay(20);
}

void batteryCheck()
{
        battery = analogRead(A2)*1.094/1024/3.9*15.9;
        tft.setCharCursor(21, 3);
        tft.print(battery);
        if (battery >= 3.95) {
                if (batState != 4) {
                        batState = 4;
                        tft.fillRect(244, 13, 14, 24, 10000);
                        tft.fillRect(260, 13, 14, 24, 10000);
                        tft.fillRect(276, 13, 14, 24, 10000);
                        tft.fillRect(292, 13, 14, 24, 10000);
                }
        } else if (battery > 3.75 && battery <= 3.95) {
                if (batState != 3) {
                        batState = 3;
                        tft.fillRect(244, 13, 14, 24, 10000);
                        tft.fillRect(260, 13, 14, 24, 10000);
                        tft.fillRect(276, 13, 14, 24, 10000);
                        tft.fillRect(292, 13, 14, 24, 0);
                }
        } else if (battery > 3.65 && battery <= 3.75) {
                if (batState != 2) {
                        batState = 2;
                        tft.fillRect(244, 13, 14, 24, 10000);
                        tft.fillRect(260, 13, 14, 24, 10000);
                        tft.fillRect(276, 13, 14, 24, 0);
                        tft.fillRect(292, 13, 14, 24, 0);
                }
        } else if (battery > 3.5 && battery <= 3.65) {
                if (batState != 1) {
                        batState = 1;
                        tft.fillRect(244, 13, 14, 24, 10000);
                        tft.fillRect(260, 13, 14, 24, 0);
                        tft.fillRect(276, 13, 14, 24, 0);
                        tft.fillRect(292, 13, 14, 24, 0);
                }
        } else if (battery <= 3.5) {
                if (batState != 0) {
                        batState = 0;
                        tft.fillRect(244, 13, 14, 24, 0);
                        tft.fillRect(260, 13, 14, 24, 0);
                        tft.fillRect(276, 13, 14, 24, 0);
                        tft.fillRect(292, 13, 14, 24, 0);
                }
        }
}

void loop(void)
{
        getBMP180();
        getSi7020();
        getSi1132();
        displayBMP180();
        displaySi7020();
        displaySi1132();        
	sendToHost();

        //If you want to use checking battery, remove the annotation
	//batteryCheck();
}

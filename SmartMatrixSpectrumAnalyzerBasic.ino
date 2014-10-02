#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include <SmartMatrix.h>
#include <FastLED.h>

SmartMatrix matrix;

AudioInputAnalog         input;
AudioAnalyzeFFT256       fft;
AudioConnection          audioConnection(input, 0, fft, 0);

// The scale sets how much sound is needed in each frequency range to
// show all 32 bars.  Higher numbers are more sensitive.
float scale = 160.0;

// An array to hold the 16 frequency bands
float level[16];

// This array holds the on-screen levels.  When the signal drops quickly,
// these are used to lower the on-screen level 1 bar per update, which
// looks more pleasing to corresponds to human sound perception.
int   shown[16];

rgb24 black = CRGB(0, 0, 0);
rgb24 red = CRGB(255, 0, 0);

byte status = 0;

void setup()
{
    Serial.begin(9600);
    //delay(1000);

    //Serial.println(F("Starting..."));

    // Initialize 32x32 LED Matrix
    matrix.begin();
    matrix.setBrightness(255);

    // Audio requires memory to work.
    AudioMemory(12);

    //Serial.println(F("Started"));
}

void loop()
{
    if (fft.available()) {
        // read the 128 FFT frequencies into 16 levels
        // music is heard in octaves, but the FFT data
        // is linear, so for the higher octaves, read
        // many FFT bins together.

        // I'm skipping the first two bins, as they seem to be unusable
        level[0] = fft.read(2);
        level[1] = fft.read(3);
        level[2] = fft.read(4);
        level[3] = fft.read(5, 6);
        level[4] = fft.read(7, 8);
        level[5] = fft.read(9, 10);
        level[6] = fft.read(11, 14);
        level[7] = fft.read(15, 19);
        level[8] = fft.read(20, 25);
        level[9] = fft.read(26, 32);
        level[10] = fft.read(33, 41);
        level[11] = fft.read(42, 52);
        level[12] = fft.read(53, 65);
        level[13] = fft.read(66, 82);
        level[14] = fft.read(83, 103);
        level[15] = fft.read(104, 127);

        matrix.fillScreen(black);

        for (int i = 0; i < 16; i++) {
            // Serial.print(level[i]);

            // TODO: conversion from FFT data to display bars should be
            // exponentially scaled.  But how keep it a simple example?
            int val = level[i] * scale;

            // trim the bars vertically to fill the matrix height
            if (val >= MATRIX_HEIGHT) val = MATRIX_HEIGHT - 1;

            if (val >= shown[i]) {
                shown[i] = val;
            }
            else {
                if (shown[i] > 0) shown[i] = shown[i] - 1;
                val = shown[i];
            }

            //Serial.print(shown[i]);
            //Serial.print(" ");

            // color based on level
            // rgb24 color = CRGB(CHSV(val * 8, 255, 255));

            // color based on band
            rgb24 color = CRGB(CHSV(i * 15, 255, 255));

            // draw the levels on the matrix
            if (shown[i] >= 0) {
                // scale the bars horizontally to fill the matrix width
                for (int j = 0; j < MATRIX_WIDTH / 16; j++) {
                    matrix.drawPixel(i * 2 + j, (MATRIX_HEIGHT - 1) - val, color);
                }
            }
        }

        matrix.swapBuffers();

        //Serial.println();

        //Serial.print(F(" cpu:"));
        //Serial.println(AudioProcessorUsageMax());

        //Serial.print(F(" free ram:"));
        //Serial.println(FreeRam());

        FastLED.countFPS();
    }
}

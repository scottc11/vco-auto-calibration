#include <mbed.h>

SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
I2C i2c(I2C_SDA, I2C_SCL);

DigitalOut led(LED1);
DigitalOut pulse(D7);    // this generates our output signwave
AnalogIn extInput(A0);

Ticker sampler;

// Amplitude     -> defined as the farthest distance the wave gets from its center/zero crossing.
// Period        -> length of one cycle of the curve
// Zero Crossing -> the common point which the wave passes its mid-point

// It brackets the zero-crossing event and uses increasingly smaller time steps to pinpoint when the zero crossing has occurred

const int sample_rate_us = 125;        // 8000hz is equal to 125us (microseconds)

const int numFreqSamples = 100;        // how many frequency calculations we want to use to obtain our average frequency prediction of the input
volatile float data[numFreqSamples];   // 
float averageFreq;                     // the running average of frewuency calculations

volatile int period;             // equal to the number of samples taken between zero crossings
volatile float frequency;        // the calculated frequency of the sine wave
int numSamplesTaken = 0;         // used in frequency calculation formula

int currValue = 0;               // the current sampled value of sinewave input
int prevValue = 0;               // the previous sampled value of sinewave input
int sampleIndex = 0;             // incrementing value to place frequency into array

int zero_crossing = 32767;       // ADC range is 0v - 3.3v, so the midpoint of the sine wave should be 1.65v (ie. 65535 / 2 = 32767)
int threshold = 500;             // for handling hysterisis
int isPositive = false;          // whether the sine wave is rising or falling


// interupt
void sampleSignal() {

  currValue = extInput.read_u16();   // convert analog voltage input (sine wave) to a 16 bit number
  

  if (currValue >= (zero_crossing + threshold) && prevValue < (zero_crossing + threshold) && isPositive) {   // maybe plus threshold zero crossing
    pulse.write(0); // is negative
    isPositive = false;
  } else if (currValue <= (zero_crossing - threshold) && prevValue > (zero_crossing - threshold) && !isPositive) {  // maybe negative threshold zero crossing
    pulse.write(1); // is positive
    period = numSamplesTaken;
    frequency = 8000 / period;
    data[sampleIndex] = frequency;
    numSamplesTaken = 0;
    sampleIndex = sampleIndex >= numFreqSamples - 1 ? 0 : sampleIndex + 1;
    isPositive = true;
  }
  
  prevValue = currValue;
  numSamplesTaken++;
}

int main() {

  sampler.attach_us(sampleSignal, sample_rate_us);

  while(1) {
    float sum;
    for (int i = 0; i < numFreqSamples; i++) {
      sum += data[i];
    }
    averageFreq = sum / numFreqSamples;
    sum = 0;
  }
}
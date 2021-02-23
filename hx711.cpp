#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <Gpio.h>
#include <unistd.h>
#include <vector>
#include <numeric>

#define CLOCK_PIN 59
#define DATA_PIN 50
#define SAMPLE_MEMORY 64
#define SPREAD 10

#define SCK_ON (clockGpio.set())
#define SCK_OFF (clockGpio.clear())
#define DT_R (dataGpio.read())
Gpio clockGpio;
Gpio dataGpio;

void reset_converter(void);
unsigned long read_cnt(long offset, int argc);
void set_gain(int r);
void setHighPri(void);

void setHighPri(void) {
  struct sched_param sched;

  memset( & sched, 0, sizeof(sched));

  sched.sched_priority = 10;
  if (sched_setscheduler(0, SCHED_FIFO, & sched))
    printf("Warning: Unable to set high priority\n");
}

void setup_gpio() {
  clockGpio.open(CLOCK_PIN, Gpio::OUTPUT);
  dataGpio.open(DATA_PIN, Gpio::INPUT);
  SCK_OFF;
}

int main(int argc, char ** argv) {
  float spread_percent = SPREAD / 100.0 / 2.0;
  std::vector<long> samples;

  setHighPri();
  setup_gpio();
  reset_converter();

  while(true) {
    long currentReading = read_cnt(0, argc);
    samples.push_back(currentReading);
    if (samples.size() > SAMPLE_MEMORY) {
      samples.erase(samples.begin());
    }

    long average = accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
    float filter_low = (float) average * (1.0 - spread_percent);
    float filter_high = (float) average * (1.0 + spread_percent);
    int cleanSamples = 0;
    long cleanSum = 0;
    for (auto s: samples) {
    	if (s > filter_low && s < filter_high) {
    	  cleanSum += s;
    	  cleanSamples++;
    	}
    }
    if (cleanSamples == 0) {
    	// we jumped a lot?
    	cleanSamples = 1;
    }
    
    printf("Reading: %ld\tSmooth avg: %ld\tSamples: %d\n", currentReading, cleanSum / cleanSamples, cleanSamples);
  }
  
  return 0;
}

void reset_converter(void) {
  SCK_ON;
  usleep(60);
  SCK_OFF;
  usleep(60);
}

void set_gain(int r) {
  int i;

  // r = 0 - 128 gain ch a
  // r = 1 - 32  gain ch b
  // r = 2 - 63  gain ch a

  while (DT_R);

  for (i = 0; i < 24 + r; i++) {
    SCK_ON;
    SCK_OFF;
  }
}

unsigned long read_cnt(long offset, int argc) {
  long count;
  int b = 0;

  count = 0;

  while (DT_R);
  b++;
  b++;
  b++;
  b++;

  for (unsigned int i = 0; i < 24; i++) {
    SCK_ON;
    count = count << 1;
    b++;
    b++;
    b++;
    b++;
    SCK_OFF;
    b++;
    b++;
    if (DT_R > 0) {
      count++;
    }
    //	b++;
    //	b++;
  }

  SCK_ON;
  b++;
  b++;
  b++;
  b++;
  SCK_OFF;
  b++;
  b++;
  b++;
  b++;
  //  count = ~0x1800000 & count;
  //  count = ~0x800000 & count;

  if (count & 0x800000) {
    count |= (long) ~0xffffff;
  }

  // if things are broken this will show actual data

  if (argc < 2) {
    for (int i = 31; i >= 0; i--) {
      printf("%d ", ((count - offset) & (1 << i)) != 0);
    }

    printf("n: %10ld     -  ", count - offset);
    printf("\n");
  }

  return (count - offset);

}

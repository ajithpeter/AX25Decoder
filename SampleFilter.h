#ifndef SAMPLEFILTER_H_
#define SAMPLEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 1058400 Hz

* 0 Hz - 8000 Hz
  gain = 5
  desired ripple = 1 dB
  actual ripple = 0.7086167650069282 dB

* 22050 Hz - 529200 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.51588642885425 dB

*/

#define SAMPLEFILTER_TAP_NUM 157

typedef struct {
  double history[SAMPLEFILTER_TAP_NUM];
  unsigned int last_index;
} SampleFilter;

void SampleFilter_init(SampleFilter* f);
void SampleFilter_put(SampleFilter* f, double input);
double SampleFilter_get(SampleFilter* f);

#endif

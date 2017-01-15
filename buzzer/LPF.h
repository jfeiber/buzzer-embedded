#ifndef LPF_H
#define LPF_H

// LPF implementation from:
// http://webcache.googleusercontent.com/search?q=cache:MoOD_M0gNtMJ:www.edn.com/design/systems-design/4320010/A-simple-software-lowpass-filter-suits-embedded-system-applications+&cd=8&hl=en&ct=clnk&gl=us

// Corresponds to how much weight battery voltage samples are given in the filter. A higher
// shift here corresponds to lower weight.
#define LPF_FILTER_SHIFT 8

static signed long filter = 0;

inline void seed_lpf(int seed_val) {
  filter = seed_val << LPF_FILTER_SHIFT;
}

inline void add_val_to_lpf(int val_to_add) {
  filter = filter - (filter >> LPF_FILTER_SHIFT) + val_to_add;
}

inline signed long get_curr_lpf_val() {
  return filter >> LPF_FILTER_SHIFT;
}

#endif

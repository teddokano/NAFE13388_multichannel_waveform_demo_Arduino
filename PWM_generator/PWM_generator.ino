#include <MsTimer2.h>

constexpr int resolution = 12;
constexpr double scale = ((int)1 << resolution) - 1;

constexpr auto period = 400;

constexpr auto pi2 = M_PI * 2.00;
constexpr auto pi4 = 2.00 * pi2;

constexpr auto phase0 = 0.00 / 3.00;
constexpr auto phase1 = 1.00 / 3.00;
constexpr auto phase2 = 2.00 / 3.00;

typedef struct _wave_setting {
  double frequency;
  double phase;
  double offset;
  double amplitude;
  int pin;
} wave_setting;

volatile bool timer_flag = false;

void timer_callback(void) {
  timer_flag = true;
}

void pwm_sin(int count, const wave_setting ws) {
  int v = (int)(scale * (ws.offset + ws.amplitude * 0.5 * sin((ws.frequency * (double)count / period) + ws.frequency * ws.phase)));
  analogWrite(ws.pin, v);
}

double biased_triangle(int count, double phase, double offset = 0.5, double amplitude = 1.0) {
  count = (count / (period / 2)) ? period - count : count;

  Serial.println(scale * (offset + amplitude * 0.5 * ((double)count / (period / 2)) * phase));

  return scale * (offset + amplitude * 0.5 * ((double)count / period) * phase);
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("8ch PWM generator");

  analogWriteResolution(resolution);

  MsTimer2::set(10, timer_callback);  // every 500 ms
  MsTimer2::start();
}

constexpr double wave_amplitude = 1.00 / 3.00;
constexpr double offset_upper = 5.00 / 6.00;
constexpr double offset_middle = 3.00 / 6.00;
constexpr double offset_lower = 1.00 / 6.00;

wave_setting waves[] = {
  { pi2, phase0, offset_upper, wave_amplitude, PWM0 },
  { pi2, phase1, offset_upper, wave_amplitude, PWM1 },
  { pi2, phase2, offset_upper, wave_amplitude, PWM2 },
  { pi4, phase0, offset_middle, wave_amplitude, PWM3 },
  { pi4, phase1, offset_middle, wave_amplitude, PWM4 },
  { pi4, phase2, offset_middle, wave_amplitude, PWM5 },
  { pi2, phase0, offset_lower, wave_amplitude, D5 },
  { pi4, phase0, offset_lower, wave_amplitude, D6 },
};

double value;

void loop() {
  if (timer_flag) {
    static int count = 0;

    for (size_t i = 0; i < sizeof(waves) / sizeof(wave_setting); i++)
      pwm_sin(count, waves[i]);

    count++;
    count %= period;

    timer_flag = false;
  }
}

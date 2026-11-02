/*
 * Sample of using MCMR(Multi Channel Multi Read) command for 8 channels
 * with conversion start by SYNC pulse (SYS_CONFIG0.ADC_SYNC = 1)
 */

#include <NAFE13388_UIM.h>

#ifdef ARDUINO_ARCH_MCX
NAFE13388_UIM afe(0, false, 3, 4, 6, 7, 4, 14);
#else
NAFE13388_UIM afe;
#endif

constexpr int SYNC_pin = D6;
constexpr bool SYNC_polarity_positive = true;
constexpr bool SYNC_pulse = SYNC_polarity_positive;

constexpr uint16_t ADC_SYNC_bit = 0x0020;   //  SYS_CONFIG0.ADC_SYNC
constexpr uint32_t DRDY_timeout_ms = 1000;  //  must be longer than one sequence (~100 ms with current config)

volatile bool drdy_flag = false;
 
void drdy_callback(void) {
  drdy_flag = true;
}

void sync_pin_setup(void) {
  pinMode(SYNC_pin, OUTPUT);
  digitalWrite(SYNC_pin, !SYNC_pulse);
}

void sync_pulse(void) {
  digitalWrite(SYNC_pin, SYNC_pulse);
  delayMicroseconds(1);  //  needs >= 2 system clocks (~434 ns)
  //delay(50);           //  to make longer pulse width
  digitalWrite(SYNC_pin, !SYNC_pulse);
}

bool wait_DRDY(uint32_t timeout_ms) {
  uint32_t start = millis();

  while (!drdy_flag) {
    if ((millis() - start) > timeout_ms)
      return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n***** Hello, NAFE13388! *****");

  SPI.begin();
  pinMode(SS, OUTPUT);  //  Required for UNO R4

  sync_pin_setup();

  if (!afe.begin()) {
    Serial.println("afe.begin() failed. Check power supply or pin connections");
    while (true)
      ;
  }
  afe.blink_leds();

  constexpr uint16_t ch_congig1 = 0x0084;

  afe.logical_channel[0].configure(0x1710, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[1].configure(0x7110, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[2].configure(0x2710, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[3].configure(0x7210, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[4].configure(0x3710, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[5].configure(0x7310, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[6].configure(0x4710, ch_congig1, 0x8400, 0x0000);
  afe.logical_channel[7].configure(0x7410, ch_congig1, 0x8400, 0x0000);

  Serial.println("\n");
  Serial.println("logical channel 0 = (AI1P-AICOM)");
  Serial.println("logical channel 1 = (AI1N-AICOM)");
  Serial.println("logical channel 2 = (AI2P-AICOM)");
  Serial.println("logical channel 3 = (AI2N-AICOM)");
  Serial.println("logical channel 4 = (AI3P-AICOM)");
  Serial.println("logical channel 5 = (AI3N-AICOM)");
  Serial.println("logical channel 6 = (AI4P-AICOM)");
  Serial.println("logical channel 7 = (AI4N-AICOM)");

  Serial.print("\r\nenabled logical channel(s) = ");
  Serial.println(afe.enabled_logical_channels());

  afe.DRDY_by_sequencer_done(true);  //  DRDY once at the end of the sequence
  afe.bit_op(NAFE13388_UIM::Register16::SYS_CONFIG0, ~ADC_SYNC_bit, ADC_SYNC_bit);
  Serial.printf("SYS_CONFIG0 = 0x%04X\r\n", afe.reg(NAFE13388_UIM::Register16::SYS_CONFIG0));

  //  Use own callback instead of library's default one.
  //  The library's DRDY wait is not accessible from sketch, so the flag is handled here.
  //  Note: the interrupt is attached with CHANGE, so this is called on both edges of DRDY.
  afe.set_DRDY_callback(drdy_callback);
}

void loop() {
  NAFE13388_UIM::volt_t data[8];

  afe.command(NAFE13388_UIM::Command::CMD_MM);  //  select MCMR mode (conversion does not start yet)
  delay(1);                                     //  allow both edges of any old DRDY to pass

  drdy_flag = false;
  sync_pulse();  //  conversion starts here

  if (!wait_DRDY(DRDY_timeout_ms)) {
    Serial.println("DRDY wait timed out");
    return;
  }

  afe.read(data);  //  burst-read CH_DATA of enabled channels and convert to volt

  for (auto i = 0; i < 8; i++) {
    Serial.print(data[i]);
    Serial.print(",  ");
  }

  Serial.println("");
}

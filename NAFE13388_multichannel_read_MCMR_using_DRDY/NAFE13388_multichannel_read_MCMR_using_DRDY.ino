/* 
 * NAFE13388-UIM board operation sample for Arduino
 * 
 *  Copyright: 2023 - 2026 Tedd OKANO
 *  Released under the MIT license
 *
 * Sample of using MCMR(Multi Channel Multi Read) command for 8 channels
 *
 * *****************************
 * ** BEFORE TRYING THIS CODE **
 * *****************************
 * This sample code shows operation with **DRDY** signal (at D4 pin) from AFE 
 * Since the DRDY signal is very short pulse, it will be captured by interrupt on D2 pin
 * Short D4 and D2 pin to handle it
 */

/*
  * ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION **
  * ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION **
  * ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION ** CAUTION **
  * 
  * The NAFE13388-UIM board and Arduino mocrocontroller cannot be connected 
  * directly on Arduino-shield socket. The 3.3V supply should be disconnected. 
  * Visit next URL page to confirm how to do it. 
  *   -->  https://github.com/teddokano/AFE_NXP_Arduino/blob/main/README.md
  */

#include <NAFE13388_UIM.h>

#ifdef ARDUINO_ARCH_MCX
NAFE13388_UIM afe(0, false, 3, 4, 6, 7, 4, 14);
#else
NAFE13388_UIM afe;
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n***** Hello, NAFE13388! *****");

  SPI.begin();
  pinMode(SS, OUTPUT);  //  Required for UNO R4

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

  afe.use_DRDY_trigger(true);
  afe.DRDY_by_sequencer_done(true);
}

void loop() {
  NAFE13388_UIM::volt_t data[8];

  //  start_and_read() returns false if the DRDY signal never arrived. The
  //  library fills the array with NAN in that case, so the values are never
  //  silently stale -- but checking the return value says why.
  if (!afe.start_and_read(data)) {
    Serial.println("DRDY wait timed out. Check the DRDY signal connection");
    return;
  }

  for (auto i = 0; i < 8; i++) {
    Serial.print(data[i]);
    Serial.print(",  ");
  }

  Serial.println("");
}

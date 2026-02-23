#include "Zigbee.h"

#ifndef ZIGBEE_MODE_ZCZR
#error "Zigbee coordinator/router mode is not selected"
#endif

// Han Headers ###

uint8_t hanCFG = 99;   // serial stop bits.
uint8_t hanEB = 99;    // mono or tri..
uint8_t subType = 99;  // if meter reply
                       // to L1 L2 L3 in mono.
uint16_t hanERR = 0;
bool hanWork = false;
uint32_t hanDelay = 0;
uint16_t hanDelayWait = 1000;  // Default delay

bool hJanz = true;
uint32_t hanDelayError = 5000;  // Janz GPRS
                                // need 35000ms.

uint16_t hTimeout = 1500;  // 1500: Some  meters
                           // are slow to reply.

uint8_t hanIndex = 0;  // 0 = setup
uint32_t hanRead = 0;
uint8_t hanCode = 0;
uint8_t hRestart = 0;

// Clock 01
uint16_t hanYY = 0;
uint8_t hanMT = 0;
uint8_t hanDD = 0;

uint8_t hanHH = 0;
uint8_t hanMM = 0;
uint8_t hanSS = 0;

// Voltage Current 6C
// mono
uint16_t hanVL1 = 0;
uint16_t hanCL1 = 0;
// tri
uint16_t hanVL2 = 0;
uint16_t hanCL2 = 0;
uint16_t hanVL3 = 0;
uint16_t hanCL3 = 0;
uint16_t hanCLT = 0;

// Total Energy T (kWh) 26
float hanTET1 = 0;
float hanTET2 = 0;
float hanTET3 = 0;

// Total Energy (kWh) 16
float hanTEI = 0;
float hanTEE = 0;

// Total Energy L1 L2 L3 (kWh) 1C
float hTEIL1 = 0;
float hTEIL2 = 0;
float hTEIL3 = 0;

// Active Power Import/Export 73
// tri
int16_t hanAPI1 = 0;
int16_t hanAPE1 = 0;
int16_t hanAPI2 = 0;
int16_t hanAPE2 = 0;
int16_t hanAPI3 = 0;
int16_t hanAPE3 = 0;
// mono / tri total. (79)
int16_t hanAPI = 0;
int16_t hanAPE = 0;

// Power Factor (7B) / Frequency (7F)
float hanPF = 0;
float hanPF1 = 0;
float hanPF2 = 0;
float hanPF3 = 0;
uint16_t hanFR = 0;

// Load Profile

// Misc

uint8_t hICP = 9;
float hCT1 = 0;
float hCT4 = 0;
uint8_t hTariff = 0;
char hCiclo[12];

char hErrTime[12];
char hErrCode[12];

char hStatus[12];
uint32_t hPerf[2] = {0, 0};
uint32_t hMnfC = 0;
uint16_t hMnfY = 0;

uint8_t nsMo = 99;
float nsIkw = 0;
float nsEkw = 0;
float nsQs = 0;

uint32_t hWtdT = 0;

uint8_t hNick = 0;
int32_t hFreeDS = 0;

// **********************

#include <ModbusMaster.h>

#ifdef ESP32C6
#undef HAN_DIR
#define HAN_DIR 3
#undef HAN_TX
#define HAN_TX 5
#undef HAN_RX
#define HAN_RX 4
#undef HAN_SERIAL
#define HAN_SERIAL Serial1
#endif

ModbusMaster node;

void preTransmission() { digitalWrite(HAN_DIR, 1); }
void postTransmission() { digitalWrite(HAN_DIR, 0); }

void hanBlink() {
//
#ifdef ESP8266
  digitalWrite(2, LOW);
  delay(50);
  digitalWrite(2, HIGH);
//
#elif ESP32C6
  digitalWrite(2, HIGH);
  delay(50);
  digitalWrite(2, LOW);
//
#elif ESP32S3
  digitalWrite(39, LOW);
  delay(50);
  digitalWrite(39, HIGH);
//
#endif
}

void ledOn() {
//
#ifdef ESP8266
  digitalWrite(2, LOW);
//
#elif ESP32C6
  digitalWrite(2, HIGH);
//
#elif ESP32S3
  digitalWrite(39, LOW);
//
#endif
}

void setDelayError(uint8_t hanRes) {
  sprintf(hStatus, "Error");
  hanCode = hanRes;
  Serial.printf("HAN: Error! %d\r\n", hanCode);
  //
  ledOn();
  //
  hanDelay = hanDelayError;
  //
}

// ###
// Han Headers ###

/* Zigbee AC measurement device configuration */
#define AC_ELECTRICAL_MEASUREMENT_ENDPOINT_NUMBER 1

uint8_t button = BOOT_PIN;

ZigbeeElectricalMeasurement zbElectricalMeasurement =
    ZigbeeElectricalMeasurement(
        AC_ELECTRICAL_MEASUREMENT_ENDPOINT_NUMBER);

void setup() {
  //

  Serial.begin(115200);

  delay(1000);
  delay(1000);

  Serial.printf("[%d] Starting...\r\n", millis());

  // Han Setup ###

  Serial.printf("[%d] HAN: Init...\r\n", millis());

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  pinMode(HAN_DIR, OUTPUT);
  digitalWrite(HAN_DIR, LOW);

  //

  sprintf(hStatus, "Init");
  hanRead = millis() + 5000;
  hWtdT = millis();
  //

  // end HanInit
  // Han Setup ###

  // Init button switch
  pinMode(button, INPUT_PULLUP);

  // Optional: set Zigbee device name and model
  zbElectricalMeasurement.setManufacturerAndModel(
      "easyhan.pt", "HanBeeC6");

  // Add analog clusters to Zigbee Analog according your
  // needs
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
      ZIGBEE_AC_PHASE_TYPE_A);
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
      ZIGBEE_AC_PHASE_TYPE_A);
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
      ZIGBEE_AC_PHASE_TYPE_A);

  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
      ZIGBEE_AC_PHASE_TYPE_B);
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
      ZIGBEE_AC_PHASE_TYPE_B);
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
      ZIGBEE_AC_PHASE_TYPE_B);

  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
      ZIGBEE_AC_PHASE_TYPE_C);
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
      ZIGBEE_AC_PHASE_TYPE_C);
  zbElectricalMeasurement.addACMeasurement(
      ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
      ZIGBEE_AC_PHASE_TYPE_C);

  // Recommended: set Multiplier/Divisor for the
  // measurements (common for all phases)

  zbElectricalMeasurement.setACMultiplierDivisor(
      ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE, 1, 10);
  // 1/10 = 0.1V

  zbElectricalMeasurement.setACMultiplierDivisor(
      ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT, 1, 10);
  // 1/10 = 0.1A

  zbElectricalMeasurement.setACMultiplierDivisor(
      ZIGBEE_AC_MEASUREMENT_TYPE_POWER, 1, 1);
  // 1/1 = 1W

  // #########

  // #########

  // Add endpoints to Zigbee Core
  Zigbee.addEndpoint(&zbElectricalMeasurement);

  // #########

  Serial.println("Starting Zigbee...");
  // When all EPs are registered, start Zigbee in Router
  // mode
  if (!Zigbee.begin(ZIGBEE_ROUTER)) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  } else {
    Serial.println("Zigbee started!");
  }
  Serial.println("Connecting to network...");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected! ");
}

void loop() {
  //

  // Han Loop ###

  uint32_t _millis = millis();

  if (((_millis - hWtdT) > 600000) & (_millis > 300000)) {
    Serial.println("HAN: Modbus Watchdog. Restarting...");
    ESP.restart();
  }

  //

  if (hanRead + hanDelay < _millis) {
    hanWork = true;
    node.clearTransmitBuffer();
    delay(50);
    node.clearResponseBuffer();
    delay(50);
    node.setTimeout(hTimeout);
    //
    Serial.printf("HAN: Index %d !\r\n", hanIndex);
    //
  }

  // # # # # # # # # # #
  // EASYHAN MODBUS BEGIN
  // # # # # # # # # # #

  uint8_t hRes;

  // # # # # # # # # # #
  // Setup: Serial
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 0)) {
    //
    // Detect Stop Bits

    node.setTimeout(1500);

    HAN_SERIAL.flush();
    HAN_SERIAL.end();
    delay(250);
#ifdef ESP8266
    HAN_SERIAL.begin(9600, SERIAL_8N1);
#else
    HAN_SERIAL.begin(9600, SERIAL_8N1, HAN_RX, HAN_TX);
#endif
    delay(250);
    node.begin(1, HAN_SERIAL);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);

    delay(250);

    uint8_t testserial;

    node.clearResponseBuffer();
    testserial = node.readInputRegisters(0x0001, 1);
    if ((testserial == 0x00) | (testserial == 0x81)) {
      hanBlink();
      hanCFG = 1;
      hanIndex++;
      hanDelay = hanDelayWait;
      Serial.print("HAN: *** 8N1 OK ***\r\n");
    } else {
      hanCode = testserial;
      Serial.printf("HAN: 8N1 Fail. Error %d\r\n",
                    hanCode);
      //
      HAN_SERIAL.flush();
      HAN_SERIAL.end();
      delay(250);
#ifdef ESP8266
      HAN_SERIAL.begin(9600, SERIAL_8N2);
#else
      HAN_SERIAL.begin(9600, SERIAL_8N2, HAN_RX, HAN_TX);
#endif
      delay(250);
      node.begin(1, HAN_SERIAL);
      node.preTransmission(preTransmission);
      node.postTransmission(postTransmission);

      delay(250);
      //
      node.clearResponseBuffer();
      testserial = node.readInputRegisters(0x0001, 1);
      if ((testserial == 0x00) | (testserial == 0x81)) {
        hanBlink();
        hanCFG = 2;
        hanIndex++;
        hanDelay = hanDelayWait;
        Serial.println("HAN: *** 8N2 OK ***\r\n");
      } else {
        Serial.printf("HAN: 8N2 Fail. Error %d\r\n",
                      hanCode);
        setDelayError(testserial);
      }
      //
    }
    //
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // Setup: EB
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 1)) {
    //
    // Detect EB Type

    uint8_t testEB = 0xff;
    uint16_t hanDTT = 0;

    testEB = node.readInputRegisters(0x006E, 1);
    if (testEB == 0x00) {
      //
      hanBlink();
      hanDTT = node.getResponseBuffer(0);
      if (hanDTT > 0) {
        hanEB = 3;
        subType = 3;
        hanIndex++;
        Serial.printf("HAN: *** EB3 *** %d / %d ***\r\n",
                      testEB, hanDTT);
      } else {
        hanEB = 1;
        subType = 3;
        hanIndex++;
        Serial.printf("HAN: *** EB1 *** %d / %d ***\r\n",
                      testEB, hanDTT);
      }
      //
    } else if ((testEB == 0x02) || (testEB == 0x04)) {
      hanEB = 1;
      subType = 1;
      hanIndex++;
      Serial.printf("HAN: *** EB1 *** %d ***\r\n",
                    testEB);
    } else {
      Serial.printf("HAN: *** Error *** %d ***\r\n",
                    testEB);
      setDelayError(testEB);
      hanIndex = 0;
      hanERR++;
    }
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // EMI Info
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 2)) {
    hRes = node.readInputRegisters(0x0003, 1);
    if (hRes == node.ku8MBSuccess) {
      hWtdT = millis();  // feed han wtd
      hMnfC = node.getResponseBuffer(1) |
              node.getResponseBuffer(0) << 16;
      hMnfY = node.getResponseBuffer(2);

      // ###

      switch (hMnfC) {
        case 6619395:  // M Janz GPRS
          if (hJanz) {
            hanDelayError = 35000;
            hJanz = false;
          }
          Serial.println("HAN: *** M Janz Tweak ***");
          break;
        case 6623491:  // T Janz GPRS
          if (hJanz) {
            hanDelayError = 35000;
            hJanz = false;
          }
          hanEB = 3;
          subType = 3;
          Serial.println("HAN: *** T Janz Tweak ***");
          break;
        case 6754306:   // T Landis+Gyr S3
        case 6754307:   // T Landis+Gyr S5
        case 11014146:  // T Sagem CX2000-9
        case 16977920:  // T Ziv 5CTD E2F
          hanEB = 3;
          subType = 3;
          Serial.printf("HAN: *** Force EB%d / %d ***",
                        hanEB, subType);
          break;
      }

      // ###

      hanBlink();
      hanDelay = hanDelayWait;
      hanIndex++;
    } else {
      hanERR++;
      setDelayError(hRes);
    }
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // Contract
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 3)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // LP ID
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 4)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // Clock ( 12 bytes )
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 5)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // Voltage Current
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 6)) {
    if (hanEB == 3) {
      hRes = node.readInputRegisters(0x006c, 7);
      if (hRes == node.ku8MBSuccess) {
        hWtdT = millis();  // feed han wtd
        hanVL1 = node.getResponseBuffer(0);
        hanCL1 = node.getResponseBuffer(1);
        hanVL2 = node.getResponseBuffer(2);
        hanCL2 = node.getResponseBuffer(3);
        hanVL3 = node.getResponseBuffer(4);
        hanCL3 = node.getResponseBuffer(5);
        hanCLT = node.getResponseBuffer(6);

        // set

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_A, hanVL1);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_A, hanCL1);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_B, hanVL2);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_B, hanCL2);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_C, hanVL3);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_C, hanCL3);

        // report

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_A);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_A);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_B);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_B);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_C);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_C);

        hanBlink();
        hanDelay = hanDelayWait;
        hanIndex++;
      } else {
        hanERR++;
        setDelayError(hRes);
      }
    } else {
      hRes = node.readInputRegisters(0x006c, 2);
      if (hRes == node.ku8MBSuccess) {
        hanVL1 = node.getResponseBuffer(0);
        hanCL1 = node.getResponseBuffer(1);

        // set

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_A, hanVL1);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_A, hanCL1);

        // report

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_VOLTAGE,
            ZIGBEE_AC_PHASE_TYPE_A);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_CURRENT,
            ZIGBEE_AC_PHASE_TYPE_A);

        hanBlink();
        hanDelay = hanDelayWait;
        hanIndex++;
      } else {
        hanERR++;
        setDelayError(hRes);
      }
    }
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // Active Power Import/Export 73 (tri)
  // Power Factor (mono) (79..)
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 7)) {
    if (hanEB == 3) {
      hRes = node.readInputRegisters(0x0073, 8);
      if (hRes == node.ku8MBSuccess) {
        hWtdT = millis();  // feed han wtd
        hanAPI1 = node.getResponseBuffer(1) |
                  node.getResponseBuffer(0) << 16;
        hanAPE1 = node.getResponseBuffer(3) |
                  node.getResponseBuffer(2) << 16;
        hanAPI2 = node.getResponseBuffer(5) |
                  node.getResponseBuffer(4) << 16;
        hanAPE2 = node.getResponseBuffer(7) |
                  node.getResponseBuffer(6) << 16;
        hanAPI3 = node.getResponseBuffer(9) |
                  node.getResponseBuffer(8) << 16;
        hanAPE3 = node.getResponseBuffer(11) |
                  node.getResponseBuffer(10) << 16;
        hanAPI = node.getResponseBuffer(13) |
                 node.getResponseBuffer(12) << 16;
        hanAPE = node.getResponseBuffer(15) |
                 node.getResponseBuffer(14) << 16;

        // set 1

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_A, hanAPI1);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_B, hanAPI2);

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_C, hanAPI3);

        // report 1

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_A);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_B);

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_C);

        hanBlink();
        hanDelay = hanDelayWait;
        hanIndex++;
        sprintf(hStatus, "OK");
      } else {
        hanERR++;
        setDelayError(hRes);
      }
    } else {
      hRes = node.readInputRegisters(0x0079, 3);
      if (hRes == node.ku8MBSuccess) {
        hanAPI = node.getResponseBuffer(1) |
                 node.getResponseBuffer(0) << 16;
        hanAPE = node.getResponseBuffer(3) |
                 node.getResponseBuffer(2) << 16;
        hanPF = node.getResponseBuffer(4) / 1000.0;

        // set 1

        zbElectricalMeasurement.setACMeasurement(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_A, hanAPI);

        // report 1

        zbElectricalMeasurement.reportAC(
            ZIGBEE_AC_MEASUREMENT_TYPE_POWER,
            ZIGBEE_AC_PHASE_TYPE_A);

        hanBlink();
        hanDelay = hanDelayWait;
        hanIndex++;
      } else {
        hanERR++;
        setDelayError(hRes);
      }
    }
    //
    //
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // Power Factor (7B) / Frequency (7F)
  // Power Factor (tri)
  // Frequency (mono)
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 8)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // Total Energy Tarifas (kWh) 26
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 9)) {
    hPerf[0] = millis();
    hRes = node.readInputRegisters(0x0026, 3);
    if (hRes == node.ku8MBSuccess) {
      hPerf[1] = millis() - hPerf[0];
      hanTET1 = (node.getResponseBuffer(1) |
                 node.getResponseBuffer(0) << 16) /
                1000.0;
      hanTET2 = (node.getResponseBuffer(3) |
                 node.getResponseBuffer(2) << 16) /
                1000.0;
      hanTET3 = (node.getResponseBuffer(5) |
                 node.getResponseBuffer(4) << 16) /
                1000.0;
      hanBlink();
      hanDelay = hanDelayWait;
      hanIndex++;
      sprintf(hStatus, "OK");
    } else {
      hanERR++;
      setDelayError(hRes);
    }
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // Total Energy (total) (kWh) 16
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 10)) {
    hRes = node.readInputRegisters(0x0016, 2);
    if (hRes == node.ku8MBSuccess) {
      hWtdT = millis();  // feed han wtd
      hanTEI = (node.getResponseBuffer(1) |
                node.getResponseBuffer(0) << 16) /
               1000.0;
      hanTEE = (node.getResponseBuffer(3) |
                node.getResponseBuffer(2) << 16) /
               1000.0;
      hanBlink();
      hanDelay = hanDelayWait;
      hanIndex++;
      //
      // netSaldo();
      //
    } else {
      hanERR++;
      setDelayError(hRes);
    }
    hanRead = millis();
    hanWork = false;
  }

  // # # # # # # # # # #
  // Total Energy (L1 L2 L3) (kWh) 1C
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 11)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // Reserved
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 12)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // Load Profile
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 13)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // Ciclo / Tariff
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 14)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // ICP
  // # # # # # # # # # #

  if (hanWork & (hanIndex == 15)) {
    hanIndex++;
  }

  // # # # # # # # # # #
  // EASYHAN MODBUS EOF
  // # # # # # # # # # #

  if (hanIndex > 10) {
    hanIndex = 5;  // skip setup and one time requests.
  }

  if (hanERR > 50000) {
    hanERR = 10000;
  }

  // end loop
  // end HanDoWork
  // Han Loop ###

  // Checking button for factory reset and reporting
  if (digitalRead(button) ==
      LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory
        // reset Zigbee and reboot
        Serial.println(
            "Resetting Zigbee to factory and rebooting "
            "in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
  }
  delay(100);
}

// EOF

#include "Mcp23017.h"
#include "Arduino.h"
#include "Wire.h"

bool Mcp23017::i2cStarted = false;

// table that holds the allocated/defined MCP23017 boards attached to the
// the WeMos D1
Mcp23017*  Mcp23017::mcpTable[8];
bool Mcp23017::mcpDefined[] = {false, false, false, false, false, false, false, false};

const byte  Mcp23017::GPIOA = 0x12;          // gpio A group of ports register code
const byte  Mcp23017::GPIOB = 0x13;          // gpio B group of ports register code

const byte Mcp23017::IODIRA  = 0x00;  // gpio A direction of IO pin register code
const byte Mcp23017::IODIRB  = 0x01;  // gpio B direction of IO pin register code

/*
        use to set for emulate mode, does not use I2C communication
        as all states are stored in this class and not the relay board.
*/
boolean emulateMode;


Mcp23017::Mcp23017(byte i2cMcpAddress) {
  emulateMode = false;

  this->i2cMcpAddress = i2cMcpAddress;

  this->gpioStateA = 0xff;
  this->gpioStateB = 0xff;

  this->gpioStateAPrevious = 0xff;
  this->gpioStateBPrevious = 0xff;

  this->gpioSateAWritten = false;
  this->gpioSateBWritten = false;

  this->mcpInitialized = false;

  byte i2cMcpAddressBase = i2cMcpAddress - 0x20;
  mcpTable[i2cMcpAddressBase] = this;
  mcpDefined[i2cMcpAddressBase] = true;
}

Mcp23017::Mcp23017() {
}


String Mcp23017::gpioAToBinaryString() {
  return Mcp23017::gpioToBinary(this->gpioStateA);
}
String  Mcp23017::gpioBToBinaryString() {
  return Mcp23017::gpioToBinary(gpioStateB);
}

String Mcp23017::writeRelaysA() {
  return Mcp23017::writeRelays(GPIOA, this->gpioStateA,
                               this->gpioSateAWritten, this->gpioStateAPrevious);
}
String Mcp23017::writeRelaysB() {
  return Mcp23017::writeRelays(GPIOB, this->gpioStateB,
                               this->gpioSateBWritten, this->gpioStateBPrevious);
}

String Mcp23017::gpioToBinary(byte byte2Bin) {
  // mcp23017 pins are wired 7 -> 0 where the relay boards
  // are wired 1 -> 8 (that is: if wire is to connect
  // the boards as adjacent and not have twisted wires
  // the pins on the mcp23017 are reversed mapped)
  //
  //     relay board pins    12345678
  //                         ||||||||   wire connection
  //     mcp23017 pins       76543210
  //
  byte i;
  byte bitN = 0b10000000; // will be shifted to map each bit of a byte
  String bitArr = "00000000";

  for (i = 8; i > 0; i--) {
    // gpioState is inverse for on/off as on is low-pwr and off is hi-pwr
    char c = (byte2Bin & bitN) ? '0' : '1';
    bitArr.setCharAt(8 - i, c);
    bitN = bitN >> 1;
  }
  return bitArr;
}

// ----------------------------------------------------------
// I2C communication: for MCP23017
// ----------------------------------------------------------
boolean Mcp23017::connectUpI2c() {
  if (!i2cStarted) {
    // setup the I2C communications channel in preparation as the
    // master (ensure the scl, sda and gnd wires are connected, if GND
    // is not commoned across the boards, the scl/sda will float and weird
    // things happen)
    //
    Wire.setClock(100000); // being explicit just makes it consistent for all slaves
    Wire.begin(); // wake up I2C bus

    // when the MCP23017 power-on's (alongside the Arduino board) the GPIO are inputs,
    // which to the 8-ch relay board are set low (the relay tiggers), so a moment of
    // relay activation occurs. However, a small delay between power-on and setting the
    // GPIO to outputs HI being small will allow:
    //
    // this delay to the Gpio setup is so small, the LED's on the relay-board flash
    // but the relays do not throw [that is: make-contact]
    delay(150);

    i2cStarted = true;
  }
  // set I/O pins to outputs
  //
  // assume all MCP23017 devices have been defined prior to this and only need
  // initializing
  boolean aFalse = false;

  int i;
  for (i = 0; i < 8; i++) {
    if (mcpDefined[i]) {
      if (!mcpTable[i]->setAllGpioToOutputs()) {
        aFalse = true;;
      }
    }
  }
  return !aFalse;
  //  return setAllGpioToOutputs();
}

boolean Mcp23017::setAllGpioToOutputs() {
  // set I/O pins to outputs
  if (writeOverI2c(i2cMcpAddress,
                   IODIRA,     // IODIRA register
                   (byte)0x00) // set all of bank A to outputs
      != 0) return false;

  if (writeOverI2c(i2cMcpAddress,
                   GPIOA, // address bank A
                   (byte)0xFF) // value to send all HIGH
      != 0) return false;

  if (writeOverI2c(i2cMcpAddress,
                   IODIRB, // IODIRA register
                   (byte)0x00) // set all of bank B to outputs
      != 0) return false;

  if (writeOverI2c(i2cMcpAddress,
                   GPIOB, // address bank B
                   (byte)0xFF) // value to send all HIGH
      != 0) return false;

  // allow the MCP23017 chip to settle
  delay(10);
  return true;
}
// ----------------------------------------------------------
// MCP23017 interface and relay board control I/F for
// HTTP or alternative IO action
// ----------------------------------------------------------

void Mcp23017::gpioAChange() {

  if (this->gpioSateAWritten) {
    this->gpioSateAWritten = false;
    this->gpioStateAPrevious = this->gpioStateA;
  }
}
void Mcp23017::gpioBChange() {
  if (this->gpioSateBWritten) {
    this->gpioSateBWritten = false;
    this->gpioStateBPrevious = this->gpioStateB;
  }
}

void Mcp23017::onRAll(bool grpA) {
  if (grpA) {
    gpioAChange();
    this->gpioStateA = 0x00;
  } else {
    gpioBChange();
    this->gpioStateB = 0x00;
  }
}

void Mcp23017::offRAll(bool grpA) {
  if (grpA) {
    gpioAChange();
    this->gpioStateA = 0xff;
  } else {
    gpioBChange();
    this->gpioStateB = 0xff;
  }
}

void Mcp23017::onR(bool grpA, int rNum) {
  if (grpA) {
    gpioAChange();
    bitClear(this->gpioStateA, 8 - rNum);
  } else {
    gpioBChange();
    bitClear(this->gpioStateB, 8 - rNum);
  }
}

void Mcp23017::offR(bool grpA, int rNum) {
  if (grpA) {
    gpioAChange();
    bitSet(this->gpioStateA, 8 - rNum);
  } else {
    gpioBChange();
    bitSet(this->gpioStateB, 8 - rNum);
  }
}

String Mcp23017::writeRelays(byte mcpGroup, byte& current,
                             boolean stateWritten, byte& previousGpio) {
  // write relays will not work unless the MCP23017
  // has been initialized for the ports to be outputs
  //
  if (!this->mcpInitialized) {
    this->mcpInitialized = setAllGpioToOutputs();

    if (!this->mcpInitialized) {
      return "I2C/MCP initialization issue";
    }
  }
  //
  int ret = writeOverI2c(i2cMcpAddress,
                         mcpGroup,  // address bank A
                         current);

  String stateWrittenError = "";
  if (ret != 0) {
    Serial.printf("Failure code: %d : writeRelays\n", ret);
    current = previousGpio;

    String retStr = String(ret);

    stateWrittenError = "I2C communication error " + retStr + ", check cabling";
  }
  previousGpio = current;

  return stateWrittenError;
}

/**
   write to a MCP23017 group the data byte.

   @return the code from the end-trnsmission (success = 0)
*/
int Mcp23017::writeOverI2c(byte i2cMcpAddress, byte mcpGroup, byte data) {
  if (emulateMode) {
    return 0;
  }
  Wire.beginTransmission(i2cMcpAddress);
  Wire.write(mcpGroup); // address bank A
  Wire.write(data);
  return Wire.endTransmission();
}

#ifndef Mcp23017_h
#define Mcp23017_h

// low is relay on, high is relay off
#define rON 0
#define rOFFb 1

// the #include statment and code go here...
#include "Arduino.h"

class Mcp23017 {
  public:
    // I2C Address of MCP23017 Chip x20-x27 allowed
    Mcp23017(byte i2cMcpAddress);
    Mcp23017();

    static boolean connectUpI2c();

    String gpioAToBinaryString();
    String gpioBToBinaryString();

    // ----------------------------------------------------------
    // MCP23017 interface and relay board control I/F for
    // HTTP or alternative IO action
    // ----------------------------------------------------------
    void onRAll(bool grpA); // group-A otherwise group-B
    void offRAll(bool grpA);
    void onR(bool grpA, int rNum);
    void offR(bool grpA, int rNum);

    String writeRelaysA();
    String writeRelaysB();

    /*
        use to set for emulate mode, does not use I2C communication
        as all states are stored in this class and not the relay board.
    */
    boolean emulateMode;

  private:
    // table that holds the allocated/defined MCP23017 boards attached to the
    // the WeMos D1
    static Mcp23017* mcpTable[8];

    static bool mcpDefined[];

    static bool i2cStarted;

    boolean mcpInitialized;

    byte i2cMcpAddress;

    static const byte  GPIOA ;          // gpio A group of ports register code
    static const byte  GPIOB ;          // gpio B group of ports register code

    static const byte IODIRA ;  // gpio A direction of IO pin register code
    static const byte IODIRB;   // gpio B direction of IO pin register code

    // these will be altered by the application via the HTTP or other avenues
    byte gpioStateA;
    byte gpioStateB;

    byte gpioStateAPrevious;
    byte gpioStateBPrevious;

    boolean gpioSateAWritten;
    boolean gpioSateBWritten;

    boolean setAllGpioToOutputs();

    String gpioToBinary(byte byte2Bin);
    String writeRelays(byte mcpGroup, byte& current, boolean stateWritten, byte& previousGpio);

    void gpioAChange();
    void gpioBChange();
    int writeOverI2c(byte i2cMcpAddress, byte mcpGroup, byte data);
};

#endif

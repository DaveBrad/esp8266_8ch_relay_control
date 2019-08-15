#ifndef SerialEE_h
#define SerialEE_h

//#include "Strings.h"
#include "Arduino.h"

//

class SerialEE {
  public:
    SerialEE();

    void print(char* chr);
    void println(char* chr);

    void printc(char chr);
    void printlnc(char chr);


    void print(String str);
    void println(String str);

    void print(int intI);
    void println(int intI);

    String toString();

  private:
    char buffEmu[1000];
    int buffEmuPtr;

    void printlnLF();

};

#endif

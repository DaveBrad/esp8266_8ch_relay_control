#include "SerialEE.h"
//#include "Strings.h"
#include "Arduino.h"

char buffEmu[1000];

int buffEmuPtr;

SerialEE::SerialEE() {
  buffEmuPtr = 0;
  SerialEE::buffEmu[buffEmuPtr] = 0;
}

void SerialEE::printc(char chr) {
  if (chr == 0) {
    return;
  }
  SerialEE::buffEmu[buffEmuPtr] = chr;
  buffEmuPtr++;
  SerialEE::buffEmu[buffEmuPtr] = 0;
}


void SerialEE::print(char* chr) {
  if (chr == 0) {
    return;
  }

  int chrI = 0;
  int bufI = buffEmuPtr;

  bool endOfChr = false;

  while (!endOfChr) {

    if (chr[chrI] != 0) {
      SerialEE::buffEmu[buffEmuPtr] = chr[chrI];
      chrI++;
      buffEmuPtr++;
      SerialEE::buffEmu[buffEmuPtr] = 0;
    } else {
      endOfChr = true;
    }
  }
}

void SerialEE::print(String str) {
  int len = str.length();
  int i;

  for (i = 0; i < len; i++) {
    SerialEE::buffEmu[buffEmuPtr] = str.charAt(i);

    buffEmuPtr++;
    SerialEE::buffEmu[buffEmuPtr] = 0;
  }
}

void SerialEE::printlnc(char chr) {
  SerialEE::printc(chr);
  SerialEE::printlnLF();
}

void SerialEE::println(char* chr) {
  SerialEE::print(chr);
  SerialEE::printlnLF();
}

void SerialEE::println(String str) {
  SerialEE::print(str);
  SerialEE::printlnLF();
}

void SerialEE::printlnLF() {
  SerialEE::buffEmu[buffEmuPtr] = 13;    // cr
  buffEmuPtr++;
  SerialEE::buffEmu[buffEmuPtr] = 10;    // LF
  buffEmuPtr++;
  SerialEE::buffEmu[buffEmuPtr] = 0;
}

void SerialEE::print(int intI) {
  char cstr[16];
  itoa(intI, cstr, 10);
  SerialEE::print(cstr);
}

void SerialEE::println(int intI) {
  SerialEE::print(intI);
  SerialEE::printlnLF();
}

String SerialEE::toString() {
  String ss = String(SerialEE::buffEmu);
  buffEmuPtr = 0;
  SerialEE::buffEmu[0] = 0;

  return ss;
}

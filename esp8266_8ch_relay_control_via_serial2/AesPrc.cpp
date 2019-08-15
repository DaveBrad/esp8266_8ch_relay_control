#include "AES.h"
#include "AesPrc.h"
//#include "base64.h"

//AES AesPrc::aes ;

AesPrc::AesPrc(){
  // noting special
}

String AesPrc::dePassword(String cipherPwd64){
  char inputBuf[100];
  cipherPwd64.toCharArray(inputBuf, cipherPwd64.length() + 1);
  Serial.print("bs64 ch:");
  Serial.println(inputBuf);

  char b64Decode[100];
  b64_decode(b64Decode, inputBuf, strlen(inputBuf));
  Serial.print("bs64 de:");
  Serial.println(b64Decode);

  return String(b64Decode);
}







//byte *AesPrc::key = (unsigned char*)"0123456789010123";
//
////real iv = iv x2 ex: 01234567 = 0123456701234567
//unsigned long long int AesPrc::my_iv = 36753562;
//
//
//
//
//
//void AscPrc::prekey (int bits){
//
//  byte plain[] = "16:1234567890123456";
//  int plainLength = sizeof(plain) - 1; // don't count the trailing /0 of the string !
//  int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
//
//  aes.iv_inc();
//  byte iv [N_BLOCK] ;
//  byte plain_p[padedLength];
//  byte cipher [padedLength] ;
//  byte check [padedLength] ;
//
//  
//  unsigned long ms = micros ();
//  aes.set_IV(my_iv);
//  aes.get_IV(iv);
//  aes.do_aes_encrypt(plain, plainLength, cipher, key, bits, iv);
//  Serial.print("Encryption took: ");
//  Serial.println(micros() - ms);
//  ms = micros ();
//  aes.set_IV(my_iv);
//  aes.get_IV(iv);
//  aes.do_aes_decrypt(cipher, padedLength, check, key, bits, iv);
//  Serial.print("Decryption took: ");
//  Serial.println(micros() - ms);
//  printf("\n\nPLAIN :");
//  aes.printArray(plain, (bool)true);
//  printf("\nCIPHER:");
//  aes.printArray(cipher, (bool)false);
//  printf("\nCHECK :");
//  aes.printArray(check, (bool)true);
//  printf("\nIV    :");
//  aes.printArray(iv, 16);
//  printf("\n");
////  printf(adjust);
//  printf("     ===============================================\n");
////  adjust++;
//}
//
//
//String prekeypwd (String cipherStrB64) {
//  Serial.println("bs64 in:" + cipherStrB64);
//
//  char inputBuf[400];
//  cipherStrB64.toCharArray(inputBuf, cipherStrB64.length() + 1);
//  Serial.println("bs64 ch:" + String(inputBuf));
//
//  char b64Decode[400];
//  char b64DecodeA[400];
//  b64_decode(b64DecodeA, inputBuf, strlen(inputBuf));
//  Serial.print("bs64 de:");
//  Serial.println(b64DecodeA);
//
//  int bits = 128;
//  byte *key = (unsigned char*)"1234567890123456";
//  //  byte *key = (unsigned char*)"1234567890123456";
//  //real iv = iv x2 ex: 01234567 = 0123456701234567
//  unsigned long long int my_iv = 0; // 1234567890123456;
//
//  //  aes.aes128_dec_single(key, b64DecodeA);
//  //  Serial.print("decrypted:");
//  //  Serial.println(b64DecodeA);
//  b64_decode(b64Decode, b64DecodeA, strlen(b64DecodeA));
//
//  b64_decode(b64Decode, inputBuf, strlen(inputBuf));
//  Serial.print("bs64B en:");
//  Serial.println(b64Decode);
//
//  if (server->hasArg("notencrypt")) {
//    return String(b64DecodeA);
//  }
//
//  byte plain[] = "dddddddddddddddd";
//  int plainLength = sizeof(plain) - 1; // don't count the trailing /0 of the string !
//  int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
//
//  byte iv [N_BLOCK] ;
//  byte check [padedLength] ;
//
//  aes.set_IV(my_iv);
//  aes.get_IV(iv);
//  aes.do_aes_decrypt((byte*)b64Decode, padedLength, check, key, bits, iv);
//
//  Serial.println("\nCHECK :");
//  Serial.println((char *)check);
//
//  Serial.println("\nCHECK b :");
//  aes.printArray(check, (bool)true);
//  Serial.println("\n");
//
//  String xxxx =  String((char*)check);
//  Serial.println("pwd  is:" + xxxx);
//
//  byte cipher [padedLength] ;
//  unsigned long ms = micros ();
//  aes.set_IV(my_iv);
//  aes.get_IV(iv);
//  aes.do_aes_encrypt(plain, plainLength, cipher, key, bits, iv);
//  Serial.print("Encryption took: ");
//  Serial.println(micros() - ms);
//
//  Serial.print("\nCIPHER:");
//  aes.printArray(cipher, (bool)false);
//  //
//  //  b64_encode(b64Decode, (char*)cipher, padedLength);
//  //
//  //  Serial.print("\nCIPHER en:");
//  //  Serial.println(b64Decode);
//
//  //  return xxxx;
//
//  //  //  b64_decode(output4, output3, strlen(output3));
//  //  //  Serial.print("bs64 de2:");
//  //  //  Serial.println(output4);
//  //  //
//  //  //  byte cipherChar[200];
//  //  //  b64_decode((char*)cipherChar, output3, strlen(output3));
//  //
//  //  //  return "dldld";
//  //
//  //  int bits = 128;
//  //  //  byte *key = (unsigned char*)"dddddddd";
//  //  byte *key = (unsigned char*)"dddddddddddddddd";
//  //
//  //  ////  byte cipherChar[200];
//  //  ////  cipherStr.toCharArray((char*)cipherChar, 200);
//
//  //  //real iv = iv x2 ex: 01234567 = 0123456701234567
//  //  unsigned long long int my_iv = 36753562;
//
//  //  byte plain[] = "dddddddddddddddd";
//  //  int plainLength = sizeof(plain) - 1; // don't count the trailing /0 of the string !
//  //  int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
//  //
//  //  byte iv [N_BLOCK] ;
//  //  byte check [padedLength] ;
//  //
//  //  aes.set_IV(my_iv);
//  //  aes.get_IV(iv);
//  //  aes.do_aes_decrypt(b64Decode, padedLength, check, key, bits, iv);
//
//  //  Serial.println("\nCHECK :");
//  //  aes.printArray(check, (bool)true);
//  //  Serial.println("\nIV    :");
//  //  aes.printArray(iv, 16);
//  //  Serial.println("\n");
//  //
//  //  String xxxx =  String((char*)check);
//  //
//  //  Serial.println("pwd  is:" + xxxx);
//  //
//
//  //  byte cipher [padedLength] ;
//  //
//  //  unsigned long ms = micros ();
//  //  aes.set_IV(my_iv);
//  //  aes.get_IV(iv);
//  //  aes.do_aes_encrypt(plain, plainLength, cipher, key, bits, iv);
//  //  Serial.print("Encryption took: ");
//  //  Serial.println(micros() - ms);
//  //
//  //  Serial.print("\nCIPHER:");
//  //  aes.printArray(cipher, (bool)false);
//
//  return xxxx;
//}

//void prekey (int bits) {
//
//  byte plain[] = "16:1234567890123456";
//  char plainC[] = "16:1234567890123456";
//
//  unsigned long long int my_iv = 36753562;
//
//  int plainLength = sizeof(plain) - 1; // don't count the trailing /0 of the string !
//  int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
//  //
//  //  char output[400];
//  //  //                12345678901234567890
//  //  int xx = b64_encode(output, plainC, 19 );
//  //  Serial.println("b64:1 " + String(output));
//  //
//  //  String encoded = base64::encode(plainC);
//  //  Serial.println("b64:2 " + encoded);
//  //
//  //  char output2[400];
//  //  b64_decode(output2, output, xx );
//  //  Serial.println("b64:3 " + String(output2));
//
//
//  //  Serial.println(mybase64.result());
//
//
//  //size_t rbase64_encode(char *output, char *input, size_t inputLen);
//
//  //  rBase64generic obj = rBase64generic();
//
//  //  char output[400];
//  //  obj->encode(char *output, char plainC, plainC.sizeOf() - 1);
//  //  Serial.println(output);
//
//
//
//  //  String dencoded = base64::decode(encoded);
//  //  Serial.println(dencoded);
//
//  //  size_t rbase64_decode(char *output, char *input, size_t inputLen);
//
//  aes.iv_inc();
//  byte iv [N_BLOCK] ;
//  byte plain_p[padedLength];
//  byte cipher [padedLength] ;
//  byte check [padedLength] ;
//
//
//  unsigned long ms = micros ();
//  aes.set_IV(my_iv);
//  aes.get_IV(iv);
//  aes.do_aes_encrypt(plain, plainLength, cipher, key, bits, iv);
//  Serial.print("Encryption took: ");
//  Serial.println(micros() - ms);
//  ms = micros ();
//  aes.set_IV(my_iv);
//  aes.get_IV(iv);
//  aes.do_aes_decrypt(cipher, padedLength, check, key, bits, iv);
//  Serial.print("Decryption took: ");
//  Serial.println(micros() - ms);
//  printf("\n\nPLAIN :");
//  aes.printArray(plain, (bool)true);
//  printf("\nCIPHER:");
//  aes.printArray(cipher, (bool)false);
//  printf("\nCHECK :");
//  aes.printArray(check, (bool)true);
//  printf("\nIV    :");
//  aes.printArray(iv, 16);
//  printf("\n");
//  //  printf(adjust);
//  printf("     ===============================================\n");
//  //  adjust++;
//}

// The following code was acquired from Arduino base64 stuff
//
//
const char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "0123456789 + / ";

int AesPrc::b64_encode(char *output, char *input, int inputLen) {
  int i = 0, j = 0;
  int encLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];

  while (inputLen--) {
    a3[i++] = *(input++);
    if (i == 3) {
      a3_to_a4(a4, a3);

      for (i = 0; i < 4; i++) {
        output[encLen++] = b64_alphabet[a4[i]];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) {
      a3[j] = '\0';
    }

    a3_to_a4(a4, a3);

    for (j = 0; j < i + 1; j++) {
      output[encLen++] = b64_alphabet[a4[j]];
    }

    while ((i++ < 3)) {
      output[encLen++] = '=';
    }
  }
  output[encLen] = '\0';
  return encLen;
}


int AesPrc::b64_decode(char *output, char *input, int inputLen) {
  int i = 0, j = 0;
  int decLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];


  while (inputLen--) {
    if (*input == '=') {
      break;
    }

    a4[i++] = *(input++);
    if (i == 4) {
      for (i = 0; i < 4; i++) {
        a4[i] = b64_lookup(a4[i]);
      }

      a4_to_a3(a3, a4);

      for (i = 0; i < 3; i++) {
        output[decLen++] = a3[i];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++) {
      a4[j] = '\0';
    }

    for (j = 0; j < 4; j++) {
      a4[j] = b64_lookup(a4[j]);
    }

    a4_to_a3(a3, a4);

    for (j = 0; j < i - 1; j++) {
      output[decLen++] = a3[j];
    }
  }
  output[decLen] = '\0';
  return decLen;
}

int AesPrc::b64_enc_len(int plainLen) {
  int n = plainLen;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

int AesPrc::b64_dec_len(char * input, int inputLen) {
  int i = 0;
  int numEq = 0;
  for (i = inputLen - 1; input[i] == '='; i--) {
    numEq++;
  }

  return ((6 * inputLen) / 8) - numEq;
}

inline void AesPrc::a3_to_a4(unsigned char * a4, unsigned char * a3) {
  a4[0] = (a3[0] & 0xfc) >> 2;
  a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
  a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
  a4[3] = (a3[2] & 0x3f);
}

inline void AesPrc::a4_to_a3(unsigned char * a3, unsigned char * a4) {
  a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
  a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
  a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
}

inline unsigned char AesPrc::b64_lookup(char c) {
  int i;
  for (i = 0; i < 64; i++) {
    if (b64_alphabet[i] == c) {
      return i;
    }
  }
  return -1;
}

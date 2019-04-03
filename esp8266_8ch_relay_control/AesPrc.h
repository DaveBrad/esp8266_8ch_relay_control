#ifndef AesPrc_h
#define AesPrc_h

#include "AES.h"

class AesPrc {
  public:
    AesPrc();

    /** given the ciphered password and change to a know string */
    static String dePassword(String cipherPwd);

  private:
    //    static AES aes;

    static int b64_encode(char *output, char *input, int inputLen);
    static int b64_decode(char *output, char *input, int inputLen);

    static int b64_enc_len(int plainLen);
    static int b64_dec_len(char * input, int inputLen);

    static inline void a3_to_a4(unsigned char * a4, unsigned char * a3);
    static inline void a4_to_a3(unsigned char * a3, unsigned char * a4);
    static inline unsigned char b64_lookup(char c);
};
#endif

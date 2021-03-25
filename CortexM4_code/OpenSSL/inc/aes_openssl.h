/*
 * Copyright 2002-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_AES_H
# define HEADER_AES_H



# include <stddef.h>
# ifdef  __cplusplus
extern "C" {
# endif

# define AES_ENCRYPT     1
# define AES_DECRYPT     0

/*
 * Because array size can't be a const in C, the following two are macros.
 * Both sizes are in bytes.
 */
# define AES_MAXNR 14
# define AES_BLOCK_SIZE 16

/* This should be a hidden type, but EVP requires that the size be known */
struct AES_KEY_OSSL_st {
# ifdef AES_LONG
    unsigned long rd_key[4 * (AES_MAXNR + 1)];
# else
    unsigned int rd_key[4 * (AES_MAXNR + 1)];
# endif
    int rounds;
};
typedef struct AES_KEY_OSSL_st AES_KEY_OSSL;

const char *AES_options(void);

int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
                        AES_KEY_OSSL *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
                        AES_KEY_OSSL *key);

void AES_encrypt(const unsigned char *in, unsigned char *out,
                 const AES_KEY_OSSL *key);
void AES_decrypt(const unsigned char *in, unsigned char *out,
                 const AES_KEY_OSSL *key);

void AES_ecb_encrypt(const unsigned char *in, unsigned char *out,
                     const AES_KEY_OSSL *key, const int enc);
void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
                     size_t length, const AES_KEY_OSSL *key,
                     unsigned char *ivec, const int enc);
void AES_cfb128_encrypt(const unsigned char *in, unsigned char *out,
                        size_t length, const AES_KEY_OSSL *key,
                        unsigned char *ivec, int *num, const int enc);
void AES_cfb1_encrypt(const unsigned char *in, unsigned char *out,
                      size_t length, const AES_KEY_OSSL *key,
                      unsigned char *ivec, int *num, const int enc);
void AES_cfb8_encrypt(const unsigned char *in, unsigned char *out,
                      size_t length, const AES_KEY_OSSL *key,
                      unsigned char *ivec, int *num, const int enc);
void AES_ofb128_encrypt(const unsigned char *in, unsigned char *out,
                        size_t length, const AES_KEY_OSSL *key,
                        unsigned char *ivec, int *num);
# if !OPENSSL_API_3
/* NB: the IV is _two_ blocks long */
void AES_ige_encrypt(const unsigned char *in, unsigned char *out,
                     size_t length, const AES_KEY_OSSL *key,
                     unsigned char *ivec, const int enc);
/* NB: the IV is _four_ blocks long */
void AES_bi_ige_encrypt(const unsigned char *in, unsigned char *out,
                        size_t length, const AES_KEY_OSSL *key,
                        const AES_KEY_OSSL *key2, const unsigned char *ivec,
                        const int enc);
# endif

int AES_wrap_key(AES_KEY_OSSL *key, const unsigned char *iv,
                 unsigned char *out,
                 const unsigned char *in, unsigned int inlen);
int AES_unwrap_key(AES_KEY_OSSL *key, const unsigned char *iv,
                   unsigned char *out,
                   const unsigned char *in, unsigned int inlen);


# ifdef  __cplusplus
}
# endif

#endif

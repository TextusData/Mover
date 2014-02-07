/* RSAKey.cc -*- c++ -*-
 * Copyright (c) 2009, 2014 Ross Biro
 * Uses RSA for an assymetric key
 *
 */

#include "textus/keys/providers/RSA/RSAKey.h"
#include "textus/keys/EncryptedKey.h"
#include "textus/keys/Key.pb.h"
#include "textus/base/init/Init.h"
#include "textus/random/PRNG.h"
#include "textus/system/SysInfo.h"

#include <stdio.h>

#define RAND_SEED_SIZE 128

namespace textus { namespace keys { namespace providers { namespace RSA {

// XXXXXXXXXXXXXXX FIXME: These values are for testing. Change to something Secure before finishing.
DEFINE_INT_ARG(default_rsa_key_bits, 4096, "default_rsa_key_bits", "Default number of bits in a generared RSA key (4096).");
DEFINE_INT_ARG(default_rsa_exponent, 65537, "default_rsa_exponent", "Default exponent in a generared RSA key (65537).");
DEFINE_INT_ARG(RSA_random_seed_size, 128, "RSA_random_seed_size", "Size in bytes of the random buffer to seed the RSA random number generator with.");

extern "C" {
#include "openssl/crypto.h"
#include "openssl/ossl_typ.h"
#include "openssl/buffer.h"
#include "openssl/pem.h"
#include "openssl/rand.h"
  extern void ERR_print_errors_fp(FILE *);
  extern void ERR_load_crypto_strings();


  //#include "thirdparty/openssl/include/openssl/err.h"

  //#include "thirdparty/openssl/include/openssl/evp.h"
  //#include "thirdparty/openssl/include/openssl/engine.h"
}

#define RSA_PKCS1_OAEP_PADDING_SIZE 41
#define DEFAULT_PADDING RSA_PKCS1_OAEP_PADDING
#define DEFAULT_PADDING_SIZE RSA_PKCS1_OAEP_PADDING_SIZE

RSAKey::RSAKey(const char *d)
{
  char *b = static_cast<char *>(alloca(strlen(d)+1));
  strncpy (b, d, strlen(d)+1);
  BIO *mem = BIO_new_mem_buf(b, strlen(b));
  RSA *newKey = RSA_new();

  if (newKey == NULL) {
    BIO_free(mem);
    return;
  }

  if (PEM_read_bio_RSAPrivateKey(mem, &newKey, NULL, NULL) == NULL) {
    ERR_print_errors_fp(stderr);
    BIO_free(mem);
    RSA_free(newKey);
    return;
  }

  BIO_free(mem);
  key = newKey;
}

RSAKey::RSAKey(const string d)
{
  char *b = static_cast<char *>(alloca(d.length()));
  memcpy (b, d.c_str(), d.length());
  BIO *mem = BIO_new_mem_buf(b, d.length());
  RSA *newKey = RSA_new();

  if (newKey == NULL) {
    BIO_free(mem);
    return;
  }

  if (PEM_read_bio_RSAPrivateKey(mem, &newKey, NULL, NULL) == NULL) {
    ERR_print_errors_fp(stderr);
    BIO_free(mem);
    RSA_free(newKey);
    return;
  }

  BIO_free(mem);
  key = newKey;
}

RSAKey::RSAKey(BUF_MEM *b)
{
  BIO *mem = BIO_new_mem_buf(b->data, b->length);
  RSA *newKey = RSA_new();

  if (newKey == NULL) {
    return;
  }

  if (!PEM_read_bio_RSAPrivateKey(mem, &newKey, NULL, NULL)) {
    RSA_free(newKey);
    return;
  }

  key = newKey;
}


size_t RSAKey::encryptPublic(void *src, void *dst, size_t src_len, size_t dst_len) const
{
  size_t segment = RSA_size(key) - DEFAULT_PADDING_SIZE - 1;
  size_t loops = (src_len + segment - 1)/segment;
  size_t left = src_len;
  size_t total = 0;

  /* validate the destination buffer size. */
  if (dst_len < loops * RSA_size(key)) {
    return -2;
  }

  for (size_t i = 0; i < loops; i++) {
    size_t this_pass = (i != (loops - 1)) ? segment : left;
    assert(this_pass <= segment);
    int ret = RSA_public_encrypt(this_pass, static_cast<unsigned char *>(src), static_cast<unsigned char *>(dst), 
				 const_cast<RSA *>(static_cast<const RSA *>(key)), DEFAULT_PADDING);
    if (ret < 0) {
      ERR_print_errors_fp(stderr);
      return ret;
    }

    src = static_cast<unsigned char *>(src) + this_pass;
    dst = static_cast<unsigned char *>(dst) + ret;
    left -= this_pass;
    total += ret;
  }
  return total;
}

size_t RSAKey::encryptPrivate(void *src, void *dst, size_t src_len, size_t dst_len) const
{
  size_t segment = RSA_size(key) - DEFAULT_PADDING_SIZE - 1;
  size_t loops = (src_len + segment - 1)/segment;
  size_t left = src_len;
  size_t total = 0;

  /* validate the destination buffer size. */
  if (dst_len < loops * RSA_size(key)) {
    return -2;
  }

  for (size_t i = 0; i < loops; i++) {
    size_t this_pass = (i != (loops - 1)) ? segment : left;
    assert(this_pass <= segment);
    int ret = RSA_private_encrypt(this_pass, static_cast<unsigned char *>(src), static_cast<unsigned char *>(dst), 
				  const_cast<RSA *>(static_cast<const RSA *>(key)), DEFAULT_PADDING);
    if (ret < 0) {
      ERR_print_errors_fp(stderr);
      return ret;
    }

    src = static_cast<unsigned char *>(src) + this_pass;
    dst = static_cast<unsigned char *>(dst) + ret;
    left -= this_pass;
    total += ret;
  }
  return total;
}

size_t RSAKey::encryptedSize(size_t src_size) const
{
  size_t rsasize = RSA_size(key);
  size_t segment = rsasize - DEFAULT_PADDING_SIZE - 1;
  size_t loops = (src_size + segment - 1)/segment;
  return loops * RSA_size(key);
}

size_t RSAKey::decryptedSize(size_t src_size) const
{
  size_t rsasize = RSA_size(key);
  size_t segment = rsasize - DEFAULT_PADDING_SIZE - 1;
  size_t loops = (src_size + RSA_size(key) - 1)/RSA_size(key);
  return loops * segment;
}

size_t RSAKey::decryptPrivate(void *src, void *dst, size_t src_len, size_t dst_len) const
{
  size_t segment = RSA_size(key) - DEFAULT_PADDING_SIZE - 1;
  size_t loops = (src_len + RSA_size(key) - 1)/RSA_size(key);
  size_t total = 0;

  if (dst_len < loops * segment) {
    return -2;
  }

  if ((src_len % RSA_size(key)) != 0) {
    return -3;
  }

  for (size_t i = 0; i < loops; i++) {
    int ret = RSA_private_decrypt(RSA_size(key), static_cast<unsigned char *>(src), static_cast<unsigned char *>(dst), 
				  const_cast<RSA *>(static_cast<const RSA *>(key)), DEFAULT_PADDING);
    if (ret < 0) {
      ERR_print_errors_fp(stderr);
      return ret;
    }
    total += ret;
    dst = static_cast<char *>(dst) + ret;
    src = static_cast<char *>(src) + RSA_size(key);
  }

  return total;
      
}

size_t RSAKey::decryptPublic(void *src, void *dst, size_t src_len, size_t dst_len) const
{
  size_t segment = RSA_size(key) - DEFAULT_PADDING_SIZE - 1;
  size_t loops = (src_len + RSA_size(key) - 1)/RSA_size(key);
  size_t total = 0;

  if (dst_len < loops * segment) {
    return -2;
  }

  if ((src_len % RSA_size(key)) != 0) {
    return -3;
  }

  for (size_t i = 0; i < loops; i++) {
    int ret = RSA_public_decrypt(RSA_size(key), static_cast<unsigned char *>(src), static_cast<unsigned char *>(dst), 
				 const_cast<RSA *>(static_cast<const RSA *>(key)), DEFAULT_PADDING);
    if (ret < 0) {
      ERR_print_errors_fp(stderr);
      return ret;
    }
    total += ret;
    dst = static_cast<char *>(dst) + ret;
    src = static_cast<char *>(src) + RSA_size(key);
  }

  return total;
      
}

KeyPB *RSAKey::protoBuf() const 
{

  if (key == NULL) {
    return NULL;
  }

  KeyPB *pb = new KeyPB();
  if (pb == NULL) {
    return NULL;
  }

   BIO *mem = BIO_new(BIO_s_mem());
   int ret = PEM_write_bio_RSAPrivateKey(mem, const_cast<RSA *>(static_cast<const RSA *>(key)), NULL, NULL, 0, NULL, NULL);
   if (!ret) {
     LOG(ERROR) << "Unable to write private key to memory.\n";
   }

   ret = PEM_write_bio_RSAPublicKey(mem, key);
   if (!ret) {
     LOG(ERROR) << "Unable to write private key to memory.\n";
   }

   BUF_MEM *bmem;
   BIO_get_mem_ptr(mem, &bmem);
   pb->set_data(bmem->data, bmem->length);
   BIO_free(mem);

  // RSA Is Currently the default, so we
  // save a little bit of space by leaving
  // out the type.
  // pb->set_type(RSA_PK_TYPE);

  return pb;
}

Key *createRSAKey(const char *c)  // This thunk prevents having to include RSAKey.h into Key.h
{
  return RSAKey::createRSAKey(c);
}

int identifyRSAKey(const char *c) {
  const char *p = strcasestr(c, "rsa");
  if (p != NULL && reinterpret_cast<unsigned long>(p) - reinterpret_cast<unsigned long>(c) < 30) {
    return RSA_PK_TYPE;
  }
  return 0;
}

Key *RSAKey::createRSAKey(const char *str)
{
  if (str == NULL) {
    return new RSAKey(RSA_generate_key(default_rsa_key_bits, default_rsa_exponent, NULL, NULL));
  }

  return new RSAKey(str);
}

int RSAKey::cmp(const Key &k2k) const {
  const RSAKey *k2 = dynamic_cast<const RSAKey *>(&k2k);
  assert(k2 != NULL);
  AUTODELETE(KeyPB *, pb1);
  AUTODELETE(KeyPB *, pb2);

  pb1 = protoBuf();
  assert(pb1 != NULL);
  pb2 = k2->protoBuf();
  assert(pb2 != NULL);
  
  return pb1->data().compare(pb2->data());
}

DEFINE_INIT_FUNCTION(RSAKeyInit2, REGISTER_KEY_PRIORITY-1)
{
  unsigned char seed[RAND_SEED_SIZE]; //some entropy to get the random number generator started.
  textus::system::SysInfo::systemInformation()->random(seed, sizeof(seed));
  textus::random::PRNG::globalPRNG()->seed();
  for (unsigned i = 0; i < sizeof(seed); i++) {
    seed[i] ^= textus::random::PRNG::globalPRNG()->rand8();
  }
  RAND_seed(seed, sizeof(seed));
  ERR_load_crypto_strings();
  return 0;
} 

}}}} // namespace textus::keys::providers::RSA


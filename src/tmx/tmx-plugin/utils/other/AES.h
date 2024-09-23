/*
 * AES.h
 *
 *  Created on: Nov 8, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_AES_H_
#define SRC_AES_H_

#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 256
#endif

#include <tmx/messages/byte_stream.hpp>
#include <openssl/evp.h>

namespace tmx {
namespace utils {

/**
  AES encryption/decryption demo program using OpenSSL EVP apis
  gcc -Wall openssl_aes.c -lcrypto
  this is public domain code.
  Saju Pillai (saju.pillai@gmail.com)
**/
class AES {
public:
	AES(const tmx::byte_stream &symmetricKey);
	virtual ~AES();

	tmx::byte_stream decrypt(tmx::byte_stream cipher);
	tmx::byte_stream encrypt(tmx::byte_stream plain);
private:
	byte_stream key_data;
	EVP_CIPHER_CTX *enc, *dec;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_AES_H_ */

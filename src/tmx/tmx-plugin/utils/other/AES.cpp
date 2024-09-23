/*
 * AES.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: gmb
 */

#include "AES.h"

using namespace std;
using namespace tmx;

namespace tmx {
namespace utils {

/**
 * Create a 256 bit key and IV using the supplied key_data. salt can be added for taste.
 * Fills in the encryption and decryption ctx objects and returns 0 on success
 **/
AES::AES(const byte_stream &symmetricKey) : key_data(symmetricKey) {
	// Based on an ASCII version of B@ttelle
	byte_stream salt = { 0x24, 0x40, 0x47, 0x74, 0x56, 0xc6, 0x6c, 0x0a };

	int i, nrounds = 5;
	unsigned char key[32], iv[32];

	memset(iv, 0, 32);

	/*
	* Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
	* nrounds is the number of times the we hash the material. More rounds are more secure but
	* slower.
	*/
	i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt.data(), key_data.data(), key_data.size(), nrounds, key, iv);
	if (i != 32) {
		key_data.clear();
	} else {
		enc = EVP_CIPHER_CTX_new();
		EVP_EncryptInit_ex(enc, EVP_aes_256_cbc(), NULL, key, iv);
		dec = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(dec, EVP_aes_256_cbc(), NULL, key, iv);
	}
}


AES::~AES() {
	EVP_CIPHER_CTX_cleanup(enc);
	EVP_CIPHER_CTX_free(enc);
	EVP_CIPHER_CTX_cleanup(dec);
	EVP_CIPHER_CTX_free(dec);
}

/*
 * Decrypt *len bytes of ciphertext
 */
byte_stream AES::decrypt(byte_stream ciphertext) {
	if (key_data.empty())
		return byte_stream_decode("");

	/* plaintext will always be equal to or lesser than length of ciphertext*/
	int p_len = ciphertext.size(), f_len = 0;
	byte_stream plaintext(10 * p_len);

	EVP_DecryptInit_ex(dec, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(dec, plaintext.data(), &p_len, ciphertext.data(), ciphertext.size());
	EVP_DecryptFinal_ex(dec, plaintext.data()+p_len, &f_len);

	plaintext.resize(p_len + f_len);
	return plaintext;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
byte_stream AES::encrypt(byte_stream plaintext)
{
	if (key_data.empty())
		return byte_stream_decode("");

	/* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
	int c_len = plaintext.size() + AES_BLOCK_SIZE, f_len = 0;
	byte_stream ciphertext(10 * c_len);

	/* allows reusing of 'e' for multiple encryption cycles */
	EVP_EncryptInit_ex(enc, NULL, NULL, NULL, NULL);

	/* update ciphertext, c_len is filled with the length of ciphertext generated */
	EVP_EncryptUpdate(enc, ciphertext.data(), &c_len, plaintext.data(), plaintext.size());

	/* update ciphertext with the final remaining bytes */
	EVP_EncryptFinal_ex(enc, ciphertext.data()+c_len, &f_len);

	ciphertext.resize(c_len + f_len);
	return ciphertext;
}

} /* namespace utils */
} /* namespace tmx */

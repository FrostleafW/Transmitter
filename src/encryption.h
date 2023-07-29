#pragma once

class Encryption {

	BCRYPT_ALG_HANDLE hwnd_alg;
	BCRYPT_KEY_HANDLE hwnd_key;

public:
	Encryption();
	bool AES_construct(BYTE* AES_key, int key_len);
	void AES_generateKey(BYTE* AES_key, int key_len);
	unsigned long AES_encrypt(BYTE* text, int text_len, BYTE* cipher, int cipher_len);
	unsigned long AES_decrypt(BYTE* cipher, int cipher_len, BYTE* text, int text_len);
	bool RSA_construct();
	bool RSA_importPublic(BYTE* public_key, unsigned long key_len);
	unsigned long RSA_exportPublic(BYTE* public_key, int key_len);
	unsigned long RSA_encrypt(BYTE* text, int text_len, BYTE* cipher, int cipher_len);
	unsigned long RSA_decrypt(BYTE* cipher, int cipher_len, BYTE* text, int text_len);
	void cleanup();
	~Encryption();
};
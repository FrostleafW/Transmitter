#include <time.h>
#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "bcrypt.lib")

#include "Encryption.h"

Encryption::Encryption() {
	hwnd_alg = NULL;
	hwnd_key = NULL;
}

bool Encryption::AES_construct(BYTE* AES_key, int key_len)
{
	NTSTATUS status;
	status = BCryptOpenAlgorithmProvider(&hwnd_alg, BCRYPT_AES_ALGORITHM, NULL, NULL);
	if (status != 0)
	{
		printf("BCryptOpenAlgorithmProvider failed with code 0x%08x\n", status);
		return false;
	}
	status = BCryptGenerateSymmetricKey(hwnd_alg, &hwnd_key, NULL, NULL, AES_key, key_len, NULL);
	if (status != 0)
	{
		printf("BCryptGenerateSymmetricKey failed with code 0x%08x\n", status);
		return false;
	}
	return true;
}

void Encryption::AES_generateKey(BYTE* AES_key, int key_len)
{
	srand((u_int)time(NULL));
	for (int i = 0; i < key_len; i++)
		AES_key[i] = rand() % 256;
}

unsigned long Encryption::AES_encrypt(BYTE* text, int text_len, BYTE* cipher, int cipher_len)
{
	NTSTATUS status;
	unsigned long len = 0;
	status = BCryptEncrypt(hwnd_key, text, text_len, NULL, NULL, NULL, cipher, cipher_len, &len, BCRYPT_BLOCK_PADDING);
	if (status != 0)
	{
		printf("BCryptEncrypt failed with code 0x%08x\n", status);
		return 0;
	}
	return len;
}

unsigned long Encryption::AES_decrypt(BYTE* cipher, int cipher_len, BYTE* text, int text_len)
{
	NTSTATUS status;
	unsigned long len = 0;
	status = BCryptDecrypt(hwnd_key, cipher, cipher_len, NULL, NULL, NULL, text, text_len, &len, BCRYPT_BLOCK_PADDING);
	if (status != 0)
	{
		printf("BCryptDecrypt failed with code 0x%08x\n", status);
		return 0;
	}
	return len;
}

bool Encryption::RSA_construct()
{
	NTSTATUS status;
	status = BCryptOpenAlgorithmProvider(&hwnd_alg, BCRYPT_RSA_ALGORITHM, NULL, NULL);
	if (status != 0)
	{
		printf("BCryptOpenAlgorithmProvider failed with code 0x%08x\n", status);
		return false;
	}
	status = BCryptGenerateKeyPair(hwnd_alg, &hwnd_key, 512, NULL);
	if (status != 0)
	{
		printf("BCryptGenerateKeyPair failed with code 0x%08x\n", status);
		return false;
	}
	status = BCryptFinalizeKeyPair(hwnd_key, NULL);
	if (status != 0)
	{
		printf("BCryptFinalizeKeyPair failed with code 0x%08x\n", status);
		return false;
	}
	return true;
}

bool Encryption::RSA_importPublic(BYTE* public_key, unsigned long key_len)
{
	NTSTATUS status;
	status = BCryptOpenAlgorithmProvider(&hwnd_alg, BCRYPT_RSA_ALGORITHM, NULL, NULL);
	if (status != 0)
	{
		printf("BCryptOpenAlgorithmProvider failed with code 0x%08x\n", status);
		return false;
	}
	status = BCryptImportKeyPair(hwnd_alg, NULL, BCRYPT_RSAPUBLIC_BLOB, &hwnd_key, public_key, key_len, NULL);
	if (status != 0)
	{
		printf("BCryptImportKeyPair failed with code 0x%08x\n", status);
		return false;
	}
	return true;
}

unsigned long Encryption::RSA_exportPublic(BYTE* public_key, int key_len)
{
	NTSTATUS status;
	unsigned long len = 0;
	status = BCryptExportKey(hwnd_key, NULL, BCRYPT_RSAPUBLIC_BLOB, public_key, key_len, &len, NULL);
	if (status != 0)
	{
		printf("BCryptExportKey failed with code 0x%08x\n", status);
		return 0;
	}
	return len;
}

unsigned long Encryption::RSA_encrypt(BYTE* text, int text_len, BYTE* cipher, int cipher_len)
{
	NTSTATUS status;
	unsigned long len = 0;
	status = BCryptEncrypt(hwnd_key, text, text_len, NULL, NULL, NULL, cipher, cipher_len, &len, BCRYPT_PAD_PKCS1);
	if (status != 0)
	{
		printf("BCryptEncrypt failed with code 0x%08x\n", status);
		return 0;
	}
	return len;
}

unsigned long Encryption::RSA_decrypt(BYTE* cipher, int cipher_len, BYTE* text, int text_len)
{
	NTSTATUS status;
	unsigned long len = 0;
	status = BCryptDecrypt(hwnd_key, cipher, cipher_len, NULL, NULL, NULL, text, text_len, &len, BCRYPT_PAD_PKCS1);
	if (status != 0)
	{
		printf("BCryptDecrypt failed with code 0x%08x\n", status);
		return 0;
	}
	return len;
}

void Encryption::Cleanup()
{
	BCryptDestroyKey(hwnd_key);
	BCryptCloseAlgorithmProvider(hwnd_alg, NULL);
	hwnd_alg = NULL;
	hwnd_key = NULL;
}

Encryption::~Encryption() {
	Cleanup();
}
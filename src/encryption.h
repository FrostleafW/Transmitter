#pragma once

bool AES_start(BCRYPT_ALG_HANDLE& hwnd_alg, BCRYPT_KEY_HANDLE& hwnd_key, BYTE* AES_key, int key_len) {
	NTSTATUS status;
	status = BCryptOpenAlgorithmProvider(&hwnd_alg, BCRYPT_AES_ALGORITHM, NULL, NULL);
	if (status != 0)
	{
		printf("BCryptOpenAlgorithmProvider failed with code 0x%08x\n", status);
		return false;
	}
	status = BCryptGenerateSymmetricKey(hwnd_alg, &hwnd_key, NULL, NULL, AES_key, key_len, 0);
	if (status != 0)
	{
		printf("BCryptGenerateSymmetricKey failed with code 0x%08x\n", status);
		return false;
	}
	return true;
}

void AES_generateKey(BYTE* AES_key) {
	srand((u_int)time(NULL));
	for (int i = 0; i < 16; i++)
		AES_key[i] = rand() % 256;
}

unsigned long AES_encrypt(BCRYPT_KEY_HANDLE& hwnd_key, BYTE* text, int text_len, BYTE* cipher, int cipher_len) {
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

unsigned long AES_decrypt(BCRYPT_KEY_HANDLE& hwnd_key, BYTE* cipher, int cipher_len, BYTE* text, int text_len) {
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

bool RSA_start(BCRYPT_ALG_HANDLE& hwnd_alg, BCRYPT_KEY_HANDLE& hwnd_key) {
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

bool RSA_importPublic(BCRYPT_ALG_HANDLE& hwnd_alg, BCRYPT_KEY_HANDLE& hwnd_key, BYTE* public_key, unsigned long len) {
	NTSTATUS status;
	status = BCryptOpenAlgorithmProvider(&hwnd_alg, BCRYPT_RSA_ALGORITHM, NULL, NULL);
	if (status != 0)
	{
		printf("BCryptOpenAlgorithmProvider failed with code 0x%08x\n", status);
		return false;
	}
	status = BCryptImportKeyPair(hwnd_alg, NULL, BCRYPT_RSAPUBLIC_BLOB, &hwnd_key, public_key, len, NULL);
	if (status != 0)
	{
		printf("BCryptImportKeyPair failed with code 0x%08x\n", status);
		return false;
	}
	return true;
}

unsigned long RSA_exportPublic(BCRYPT_KEY_HANDLE& hwnd_key, BYTE* public_key, int key_len) {
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

unsigned long RSA_encrypt(BCRYPT_KEY_HANDLE& hwnd_key, BYTE* text, int text_len, BYTE* cipher, int cipher_len) {
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

unsigned long RSA_decrypt(BCRYPT_KEY_HANDLE& hwnd_key, BYTE* cipher, int cipher_len, BYTE* text, int text_len) {
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

void Key_cleanup() {
	BCryptDestroyKey(G_hwnd_key);
	BCryptCloseAlgorithmProvider(G_hwnd_alg, NULL);
	G_hwnd_alg = NULL;
	G_hwnd_key = NULL;
}
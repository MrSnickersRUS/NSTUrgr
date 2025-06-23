#ifndef CIPHER_H
#define CIPHER_H

#include "../io.h"
#include "interface.h"

#include <vector>    
#include <stdexcept>

std::vector<unsigned char> vernamCipher(const std::vector<unsigned char>& inputText, const std::vector<unsigned char>& key, const std::vector<unsigned char>* pNonce);
std::vector<unsigned char> autokeyCipher(const std::vector<unsigned char>& inputText, const std::vector<unsigned char>& key, const std::vector<unsigned char>* pNonce);
std::vector<unsigned char> salsa20Cipher(const std::vector<unsigned char>& inputText, const std::vector<unsigned char>& key, const std::vector<unsigned char>* pNonce);

void executeInput(int cipherChoice, int inputChoice, int keyChoice);

#endif
#include "ciphers.h"

#include <vector>
#include <stdexcept>

using namespace std;

// Шифрование
vector<unsigned char> autokeyCipher(
    const vector<unsigned char>& inputText,
    const vector<unsigned char>& key,
    const vector<unsigned char>* pNonce)
{
    vector<unsigned char> cipherText(inputText.size());
    
    // Первый символ шифруется отдельно
    cipherText[0] = static_cast<unsigned char>((inputText[0] + key[0]) % 256);
    
    // Гамма - это предыдущий символ открытого текста
    for (size_t i = 1; i < inputText.size(); ++i) {
        cipherText[i] = static_cast<unsigned char>((inputText[i] + inputText[i-1]) % 256);
    }
    return cipherText;
}

// Дешифрование
vector<unsigned char> autokeyDecipher(
    const vector<unsigned char>& cipherText,
    const vector<unsigned char>& key,
    const vector<unsigned char>* pNonce)
{
    vector<unsigned char> decryptedText(cipherText.size());
    
    // Первый символ дешифруется отдельно
    decryptedText[0] = static_cast<unsigned char>((256 + cipherText[0] - key[0]) % 256);

    // Гамма теперь - это предыдущий дешифрованный символ
    for (size_t i = 1; i < cipherText.size(); ++i) {
        decryptedText[i] = static_cast<unsigned char>((256 + cipherText[i] - decryptedText[i-1]) % 256);
    }
    
    return decryptedText;
}

// Экспортируемая функция для создания модуля
extern "C" CipherModule* createCipherModule() {
    static CipherModule autokeyModule = {
        "AUTOKEY", // Название
        autokeyCipher, // Шифрование
        autokeyDecipher // Дешифрование
    };
    return &autokeyModule;
}
#include "ciphers.h"

#include <vector>
#include <stdexcept>

using namespace std;

// Вспомогательная функция XOR для Вернама
vector<unsigned char> xorText(const vector<unsigned char>& data, const vector<unsigned char>& key) {
    vector<unsigned char> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key[i];
    }
    return result;
}

// Основная функция шифра Вернама
vector<unsigned char> vernamCipher(
    const vector<unsigned char>& inputText,
    const vector<unsigned char>& key,
    const vector<unsigned char>* pNonce)
{
    // Проверка длины ключа
    if (key.size() < inputText.size()) {
        throw invalid_argument("Ключ для шифра Вернама должен быть не короче текста.");
    }
    
    // Шифрование
    vector<unsigned char> cipherText = xorText(inputText, key);
    
    return cipherText;
}

// Экспортируемая функция для создания модуля
extern "C" CipherModule* createCipherModule() {
    static CipherModule vernamModule = {
        "VERNAM", // Название
        vernamCipher, // Шифрование
        vernamCipher // Дешифрование
    };
    return &vernamModule;
}
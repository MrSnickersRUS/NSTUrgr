#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>
#include <string>

// Объявляем тип функции для шифрования/дешифрования
typedef std::vector<unsigned char> (*CipherFunc)(
    const std::vector<unsigned char>& inputText,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>* pNonce // Указатель на Nonce может быть nullptr
);

// Структура с описанием шифра
struct CipherModule {
    std::string name;
    CipherFunc encryptFunction;
    CipherFunc decryptFunction;
};

// Функция, которую каждая .so будет экспортировать
extern "C" CipherModule* createCipherModule();

#endif
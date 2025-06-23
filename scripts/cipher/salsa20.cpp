#include "ciphers.h"

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <array>

using namespace std;

// Циклический сдвиг влево
uint32_t rotl32(uint32_t n, int c) {
    return (n << c) | (n >> (32 - c));
}

// Преобразование 4 байтов в 32-битное слово
uint32_t bytesToWord(const unsigned char* bytes) {
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[3]) << 24);
}

// Преобразования 32-битного слова в 4 байта
void wordToBytes(uint32_t word, unsigned char* bytes) {
    bytes[0] = static_cast<unsigned char>(word);
    bytes[1] = static_cast<unsigned char>(word >> 8);
    bytes[2] = static_cast<unsigned char>(word >> 16);
    bytes[3] = static_cast<unsigned char>(word >> 24);
}

// Четверть раунда
void quarterRound(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    b ^= rotl32(a + d, 7);
    c ^= rotl32(b + a, 9);
    d ^= rotl32(c + b, 13);
    a ^= rotl32(d + c, 18);
}

// Раунд со столбцами
void columnRound(uint32_t* state) {
    quarterRound(state[0], state[4], state[8], state[12]);
    quarterRound(state[1], state[5], state[9], state[13]);
    quarterRound(state[2], state[6], state[10], state[14]);
    quarterRound(state[3], state[7], state[11], state[15]);
}

// Раунд со строками
void rowRound(uint32_t* state) {
    quarterRound(state[0], state[1], state[2], state[3]);
    quarterRound(state[5], state[6], state[7], state[4]);
    quarterRound(state[10], state[11], state[8], state[9]);
    quarterRound(state[15], state[12], state[13], state[14]);
}


// Генерация 64-байтового блока ключевого потока
vector<unsigned char> blockGenerator(
    const vector<unsigned char>& key,
    const vector<unsigned char>& nonce,
    uint64_t blockCounter)
{
    // Salsa20 константы
    const array<uint32_t, 4> sigma = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
    const array<uint32_t, 4> tau = {0x61707865, 0x3120646e, 0x79622d32, 0x6b206574};

    uint32_t currentState[16];

    // Выбираем константы в зависимости от размера ключа
    const array<uint32_t, 4>* currentConstants = nullptr;
    if (key.size() == 32) currentConstants = &sigma;
    else if (key.size() == 16) currentConstants = &tau;
    else throw invalid_argument("Ключ должен быть 16 или 32 байта.");

    // Инициализация константных слов состояния - каждое первое слово в строках матрицы
    currentState[0] = (*currentConstants)[0];
    currentState[5] = (*currentConstants)[1];
    currentState[10] = (*currentConstants)[2];
    currentState[15] = (*currentConstants)[3];

    // Инициализация ключевых слов состояния
    // Слова ключа занимают позиции 1-4 и 11-14 в состоянии Salsa20
    // Если ключ 128-битный, то ключевые слова в позициях 1-4 дублируются в 11-14.
    // Если ключ 256-битный, то 1-4 - первая половина, 11-14 - вторая половина.

    // Заполняем первые 4 ключевых слова
    for (int i = 0; i < 4; ++i) {
        currentState[1 + i] = bytesToWord(&key[i * 4]);
    }

    // Заполняем вторые 4 ключевых слова
    if (key.size() == 32) { // 256-битный ключ
        for (int i = 0; i < 4; ++i) {
            currentState[11 + i] = bytesToWord(&key[(4 + i) * 4]);
        }
    } 
    else { // 128-битный ключ, дублируем первые 4 слова
        for (int i = 0; i < 4; ++i) {
            currentState[11 + i] = bytesToWord(&key[i * 4]);
        }
    }

    currentState[6] = bytesToWord(&nonce[0]);
    currentState[7] = bytesToWord(&nonce[4]);

    // Счетчик блока (8 байт) - 2 слова
    currentState[8] = static_cast<uint32_t>(blockCounter);
    currentState[9] = static_cast<uint32_t>(blockCounter >> 32);

    // Копируем начальное состояние для финального сложения
    uint32_t workingState[16];
    for (int i = 0; i < 16; ++i) {
        workingState[i] = currentState[i];
    }

    // 10 двойных раундов со строками и столбцами
    for (int i = 0; i < 10; ++i) { 
        columnRound(workingState); 
        rowRound(workingState);    
    }

    // Финальное сложение с начальным состоянием
    vector<unsigned char> outputBlock(64);
    for (int i = 0; i < 16; ++i) {
        uint32_t finalWord = workingState[i] + currentState[i]; 
        wordToBytes(finalWord, &outputBlock[i * 4]);
    }

    return outputBlock;
}

// Основная функция Salsa20 шифрования/дешифрования
vector<unsigned char> salsa20Cipher(
    const vector<unsigned char>& inputText,
    const vector<unsigned char>& key,
    const vector<unsigned char>* pNonce) 
{
    // Nonce для Salsa20 обязателен и проверяется на корректность
    if (!pNonce || pNonce->empty() || pNonce->size() != 8)
        throw invalid_argument("Salsa20. Nonce должен быть 8 байт");
    const vector<unsigned char>& nonce = *pNonce; 

    // Проверки длины ключа (уже есть в blockGenerator, но для ясности можно оставить и здесь)
    if (! (key.size() == 16 || key.size() == 32) )
        throw invalid_argument("Salsa20. ключ должен быть 16 или 32 байта");

    vector<unsigned char> outputText;
    outputText.reserve(inputText.size()); 

    for (uint64_t blockCounter = 0; blockCounter * 64 < inputText.size(); ++blockCounter) {
        vector<unsigned char> keystreamBlock = blockGenerator(key, nonce, blockCounter);

        size_t currentBlockStart = blockCounter * 64;
        size_t currentBlockEnd = min(currentBlockStart + 64, inputText.size());
        size_t blockLength = currentBlockEnd - currentBlockStart;

        for (size_t i = 0; i < blockLength; ++i) {
            outputText.push_back(inputText[currentBlockStart + i] ^ keystreamBlock[i]);
        }
    }    
    return outputText; 
}

// Экспортируемая функция для создания модуля
extern "C" CipherModule* createCipherModule() {
    static CipherModule salsa20Module = {
        "SALSA20",
        salsa20Cipher,
        salsa20Cipher
    };
    return &salsa20Module;
}
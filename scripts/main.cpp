#include "cipher/ciphers.h"
#include "cipher/interface.h"

#include <iostream>
#include <limits>
#include <map>
#include <dlfcn.h>
#include <vector>
#include <string>

using namespace std;

// Мап для загруженных модулей и их хэндлов
map<string, CipherModule*> loadedCiphers;
map<string, void*> dlHandles;

// Статус для каждого шифра
bool isVernamWorking = false;
bool isAutokeyWorking = false;
bool isSalsa20Working = false;

// Чистка экрана
void clearScreen() {
    system("clear");
}

// Загрузка конкретного шифра
void loadCipherModule(const string& cipherName) {
    if (loadedCiphers.count(cipherName)) return;

    string libPath = "./lib" + cipherName + ".so";
    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
        libPath = "lib" + cipherName + ".so";
        handle = dlopen(libPath.c_str(), RTLD_LAZY);
        if (!handle) throw runtime_error("Ошибка при загрузке библиотеки " + libPath + ": " + dlerror());
    }

    CipherModule* (*createFunction)() = reinterpret_cast<CipherModule* (*)()>(dlsym(handle, "createCipherModule"));
    if (!createFunction) {
        dlclose(handle);
        throw runtime_error("Не удалось найти функцию createCipherModule в библиотеке " + libPath + ": " + dlerror());
    }

    CipherModule* module = createFunction();
    loadedCiphers[module->name] = module;
    dlHandles[module->name] = handle;

    if (cipherName == "VERNAM") isVernamWorking = true;
    else if (cipherName == "AUTOKEY") isAutokeyWorking = true;
    else if (cipherName == "SALSA20") isSalsa20Working = true;
}

// Манипуляции с введенными значениями
void executeInput(int cipherChoice, int inputChoice, int keyChoice) {
    vector<unsigned char> keyBytes;
    vector<unsigned char> nonceBytes;
    vector<unsigned char>* pInputTextBytes = nullptr;
    vector<unsigned char> inputTextFromManualInput;

    FileData inputFileData;

    switch (inputChoice) {
        case 1:
            inputTextFromManualInput = readBytesFromInput("текст");
            pInputTextBytes = &inputTextFromManualInput;
            break;
        case 2:
            inputFileData = readBytesFromFile("source/input/input.txt");
            pInputTextBytes = &inputFileData.content;
            break;
        default: throw invalid_argument("Неверный выбор способа ввода текста. Нужно выбрать 1 или 2");
    }

    string cipherName;
    switch (cipherChoice) {
        case 1: cipherName = "VERNAM"; break;
        case 2: cipherName = "AUTOKEY"; break;
        case 3: cipherName = "SALSA20"; break;
        default: throw invalid_argument("Неверный выбор шифра. Нужно выбрать 1, 2 или 3"); 
    }

    if (loadedCiphers.find(cipherName) == loadedCiphers.end())
        throw runtime_error("Ошибка. Шифр " + cipherName + " не загружен. Проверьте файлы .so");
    CipherModule* currentCipher = loadedCiphers[cipherName];

    if (cipherChoice == 3) {
        size_t requiredKeyLen = 32;

        switch (keyChoice) {
            case 1: 
                keyBytes = readBytesFromInput("ключ (16 или 32 байта)");
                break;
            case 2: 
                keyBytes = readBytesFromFile("source/input/salsa20_key.txt").content;
                while (!keyBytes.empty() && (keyBytes.back() == '\n' || keyBytes.back() == '\r')) {
                    keyBytes.pop_back();
                }
                break;
            case 3: 
                keyBytes = genRandomKey(requiredKeyLen);
                cout << "Сгенерирован случайный ключ для Salsa20 (" << requiredKeyLen << " байт)" << endl;
                break;
            default: throw invalid_argument("Неверный выбор способа ввода ключа для Salsa20");
        }

        // Проверка длины ключа для Salsa20
        if (!(keyBytes.size() == 16 || keyBytes.size() == 32))
             throw invalid_argument("Ключ для Salsa20 должен быть 16 или 32 байта. Проверьте содержимое файла.");

        nonceBytes = genRandomKey(8);
        cout << "Сгенерирован случайный Nonce для Salsa20 (8 байт)." << endl;

    } 
    else { // Для Вернама и автоключа
        switch (keyChoice) {
            case 1: 
                keyBytes = readBytesFromInput("ключ"); 
                break;
            case 2: 
                keyBytes = readBytesFromFile("source/input/key.txt").content;
                while (!keyBytes.empty() && (keyBytes.back() == '\n' || keyBytes.back() == '\r')) {
                    keyBytes.pop_back();
                }
                break;
            case 3:
                if (cipherChoice == 1) {
                    keyBytes = genRandomKey(pInputTextBytes->size());
                    cout << "Сгенерирован случайный ключ для Вернама (" << pInputTextBytes->size() << " байт)." << endl;
                } else {
                    keyBytes = genRandomKey(16);
                    cout << "Сгенерирован случайный ключ для автоключа (16 байт)." << endl;
                }
                break;
            default: throw invalid_argument("Неверный выбор способа ввода ключа");
        }
    }

    const vector<unsigned char>* pNonce = (cipherChoice == 3) ? &nonceBytes : nullptr;
    
    string extension = (inputChoice == 2) ? getFileExtension(inputFileData.fileName) : ".txt";
    
    vector<unsigned char> encryptedText = currentCipher->encryptFunction(*pInputTextBytes, keyBytes, pNonce);
    string encryptedFileName = "source/output/" + cipherName + "_encrypted" + extension;
    writeBytesToFile(encryptedFileName, encryptedText);

    vector<unsigned char> decryptedText = currentCipher->decryptFunction(encryptedText, keyBytes, pNonce);
    string decryptedFileName = "source/output/" + cipherName + "_decrypted" + extension;
    writeBytesToFile(decryptedFileName, decryptedText); 
}

// Точка входа
int main() {
    try {
        loadCipherModule("VERNAM");
        loadCipherModule("AUTOKEY");
        loadCipherModule("SALSA20");
    } 
    catch (const exception& e) {
        cerr << "Критическая ошибка при запуске. Не удалось загрузить модули шифров" << endl;
        cerr <<  e.what() << endl;
        cerr << "Проверьте, что файлы .so находятся в той же директории, что и исполняемый файл" << endl;
        return 1;
    }

    while (true) {
        clearScreen();
        cout << "Выберите шифр" << endl
            << "1. Шифр Вернама " << (isVernamWorking ? "(Работает)" : "(Не работает)") << endl
            << "2. Шифр с автоключом " << (isAutokeyWorking ? "(Работает)" : "(Не работает)") << endl
            << "3. Salsa20 " << (isSalsa20Working ? "(Работает)" : "(Не работает)") << endl
            << "0. Выход" << endl;
        
        try {
            int cipherChoice;
            cin >> cipherChoice;
            if (!cin) { // Проверка, что ввод является числом
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Некорректный ввод. Введите число.");
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (cipherChoice == 0) break;
            if (cipherChoice < 1 || cipherChoice > 3)
                throw invalid_argument("Неверный выбор шифра. Нужно выбрать 1, 2 или 3");

            if ((cipherChoice == 1 && !isVernamWorking) ||
                (cipherChoice == 2 && !isAutokeyWorking) ||
                (cipherChoice == 3 && !isSalsa20Working))
                throw runtime_error("Выбранный шифр не загружен или не работает");
            
            bool backToCipherChoice = false;
            while (!backToCipherChoice) {
                clearScreen();
                cout << "Выберите способ ввода данных" << endl
                    << "1. Ввести данные вручную" << endl
                    << "2. Считать данные из файла" << endl
                    << "0. Назад" << endl;

                int inputChoice;
                cin >> inputChoice;
                if (!cin) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    throw invalid_argument("Некорректный ввод. Введите число.");
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (inputChoice == 0) {
                    backToCipherChoice = true; 
                    break;
                }
                if (inputChoice < 1 || inputChoice > 2)
                    throw invalid_argument("Неверный выбор ввода данных. Нужно выбрать 1 или 2");

                while (true) {
                    clearScreen();
                    cout << "Выберите способ ввода ключа" << endl
                        << "1. Ввести ключ вручную" << endl
                        << "2. Считать ключ из файла" << endl
                        << "3. Сгенерировать случайный ключ" << endl
                        << "0. Назад" << endl;

                    int keyChoice;
                    cin >> keyChoice;
                    if (!cin) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        throw invalid_argument("Некорректный ввод. Введите число.");
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (keyChoice == 0) break;
                    if (keyChoice < 1 || keyChoice > 3)
                        throw invalid_argument("Неверный выбор ввода ключа. Нужно выбрать 1, 2 или 3");
                    
                    executeInput(cipherChoice, inputChoice, keyChoice);

                    cout << "\nШифрование и дешифрование завершено успешно." << endl;
                    cout << "Результаты сохранены в папку source/output/" << endl;
                    cout << "\nНажмите Enter для продолжения...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    backToCipherChoice = true; 
                    break;
                }
                if (backToCipherChoice) break;
            }
        }
        catch (const exception& e) {
            cout << "\n!!! Ошибка. " << e.what() << " !!!" << endl;
            cout << "\nНажмите Enter для продолжения...";
            // Безопасное ожидание ввода
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        }
    }

    for (auto const& [name, handle] : dlHandles) {
        if(handle) dlclose(handle);
    }
    return 0;
}
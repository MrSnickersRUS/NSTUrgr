#include "io.h"

#include <iterator>
#include <stdexcept>
#include <random>

using namespace std;

// Вспомогательная функция для удаления пробелов по краям строки
void trimWhitespace(string& str) {
    const string whitespace = " \t\n\r\f\v"; // Все возможные пробельные символы
    // Удаляем пробелы в конце
    size_t end = str.find_last_not_of(whitespace);
    if (string::npos != end) str.erase(end + 1);
    // Удаляем пробелы в начале
    size_t start = str.find_first_not_of(whitespace);
    if (string::npos != start) str.erase(0, start);
}

// Обновленная функция для удаления кавычек
void quotationRemover(string& fileName) {
    trimWhitespace(fileName);

    // Удаляем одинарные кавычки
    if (fileName.size() >= 2 && fileName.front() == '\'' && fileName.back() == '\'')
        fileName = fileName.substr(1, fileName.size() - 2);
    // Удаляем двойные кавычки
    else if (fileName.size() >= 2 && fileName.front() == '"' && fileName.back() == '"')
        fileName = fileName.substr(1, fileName.size() - 2);
}

// Функция для чтения байтов из файла
FileData readBytesFromFile(const string& defaultFileName) {
    cout << "Введите имя/путь до файла (или Enter для использования '" << defaultFileName << "'): ";
    string fileName;
    getline(cin, fileName);
    if (fileName.empty()) fileName = defaultFileName;
    quotationRemover(fileName);

    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cout << "Файл \"" << fileName << "\" не найден. Создать новый? (y/n): ";
        string answer;
        getline(cin, answer);
        if (answer == "y" || answer == "Y" || answer.empty()) {
            ofstream fout(fileName, ios::binary);
            if (!fout.is_open()) throw runtime_error("Не удалось создать файл: " + fileName);
            cout << "Введите текст для записи в файл" << endl;
            string textInput;
            getline(cin, textInput);
            fout.write(reinterpret_cast<const char*>(textInput.data()), textInput.size());
            fout.close();
            return {fileName, vector<unsigned char>(textInput.begin(), textInput.end())};
        } 
        else throw runtime_error("Создание отменено: " + fileName);
    }
    
    vector<unsigned char> content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    if (content.empty()) throw runtime_error("Файл \"" + fileName + "\" пустой или не содержит байтов.");
    return {fileName, content};
}

// Функция для записи байтов в файл
void writeBytesToFile(const string& defaultFileName, const vector<unsigned char>& content) {
    cout << "Введите имя/путь до файла для сохранения (или Enter для использования '" << defaultFileName << "'): ";
    string fileName;
    getline(cin, fileName);
    if (fileName.empty()) fileName = defaultFileName;

    quotationRemover(fileName);

    ofstream file(fileName, ios::binary);
    if (!file.is_open()) throw runtime_error("Не удалось открыть/создать файл: " + fileName);
    
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
    cout << "Содержимое записано в файл: " << fileName << endl;
    file.close();
}

// Функция для чтения байтов из ввода пользователя
vector<unsigned char> readBytesFromInput(const string& inputArg) {
    cout << "Введите " << inputArg << ": ";
    string inputStr;
    getline(cin, inputStr);

    if (inputStr.empty() && inputArg == "текст") throw invalid_argument("Введен пустой текст");
    return vector<unsigned char>(inputStr.begin(), inputStr.end());
}

// Функция для генерации случайного ключа заданной длины
vector<unsigned char> genRandomKey(size_t length) {
    vector<unsigned char> randomBytes(length);
    random_device rd;

    for (size_t i = 0; i < length; ++i) {
        randomBytes[i] = static_cast<unsigned char>(rd());
    }
    return randomBytes;
}

// Функция для получения расширения файла
string getFileExtension(const string& fileName) {
    size_t pos = fileName.find_last_of('.');
    if (pos != string::npos && pos > 0) return fileName.substr(pos);
    return "";
}
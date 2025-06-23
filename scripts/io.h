#ifndef IO_H
#define IO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Структура для хранения данных файла
struct FileData {
    std::string fileName;
    std::vector<unsigned char> content;
};

void trimWhitespace(std::string& str);
void quotationRemover(std::string& fileName);
FileData readBytesFromFile(const std::string& defaultFileName);
void writeBytesToFile(const std::string& defaultFileName, const std::vector<unsigned char>& content);
std::vector<unsigned char> readBytesFromInput(const std::string& arg);
std::vector<unsigned char> genRandomKey(size_t length);
std::string getFileExtension(const std::string& fileName);

#endif
# Конфигурация компилятора и флагов
CXX = g++
CXXFLAGS = -Wall -fPIC -std=c++17 -I./scripts -I./scripts/cipher # Флаги компиляции и пути для инклудов

# Конфигурация директорий сборки
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin

# Имена файлов
MAIN_EXEC = $(BIN_DIR)/cipherApp # Имя исполняемого файла
EXEC_NAME = cipherApp # Имя исполняемого файла для установки

# Имена шифров для библиотек
CIPHER_NAMES = VERNAM AUTOKEY SALSA20

# Исходные файлы
SRC_MAIN_CPP = scripts/main.cpp
SRC_IO_CPP = scripts/io.cpp
SRC_VERNAM_CPP = scripts/cipher/vernam.cpp
SRC_AUTOKEY_CPP = scripts/cipher/autokey.cpp
SRC_SALSA20_CPP = scripts/cipher/salsa20.cpp

# Объектные файлы
OBJ_MAIN = $(OBJ_DIR)/scripts/main.o
OBJ_IO = $(OBJ_DIR)/scripts/io.o
OBJ_VERNAM = $(OBJ_DIR)/scripts/cipher/vernam.o
OBJ_AUTOKEY = $(OBJ_DIR)/scripts/cipher/autokey.o
OBJ_SALSA20 = $(OBJ_DIR)/scripts/cipher/salsa20.o

# Список всех объектных файлов для генерации зависимостей
ALL_OBJECTS = $(OBJ_MAIN) $(OBJ_IO) $(OBJ_VERNAM) $(OBJ_AUTOKEY) $(OBJ_SALSA20)

# Файлы зависимостей
DEPS = $(ALL_OBJECTS:.o=.d)

# Утилита для создания директорий
MKDIR_P = mkdir -p

# Главные цели
.PHONY: all clean install directories

all: directories $(LIB_DIR)/libVERNAM.so $(LIB_DIR)/libAUTOKEY.so $(LIB_DIR)/libSALSA20.so $(MAIN_EXEC)

# Цель для создания необходимых директорий.
directories:
	@$(MKDIR_P) $(OBJ_DIR)/scripts/cipher $(OBJ_DIR)/scripts $(LIB_DIR) $(BIN_DIR)

$(LIB_DIR)/libVERNAM.so: $(OBJ_VERNAM) $(OBJ_IO)
	@echo "Linking shared library $@"
	$(CXX) -shared $^ -o $@

$(LIB_DIR)/libAUTOKEY.so: $(OBJ_AUTOKEY) $(OBJ_IO)
	@echo "Linking shared library $@"
	$(CXX) -shared $^ -o $@

$(LIB_DIR)/libSALSA20.so: $(OBJ_SALSA20) $(OBJ_IO)
	@echo "Linking shared library $@"
	$(CXX) -shared $^ -o $@

$(MAIN_EXEC): $(OBJ_MAIN) $(OBJ_IO) $(LIB_DIR)/libVERNAM.so $(LIB_DIR)/libAUTOKEY.so $(LIB_DIR)/libSALSA20.so
	@echo "Компоновка $@..."
	$(CXX) $(CXXFLAGS) $(OBJ_MAIN) $(OBJ_IO) -L$(LIB_DIR) -lVERNAM -lAUTOKEY -lSALSA20 -Wl,-rpath='$$ORIGIN/../lib' -ldl -o $@

$(OBJ_DIR)/%.o: %.cpp
	@echo "Компиляция $< в $@"
	@$(MKDIR_P) $(@D) # Создаем директорию для объектного файла, если ее нет
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP

-include $(DEPS)

clean:
	@echo "Удаление директории build..."
	@rm -rf $(BUILD_DIR)

# Конфигурация для .deb пакета
DEB_NAME = cipherApp
DEB_VERSION = 1.0
DEB_ARCH = $(shell dpkg --print-architecture)
DEB_DIR = $(BUILD_DIR)/$(DEB_NAME)_$(DEB_VERSION)_$(DEB_ARCH)

.PHONY: deb

# Цель для создания .deb пакета
deb: all
	@echo "Создание структуры для DEB пакета..."
    # 1. Создаем иерархию папок
	@$(MKDIR_P) $(DEB_DIR)/DEBIAN
	@$(MKDIR_P) $(DEB_DIR)/usr/local/bin
	@$(MKDIR_P) $(DEB_DIR)/usr/local/lib

	@echo "Копирование скомпилированных файлов..."
    # 2. Копируем исполняемый файл и библиотеки
	cp $(MAIN_EXEC) $(DEB_DIR)/usr/local/bin/
	cp $(LIB_DIR)/*.so $(DEB_DIR)/usr/local/lib/

	@echo "Создание файла control..."
    # 3. Создаем control файл
	@echo "Package: $(DEB_NAME)" > $(DEB_DIR)/DEBIAN/control
	@echo "Version: $(DEB_VERSION)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Architecture: $(DEB_ARCH)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Maintainer: shuballity" >> $(DEB_DIR)/DEBIAN/control
	@echo "Description: простое приложение для шифрования файлов" >> $(DEB_DIR)/DEBIAN/control

	@echo "Создание postinst скрипта..."
    # 4. Создаем postinst скрипт
	@echo "#!/bin/sh" > $(DEB_DIR)/DEBIAN/postinst
	@echo "set -e" >> $(DEB_DIR)/DEBIAN/postinst
	@echo "ldconfig" >> $(DEB_DIR)/DEBIAN/postinst
	@echo "exit 0" >> $(DEB_DIR)/DEBIAN/postinst
	@chmod +x $(DEB_DIR)/DEBIAN/postinst

	@echo "Сборка DEB пакета..."
    # 5. Собираем пакет
	dpkg-deb --build $(DEB_DIR)

	@echo "DEB пакет готов: $(DEB_DIR).deb"
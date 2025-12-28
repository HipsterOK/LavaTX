#!/bin/bash

# Скрипт для сборки и заливки прошивки LavaTX для TBS Tango/Mambo
# Использует последовательность команд, проверенную на практике

set -e  # Выход при ошибке

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
FIRMWARE_BIN="$BUILD_DIR/firmware.bin"

echo "=== Сборка и заливка LavaTX для TBS ==="
echo "Проект: $PROJECT_DIR"

# Проверка зависимостей
check_deps() {
    local deps=("cmake" "make" "arm-none-eabi-gcc" "st-flash")
    local missing=()

    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done

    if [ ${#missing[@]} -ne 0 ]; then
        echo "Ошибка: отсутствуют зависимости: ${missing[*]}"
        echo "Установите их:"
        echo "  Ubuntu/Debian: sudo apt install cmake make gcc-arm-none-eabi stlink-tools"
        exit 1
    fi
}

check_deps

echo "1. Очистка старой сборки..."
rm -rf "$BUILD_DIR"

echo "2. Создание директории сборки..."
mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

echo "3. Настройка CMake для TBS TANGO..."
cmake "$PROJECT_DIR" -DPCB=TANGO

echo "4. Сборка прошивки..."
make -j$(nproc) firmware

if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "Ошибка: файл прошивки не найден: $FIRMWARE_BIN"
    exit 1
fi

echo "Прошивка собрана успешно! Размер: $(stat -c%s "$FIRMWARE_BIN") байт"
echo "Файл прошивки: $FIRMWARE_BIN"
echo ""
echo "Для заливки выполните:"
echo "  st-flash --reset write $FIRMWARE_BIN 0x08000000"
echo ""
echo "ИЛИ используйте скрипт для заливки:"
echo "  ./flash.sh"
echo ""
echo "Проверьте прошивку перед заливкой!"

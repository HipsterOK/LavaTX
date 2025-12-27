#!/bin/bash

# Скрипт для заливки прошивки LavaTX на TBS Tango/Mambo
# Использует ST-Link

set -e  # Выход при ошибке

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
FIRMWARE_BIN="$BUILD_DIR/firmware.bin"

echo "=== Заливка прошивки LavaTX на TBS ==="

# Проверка наличия файла прошивки
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "Ошибка: файл прошивки не найден: $FIRMWARE_BIN"
    echo "Сначала выполните сборку: ./build.sh"
    exit 1
fi

echo "Файл прошивки найден: $FIRMWARE_BIN"
echo "Размер: $(stat -c%s "$FIRMWARE_BIN") байт"
echo ""

# Проверка ST-Link
if ! command -v st-flash &> /dev/null; then
    echo "Ошибка: st-flash не найден"
    echo "Установите: sudo apt install stlink-tools"
    exit 1
fi

echo "Проверка подключения ST-Link..."
if ! st-flash --version >/dev/null 2>&1; then
    echo "Ошибка: ST-Link не найден или не работает"
    exit 1
fi

echo "ST-Link готов к работе"
echo ""

echo "ВНИМАНИЕ! Убедитесь, что:"
echo "  - ST-Link подключен к компьютеру"
echo "  - ST-Link подключен к плате TBS"
echo "  - Плата TBS в режиме программирования (BOOT)"
echo ""

read -p "Продолжить заливку? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Заливка отменена"
    exit 0
fi

echo "Заливка прошивки..."
st-flash --reset write "$FIRMWARE_BIN" 0x08000000

echo ""
echo "Готово! Прошивка успешно залита."
echo "Перезагрузите устройство и выньте его из режима программирования."

#!/bin/bash

# Скрипт для заливки прошивки TBS Tango через ST-Link
# Использует st-flash для программирования STM32F4

set -e  # Выход при ошибке

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
FIRMWARE_BIN="$BUILD_DIR/firmware.bin"

echo "=== Заливка прошивки TBS Tango ==="
echo "Проект: $PROJECT_DIR"

# Проверка зависимостей
if ! command -v st-flash &> /dev/null; then
    echo "Ошибка: st-flash не найден"
    echo "Установите: sudo apt install stlink-tools"
    exit 1
fi

# Проверка файла прошивки
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "Ошибка: файл прошивки не найден: $FIRMWARE_BIN"
    echo "Сначала соберите прошивку: ./build.sh"
    exit 1
fi

echo "Файл прошивки: $FIRMWARE_BIN"
echo "Размер: $(stat -c%s "$FIRMWARE_BIN") байт"
echo ""

# Предупреждение
echo "⚠️  ВНИМАНИЕ:"
echo "  - Убедитесь, что ST-Link подключен"
echo "  - Убедитесь, что TBS Tango включен и в режиме программирования"
echo "  - Процесс займет несколько секунд"
echo ""

read -p "Продолжить заливку? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Заливка отменена"
    exit 0
fi

echo ""
echo "Заливка прошивки..."
echo "Команда: st-flash --reset write $FIRMWARE_BIN 0x08000000"
echo ""

if st-flash --reset write "$FIRMWARE_BIN" 0x08000000; then
    echo ""
    echo "✅ Заливка завершена успешно!"
    echo "Отключите ST-Link и перезагрузите TBS Tango"
else
    echo ""
    echo "❌ Ошибка заливки!"
    echo "Проверьте подключение ST-Link и повторите попытку"
    exit 1
fi

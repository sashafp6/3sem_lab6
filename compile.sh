#!/bin/bash

echo "Компиляция программы магазина мебели"

# Установите зависимости
sudo apt-get update
sudo apt-get install -y libpq-dev g++

# Компиляция
g++ -o furniture_store main.cpp -lpq -std=c++11 -Wall -Wextra

if [ $? -eq 0 ]; then
    echo " Компиляция успешна!"
    echo "Запуск программы: ./furniture_store"
else
    echo " Ошибка компиляции"
    exit 1
fi

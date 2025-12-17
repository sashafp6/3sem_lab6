#include "database.hpp"
#include "queries.hpp"
#include <iostream>
#include <iomanip>
#include <limits>

void clear_input() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    try {
        // Инициализация базы данных
        Database& db = Database::get_instance();
        
        if (!db.is_connected()) {
            std::cerr << "Не удалось подключиться к базе данных.\n";
            return 1;
        }
        
        Queries q;
        
        std::cout << "========================================\n";
        std::cout << "    СИСТЕМА УПРАВЛЕНИЯ МАГАЗИНОМ МЕБЕЛИ\n";
        std::cout << "========================================\n";
        
        while (true) {
            std::cout << "\n========== МЕНЮ ==========\n";
            std::cout << "1. Показать все товары\n";
            std::cout << "2. Показать всех клиентов\n";
            std::cout << "3. Выход\n";
            std::cout << "Выберите действие: ";
            
            int choice;
            std::cin >> choice;
            
            switch (choice) {
                case 1: {
                    auto products = q.get_all_products();
                    
                    std::cout << "\n--- СПИСОК ТОВАРОВ ---\n";
                    std::cout << std::left << std::setw(5) << "ID" 
                              << std::setw(30) << "Название" 
                              << std::setw(15) << "Категория"
                              << std::setw(10) << "Цена"
                              << std::setw(10) << "Кол-во" << std::endl;
                    std::cout << std::string(70, '-') << std::endl;
                    
                    for (const auto& p : products) {
                        std::cout << std::left << std::setw(5) << p.id 
                                  << std::setw(30) << (p.name.length() > 28 ? p.name.substr(0, 25) + "..." : p.name)
                                  << std::setw(15) << p.category
                                  << std::setw(10) << std::fixed << std::setprecision(2) << p.price
                                  << std::setw(10) << p.quantity << std::endl;
                    }
                    break;
                }
                case 2: {
                    auto customers = q.get_all_customers();
                    
                    std::cout << "\n--- СПИСОК КЛИЕНТОВ ---\n";
                    for (const auto& c : customers) {
                        c.print();
                    }
                    break;
                }
                case 3:
                    std::cout << "Выход из программы. До свидания!\n";
                    return 0;
                default:
                    std::cout << "Неверный выбор. Попробуйте снова.\n";
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

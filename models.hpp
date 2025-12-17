#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <iostream>
#include <ctime>

struct Product {
    int id;
    std::string name;
    std::string category;
    std::string description;
    double price;
    std::string dimensions;
    std::string material;
    int quantity;
    int warehouse_id;
    
    Product() : id(0), price(0.0), quantity(0), warehouse_id(0) {}
    
    void print() const {
        std::cout << "ID: " << id 
                  << " | Название: " << name 
                  << " | Категория: " << category
                  << " | Цена: " << price
                  << " | Количество: " << quantity
                  << " | Склад: " << warehouse_id << std::endl;
    }
};

struct Customer {
    int id;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::string phone;
    std::string address;
    
    Customer() : id(0) {}
    
    void print() const {
        std::cout << "ID: " << id 
                  << " | Имя: " << first_name << " " << last_name
                  << " | Email: " << email
                  << " | Телефон: " << phone << std::endl;
    }
};

struct Order {
    int id;
    int customer_id;
    std::string order_date;
    std::string status;
    double total_amount;
    
    Order() : id(0), customer_id(0), total_amount(0.0) {}
    
    void print() const {
        std::cout << "Заказ №" << id 
                  << " | Дата: " << order_date
                  << " | Статус: " << status
                  << " | Сумма: " << total_amount << std::endl;
    }
};

struct OrderItem {
    int order_id;
    int product_id;
    std::string product_name;
    int quantity;
    double unit_price;
    double subtotal;
};

struct SalesReport {
    int product_id;
    std::string product_name;
    std::string category;
    int total_sold;
    double total_revenue;
};

#endif

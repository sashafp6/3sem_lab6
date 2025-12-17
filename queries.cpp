#include "queries.hpp"
#include <iomanip>

Queries::Queries() : db(std::make_shared<Database>(Database::get_instance())) {}

std::vector<Product> Queries::get_all_products() {
    std::vector<Product> products;
    
    std::string query = R"(
        SELECT p.product_id, p.product_name, c.category_name, 
               p.description, p.price, p.dimensions, p.material,
               COALESCE(SUM(pw.quantity), 0) as total_quantity
        FROM products p
        JOIN categories c ON p.category_id = c.category_id
        LEFT JOIN product_warehouse pw ON p.product_id = pw.product_id
        GROUP BY p.product_id, p.product_name, c.category_name, 
                 p.description, p.price, p.dimensions, p.material
        ORDER BY p.product_id;
    )";
    
    try {
        pqxx::result res = db->execute_query(query);
        
        for (const auto& row : res) {
            products.push_back(result_to_product(row));
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка получения товаров: " << e.what() << std::endl;
    }
    
    return products;
}

std::vector<Customer> Queries::get_all_customers() {
    std::vector<Customer> customers;
    
    std::string query = "SELECT * FROM customers ORDER BY customer_id;";
    
    try {
        pqxx::result res = db->execute_query(query);
        
        for (const auto& row : res) {
            customers.push_back(result_to_customer(row));
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка получения клиентов: " << e.what() << std::endl;
    }
    
    return customers;
}

Product Queries::result_to_product(const pqxx::row& row) {
    Product product;
    product.id = row["product_id"].as<int>();
    product.name = row["product_name"].as<std::string>();
    product.category = row["category_name"].as<std::string>();
    product.description = row["description"].as<std::string>();
    product.price = row["price"].as<double>();
    product.dimensions = row["dimensions"].as<std::string>();
    product.material = row["material"].as<std::string>();
    product.quantity = row["total_quantity"].as<int>();
    product.warehouse_id = 0; // Не используем в упрощенной версии
    return product;
}

Customer Queries::result_to_customer(const pqxx::row& row) {
    Customer customer;
    customer.id = row["customer_id"].as<int>();
    customer.first_name = row["first_name"].as<std::string>();
    customer.last_name = row["last_name"].as<std::string>();
    customer.email = row["email"].as<std::string>();
    customer.phone = row["phone"].as<std::string>();
    customer.address = row["address"].as<std::string>();
    return customer;
}

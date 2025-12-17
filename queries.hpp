#ifndef QUERIES_HPP
#define QUERIES_HPP

#include "database.hpp"
#include "models.hpp"
#include <vector>
#include <memory>

class Queries {
private:
    std::shared_ptr<Database> db;
    
public:
    Queries();
    
    // Товары
    std::vector<Product> get_all_products();
    std::vector<Customer> get_all_customers();
    
private:
    Product result_to_product(const pqxx::row& row);
    Customer result_to_customer(const pqxx::row& row);
};

#endif

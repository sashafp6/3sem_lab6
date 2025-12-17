#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <iostream>
#include <vector>

class Database {
private:
    std::shared_ptr<pqxx::connection> connection;
    
public:
    Database(const std::string& conn_str);
    ~Database();
    
    bool is_connected() const;
    std::shared_ptr<pqxx::connection> get_connection() const;
    
    pqxx::result execute_query(const std::string& query);
    
    static Database& get_instance();
    
private:
    static std::shared_ptr<Database> instance;
};

#endif

#include "database.hpp"
#include <stdexcept>

std::shared_ptr<Database> Database::instance = nullptr;

Database::Database(const std::string& conn_str) {
    try {
        connection = std::make_shared<pqxx::connection>(conn_str);
        if (connection->is_open()) {
            std::cout << "✓ Подключение к базе данных: " << connection->dbname() << std::endl;
        } else {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }
    } catch (const std::exception &e) {
        std::cerr << "✗ Ошибка подключения: " << e.what() << std::endl;
        throw;
    }
}

Database::~Database() {
    if (connection && connection->is_open()) {
        connection->close();
        std::cout << "Соединение с базой данных закрыто." << std::endl;
    }
}

bool Database::is_connected() const {
    return connection && connection->is_open();
}

std::shared_ptr<pqxx::connection> Database::get_connection() const {
    return connection;
}

pqxx::result Database::execute_query(const std::string& query) {
    if (!is_connected()) {
        throw std::runtime_error("Нет подключения к базе данных");
    }
    
    try {
        pqxx::work tx(*connection);
        pqxx::result res = tx.exec(query);
        tx.commit();
        return res;
    } catch (const std::exception &e) {
        std::cerr << "Ошибка выполнения запроса: " << e.what() << std::endl;
        throw;
    }
}

Database& Database::get_instance() {
    if (!instance) {
        // ИЗМЕНИТЕ ПАРОЛЬ НА СВОЙ!
        std::string conn_str = "host=localhost port=5432 dbname=furniture_store "
                               "user=postgres password=postgres";
        instance = std::make_shared<Database>(conn_str);
    }
    return *instance;
}

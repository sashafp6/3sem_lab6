#include <iostream>
#include <string>
#include <vector>
#include <libpq-fe.h>
#include <iomanip>

using namespace std;

class FurnitureStoreDB {
private:
    PGconn* connection; // указатель на соединение с БД, PGconn* - тип из libpq, хранит информацию о подключении
    
public:
    // Конструктор класса
    FurnitureStoreDB(const string& conninfo) {
        connection = PQconnectdb(conninfo.c_str());  // подключаемся к БД
        if (PQstatus(connection) != CONNECTION_OK) { // проверяем успешность
            cerr << "Connection to database failed: " << PQerrorMessage(connection) << endl;
            exit(1);  // выходим при ошибке
        }
        cout << "Connected to database successfully!" << endl;
    }
    
    // Деструктор класса
    ~FurnitureStoreDB() {
        PQfinish(connection); // закрываем соединение при уничтожении объекта
    }
    // 1 метод Добавление нового клиента
    bool addClient(const string& firstName, const string& lastName, 
                   const string& email, const string& phone, 
                   const string& address) {
        // создаем SQL запрос с параметрами ($1, $2, $3, $4, $5)
        string query = "INSERT INTO clients (first_name, last_name, email, phone, address) "
                      "VALUES ($1, $2, $3, $4, $5)";
        // создаем массив параметров для запроса
        const char* params[5] = {
            firstName.c_str(),   
            lastName.c_str(),    
            email.c_str(),      
            phone.c_str(),      
            address.c_str()     
        };
        
        // выполняем параметризованный запрос к базе данных
        PGresult* res = PQexecParams(connection,  // соединение с БД
                                     query.c_str(),  // SQL запрос
                                     5,  // количество параметров
                                     NULL,  // типы параметров (автоопределение)
                                     params,  // массив значений параметров
                                     NULL,  // длины параметров
                                     NULL,  // форматы параметров
                                     0  // формат результата (0=текст, 1=бинарный)
                                    );
        
        // проверяем успешность выполнения команды
        bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
        // освобождаем память результата запроса
        PQclear(res);
        return success;
    }
    
    // 2. Метод Поиск товаров по категории
    void searchProductsByCategory(int categoryId) {
        // SQL запрос с для получения товаров и их категорий
        string query = 
            "SELECT p.product_id, p.product_name, p.price, p.stock_quantity, "
            "c.category_name "
            "FROM products p "  // таблица товаров с псевдонимом p
            "JOIN categories c ON p.category_id = c.category_id "  // соединяем с категориями
            "WHERE p.category_id = $1 AND p.stock_quantity > 0 "  // фильтр по категории и наличию
            "ORDER BY p.price";  // сортируем по цене
        
        // преобразуем ID категории из int в string
        string catIdStr = to_string(categoryId);
        // создаем массив с одним параметром
        const char* params[1] = {catIdStr.c_str()};
        
        // выполняем запрос с параметром
        PGresult* res = PQexecParams(connection, query.c_str(), 1, NULL, params, 
                                     NULL, NULL, 0);
        
        // проверяем успешность выполнения запроса с возвратом данных
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            // получаем количество строк в результате
            int rows = PQntuples(res);
            cout << "\nТовары в категории" << endl;
            for (int i = 0; i < rows; i++) {
                cout << "ID: " << PQgetvalue(res, i, 0)  // колонка 0 - product_id
                     << ", Название: " << PQgetvalue(res, i, 1)  // колонка 1 - product_name
                     << ", Цена: " << PQgetvalue(res, i, 2)  // колонка 2 - price
                     << ", В наличии: " << PQgetvalue(res, i, 3)  // колонка 3 - stock_quantity
                     << ", Категория: " << PQgetvalue(res, i, 4)  // колонка 4 - category_name
                     << endl;
            }
        } else {
            cout << "Нет товаров в данной категории." << endl;
        }
        PQclear(res);
    }
    
    // 3. Метод Создание нового заказа
    int createOrder(int clientId, const string& shippingAddress) {
        // SQL запрос для создания заказа с возвратом ID
        string query = 
            "INSERT INTO orders (client_id, shipping_address) "
            "VALUES ($1, $2) RETURNING order_id";  // RETURNING возвращает сгенерированный ID
        
        // преобразуем clientId в string для параметра
        string clientIdStr = to_string(clientId);
        // массив параметров: client_id и адрес доставки
        const char* params[2] = {clientIdStr.c_str(), shippingAddress.c_str()};
        
        // выполняем запрос
        PGresult* res = PQexecParams(connection, query.c_str(), 2, NULL, params, 
                                     NULL, NULL, 0);
        
        // инициализируем orderId значением -1 (ошибка по умолчанию)
        int orderId = -1;
        // проверяем успешность и наличие возвращенного значения
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            // преобразуем строковое значение в целое число
            orderId = atoi(PQgetvalue(res, 0, 0));  // строка 0, колонка 0
            cout << "Заказ создан! Номер заказа: " << orderId << endl;
        } else {
            cout << "Ошибка при создании заказа." << endl;
            cout << "Возможно, клиент с ID " << clientId << " не существует." << endl;
        }
        PQclear(res);
        // возвращаем ID заказа или -1 при ошибке
        return orderId;
    }
    
    // 4. Метод Добавление товара в заказ
    bool addProductToOrder(int orderId, int productId, int quantity) {
        // проверка наличия товара на складе
        if (!checkStock(productId, quantity)) {
            cout << "Товара недостаточно на складе." << endl;
            return false;  // если товара недостаточно, возвращаем false
        }
        
        // получаем текущую цену товара из базы данных
        string priceQuery = "SELECT price FROM products WHERE product_id = $1";
        string prodIdStr = to_string(productId);
        const char* priceParams[1] = {prodIdStr.c_str()};
        
        // выполняем запрос для получения цены
        PGresult* priceRes = PQexecParams(connection, priceQuery.c_str(), 1, NULL, 
                                          priceParams, NULL, NULL, 0);
        
        // проверяем успешность запроса и наличие товара
        if (PQresultStatus(priceRes) != PGRES_TUPLES_OK || PQntuples(priceRes) == 0) {
            cout << "Товар с ID " << productId << " не найден." << endl;
            PQclear(priceRes);  // освобождаем память
            return false;  // возвращаем false при ошибке
        }
        
        // преобразуем строковое значение цены в double
        double unitPrice = atof(PQgetvalue(priceRes, 0, 0));  // atof - ASCII to float
        PQclear(priceRes);
        
        // SQL запрос для добавления товара в заказ
        string query = 
            "INSERT INTO order_items (order_id, product_id, quantity, unit_price) "
            "VALUES ($1, $2, $3, $4)";
        
        // подготавливаем параметры для запроса
        string orderIdStr = to_string(orderId);
        string qtyStr = to_string(quantity);
        string priceStr = to_string(unitPrice);  // преобразуем double в string
        
        // массив параметров для запроса
        const char* params[4] = {
            orderIdStr.c_str(),  // $1 - ID заказа
            prodIdStr.c_str(),   // $2 - ID товара
            qtyStr.c_str(),      // $3 - количество
            priceStr.c_str()     // $4 - цена за единицу
        };
        
        // Выполняем запрос добавления товара в заказ
        PGresult* res = PQexecParams(connection, query.c_str(), 4, NULL, params, 
                                     NULL, NULL, 0);
        
        // Проверяем успешность выполнения команды
        bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
        
        if (success) {
            cout << "Товар добавлен в заказ!" << endl;
            // Обновляем общую сумму заказа после добавления товара
            updateOrderTotal(orderId);
            // Обновляем остатки товара на складе
            updateProductStock(productId, quantity);
        } else {
            cout << "Ошибка при добавлении товара в заказ." << endl;
            cout << "Возможно, заказ с ID " << orderId << " не существует." << endl;
        }
        PQclear(res);
        return success;
    }
    
    // 5. Метод: Обновление общей суммы заказа
    void updateOrderTotal(int orderId) {
        // SQL запрос для обновления суммы заказа
        // используем подзапрос для вычисления суммы всех позиций заказа
        string query = 
            "UPDATE orders SET total_amount = "
            "(SELECT COALESCE(SUM(subtotal), 0) FROM order_items WHERE order_id = $1) "
            "WHERE order_id = $2";
        
        // преобразуем orderId в string
        string orderIdStr = to_string(orderId);
        // массив параметров (один параметр используется дважды)
        const char* params[2] = {orderIdStr.c_str(), orderIdStr.c_str()};
        
        // выполняем запрос обновления
        PGresult* res = PQexecParams(connection, query.c_str(), 2, NULL, params, 
                                     NULL, NULL, 0);
        PQclear(res);
    }
    
    // 6. Метод: Обновление остатков товара
    void updateProductStock(int productId, int quantity) {
        // SQL запрос для уменьшения остатка товара
        string query = 
            "UPDATE products SET stock_quantity = stock_quantity - $1 "
            "WHERE product_id = $2";
        
        // преобразуем параметры в string
        string prodIdStr = to_string(productId);
        string qtyStr = to_string(quantity);
        // массив параметров: количество для вычитания и ID товара
        const char* params[2] = {qtyStr.c_str(), prodIdStr.c_str()};
        
        // выполняем запрос обновления
        PGresult* res = PQexecParams(connection, query.c_str(), 2, NULL, params, 
                                     NULL, NULL, 0);
        PQclear(res);
    }
    
    // 7. Метод: Получение статистики продаж
    void getSalesStatistics() {
        // SQL запрос с несколькими JOIN и агрегатными функциями
        string query = 
            "SELECT "
            "c.category_name, "  // название категории
            "COUNT(DISTINCT oi.order_id) as orders_count, "  // количество уникальных заказов
            "SUM(oi.quantity) as total_quantity, "  // общее количество проданных товаров
            "SUM(oi.subtotal) as total_revenue, "   // общая выручка
            "AVG(oi.unit_price) as avg_price "      // средняя цена товаров в заказах
            "FROM order_items oi "  // основная таблица - позиции заказов
            "JOIN products p ON oi.product_id = p.product_id "  // соединяем с товарами
            "JOIN categories c ON p.category_id = c.category_id "  // соединяем с категориями
            "JOIN orders o ON oi.order_id = o.order_id "  // соединяем с заказами
            "WHERE o.status != 'cancelled' "  // исключаем отмененные заказы
            "GROUP BY c.category_name "  // группируем по категориям
            "HAVING SUM(oi.subtotal) > 0 "  // фильтруем группы с выручкой > 0
            "ORDER BY total_revenue DESC";  // сортируем по выручке (убывание)
        
        // Выполняем запрос (без параметров)
        PGresult* res = PQexec(connection, query.c_str());
        
        // Проверяем успешность выполнения
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);  // количество строк результата
            if (rows > 0) {
                cout << "\nСтатистика продаж по категориям " << endl;
                // заголовки колонок с форматированием
                cout << left << setw(20) << "Категория"  // setw - ширина колонки
                     << setw(15) << "Заказов" 
                     << setw(15) << "Количество" 
                     << setw(15) << "Выручка" 
                     << setw(15) << "Ср. цена" << endl;
                // разделительная линия
                cout << string(80, '-') << endl;
                
                // выводим данные по строкам
                for (int i = 0; i < rows; i++) {
                    cout << left << setw(20) << PQgetvalue(res, i, 0)  // категория
                         << setw(15) << PQgetvalue(res, i, 1)  // количество заказов
                         << setw(15) << PQgetvalue(res, i, 2)  // количество товаров
                         << setw(15) << PQgetvalue(res, i, 3)  // выручка
                         << setw(15) << PQgetvalue(res, i, 4) << endl;  // средняя цена
                }
            } else {
                cout << "\nНет данных для статистики." << endl;
            }
        } else {
            cout << "\nОшибка при получении статистики." << endl;
        }
        PQclear(res);
    }
    
    // 8. Метод: Поиск клиентов с наибольшими заказами
    void getTopClients(int limit = 5) {
        // SQL запрос для получения топ клиентов по потраченной сумме
        string query = 
            "SELECT c.client_id, c.first_name, c.last_name, c.email, "
            "COUNT(o.order_id) as total_orders, "  // количество заказов клиента
            "COALESCE(SUM(o.total_amount), 0) as total_spent "  // общая потраченная сумма
            "FROM clients c "  // основная таблица - клиенты
            "LEFT JOIN orders o ON c.client_id = o.client_id "  // LEFT JOIN для всех клиентов
            "WHERE o.status != 'cancelled' OR o.order_id IS NULL "  // исключаем отмененные заказы
            "GROUP BY c.client_id "  // группируем по клиентам
            "ORDER BY total_spent DESC "  // сортируем по потраченной сумме (убывание)
            "LIMIT $1";  // ограничение количества результатов
        
        // преобразуем limit в string для параметра
        string limitStr = to_string(limit);
        const char* params[1] = {limitStr.c_str()};
        
        // выполняем параметризованный запрос
        PGresult* res = PQexecParams(connection, query.c_str(), 1, NULL, params, 
                                     NULL, NULL, 0);
        
        // проверяем успешность выполнения
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            cout << "\nТоп клиентов" << endl;
            if (rows > 0) {
                // выводим данные каждого клиента
                for (int i = 0; i < rows; i++) {
                    cout << "ID: " << PQgetvalue(res, i, 0)  // client_id
                         << ", Имя: " << PQgetvalue(res, i, 1) << " " << PQgetvalue(res, i, 2)  // имя и фамилия
                         << ", Email: " << PQgetvalue(res, i, 3)  // email
                         << ", Заказов: " << PQgetvalue(res, i, 4)  // количество заказов
                         << ", Потрачено: " << PQgetvalue(res, i, 5)  // общая сумма
                         << endl;
                }
            } else {
                cout << "Нет данных о клиентах." << endl;
            }
        } else {
            cout << "\nОшибка при получении списка клиентов." << endl;
        }
        PQclear(res);
    }
    
    // 9. Метод: Обновление статуса заказа
    bool updateOrderStatus(int orderId, const string& status) {
        // SQL запрос для обновления статуса заказа
        string query = "UPDATE orders SET status = $1 WHERE order_id = $2";
        
        // подготавливаем параметры
        string orderIdStr = to_string(orderId);
        const char* params[2] = {status.c_str(), orderIdStr.c_str()};
        
        // выполняем запрос
        PGresult* res = PQexecParams(connection, query.c_str(), 2, NULL, params, 
                                     NULL, NULL, 0);
        
        // проверяем успешность выполнения команды
        bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
        if (success) {
            cout << "Статус заказа обновлен!" << endl;
        } else {
            cout << "Ошибка при обновлении статуса." << endl;
            cout << "Возможно, заказ с ID " << orderId << " не существует." << endl;
        }
        PQclear(res);
        return success;
    }
    
    // 10. Метод: Получение деталей заказа
    void getOrderDetails(int orderId) {
        // SQL запрос с несколькими JOIN для получения полной информации о заказе
        string query = 
            "SELECT o.order_id, o.order_date, o.status, o.total_amount, "
            "c.first_name, c.last_name, "  // данные клиента
            "p.product_name, oi.quantity, oi.unit_price, oi.subtotal "  // данные позиций заказа
            "FROM orders o "  // основная таблица - заказы
            "JOIN clients c ON o.client_id = c.client_id "  // INNER JOIN с клиентами
            "LEFT JOIN order_items oi ON o.order_id = oi.order_id "  // LEFT JOIN с позициями
            "LEFT JOIN products p ON oi.product_id = p.product_id "  // LEFT JOIN с товарами
            "WHERE o.order_id = $1 "  // фильтр по ID заказа
            "ORDER BY p.product_name";  // сортируем по названию товара
        
        // преобразуем orderId в string для параметра
        string orderIdStr = to_string(orderId);
        const char* params[1] = {orderIdStr.c_str()};
        
        // выполняем запрос
        PGresult* res = PQexecParams(connection, query.c_str(), 1, NULL, params, 
                                     NULL, NULL, 0);
        
        // проверяем успешность выполнения
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            if (rows > 0) {
                cout << "\n Детали заказа #" << orderId << " ===" << endl;
                // Выводим общую информацию о заказе (из первой строки)
                cout << "Клиент: " << PQgetvalue(res, 0, 4) << " " << PQgetvalue(res, 0, 5) << endl;
                cout << "Дата: " << PQgetvalue(res, 0, 1) << endl;
                cout << "Статус: " << PQgetvalue(res, 0, 2) << endl;
                cout << "Общая сумма: " << PQgetvalue(res, 0, 3) << endl;
                
                bool hasItems = false;  // флаг наличия товаров в заказе
                cout << "\nПозиции заказа:" << endl;
                
                // перебираем все строки результата (могут быть несколько позиций)
                for (int i = 0; i < rows; i++) {
                    // проверяем наличие товара (product_name может быть NULL для заказов без товаров)
                    if (PQgetvalue(res, i, 6) != NULL) {
                        hasItems = true;  // устанавливаем флаг
                        // выводим информацию о позиции заказа
                        cout << "  - " << PQgetvalue(res, i, 6)  // название товара
                             << " x" << PQgetvalue(res, i, 7)    // количество
                             << " по " << PQgetvalue(res, i, 8)  // цена за единицу
                             << " = " << PQgetvalue(res, i, 9) << endl;  // сумма по позиции
                    }
                }
                if (!hasItems) {
                    cout << "  В заказе нет товаров." << endl;
                }
            } else {
                cout << "Заказ с ID " << orderId << " не найден." << endl;
            }
        } else {
            cout << "Ошибка при получении деталей заказа." << endl;
        }
        PQclear(res);
    }
    
    // 11. Метод: Проверка наличия товара на складе
    bool checkStock(int productId, int requestedQuantity) {
        // SQL запрос для получения остатка товара
        string query = "SELECT stock_quantity FROM products WHERE product_id = $1";
        
        // подготавливаем параметр
        string prodIdStr = to_string(productId);
        const char* params[1] = {prodIdStr.c_str()};
        
        // выполняем запрос
        PGresult* res = PQexecParams(connection, query.c_str(), 1, NULL, params, 
                                     NULL, NULL, 0);
        
        bool available = false;  // результат проверки (по умолчанию false)
        // проверяем успешность выполнения и наличие результата
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            // преобразуем строковое значение остатка в int
            int stock = atoi(PQgetvalue(res, 0, 0));  // atoi - из ASCII в целое число
            // проверяем достаточно ли товара на складе
            available = (stock >= requestedQuantity);
        }
        PQclear(res);
        return available;
    }
    
    // 12. Метод: Поиск дубликатов email клиентов
    void findDuplicateEmails() {
        // SQL запрос с GROUP BY и HAVING для поиска дубликатов
        string query = 
            "SELECT email, COUNT(*) as duplicate_count "  // email и количество повторений
            "FROM clients "  // таблица клиентов
            "GROUP BY email "  // группируем по email
            "HAVING COUNT(*) > 1";  // фильтруем группы с количеством > 1
        
        // выполняем запрос (без параметров)
        PGresult* res = PQexec(connection, query.c_str());
        
        // проверяем успешность выполнения
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            if (rows > 0) {
                // если найдены дубликаты, выводим их
                cout << "\n Найдены дубликаты email" << endl;
                for (int i = 0; i < rows; i++) {
                    cout << "Email: " << PQgetvalue(res, i, 0)  // email
                         << ", Дубликатов: " << PQgetvalue(res, i, 1) << endl;  // количество
                }
            } else {
                cout << "\nДубликаты email не найдены." << endl;
            }
        } else {
            cout << "\nОшибка при поиске дубликатов email." << endl;
        }
        PQclear(res);
    }
    
    // 13. Метод: Показать всех клиентов
    void showAllClients() {
        // SQL запрос для получения всех клиентов
        string query = 
            "SELECT client_id, first_name, last_name, email, phone, address "
            "FROM clients ORDER BY client_id";  // сортируем по ID клиента
        
        // выполняем запрос
        PGresult* res = PQexec(connection, query.c_str());
        
        // проверяем успешность выполнения
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            cout << "\nСписок всех клиентов" << endl;
            // заголовки колонок с форматированием
            cout << left << setw(5) << "ID" 
                 << setw(15) << "Имя" 
                 << setw(15) << "Фамилия" 
                 << setw(25) << "Email" 
                 << setw(15) << "Телефон" << endl;
            // разделительная линия
            cout << string(75, '-') << endl;
            
            if (rows > 0) {
                // выводим данные каждого клиента
                for (int i = 0; i < rows; i++) {
                    cout << left << setw(5) << PQgetvalue(res, i, 0)  // ID
                         << setw(15) << PQgetvalue(res, i, 1)  // имя
                         << setw(15) << PQgetvalue(res, i, 2)  // фамилия
                         << setw(25) << PQgetvalue(res, i, 3)  // email
                         << setw(15) << PQgetvalue(res, i, 4) << endl;  // телефон
                }
                cout << "\nВсего клиентов: " << rows << endl;
            } else {
                cout << "Нет зарегистрированных клиентов." << endl;
            }
        } else {
            cout << "\nОшибка при получении списка клиентов." << endl;
        }
        PQclear(res);
    }
};

void displayMenu() {
    cout << "\n=== Система управления магазином мебели ===" << endl;
    cout << "1. Добавить нового клиента" << endl;
    cout << "2. Поиск товаров по категории" << endl;
    cout << "3. Создать новый заказ" << endl;
    cout << "4. Добавить товар в заказ" << endl;
    cout << "5. Показать статистику продаж" << endl;
    cout << "6. Показать топ клиентов" << endl;
    cout << "7. Обновить статус заказа" << endl;
    cout << "8. Показать детали заказа" << endl;
    cout << "9. Проверить наличие товара" << endl;
    cout << "10. Найти дубликаты email" << endl;
    cout << "11. Показать всех клиентов" << endl;
    cout << "0. Выход" << endl;
    cout << "Выберите действие: ";
}

int main() {
    // подключение к базе данных
    string conninfo = "host=localhost dbname=furniture_store user=postgres password=123456";
    FurnitureStoreDB db(conninfo);
    
    int choice;
    do {
        displayMenu();
        cin >> choice;
        cin.ignore(); // очистка буфера
        
        switch (choice) {
            case 1: {
                // Добавление нового клиента
                string firstName, lastName, email, phone, address;
                cout << "Имя: ";
                getline(cin, firstName);
                cout << "Фамилия: ";
                getline(cin, lastName);
                cout << "Email: ";
                getline(cin, email);
                cout << "Телефон: ";
                getline(cin, phone);
                cout << "Адрес: ";
                getline(cin, address);
                
                if (db.addClient(firstName, lastName, email, phone, address)) {
                    cout << "Клиент успешно добавлен!" << endl;
                } else {
                    cout << "Ошибка при добавлении клиента." << endl;
                    cout << "Возможно, клиент с таким email уже существует." << endl;
                }
                break;
            }
            
            case 2: {
                // Поиск товаров по категории
                int categoryId;
                cout << "Введите ID категории (1-Диваны, 2-Кровати, 3-Столы, 4-Стулья, 5-Шкафы): ";
                cin >> categoryId;
                db.searchProductsByCategory(categoryId);
                break;
            }
            
            case 3: {
                // Создание нового заказа
                int clientId;
                string address;
                
                // Сначала покажем клиентов для удобства
                cout << "\nСоздание нового заказа:" << endl;
                cout << "Для просмотра списка клиентов выберите пункт 11 в меню." << endl;
                cout << "ID клиента: ";
                cin >> clientId;
                cin.ignore();
                cout << "Адрес доставки: ";
                getline(cin, address);
                
                int orderId = db.createOrder(clientId, address);
                if (orderId != -1) {
                    // Заказ создан успешно
                }
                break;
            }
            
            case 4: {
                // Добавление товара в заказ
                int orderId, productId, quantity;
                cout << "Номер заказа: ";
                cin >> orderId;
                cout << "ID товара (для просмотра товаров используйте пункт 2): ";
                cin >> productId;
                cout << "Количество: ";
                cin >> quantity;
                
                db.addProductToOrder(orderId, productId, quantity);
                break;
            }
            
            case 5:
                // Статистика продаж
                db.getSalesStatistics();
                break;
                
            case 6:
                // Топ клиентов
                db.getTopClients();
                break;
                
            case 7: {
                // Обновление статуса заказа
                int orderId;
                string status;
                cout << "Номер заказа: ";
                cin >> orderId;
                cin.ignore();
                cout << "Новый статус (pending/processing/shipped/delivered/cancelled): ";
                getline(cin, status);
                
                db.updateOrderStatus(orderId, status);
                break;
            }
            
            case 8: {
                // Детали заказа
                int orderId;
                cout << "Номер заказа: ";
                cin >> orderId;
                db.getOrderDetails(orderId);
                break;
            }
            
            case 9: {
                // Проверка наличия товара
                int productId, quantity;
                cout << "ID товара: ";
                cin >> productId;
                cout << "Необходимое количество: ";
                cin >> quantity;
                
                if (db.checkStock(productId, quantity)) {
                    cout << "Товар есть в наличии!" << endl;
                } else {
                    cout << "Товара недостаточно на складе." << endl;
                }
                break;
            }
            
            case 10:
                // Поиск дубликатов email
                db.findDuplicateEmails();
                break;
                
            case 11:
                // Показать всех клиентов
                db.showAllClients();
                break;
                
            case 0:
                cout << "Выход из программы..." << endl;
                break;
                
            default:
                cout << "Неверный выбор. Попробуйте снова." << endl;
        }
        
        // Пауза для удобства чтения
        if (choice != 0) {
            cout << "\nНажмите Enter для продолжения...";
            cin.ignore();
        }
        
    } while (choice != 0);
    
    return 0;
}

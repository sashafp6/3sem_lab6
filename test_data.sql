-- Очистка таблиц
DELETE FROM order_items;
DELETE FROM orders;
DELETE FROM products;
DELETE FROM categories;
DELETE FROM clients;

-- Сброс автоинкрементных счетчиков
ALTER SEQUENCE clients_client_id_seq RESTART WITH 1;
ALTER SEQUENCE categories_category_id_seq RESTART WITH 1;
ALTER SEQUENCE products_product_id_seq RESTART WITH 1;
ALTER SEQUENCE orders_order_id_seq RESTART WITH 1;
ALTER SEQUENCE order_items_order_item_id_seq RESTART WITH 1;

-- КЛИЕНТЫ 
INSERT INTO clients (first_name, last_name, email, phone, address) VALUES
('Анна', 'Иванова', 'anna@mail.ru', '+79161111111', 'Москва, ул. Ленина 10'),
('Иван', 'Петров', 'ivan@gmail.com', '+79162222222', 'СПб, пр. Мира 25'),
('Мария', 'Сидорова', 'maria@yandex.ru', '+79163333333', 'Екатеринбург, ул. Пушкина 15');

--  КАТЕГОРИИ
INSERT INTO categories (category_name, description) VALUES
('Диваны', 'Мягкая мебель для гостиной'),
('Кровати', 'Спальная мебель'),
('Столы', 'Обеденные и рабочие столы'),
('Стулья', 'Сидения для дома и офиса');

-- ТОВАРЫ
INSERT INTO products (product_name, description, price, stock_quantity, category_id) VALUES
('Диван угловой "Комфорт"', 'Угловой диван с ортопедическими подушками', 45000.00, 10, 1),
('Диван прямой "Милан"', 'Прямой диван еврокнижка', 32000.00, 8, 1),
('Кровать двуспальная "Люкс"', 'Кровать 180x200 см, массив дерева', 85000.00, 5, 2),
('Кровать односпальная', 'Металлическая кровать 90x200 см', 15000.00, 12, 2),
('Обеденный стол "Семейный"', 'Стол из массива дерева 120x80 см', 28000.00, 7, 3),
('Компьютерный стол "Офис"', 'Угловой компьютерный стол с полками', 12000.00, 15, 3),
('Стул обеденный "Классик"', 'Деревянный стул с мягким сиденьем', 3500.00, 25, 4),
('Стул офисный "Эргономик"', 'Офисное кресло с регулировкой', 12500.00, 10, 4);

-- ЗАКАЗЫ
INSERT INTO orders (client_id, order_date, status, shipping_address) VALUES
(1, '2024-03-10 14:30:00', 'delivered', 'Москва, ул. Ленина 10'),
(2, '2024-03-12 11:15:00', 'processing', 'СПб, пр. Мира 25'),
(1, '2024-03-15 09:45:00', 'pending', 'Москва, ул. Ленина 10');

-- ПОЗИЦИИ ЗАКАЗОВ
INSERT INTO order_items (order_id, product_id, quantity, unit_price) VALUES
(1, 1, 1, 45000.00),  -- Диван угловой
(1, 5, 1, 28000.00),  -- Обеденный стол
(1, 7, 4, 3500.00),   -- 4 стула
(2, 3, 1, 85000.00),  -- Кровать двуспальная
(3, 6, 1, 12000.00);  -- Компьютерный стол

-- ОБНОВЛЕНИЕ СУММ ЗАКАЗОВ
UPDATE orders o                     -- Обновляем таблицу orders
SET total_amount = (               -- Устанавливаем total_amount равным...
  SELECT COALESCE(SUM(subtotal), 0)   -- Сумма subtotal, если NULL - 0
  FROM order_items oi               -- Из таблицы order_items
  WHERE oi.order_id = o.order_id    -- Только позиции ЭТОГО заказа
)

-- ОБНОВЛЕНИЕ ОСТАТКОВ ТОВАРОВ
UPDATE products p SET stock_quantity = stock_quantity - ( --для каждого товара (products p), вычисляем: сколько его купили во ВСЕХ заказах, вычитаем это количество из stock_quantity
  SELECT COALESCE(SUM(oi.quantity), 0)
  FROM order_items oi
  WHERE oi.product_id = p.product_id
);

-- ПРОВЕРКА
SELECT 'clients: ' || COUNT(*)::text FROM clients
UNION ALL
SELECT 'categories: ' || COUNT(*)::text FROM categories
UNION ALL
SELECT 'products: ' || COUNT(*)::text FROM products
UNION ALL
SELECT 'orders: ' || COUNT(*)::text FROM orders
UNION ALL
SELECT 'order_items: ' || COUNT(*)::text FROM order_items; 
--     COUNT(*) - считает строки в таблице, ::text - преобразует число в текст, 'clients: ' || COUNT(*)::text - конкатенация: "clients: 3", UNION ALL - объединяет все SELECT в одну таблицу

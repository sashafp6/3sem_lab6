INNER JOIN с WHERE и агрегатной функцией
--Получение общей выручки по категориям
SELECT 
    c.category_name,
    COUNT(DISTINCT o.order_id) as order_count, --подсчет уникальных заказов
    SUM(oi.subtotal) as total_revenue, --сумма выручки
    AVG(oi.unit_price) as average_price -- средняя цена товара в заказах
FROM categories c
INNER JOIN products p ON c.category_id = p.category_id --соединяем с товарами
INNER JOIN order_items oi ON p.product_id = oi.product_id --соединяем с позициями заказов
INNER JOIN orders o ON oi.order_id = o.order_id --соединяем с заказами
WHERE o.status = 'delivered' --только доставленные заказы
GROUP BY c.category_name -- группируем по названию категории
HAVING SUM(oi.subtotal) > 1000 --группы с выручкой меньше 1к
ORDER BY total_revenue DESC; --сортировка выручки по убыванию

-- LEFT JOIN для поиска клиентов без заказов
SELECT 
    c.client_id,
    c.first_name,
    c.last_name,
    c.email,
    COUNT(o.order_id) as order_count --подсчет заказов
FROM clients c --основная табл клиенты
LEFT JOIN orders o ON c.client_id = o.client_id --все клиенты + их заказы
GROUP BY c.client_id --группировка по клиенту
HAVING COUNT(o.order_id) = 0 --оставляем тех у кого 0 заказов
ORDER BY c.registration_date DESC; --сначала новые клиенты

-- Подзапрос для поиска товаров выше средней цены
SELECT 
    product_name,
    price,
    stock_quantity,
    (SELECT category_name FROM categories WHERE category_id = p.category_id) as category --подзапрос в SELECT
FROM products p --из основн табл продукты
WHERE price > (SELECT AVG(price) FROM products) --подзапрос по средней цене
ORDER BY price DESC;

-- FULL JOIN для анализа всех заказов и клиентов
SELECT 
    COALESCE(c.first_name || ' ' || c.last_name, 'Нет клиента') as client_name, --если пусто, нет клиента
    COALESCE(o.order_id::text, 'Нет заказа') as order_id, -- нет заказа
    o.status,
    o.total_amount
FROM clients c
FULL JOIN orders o ON c.client_id = o.client_id -- все клиенты + все заказы
WHERE o.order_date >= CURRENT_DATE - INTERVAL '30 days' --заказы за последние 30 дней
ORDER BY o.order_date DESC;

-- RIGHT JOIN для анализа категорий с товарами и без
SELECT 
    c.category_name,
    COUNT(p.product_id) as product_count, --подсчет товаров в категории
    COALESCE(SUM(p.stock_quantity), 0) as total_stock --если пусто, то 0
FROM products p
RIGHT JOIN categories c ON p.category_id = c.category_id --все категории + товары
GROUP BY c.category_name
ORDER BY product_count DESC;

-- Запрос с UNION для объединения данных
SELECT 
    'Клиент' as type, --добавление колонки клиент
    first_name || ' ' || last_name as name,
    email as contact,
    registration_date as date
FROM clients
WHERE registration_date >= '2024-01-01'

UNION ALL --объединение двух SELECT

SELECT 
    'Заказ' as type,
    'Заказ #' || order_id::text as name, --преобразование числа в текст
    status as contact,
    order_date as date
FROM orders
WHERE order_date >= '2024-01-01'
ORDER BY date DESC; --сортировка объединенного результата

--  Запрос с оконной функцией для ранжирования
SELECT 
    product_name,
    price,
    category_name,
    RANK() OVER (PARTITION BY p.category_id ORDER BY price DESC) as price_rank,--ранг внутри категории
    DENSE_RANK() OVER (ORDER BY stock_quantity DESC) as stock_rank --ранг по всему результату
FROM products p
JOIN categories c ON p.category_id = c.category_id
WHERE stock_quantity > 0; --остаток на складе больше 0

-- Запрос с CASE для категоризации цен
SELECT 
    product_name,
    price,
    CASE 
        WHEN price < 100 THEN 'Бюджетный'
        WHEN price BETWEEN 100 AND 500 THEN 'Средний'
        WHEN price BETWEEN 501 AND 1000 THEN 'Премиум'
        ELSE 'Люкс'
    END as price_category,
    stock_quantity
FROM products
ORDER BY price;

--  Рекурсивный запрос для иерархии категорий
WITH RECURSIVE category_tree AS (
    SELECT category_id, category_name, NULL::INTEGER as parent_id, 1 as level --начальный запрос
    FROM categories
    WHERE category_name = 'Мебель для гостиной'
    
    UNION ALL --объединяем с рекурсивной частью
    
    SELECT c.category_id, c.category_name, ct.category_id, ct.level + 1 --рекурсивная часть
    FROM categories c
    JOIN category_tree ct ON c.category_id = ct.parent_id --соединяем с предыдущим кровнем
)
SELECT * FROM category_tree; --вывод результата

-- Запрос с EXISTS для проверки наличия заказов
SELECT 
    c.first_name,
    c.last_name,
    c.email
FROM clients c
WHERE EXISTS ( --проверка существования подзапроса
    SELECT 1 --возвращение константы
    FROM orders o 
    WHERE o.client_id = c.client_id -- Для каждого клиента проверяем его заказы
    AND o.total_amount > 500 --заказы меньше 500
    AND o.order_date >= CURRENT_DATE - INTERVAL '90 days' -- за последние 90 дней
)
ORDER BY c.last_name;

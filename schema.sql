-- таблица клиентов
CREATE TABLE clients (
    client_id SERIAL PRIMARY KEY,          -- уникальный ID, автоувеличение, первичный ключ
    first_name VARCHAR(50) NOT NULL,       -- имя, строка до 50 символов, обязательно
    last_name VARCHAR(50) NOT NULL,        -- фамилия, строка до 50 символов, обязательно
    email VARCHAR(100) UNIQUE NOT NULL,    -- еmail, до 100 символов, уникальный, обязательно
    phone VARCHAR(20),                     -- телефон, до 20 символов, может быть NULL
    registration_date DATE DEFAULT CURRENT_DATE,  -- дата регистрации, по умолчанию сегодня
    address TEXT                           -- адрес, текст без ограничения длины
);

-- таблица категорий
CREATE TABLE categories (
    category_id SERIAL PRIMARY KEY,        -- уникальный ID категории
    category_name VARCHAR(100) NOT NULL,   -- название категории, обязательно
    description TEXT                       -- описание категории (может быть NULL)
);

-- таблица товаров
CREATE TABLE products (
    product_id SERIAL PRIMARY KEY,                           -- уникальный ID товара
    product_name VARCHAR(200) NOT NULL,                      -- название товара, обязательно
    description TEXT,                                        -- описание товара
    price DECIMAL(10,2) NOT NULL CHECK (price > 0),         -- цена, 10 цифр/2 после запятой, >0
    stock_quantity INTEGER DEFAULT 0 CHECK (stock_quantity >= 0),  --INTEGER = целое число (-2 млрд до +2 млрд),     DEFAULT 0 = если не указать, будет 0, CHECK (stock_quantity >= 0) = остаток не может быть отрицательным
    category_id INTEGER REFERENCES categories(category_id) ON DELETE SET NULL,  -- REFERENCES = внешний ключ (foreign key), categories(category_id) = ссылается на столбец category_id в таблице categories, ON DELETE SET NULL = если категорию удалить, у товара category_id станет NULL    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP          -- TIMESTAMP = дата и время (год-месяц-день час:минута:секунда),DEFAULT CURRENT_TIMESTAMP = автоматически ставит текущие дату и время
);

-- таблица заказов
CREATE TABLE orders (
    order_id SERIAL PRIMARY KEY,                                       -- уникальный ID заказа
    client_id INTEGER REFERENCES clients(client_id) ON DELETE CASCADE, -- ссылка на клиента, Если удалить клиента, все его заказы тоже удалятся
    order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                    -- дата заказа, авто
    status VARCHAR(20) DEFAULT 'pending' CHECK (status IN ('pending', 'processing', 'shipped', 'delivered', 'cancelled')),  -- Ограничивает возможные значения статуса, IN = должен быть одним из перечисленных, рабочий процесс: ожидание - обработка - отправлен - доставлен/отменен
    total_amount DECIMAL(10,2) DEFAULT 0,                              -- общая сумма заказа
    shipping_address TEXT NOT NULL                                     -- адрес доставки
);

-- таблица позиций заказа
CREATE TABLE order_items (
    order_item_id SERIAL PRIMARY KEY,                                   -- уникальный ID позиции
    order_id INTEGER REFERENCES orders(order_id) ON DELETE CASCADE,    -- ссылка на заказ, удаляем заказ - удаляем все его позиции
    product_id INTEGER REFERENCES products(product_id) ON DELETE RESTRICT,  -- ссылка на товар, НЕЛЬЗЯ удалить товар, если он есть в заказах
    quantity INTEGER NOT NULL CHECK (quantity > 0),                    -- количество, >0
    unit_price DECIMAL(10,2) NOT NULL,                                 --цена на момент заказа
    subtotal DECIMAL(10,2) GENERATED ALWAYS AS (quantity * unit_price) STORED  -- вычисляемое поле, GENERATED ALWAYS AS = значение всегда вычисляется по формуле (quantity * unit_price) = формула расчета, STORED = значение хранится в базе 
);

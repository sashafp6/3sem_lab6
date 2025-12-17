cmake_minimum_required(VERSION 3.10)
project(FurnitureStore)

set(CMAKE_CXX_STANDARD 17)

# Находим PostgreSQL
find_package(PostgreSQL REQUIRED)

# Ищем libpqxx вручную
find_path(PQXX_INCLUDE_DIR pqxx/pqxx
    PATHS /usr/include /usr/local/include
)

find_library(PQXX_LIBRARY pqxx
    PATHS /usr/lib /usr/local/lib
)

if(PQXX_INCLUDE_DIR AND PQXX_LIBRARY)
    message(STATUS "Found libpqxx: ${PQXX_LIBRARY}")
    message(STATUS "libpqxx include dir: ${PQXX_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "libpqxx not found!")
endif()

# Исходные файлы
set(SOURCES
    main.cpp
    database.cpp
    queries.cpp
)

# Создаем исполняемый файл
add_executable(furniture_store ${SOURCES})

# Связываем библиотеки
target_include_directories(furniture_store PRIVATE 
    ${PostgreSQL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIR}
)

target_link_libraries(furniture_store 
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARY}
)

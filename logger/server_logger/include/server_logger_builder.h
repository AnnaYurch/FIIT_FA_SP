#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H //защита файла от повтоного подключения
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H //если не определено - определяем

#include <logger_builder.h>
#include <unordered_map>
#include "server_logger.h"

class server_logger_builder final : public logger_builder
{

    std::string _destination; //арес назначения для логгирования: аррес сервера/путь к файлу
    std::string _format = "[%d %t] %s: %m"; //..

    std::unordered_map<logger::severity, std::pair<std::string, bool>> _output_streams;
    //unordered_map - контернер, который хранит данные в виде ключ-значение (хранит элементы в хаусе)
    //logger::severity - определяет важность сообщения (error, critical, warnings...)
    //pair - хранит 2 элемента (string - путь, куда нужно выводить лог, bool - активен поток или нет)
public:
    //конструктор, которй задает адрес по умолчанию
    server_logger_builder() : _destination("http://127.0.0.1:9200"){}

public:

    logger_builder& add_file_stream(
        std::string const &stream_file_path,
        logger::severity severity) & override; //& - только для lvalue, override - переопределяем метод

    logger_builder& add_console_stream(
        logger::severity severity) & override;

    logger_builder& transform_with_configuration(
        std::string const &configuration_file_path,
        std::string const &configuration_path) & override;

    logger_builder& set_destination(const std::string& dest) & override;

    logger_builder& clear() & override;

    logger_builder& set_format(const std::string& format) & override;

    [[nodiscard]] logger *build() const override;
    //[[nodiscard]] - если результат не используется, будет предупреждение
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H
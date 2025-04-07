#include <not_implemented.h>
#include "../include/server_logger_builder.h"

#include <fstream> //..
#include <sstream> //..

logger_builder& server_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity) &
{
    _output_streams[severity] = {stream_file_path, true};
    return *this;
    //добавляем в карту (уровеньважности - путькфайлуифлаг) чтобы разделять контексты severity
}

logger_builder& server_logger_builder::add_console_stream(
    logger::severity severity) &
{
    _output_streams[severity] = {"console", true}; //ЧТО ЗА console?
    return *this;
    //аналогично, но выод идет четко в консоль
}

logger_builder& server_logger_builder::transform_with_configuration( //ЧО?
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    std::ifstream config_file(configuration_file_path);
    if (config_file.is_open()) {
        std::string line;
        while (std::getline(config_file, line)) {
            // Example: parse configuration and populate _output_streams
            // This should handle parsing based on your specific configuration format
        }
    }
    return *this;
}

logger_builder& server_logger_builder::clear() & //чистим все настройки потока вывода
{
    _output_streams.clear(); //ЧТО ЗА clear?
    return *this;
}

logger *server_logger_builder::build() const //строим и возвращаем server_logger
{
    return new server_logger(_destination, _output_streams); //ЧТО ТУТ ПРОИСХОДИТ?
}






logger_builder& server_logger_builder::set_destination(const std::string& dest) &
{
    _destination = dest;
    return *this;
    //устанавливаем путь или адрес куда будет направлен лог
}

logger_builder& server_logger_builder::set_format(const std::string &format) &
{
    _format = format;
    return *this;
    //установка формата сообщений
}
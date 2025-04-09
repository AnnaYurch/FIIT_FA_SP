#include <not_implemented.h>
#include "../include/server_logger_builder.h"

#include <fstream> //..
#include <sstream> //..
#include <nlohmann/json.hpp> //..


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
    if (!config_file.is_open())
        return *this;

    nlohmann::json config_json;
    try {
        config_file >> config_json;
    } catch (...) {
        return *this;
    }

    nlohmann::json* target = &config_json;
    size_t pos = 0;
    while (pos < configuration_path.size()) {
        auto next = configuration_path.find('/', pos);
        std::string key = configuration_path.substr(pos, next - pos);
        if (!target->contains(key)) return *this;
        target = &(*target)[key];
        if (next == std::string::npos) break;
        pos = next + 1;
    }

    if (target->contains("destination"))
        _destination = (*target)["destination"].get<std::string>();

    if (target->contains("format"))
        _format = (*target)["format"].get<std::string>();

    if (target->contains("streams")) {
        for (const auto& stream : (*target)["streams"]) {
            auto severity_str = stream["severity"].get<std::string>();
            logger::severity sev;

            if (severity_str == "trace") sev = logger::severity::trace;
            else if (severity_str == "debug") sev = logger::severity::debug;
            else if (severity_str == "information") sev = logger::severity::information;
            else if (severity_str == "warning") sev = logger::severity::warning;
            else if (severity_str == "error") sev = logger::severity::error;
            else if (severity_str == "critical") sev = logger::severity::critical;
            else continue;

            std::string file_path = stream.value("file", "");
            bool to_console = stream.value("console", false);

            _output_streams[sev] = std::make_pair(file_path, to_console);
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
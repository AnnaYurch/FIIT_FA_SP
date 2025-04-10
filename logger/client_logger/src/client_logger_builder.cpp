#include <filesystem>
#include <utility>
#include <not_implemented.h>
#include "../include/client_logger_builder.h"
#include <not_implemented.h>

using namespace nlohmann;

logger_builder& client_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity) & //только для lvalue
{
    auto canonical_path = std::filesystem::weakly_canonical(stream_file_path).string();
    //делаем путь красивым (без /../) и строчкой

    //не перезаписывает значение, если ключ уже есть
    auto[it, inserted] = _output_streams.try_emplace( //it - итератор на найденную/вставленую пару
        severity,
        std::make_pair(std::forward_list<client_logger::refcounted_stream>(), false)
    );
    //sev - pair: список лог-файлов и флаг писать лм в консоль

    //it->second.first - дает нам нам список с файлами
    it->second.first.emplace_front(canonical_path);

    return *this;
}

logger_builder& client_logger_builder::add_console_stream(
    logger::severity severity) &
{
    auto[it, inserted] = _output_streams.try_emplace(
        severity,
        std::make_pair(std::forward_list<client_logger::refcounted_stream>(), false)
    );

    it->second.second = true;

    return *this;
}

logger_builder& client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    throw not_implemented("logger_builder& client_logger_builder::transform_with_configuration(std::string const &,std::string const &) &", "your code should be here...");
}

logger_builder& client_logger_builder::clear() &
{
    throw not_implemented("logger_builder& client_logger_builder::clear() &", "your code should be here...");
}

logger *client_logger_builder::build() const
{
    throw not_implemented("logger *client_logger_builder::build() const", "your code should be here...");
}

logger_builder& client_logger_builder::set_format(const std::string &format) &
{
    throw not_implemented("logger_builder& client_logger_builder::set_format(const std::string &) &", "your code should be here...");
}

void client_logger_builder::parse_severity(logger::severity sev, nlohmann::json& j)
{
    throw not_implemented("void client_logger_builder::parse_severity(logger::severity, nlohmann::json&)", "your code should be here...");
}

logger_builder& client_logger_builder::set_destination(const std::string &format) &
{
    throw not_implemented("logger_builder *client_logger_builder::set_destination(const std::string &format)", "invalid call");
}
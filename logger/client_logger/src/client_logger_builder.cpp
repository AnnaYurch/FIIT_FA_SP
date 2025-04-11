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

//читает json-конфигурацию и применяет настройки из нее для логгера
logger_builder& client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path, //путь к json
    std::string const &configuration_path) & //ключик для нахождения нужного блока ("log_config")
{
    std::ifstream file(configuration_file_path);
    //открываем файл с конфигурацией

    if (!file.is_open())
        throw std::ios_base::failure("File " + configuration_file_path + " could not be opened");
        //исключение кот выбрасывается при ошибке рыботы с потоками ввода/вывода

    json data = json::parse(file);
    //разбираем содержимое json и превращаем его в "структуру данных"

    file.close();

    auto it = data.find(configuration_path);
    //ищем, нужный нам обьект (логер) по пути

    //если не нашли или это не обьект
    if (it == data.end() || !it->is_object()) //нашли но не {}
        return *this;

    parse_severity(logger::severity::trace, (*it)["trace"]); //ищем список файлов
    parse_severity(logger::severity::debug, (*it)["debug"]); //нужно ли выводить в консоль
    parse_severity(logger::severity::information, (*it)["information"]); //и + эти настройки в _output_streams
    parse_severity(logger::severity::warning, (*it)["warning"]);
    parse_severity(logger::severity::error, (*it)["error"]);
    parse_severity(logger::severity::critical, (*it)["critical"]);

    auto format = it->find("format");

    if (format != it->end() && format->is_string()) 
    {
        _format = format.value(); 
    }

    return *this;
}

logger_builder &client_logger_builder::clear() &
{
    _output_streams.clear(); //очищаем настройки с путями и ур логгирования
    _format = "%m"; //дефолтный формат

    return *this;
}

logger *client_logger_builder::build() const //создаем логер на основе настроек
{
    return new client_logger(_output_streams, _format);
}

//вручную установаливаем формат логирования
logger_builder &client_logger_builder::set_format(const std::string &format) &
{
    _format = format;
    return *this;
}

void client_logger_builder::parse_severity(logger::severity sev, nlohmann::json& j)
//обьект из json
{
    if (j.empty() || !j.is_object())
        return;

    //находим местечно куда записывать настройки
    auto it = _output_streams.find(sev);

    auto data_it = j.find("paths");
    if (data_it != j.end() && data_it->is_array())
    {
        json data = *data_it; //копируем массив

        for (const json& js: data)
        {
            if (js.empty() || !js.is_string())
                continue;
            const std::string& path = js;
            if (it == _output_streams.end()) //если пуст
            {
                it = _output_streams.emplace(sev, std::make_pair(std::forward_list<client_logger::refcounted_stream>(),
                                                                 false)).first;
            }

            it->second.first.emplace_front(std::filesystem::weakly_canonical(path).string());
        }
    }

    auto console = j.find("console");

    if (console != j.end() && console->is_boolean())
    {
        if (it == _output_streams.end())
        {
            it = _output_streams.emplace(sev, std::make_pair(std::forward_list<client_logger::refcounted_stream>(), false)).first;
        }
        it->second.second = console->get<bool>();
    }
}

logger_builder &client_logger_builder::set_destination(const std::string &format) &
{
    return *this;
}
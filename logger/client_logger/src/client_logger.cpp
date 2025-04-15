#include <string>
#include <sstream>
#include <algorithm>
#include <utility>
#include "../include/client_logger.h"
#include <not_implemented.h>

//ключ: имя файла или идентификатор потока ... пара: счетчик ссылок и обьект потока вывода
//_global_streams - нужна для хранения всех потоков вывода, которые были использованы в проге (контейнер)
//нужно, чтобы не создавать один и тот же поток несколько раз
std::unordered_map<std::string, std::pair<size_t, std::ofstream>> client_logger::refcounted_stream::_global_streams;


logger& client_logger::log(const std::string &text, logger::severity severity) & //только на lvalue
{
    std::string formatted = make_format(text, severity); //получаем готовую строку для вывода

    //для каждого элемента в контейнере _output_streams
    //const auto& - const чтобы слуяайно не изменить sev, pair
    for (const auto& [sev, pair] : _output_streams) {
        if (sev != severity) continue; // ищем нужную severity
        const auto& [streams, is_console] = pair; // расспаковываем pair

        for (const auto& s : streams) { // идем по всем path и пишем сообщение
            if (s._stream.second != nullptr) { 
                *s._stream.second << formatted << std::endl; //записываем в поток строку
            }
        }

        if (is_console) {
            std::cout << formatted << std::endl;
        }
    }

    return *this;
}

std::string client_logger::make_format(const std::string &message, severity sev) const
{
    std::ostringstream res; //тип std::cout, но все записывается в строку

    for(auto cur_cur = _format.begin(), end_end = _format.end(); cur_cur != end_end; ++cur_cur)
    {
        flag type = flag::NO_FLAG;
        if (*cur_cur == '%')
        {
            type = char_to_flag(*(cur_cur + 1));
        }

        if (type != flag::NO_FLAG)
        {
            switch (type)
            {
                case flag::DATE:
                    res << current_date_to_string(); 
                    break;
                case flag::TIME:
                    res << current_time_to_string(); 
                    break;
                case flag::SEVERITY:
                    res << severity_to_string(sev); 
                    break;
                default:
                    res << message;
                    break;
            }
            ++cur_cur;
        } else
        {
            res << *cur_cur;
        }
    }

    return res.str(); //метод дурацкого класса сверху, кот возвращает строку, которую мы записали
}

//std::forward_list<refcounted_stream - список потоков файлов куда лог писать
client_logger::client_logger( //1 конструктор
        const std::unordered_map<logger::severity, std::pair<std::forward_list<refcounted_stream>, bool>> &streams,
        std::string format)
{
    _output_streams = streams;
    _format = std::move(format);
}

client_logger::flag client_logger::char_to_flag(char c) noexcept
{
    switch (c)
    {
        case 'd':
            return flag::DATE;
        case 't':
            return flag::TIME;
        case 's':
            return flag::SEVERITY;
        case 'm':
            return flag::MESSAGE;
        default:
            return flag::NO_FLAG;
    }
}

//копирующий конструктор
client_logger::client_logger(const client_logger &other)
{
    _output_streams = other._output_streams;
    _format = other._format;
}

//копирующий оператор присваивания (независимые копии)
client_logger &client_logger::operator=(const client_logger &other)
{
    if (this != &other) {
        _output_streams = other._output_streams;
        _format = other._format;
    }
    return *this; //чтоб цепочки можно было писать
}

//конструктор перемещения с rvalue (воруем внутр память)
//благодаря noexcept конструктор не выкинет исключений
client_logger::client_logger(client_logger &&other) noexcept
{
    _output_streams = std::move(other._output_streams);
    _format = std::move(other._format);
}

//оператор перемещения (забирает ресурсы временного обьекта)
client_logger &client_logger::operator=(client_logger &&other) noexcept
{
    if (this != &other) {
        _output_streams = std::move(other._output_streams);
        _format = std::move(other._format);
    }
    return *this;
}

client_logger::~client_logger() noexcept = default;

//конструктор refcounted_stream
client_logger::refcounted_stream::refcounted_stream(const std::string &path)
{
    //путь к файлу (куда запишем лог) и указатель на поток вывода (в файл)
    _stream = std::make_pair(path, nullptr);
}

//////////////////////////////////////

//хрень чтобы один файл не открывался несколько раз, если его неск логов юзают
client_logger::refcounted_stream::refcounted_stream(const client_logger::refcounted_stream &oth)
{
    //копируем имя файла, а дальше чекаем открыт ли он 
    _stream = std::make_pair(oth._stream.first, nullptr);
    auto cur = _global_streams.find(oth._stream.first);

    if (cur != _global_streams.end()) //если нашли
    {
        ++cur->second.first;
        _stream.second = &cur->second.second;
    } else
    {
        //вставка файла в глобальный список
        auto inserted = _global_streams.emplace(_stream.first, std::make_pair<size_t>(1, std::ofstream(oth._stream.first)));

        if (!inserted.second || !inserted.first->second.second.is_open())
        {
            if (inserted.second) //если вставка получилась, но файл не открылся
            {
                _global_streams.erase(inserted.first); //killl
            }
            throw std::ios_base::failure("File " + oth._stream.first + " could not be opened");
        }

        _stream.second = &inserted.first->second.second;
        //сохраняем укаатель на поток
    }
}

client_logger::refcounted_stream &
client_logger::refcounted_stream::operator=(const client_logger::refcounted_stream &oth)
{
    if (this == &oth)
        return *this;

    if (_stream.second != nullptr)
    {
        auto it = _global_streams.find(_stream.first);

        if (it != _global_streams.end())
        {
            --it->second.first;

            if (it->second.first == 0)
            {
                it->second.second.close();
                _global_streams.erase(it);
            }
        }
    }

    _stream.first = oth._stream.first;
    _stream.second = oth._stream.second;

    if (_stream.second != nullptr)
    {
        auto it = _global_streams.find(_stream.first);
        ++it->second.first;
    }

    return *this;
}

client_logger::refcounted_stream::refcounted_stream(client_logger::refcounted_stream &&oth) noexcept
{
    std::swap(_stream.first, oth._stream.first);
    _stream.second = std::exchange(oth._stream.second, nullptr); //oth._stream.second == nullptr плюсом
}

client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(client_logger::refcounted_stream &&oth) noexcept
{
    if (this == &oth)
        return *this;

    std::swap(_stream.first, oth._stream.first);
    std::swap(_stream.second, oth._stream.second);

    return *this;
}

client_logger::refcounted_stream::~refcounted_stream()
{
    if (_stream.second != nullptr)
    {
        auto it = _global_streams.find(_stream.first);

        if (it != _global_streams.end())
        {
            --it->second.first;

            if (it->second.first == 0)
            {
                it->second.second.close();
                _global_streams.erase(it);
            }
        }
    }
}

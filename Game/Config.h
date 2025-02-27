#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config//отвечает за обработку настроек из файла settings.json, а после закружает и сохраняет в config.
{
  public:
    Config()
    {
        reload();
    }

    void reload() //Функция reload() отвечает за загрузку настроек из файла settings.json. Она открывает файл, считывает его содержимое в объект config (типа json), а затем закрывает файл. Это позволяет обновить настройки, хранящиеся в объекте Config, при каждом вызове этой функции.
    {
        std::ifstream fin(project_path + "settings.json");
        fin >> config;
        fin.close();
    }

    auto operator()(const string &setting_dir, const string &setting_name) const //Оператор круглые скобки(operator()) определён для объекта класса Config,чтобы обеспечить удобный доступ к значениям настроек. Он позволяет использовать объект Config как функцию, принимающую два аргумента: путь к директории настроек (setting_dir) и имя конкретной настройки (setting_name). В результате возвращается значение настройки из загруженного конфигурационного файла.
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config;
};

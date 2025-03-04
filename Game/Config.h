#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config//�������� �� ��������� �������� �� ����� settings.json, � ����� ��������� � ��������� � config.
{
  public:
    Config()
    {
        reload();
    }

    void reload() //������� reload() �������� �� �������� �������� �� ����� settings.json. ��� ��������� ����, ��������� ��� ���������� � ������ config (���� json), � ����� ��������� ����. ��� ��������� �������� ���������, ���������� � ������� Config, ��� ������ ������ ���� �������.
    {
        std::ifstream fin(project_path + "settings.json");
        fin >> config;
        fin.close();
    }

    auto operator()(const string &setting_dir, const string &setting_name) const //�������� ������� ������(operator()) �������� ��� ������� ������ Config,����� ���������� ������� ������ � ��������� ��������. �� ��������� ������������ ������ Config ��� �������, ����������� ��� ���������: ���� � ���������� �������� (setting_dir) � ��� ���������� ��������� (setting_name). � ���������� ������������ �������� ��������� �� ������������ ����������������� �����.
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config;
};

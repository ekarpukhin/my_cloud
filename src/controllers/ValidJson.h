#ifndef YASHKA_VALIDJSON_H
#define YASHKA_VALIDJSON_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <drogon/drogon.h>
#include "json.hpp"

#include "../models/SystemItem.h"

//Критерий ID для поиска элемента в базе данных
#define CRITERIA_ID( A ) Criteria(SystemItem::Cols::_id, CompareOperator::EQ, A)

using namespace std;
using json = nlohmann::json;
using namespace drogon;
using namespace orm;
using namespace drogon_model::sqlite3;


class Item
    /**
 * Класс переданного элемента из items
 * Содержит все поля и метод вывода их значений в консоль
 */
{
public:
    string id, type, parentId, url, date, children;
    long long size;
    Item(json &data) {
        if (!data["id"].empty()) id = data["id"];
        if (!data["type"].empty()) type = data["type"];
        if (!data["size"].empty()) size = data["size"];
        if (!data["parentId"].empty()) parentId = data["parentId"];
        if (!data["url"].empty()) url = data["url"];
        if (!data["date"].empty()) date = data["date"];
        if (!data["children"].empty()) children = data["children"];
    }

    void print(){
        cout << "------------------------\n";
        cout << "ID: " << id << "\n";
        cout << "TYPE: " << type << "\n";
        cout << "SIZE: " << size << "\n";
        cout << "URL: " << url << "\n";
        cout << "DATE: " << date << "\n";
        cout << "PID: " << parentId << "\n";
        cout << "CHi: " << children << "\n";
        cout << "------------------------\n";
    }
};

void remove_from(string&  str, char s)
/**
     * Удаляет передаваемый символ в строке
     */
{
    str.erase(std::remove(str.begin(), str.end(), s), str.end());
}

string remove_quotes(string str)
/**
     * Удаляет все кавычки из строки
     */
{
    string str2 = str;
    remove_from(str2, '\"');
    return str2;
}

bool validDate(const string& s)
/**
     * Проверяет валидность (правильность формата) передаваемый строки даты
     */
{
    const regex r(R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2}(?:\.\d*)?)((-(\d{2}):(\d{2})|Z)?)$)");
    return regex_match(s, r);
}

bool validType(const string & type)
/**
     * Определяет правильность передаваемого типа объекта
     */
{
    if (type.empty() || type != "FOLDER" && type != "FILE" ) return false;
    return true;
}

void updateSizeParents(const string& parent_id, const long long _size)
/**
     * Рекрсивно обновляет размер (добавляет) у родителсьских папок
     */
{
    if (parent_id.empty() || _size == 0) return;
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    auto parent_el = mp.findBy(CRITERIA_ID(parent_id))[0];
    long long new_size = 0;
    if (!parent_el.getValueOfSize().empty()) new_size = stoll(parent_el.getValueOfSize());
    new_size += _size;
    parent_el.setSize(to_string(new_size));
    mp.update(parent_el);
    updateSizeParents(parent_el.getValueOfParentid(), _size);
}

void updateDateParents(const string& parent_id, const string& new_date)
/**
     * Рекрсивно обновляет дату у родительских папок на передаваемую
     */
{
    if (parent_id.empty()) return;
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    auto parent_el = mp.findBy(CRITERIA_ID(parent_id))[0];
    parent_el.setDate(new_date);
    mp.update(parent_el);
    updateDateParents(parent_el.getValueOfParentid(), new_date);
    cout << "Success! " << parent_el.getValueOfId() <<"\n";
}

json remove_el_json(const string& old_json_str,const string& field , const string& _id)
/**
     * Удаляем соответствующий элемент из массива JSON в выбранном поле
     */
{
    cout << old_json_str << "\n";
    json new_json, old_json = json::parse(old_json_str);
    for (const auto& el: old_json[field]) {
        if (el != _id) new_json[field].push_back(el);
    }
    cout << new_json;
    return new_json;
}

int validJSON(json& data)
/**
     * Добавляет или обновляет элемент в базу данных
     * Если необходимо меняет Размеры и Даты у родительских папок
     */
{
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);

    if (data["items"].empty()) return 400;
    if (data["updateDate"].empty() || !validDate(data["updateDate"])) return 400;
    for (auto item_json: data["items"]){
        Item item(item_json);
        if (item_json["id"].empty()) return 400;
        if (item_json["type"].empty() || !validType(item.type)) return 400;
        auto el = mp.findBy(CRITERIA_ID(item.id));
        if (el.empty()) {
            //////////////// CREATE NEW ITEM /////////////////////
            if (!item_json["parentId"].empty()) {
                auto parent_el = mp.findBy(CRITERIA_ID(item.parentId));
                if (parent_el.empty()) return 400;
                json child_check;
                if (!parent_el[0].getValueOfChildren().empty()) {
                    child_check = json::parse(parent_el[0].getValueOfChildren());
                }
                child_check["children"].push_back(item.id);
                parent_el[0].setChildren(to_string(child_check));
                mp.update(parent_el[0]);
                if (item.type== "FILE") updateSizeParents(parent_el[0].getValueOfId(), item.size);
                updateDateParents(parent_el[0].getValueOfId(), data["updateDate"]);
            }
            SystemItem new_el;
            new_el.setId(item.id);
            if (item_json["url"].empty()) new_el.setUrlToNull();
            else new_el.setUrl(item.url);
            new_el.setDate(data["updateDate"]);
            if (item_json["parentId"].empty()) new_el.setParentidToNull();
            else new_el.setParentid(item.parentId);
            if (item_json["size"].empty()) new_el.setSizeToNull();
            else new_el.setSize(to_string(item.size));
            new_el.setType(item.type);
            new_el.setChildrenToNull();
            mp.insert(new_el);
        } else {
            ////////////// UPDATE ITEM and PARENT's children //////////////
            //New parent
            if (!item_json["parentId"].empty() &&
                (el[0].getValueOfParentid().empty() || el[0].getValueOfParentid() != item.parentId)){
                auto new_parent_el = mp.findBy(CRITERIA_ID(item.parentId));
                if (new_parent_el.empty()) return 400;
                json child_check;
                if (!new_parent_el[0].getValueOfChildren().empty()) {
                    child_check = json::parse(new_parent_el[0].getValueOfChildren());
                }
                child_check["children"].push_back(item.id);
                new_parent_el[0].setChildren(to_string(child_check));
                mp.update(new_parent_el[0]);
                if (el[0].getValueOfSize().empty()) updateSizeParents(new_parent_el[0].getValueOfId(), stoll(el[0].getValueOfSize()));
                updateDateParents(new_parent_el[0].getValueOfId(), data["updateDate"]);
            }
            //Old parent
            if (!el[0].getValueOfParentid().empty() && (item_json["parentId"].empty() || item.parentId != el[0].getValueOfParentid())) {
                auto parent_el = mp.findBy(CRITERIA_ID(el[0].getValueOfParentid()));
                parent_el[0].setChildren(to_string(remove_el_json(parent_el[0].getValueOfChildren(), "children", item.id)));
                mp.update(parent_el[0]);
                if (el[0].getValueOfSize().empty()) updateSizeParents(parent_el[0].getValueOfId(), - stoll(el[0].getValueOfSize()));
                updateDateParents(parent_el[0].getValueOfId(), data["updateDate"]);
            }
            if (!el[0].getValueOfParentid().empty() && item.type == "FILE")
                if (el[0].getValueOfSize().empty() && !item_json["size"].empty() ||
                        el[0].getValueOfSize().empty() && item_json["size"].empty() ||
                        el[0].getValueOfSize() != item_json["size"]) {
                    auto parent_el = mp.findBy(CRITERIA_ID(el[0].getValueOfParentid()));
                    parent_el[0].setChildren(to_string(remove_el_json(parent_el[0].getValueOfChildren(), "children", item.id)));
                    long long delta = 0;
                    if (!item_json["size"].empty()) delta = item_json["size"];
                    delta -=  stoll(el[0].getValueOfSize());
                    updateSizeParents(parent_el[0].getValueOfId(), delta);
                }
            if (item_json["parentId"].empty()) el[0].setParentidToNull();
            else el[0].setParentid(item.parentId);
            if (item_json["url"].empty()) el[0].setUrlToNull();
            else el[0].setUrl(item.url);
            el[0].setDate(data["updateDate"]);
            if (item_json["size"].empty()) {
                if (el[0].getValueOfSize().empty()) el[0].setSizeToNull();
            } else el[0].setSize(to_string(item.size));

            mp.update(el[0]);
        }
    }

    return 200;
}

string imports_responseJson(const string& reqJSON_str)
/**
     * Запускает процесс обновления или обновления элемента
     * Принимает  JSON в формате строки от контроллера
     * Распаршивает его и отсылает на проверку
     * Возвращает JSON строку с кодом ответа
     * {
     *  "status": 200
     * } - все прошло успешно
     */
{
    json answer, reqJSON = json::parse(reqJSON_str);
    answer["status"] = validJSON(reqJSON);
    return answer.dump();
}

json searchChildren(const string& _id)
/**
     * Рекурсивный спуск по папкам и файлам с сбором информации о них в JSON
     * Передаваемый ID точно должен быть в БД
     */
{
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    json res;
    auto el = mp.findBy(CRITERIA_ID(_id))[0];
    res["type"] = (el.getValueOfType().empty())? "": el.getValueOfType();
    res["id"] = _id;
    if (!el.getValueOfSize().empty()) res["size"] = stoll(el.getValueOfSize());
    else res["size"] = 0;
    if (el.getValueOfUrl().empty()) res["url"] = nullptr;
    else res["url"] = el.getValueOfUrl();
    if (el.getValueOfParentid().empty()) res["parentId"] = nullptr;
    else res["parentId"] = el.getValueOfParentid();
    res["date"] = (el.getValueOfDate().empty())? "": el.getValueOfDate();
    if (!el.getValueOfChildren().empty()) {
        res["children"] = json::array();
        json child_check = json::parse(el.getValueOfChildren());
        for (const auto& child: child_check["children"]) {
            res["children"].push_back(searchChildren(remove_quotes(child.dump())));
        }
    } else res["children"] = nullptr;
    return res;
}

int nodesSearch(const string& _id, json& answer)
/**
     * Запускает рекурсивное прохождение по файлам и пакам для сбора информации в JSON
     * Если нет элемента с заданным ID, возвращает 404
     * Если все прошло успешно, возвращает 200
     * Создает поле "answer" в приходящем JSON
     */
{
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    auto el = mp.findBy(CRITERIA_ID(_id));
    if (el.empty()) return 404;
    answer["answer"] = searchChildren(_id);
    return 200;
}

string nodes_responseJson(const string& reqJSON_str)
/**
     * Запускает формирование JSON со всей информаией об элементе по его ID
     * Принимает  JSON в формате строки от контроллера
     * Распаршивает его и отсылает на проверку
     * Возвращает JSON строку с кодом ответа и сам получившийся JSON
     * {
     *  "status": 200
     *  "answer": {...JSON...}
     * } - все прошло успешно
     */
{
    json answer, reqJSON = json::parse(reqJSON_str);
    answer["status"] = nodesSearch(reqJSON.value("id", "id"), answer);
    cout << answer["status"] << "\n";
    return answer.dump();
}

void deleteChildren(const string &_id)
/**
     * Функция для рекусивного удаления обьектов от нынешнего по детям
     * При этом, объект с приходящим ID обязательно должен существовать
     */
{
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    auto el = mp.findBy(CRITERIA_ID(_id))[0];
    if (!el.getValueOfChildren().empty()) {
        json child_check = json::parse(el.getValueOfChildren());
        for (const auto& child: child_check["children"]) {
            deleteChildren(remove_quotes(child.dump()));
        }
    }
    mp.deleteBy(CRITERIA_ID(_id));
}

int nodesDelete(const string& _id, const string& date)
/**
     * Запускает рекурсивное удаление файлов и папок
     * Если нет элемента с заданным ID, возвращает 404
     * Если все прошло успешно, возвращает 200
     * Обновляет дату и размер родительских папок
     */
{
    if (date.empty() || !validDate(date)) return 400;
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    auto el = mp.findBy(CRITERIA_ID(_id));
    if (el.empty()) return 404;
    if (!el[0].getValueOfParentid().empty()) {
        auto parent_el = mp.findOne(CRITERIA_ID(el[0].getValueOfParentid()));
        cout << "1: " << el[0].getValueOfSize() << "\n";
        if (!el[0].getValueOfSize().empty()) updateSizeParents(parent_el.getValueOfId(), - stoll(el[0].getValueOfSize()));
        updateDateParents(parent_el.getValueOfId(), date);
        parent_el.setChildren(to_string(remove_el_json(parent_el.getValueOfChildren(), "children", _id)));
        mp.update(parent_el);
    }
    deleteChildren(_id);
    return 200;
}


string delete_responseJson(const string& reqJSON_str)
/**
     * Запускает процессы удаления обьекта и его детей по ID
     * Принимает  JSON в формате строки от контроллера
     * Распаршивает его и отсылает на проверку
     * Возвращает JSON строку с кодом ответа
     * {
     *  "status": 200
     * } - все прошло успешно
     */
{
    json answer, reqJSON = json::parse(reqJSON_str);
    answer["status"] = nodesDelete(reqJSON.value("id", "id"), reqJSON.value("params", "params"));
    cout << answer["status"] << "\n";
    return answer.dump();
}

string clear_responseJSON()
/**
     * Функция позволяющая чистить всю базу данных
     */
{
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    auto elements = mp.findAll();
    for (const auto& el: elements) {
        mp.deleteOne(el);
    }
    json res_json;
    res_json["status"] = 200;
    return res_json.dump();
}

long long c_toi(const char s)
/**
 * Возвращает символ в long long
 */
{
    return s - '0';
}

long long pars_date(const string& date)
/**
 * Превращает строку даты в long long
 */
{
    long long year = c_toi(date[0])*1000 + c_toi(date[1])*100 + c_toi(date[2]) * 10 + c_toi(date[3]);
    long long month = c_toi(date[5])*10 + c_toi(date[6]);
    long long dd = c_toi(date[8])*10 + c_toi(date[9]);
    long long hh = c_toi(date[11])*10 + c_toi(date[12]);
    long long mm = c_toi(date[14])*10 + c_toi(date[15]);
    long long ss = c_toi(date[17])*10 + c_toi(date[18]);
    long long ms = c_toi(date[20])*100 + c_toi(date[21]) * 10 + c_toi(date[22]);
//    return (((((year* 366 +  month) * 31 + dd) * 24 + hh) * 60 + mm) * 60 + ss) * 1000 + ms;
    return (((((year* 1000 +  month) * 100 + dd) * 100 + hh) * 100 + mm) * 100 + ss) * 1000 + ms;
}

bool in_diapason(const string& origin_date, const string& date_date)
/**
 * проверяет лежит ли дата элемента в указанном диапазоне
 */
{
    long long origin = pars_date(origin_date), date = pars_date(date_date);
    return abs(origin - date) <= 100*100*100*1000;
}

json updatesChildren(const SystemItem& el)
/**
 * Собирает JSON на отправку с информацией о элементе
 */
{
    json res;
    res["type"] = (el.getValueOfType().empty())? "": el.getValueOfType();
    res["id"] = el.getValueOfId();
    if (!el.getValueOfSize().empty()) res["size"] = stoll(el.getValueOfSize());
    else res["size"] = 0;
    if (el.getValueOfUrl().empty()) res["url"] = nullptr;
    else res["url"] = el.getValueOfUrl();
    if (el.getValueOfParentid().empty()) res["parentId"] = nullptr;
    else res["parentId"] = el.getValueOfParentid();
    res["date"] = (el.getValueOfDate().empty())? "": el.getValueOfDate();
    return res;
}

int nodesUpdates(const string& date, json& answer)
/**
     * Рекурсивный спуск по папкам и файлам с сбором информации о них в JSON
     * Находит все корневые папки и идет вниз по тем кто
     */
{
    if (date.empty() || !validDate(date)) return 400;
    auto dbClientPtr = drogon::app().getDbClient();
    Mapper<SystemItem> mp(dbClientPtr);
    json res;
    auto elements = mp.findAll();
    for (const auto& el: elements) {
        if (in_diapason(date, el.getValueOfDate())) {
            res.push_back(updatesChildren(el));
        }
    }
    answer["answer"] = res;
    return 200;
}

string updates_responseJSON(const string& reqJSON_str)
/**
     * Запускает процессы последовательного выявления элементов измененных за последние 24 часа
     * Принимает  JSON в формате строки от контроллера
     * Распаршивает его и отсылает на проверку
     * Возвращает JSON строку с кодом ответа и JSON с измененными за последние 24 часа элементами
     * {
     *  "status": 200,
     *  "answer": {...JSON...}
     * } - все прошло успешно
     */
{
    json answer, reqJSON = json::parse(reqJSON_str);
    answer["status"] = nodesUpdates(reqJSON.value("params", "params"), answer);
    cout << answer["status"] << "\n";
    return answer.dump();
}

#endif //YASHKA_VALIDJSON_H

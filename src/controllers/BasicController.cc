#include "BasicController.h"
#include "ValidJson.h"
#include <drogon/drogon.h>

#ifndef BODY_FAILED_JSON_PARSE
#define BODY_FAILED_JSON_PARSE "Some went wrong in Json-parse function"
#endif

std::string someFunction(std::string json_str) {
    std::cout << "Success!\n" << json_str;
    Json::Value answer;

    answer["status"] = 200;              //Test status code
    return answer.toStyledString();
}

void status_200(Json::Value answer, std::function<void (const HttpResponsePtr &)> callback)
/**
     * Возвращает статус сервера 200 и в зависимости от типа реквеста JSON и вызывает callback
     */
{
    if (answer["message"] == "Nodes history" || answer["message"] == "Updates" || answer["message"] == "Nodes") {
        auto response = HttpResponse::newHttpJsonResponse(answer["answer"]);
        response->setStatusCode(k200OK);
        callback(response);
    }
    else {
        auto response = HttpResponse::newHttpResponse();
        response->setStatusCode(k200OK);
        response->setBody(answer["message"].toStyledString());
        callback(response);
    }
}

void status_400(std::function<void (const HttpResponsePtr &)> callback)
/**
     * Возвращает статус сервера 400 и вызывает callback
     */
{
    Json::Value answer;
    answer["code"] = 400;
    answer["message"] = "Validation Failed";
    auto response = HttpResponse::newHttpJsonResponse(answer);
    response->setStatusCode(k400BadRequest);
    callback(response);
}

void status_404(std::function<void (const HttpResponsePtr &)> callback)
/**
     * Возвращает статус сервера 404 и вызывает callback
     */
{
    Json::Value answer;
    answer["code"] = 404;
    answer["message"] = "Item not found";
    auto response = HttpResponse::newHttpJsonResponse(answer);
    response->setStatusCode(k404NotFound);
    callback(response);
}

void response_to_localhost(bool successParse, Json::Value answer, std::function<void (const HttpResponsePtr &)> callback)
/**
     * Вызывает необходимую функцию для воозврата статутсы и в случае надобности JSON-a
     */
{
    if (!successParse) status_200(BODY_FAILED_JSON_PARSE, std::move(callback));
    else {
        if (answer["status"] == 200) status_200(answer, std::move(callback));
        if (answer["status"] == 400) status_400(std::move(callback));
        if (answer["status"] == 404) status_404(std::move(callback));
    }
}

void BasicController::imports(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback)
/**
     * Метод вызываемый переходе по путю /imports
     * Переводит в строку приходящий JSON, отсылает его дальше и делает response
     */
{
    LOG_DEBUG << "IMPORTS";
    Json::Value ret = *(req->getJsonObject());
    Json::Reader reader;
    Json::Value answer_from_BE;
    bool successParse = reader.parse(imports_responseJson(ret.toStyledString()), answer_from_BE);
    answer_from_BE["message"] = "Imports";
    cout << answer_from_BE << "\n";
    response_to_localhost(successParse, answer_from_BE, std::move(callback));
}

void BasicController::delete_id(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback,
                                const std::string _id, const std::string params)
/**
* Метод вызываемый переходе по путю /delete/{id}?={date}
*/
{
    LOG_DEBUG << "DELETING ID:" << _id;
    Json::Value ret;
    ret["id"] = _id;
    ret["params"] = params;
    Json::Reader reader;
    Json::Value answer_from_BE;
    bool successParse = reader.parse(delete_responseJson(ret.toStyledString()), answer_from_BE);
    answer_from_BE["message"] = "Delete";
    response_to_localhost(successParse, answer_from_BE, std::move(callback));
}

void BasicController::nodes_id(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string _id)
/**
     * Метод вызываемый переходе по путю /nodes/{id}
     */
{
    LOG_DEBUG << "NODES ID:" << _id;
    Json::Value ret;
    ret["id"] = _id;
    Json::Reader reader;
    Json::Value answer_from_BE;
    bool successParse = reader.parse(nodes_responseJson(ret.toStyledString()), answer_from_BE);
    answer_from_BE["message"] = "Nodes";
    cout << answer_from_BE << "\n";
    response_to_localhost(successParse, answer_from_BE, std::move(callback));
}

void BasicController::updates(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string params)
/**
     * Метод вызываемый переходе по путю /updates/{id}?={date}
     */
{
    LOG_DEBUG << "UPDATES";
    Json::Value ret;
    ret["params"] = params;
    Json::Reader reader;
    Json::Value answer_from_BE;
    bool successParse = reader.parse(someFunction(ret.toStyledString()).c_str(), answer_from_BE);
    answer_from_BE["message"] = "Updates";
    response_to_localhost(successParse, answer_from_BE, std::move(callback));
}

void BasicController::node_id_history(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string _id)
/**
     * Метод вызываемый переходе по путю /node/{id}/history
     */
{
    LOG_DEBUG << "NODE HISTORY ID:" << _id;
    Json::Value ret;
    ret["id"] = _id;
    Json::Reader reader;
    Json::Value answer_from_BE;
    bool successParse = reader.parse(someFunction(ret.toStyledString()).c_str(), answer_from_BE);
    answer_from_BE["message"] = "Nodes history";
    response_to_localhost(successParse, answer_from_BE, std::move(callback));
}

void BasicController::clear_db(const drogon::HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
/**
     * Метод вызываемый переходе по путю /clear
     * Путь для очищения базы данных
     */
{
    LOG_DEBUG << "CLEAR DB";
    Json::Reader reader;
    Json::Value answer_from_BE;
    bool successParse = reader.parse(clear_responseJSON(), answer_from_BE);
    answer_from_BE["message"] = "Clear db";
    response_to_localhost(successParse, answer_from_BE, std::move(callback));
}



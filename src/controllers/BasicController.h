#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class BasicController : public drogon::HttpController<BasicController>
        /**
         * Основной класс контроллеров
         * Обрабытвает пути
         */
{
public:
    METHOD_LIST_BEGIN

        ADD_METHOD_TO(BasicController::imports, "/imports", Post);
        ADD_METHOD_TO(BasicController::delete_id, "/delete/{id}?date={params}", Delete);
        ADD_METHOD_TO(BasicController::nodes_id, "/nodes/{id}", Get);
        ADD_METHOD_TO(BasicController::updates, "/updates?date={params}", Get);
        ADD_METHOD_TO(BasicController::node_id_history, "/node/{id}/history", Get);

        ADD_METHOD_TO(BasicController::clear_db, "/clear", Get);

    METHOD_LIST_END
    void imports(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback);
    void delete_id(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback,
                   const std::string _id, const std::string params);
    void nodes_id(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string _id);
    void updates(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string params);
    void node_id_history(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string _id);
    void clear_db(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback);
};


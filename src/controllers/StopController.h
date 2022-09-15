#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class StopController : public drogon::HttpSimpleController<StopController>
        /**
         * Класс контроллера отвечающего за попытку остановки сервера
         */
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    // list path definitions here;
    PATH_ADD("/stop", Get);
    PATH_LIST_END
};

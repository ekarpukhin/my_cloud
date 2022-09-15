#include "StopController.h"
#include "iostream"

void StopController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
/**
     * Метод вызываемый переходе по путю
     * Вызывает остановку работы сервера
     */
{
    std::cout << "STOPPING...:";
    drogon::app().quit();
}

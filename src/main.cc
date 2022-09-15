#include <drogon/drogon.h>
#include <iostream>
int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("0.0.0.0",80);
    std::cout << "\nRunning!\n";
    //Load config file
    drogon::app().loadConfigFile("../config.json");
    drogon::app().run();

    return 0;
}

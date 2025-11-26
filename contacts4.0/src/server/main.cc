#include <iostream>

#include "httplib.h"

using namespace std;
using namespace httplib;

int main() {
    cout << "---------服务启动---------" << endl;
    Server server;

    server.Post("/test-post", [](const Request& req, Response& res) {
        cout << "接收到post请求!" << endl;
        res.status = 200;
    });

    server.Get("/test-get", [](const Request& req, Response& res) {
        cout << "接收到get请求!" << endl;
        res.status = 200;
    });

    server.listen("0.0.0.0", 8180);

    return 0;
}

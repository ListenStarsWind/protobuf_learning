#include <iostream>

#include "httplib.h"

using namespace std;
using namespace httplib;

int main() {
    Client client("127.0.0.1", 8180);  // 本地回环

    Result res1 = client.Post("/test-post");
    if (res1->status == 200) {
        cout << "调用post成功!" << endl;
    }

    Result res2 = client.Get("/test-get");
    if (res2->status == 200) {
        cout << "调用get成功!" << endl;
    }

    return 0;
}

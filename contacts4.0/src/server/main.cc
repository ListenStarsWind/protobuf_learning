#include <iostream>
#include <random>
#include <string>

#include "httplib.h"
#include "proto/add_contact.pb.h"

using namespace std;
using namespace httplib;

class ContactsException : public exception {
   public:
    ContactsException(std::string m, int s) : message(m), status(s) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

    int code() const {
        return status;
    }

   private:
    std::string message;
    int status;
};

// C++ 风格的随机数生成
uint random_char() {
    // 从系统提供的随机源取一个随机种子（有些平台是硬件级别真随机，有些是软件级时间伪随机）
    std::random_device rd;
    // 用这个种子初始化伪随机数引擎
    std::mt19937 gen(rd());
    // 为生成的结果进行处理映射, 从而约束在某一范围
    std::uniform_int_distribution<> dis(0, 255);
    // 返回生成的随机数
    return dis(gen);
}

std::string generate_uid(uint len = 16) {
    std::string s;
    for (uint i = 0; i < len; ++i) {
        int rc = random_char();
        // 把结果格式化成十六进制形式, 且不足2位用0补齐
        auto hex = format("{:02x}", rc);
        s += hex;
    }
    return s;
}

int main() {
    cout << "---------服务启动---------" << endl;
    Server server;

    server.Post("/contacts/add", [](const Request& req, Response& res) {
        cout << "接收到post请求: /contacts/add" << endl;

        add_contact::AddContactRequest request;
        add_contact::AddContactResponse response;

        try {
            // 对负载进行反序列化
            if (!request.ParseFromString(req.body)) {
                // 4xx 表示服务端无法服务
                throw ContactsException("客户请求无法反序列化!", 400);
            }

            // 不写进文件了, 因为我们没写 message contact, 为了方便, 就直接打印了
            cout << "新增联系人↓\n";
            cout << format("联系人姓名: {}\n", request.name());
            cout << format("联系人年龄: {}\n", request.age());
            int size = request.phones_size();
            for (int i = 0; i < size; ++i) {
                auto phone = request.phones(i);
                cout << format("电话{} : {}\n", i + 1, phone);
            }
            cout << endl;

            // 构造并序列化应答负载
            response.set_success(true);
            response.set_uid(generate_uid());

            std::string body;
            if (!response.SerializeToString(&body)) {
                // 5xx 表示服务端处理请求出错
                throw ContactsException("客户应答无法序列化!", 500);
            }
            res.body = body;

        } catch (ContactsException& e) {
            res.status = e.code();
            response.set_success(false);
            response.set_error_desc(e.what());
            std::string body;
            if (response.SerializeToString(&body)) {
                res.body = body;
                res.set_header("Content-Type", "application/protobuf");
            }
            cout << format("/contacts/add 应用级异常: {}\n", e.what());
        }
    });

    server.listen("0.0.0.0", 8180);

    return 0;
}

#include <exception>
#include <format>
#include <iostream>
#include <string>

#include "httplib.h"
#include "proto/add_contact.pb.h"

using namespace std;
using namespace httplib;

void menu() {
    cout << "-------------------------------------" << endl;
    cout << "----------选择对应的通讯录操作----------" << endl;
    cout << "------------ 1. 新增联系人 ------------" << endl;
    cout << "------------ 2. 删除联系人 ------------" << endl;
    cout << "---------- 3. 查看联系人列表 ----------" << endl;
    cout << "-------- 4. 查看联系人详细信息 --------" << endl;
    cout << "-------------- 0. 退出--------------" << endl;
    cout << "-------------------------------------" << endl;
}

enum option_type {
    QUIT_CONTACT = 0,
    ADD_CONTACT = 1,
    DELETE_CONTACT = 2,
    FIND_CONTACTS = 3,
    FIND_CONTACT_DETAIL = 4
};

class ContactsException : public exception {
   public:
    ContactsException(std::string m) : message(m) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

   private:
    std::string message;
};

void AddContact();

// 短连接模式
int main() {
    try {
        menu();
        cout << "----> 请选择: ";
        int option = 0;
        cin >> option;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        switch (option) {
            case option_type::QUIT_CONTACT:
                cout << "-----程序退出-----" << endl;
                break;
            case option_type::ADD_CONTACT:
                AddContact();
                break;
            case option_type::DELETE_CONTACT:
                cout << "这个选项没写, 以后再来探索吧" << endl;
                break;
            case option_type::FIND_CONTACTS:
                cout << "这个选项没写, 以后再来探索吧" << endl;
                break;
            case option_type::FIND_CONTACT_DETAIL:
                cout << "这个选项没写, 以后再来探索吧" << endl;
                break;
            default:
                cout << "如果这是图形化界面, 我根本不会让你有机会来这里!" << endl;
                break;
        }
    } catch (ContactsException& e) {
        cerr << format("客户端应用层异常: {}\n", e.what());
    }
    return 0;
}

// 收集联系人参数
void collectContactInfo(add_contact::AddContactRequest& req) {
    cout << "开始添加新联系人: \n";
    cout << "请依据提示, 依次输入新联系人的姓名, 年龄, 联系电话 \n";

    uint32_t age;
    string name, phone;

    cout << "请输入新联系人的姓名: ";
    getline(cin, name);

    cout << "请输入新联系人的年龄: ";
    cin >> age;

    // 一直读取整型与换行之前的杂项内容, 包括换行本身
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "请输入新联系人的电话号码, 支持多个号码保存, 如果确认, 请直接回车\n";
    for (int i = 0;; ++i) {
        cout << format("第{}个: ", i + 1);
        getline(cin, phone);
        if (phone.empty()) break;
        req.add_phones(phone);
    }

    req.set_name(name);
    req.set_age(age);

    cout << "正在打包收集信息并发送\n";
}

void AddContact() {
    // 构造并序列化请求负载
    add_contact::AddContactRequest req_body;
    collectContactInfo(req_body);
    string body;
    if (!req_body.SerializeToString(&body)) {
        throw ContactsException("请求序列化失败!");
    }

    // 进行 post 调用
    Client c("127.0.0.1", 8180);  // 方便起见, 我们就用本地回环了
    auto res = c.Post("/contacts/add", body, "application/protobuf");
    if (!res) {
        // 会话层出现了错误
        throw ContactsException(
            format("/contacts/add 资源请求失败: {}", httplib::to_string(res.error())));
    }

    // 反序列化应答负载
    add_contact::AddContactResponse resp;
    bool parse = resp.ParseFromString(res->body);
    if (!parse) {
        throw ContactsException(
            format("/contacts/add 资源应答序列化失败, 会话层状态码: {}", res->status));
    } else if (res->status != 200) {
        // 2xx 是请求正常处理码, 不等于就是对端应用层异常
        throw ContactsException(
            format("/contacts/add 资源应答异常, 会话层状态码: {}, 服务端错误原因: {}", res->status,
                   resp.error_desc()));
    } else if (!resp.success()) {
        // 对端调用结束, 但结果异常, 责任可能在于客户提交参数有误
        throw ContactsException(format("/contacts/add 资源应答无法处理: {}", resp.error_desc()));
    }

    // 输出结果
    cout << format("新增联系人成功, 联系人身份码为: {}\n", resp.uid());
}

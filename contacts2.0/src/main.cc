#include <unistd.h>

#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include "contacts.pb.h"

using namespace std;

int main() {
    string file_name = "contacts.bin";  // bin 的意思是它是二进制的
    string file_temp_name = file_name + ".temp";

    // 打开, 或者创建一份文件, 作为通讯录的初始化
    fstream input(file_name.c_str(), ios::in | ios::binary);  // 二进制, 不存在直接创建

    contacts::Contacts contacts;

    // 根据是否创建文件, 或者说, 是否存在目标文件, 进行条件处理
    if (!input) {
        // 新创建的
        cout << format("配置文件不存在, 已经自动创建\n");
    } else {
        // 原来就存在, 需要进行初始化
        if (!contacts.ParseFromIstream(&input)) {
            cout << format("对于配置文件的反序列化是失败的\n");
            input.close();
            return 1;
        }
    }

    // 增加一个新的联系人
    auto people = contacts.add_contacts();
    cout << format("正在添加一个新的联系人: \n");
    cout << format("请输入联系人的姓名: ");
    string name;
    getline(cin, name);
    people->set_name(name);
    cout << format("请输入年龄: ");
    int age;
    cin >> age;
    people->set_age(age);

    cin.ignore(256, '\n');  // 读出 age 残留的空行, 避免影响到号码的输入, 它会在输入缓冲区里,
                            // 一直读, 直到读完, 或者读了 256 字节, 亦或者 读到 `\n` 才停下
    for (int i = 0;; ++i) {
        cout << format("请输入联系人电话号码, 直接回车结束记录, 第{}份: ", i + 1);
        string phone;
        getline(cin, phone);
        if (phone.empty()) break;
        auto p = people->add_phones();
        p->set_country("86");
        p->set_number(phone);
    }
    cout << format("一个新的联系人已经添加\n");

    // 持久化数据
    fstream output(file_temp_name.c_str(),
                   ios::out | ios::trunc | ios::binary);  // 写打开, 并且清空内容

    if (!contacts.SerializePartialToOstream(&output)) {
        cout << format("数据持久化是失败的\n");
        input.close();
        output.close();
        return 2;
    }

    input.close();
    output.close();

    // 原子性安全替换, 确保保存安全
    // 把 old 的名字改成 new, 从而实现文件覆写
    ::rename(file_temp_name.c_str(), file_name.c_str());

    return 0;
}

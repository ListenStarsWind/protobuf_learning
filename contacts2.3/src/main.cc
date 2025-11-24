#include <unistd.h>

#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include "contacts.pb.h"

using namespace std;

int func1() {
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

        cout << format("您输入的电话号码类型是: 1.移动电话, 2.固定电话");
        int type = 0;
        cin >> type;
        cin.ignore(256, '\n');
        switch (type) {
            case 1:
                p->set_type(::phone::PhoneInfo_PhoneType::PhoneInfo_PhoneType_MP);
                break;
            case 2:
                p->set_type(::phone::PhoneInfo_PhoneType::PhoneInfo_PhoneType_TEL);
                break;
            default:
                cout << format("你选了什么? 怎么找不到值");
                break;
        }
    }

    contacts::Address address;
    cout << format("请输入联系人地址: ");
    string addr;
    getline(cin, addr);
    address.set_address(addr);

    auto _address = people->mutable_address();
    _address->PackFrom(
        address);  // 由于它其实涉及到序列化的过程, 所以也有失败的可能, 但当前我们就不判断了

    cout << format("请选择其它的备用联系方式: 1. qq  2. 微信 ");
    int temp = 0;
    cin >> temp;
    cin.ignore(256, '\n');

    switch (temp) {
        case 1: {
            cout << format("请输入QQ号: ");
            string qq;
            getline(cin, qq);
            people->set_qq(qq);
            break;
        }
        case 2: {
            cout << format("请输入微信号: ");
            string wechat;
            getline(cin, wechat);
            people->set_wechat(wechat);
            break;
        }
        default:
            cout << format("默认不增加备用联系方式");
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

int func2() {
    string file_name = "contacts.bin";

    fstream input(file_name.c_str(), ios::in | ios::binary);
    // 假设文件就是存在的

    contacts::Contacts contacts;
    if (!contacts.ParseFromIstream(&input)) {
        cout << format("对于配置文件的反序列化是失败的\n");
        input.close();
        return 1;
    }

    input.close();

    // 打印内容
    auto size = contacts.contacts_size();
    for (int i = 0; i < size; ++i) {
        auto people = contacts.contacts(i);
        cout << format("联系人姓名: {}\n", people.name());
        cout << format("联系人年龄: {}\n", people.age());
        for (int j = 0; j < people.phones_size(); ++j) {
            auto phone = people.phones(j);
            cout << format("第{}份电话号码: {}, 类型: {}\n", j + 1, phone.number(),
                           phone.PhoneType_Name(people.phones(j).type()));
        }

        // 如果二进制流里面有该字段, 并且是我们期望的类型
        if (people.has_address() && people.address().Is<contacts::Address>()) {
            contacts::Address address;
            people.address().UnpackTo(&address);  // 这里也不判断成功与否了
            if (!address.address().empty()) {
                cout << format("联系人地址: {}\n", address.address());
            }
        }

        switch (people.other_contact_case()) {
            case contacts::PeopleInfo::kQq:
                cout << format("备用qq联系方式: {}", people.qq());
                break;
            case contacts::PeopleInfo::kWechat:
                cout << format("备用微信联系方式: {}", people.wechat());
                break;
            default:
                break;
        }
        cout << endl;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << format("错误的参数");
    }

    int val = stoi(argv[1]);

    if (val == 1) {
        return func1();
    }
    if (val == 2) {
        return func2();
    }
    return 0;
}

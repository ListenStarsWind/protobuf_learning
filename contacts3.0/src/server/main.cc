#include <unistd.h>

#include <format>
#include <fstream>
#include <limits>
#include <string>

#include "proto/contacts.pb.h"

int main() {
    using namespace std;
    using namespace contacts;

    string fName = "contacts.bin";
    string fTName = fName + ".temp";

    fstream input(fName.c_str(), ios::in | ios::binary);

    Contacts contacts;
    if (!input) {
        cout << "配置文件不存在, 将自动创建\n";
    } else {
        if (!contacts.ParseFromIstream(&input)) {
            cout << "通讯录初始化失败\n";
            // 生命周期结束, 自动析构并关闭文件
            return 1;
        }
    }

    auto people = contacts.add_contacts();

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
        people->add_phones(phone);
    }

    people->set_name(name);
    people->set_age(age);

    cout << "一个新的联系人已经添加成功\n";

    fstream output(fTName.c_str(), ios::out | ios::binary | ios::trunc);

    if (!contacts.SerializePartialToOstream(&output)) {
        cout << "数据持久化失败";
        return 2;
    }

    ::rename(fTName.c_str(), fName.c_str());
    return 0;
}
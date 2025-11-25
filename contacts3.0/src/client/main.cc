#include <format>
#include <fstream>
#include <string>

#include "proto/contacts.pb.h"

int main() {
    using namespace std;
    using namespace contacts;

    string fName = "contacts.bin";

    fstream input(fName.c_str(), ios::in | ios::binary);

    if (!input) {
        cout << "中间媒介不存在, 反序列化无法进行\n";
        return 1;
    }

    Contacts contacts;
    if (!contacts.ParseFromIstream(&input)) {
        cout << "反序列化有误, 将停止程序\n";
        return 2;
    }

    input.close();

    auto size = contacts.contacts_size();
    for (int i = 0; i < size; ++i) {
        cout << format("----------联系人{}----------\n", i + 1);
        auto people = contacts.contacts(i);
        cout << format("姓名: {}\n", people.name());
        cout << format("年龄: {}\n", people.age());
        int phones_size = people.phones_size();
        for (int j = 0; j < phones_size; ++j) {
            cout << format("电话{}: {} \n", j + 1, people.phones(j));
        }
        cout << endl;
    }

    return 0;
}
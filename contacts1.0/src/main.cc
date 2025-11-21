#include <format>
#include <iostream>
#include <string>

#include "contacts.pb.h"

int main()
{
    using namespace std;

    string message;

    {
        // 实例化一个联系人对象
        contacts::PeopleInfo people;
        people.set_name("芙宁娜·德·枫丹");
        people.set_age(500);

        // 序列化到 message;
        if(!people.SerializeToString(&message))
        {
            cout << format("序列化至字符串失败\n");
            exit(1);
        }
    }

    // protobuf 是二进制流, 内容不一定是可见的字符
    cout << format("{}\n", message);

    // 另一个局部空间, 模拟传输到另一处的效果
    {
        contacts::PeopleInfo people;
        if(!people.ParseFromString(message))
        {
            cout << format("失败的反序列化");
        }

        cout << format("姓名:{}, 年龄{}多岁\n", people.name(), people.age());
    }


    return 0;
}

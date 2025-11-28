#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "proto/person.pb.h"

using json = nlohmann::json;
using namespace std;

Person MakePerson(int i) {
    Person p;
    p.set_name("User_" + std::to_string(i));
    p.set_age(20 + (i % 10));
    p.add_tags("tagA");
    p.add_tags("tagB");
    return p;
}

json MakeJsonPerson(int i) {
    return {
        {"name", "User_" + std::to_string(i)},
        {"age", 20 + (i % 10)},
        {"tags", {"tagA", "tagB"}}
    };
}

int main() {
    const int N = 50000;   // *** 调节循环次数，模拟数据变多 ***
    std::vector<Person> pb_data;
    std::vector<json> json_data;

    pb_data.reserve(N);
    json_data.reserve(N);

    // 准备数据
    for (int i = 0; i < N; ++i) {
        pb_data.push_back(MakePerson(i));
        json_data.push_back(MakeJsonPerson(i));
    }

    // ----------------- Protobuf serialize ------------------
    auto t1 = chrono::high_resolution_clock::now();

    size_t pb_total_size = 0;
    std::vector<std::string> pb_encoded;
    pb_encoded.reserve(N);

    for (int i = 0; i < N; ++i) {
        std::string out;
        pb_data[i].SerializeToString(&out);
        pb_total_size += out.size();
        pb_encoded.push_back(std::move(out));
    }

    auto t2 = chrono::high_resolution_clock::now();

    // ----------------- JSON serialize ----------------------
    size_t json_total_size = 0;
    std::vector<std::string> json_encoded;
    json_encoded.reserve(N);

    auto t3 = chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        std::string out = json_data[i].dump(); // 默认紧凑格式
        json_total_size += out.size();
        json_encoded.push_back(std::move(out));
    }

    auto t4 = chrono::high_resolution_clock::now();

    // ----------------- 输出结果 -----------------------------
    auto pb_time = chrono::duration<double, milli>(t2 - t1).count();
    auto json_time = chrono::duration<double, milli>(t4 - t3).count();

    cout << "================ 结果 ================\n";
    cout << "数据条目数 N = " << N << "\n\n";
    cout << "Protobuf:" << "\n";
    cout << "  序列化耗时:  " << pb_time << " ms\n";
    cout << "  总大小:      " << pb_total_size << " bytes\n\n";
    cout << "JSON:" << "\n";
    cout << "  序列化耗时:  " << json_time << " ms\n";
    cout << "  总大小:      " << json_total_size << " bytes\n\n";

    cout << "大小比 (JSON / PB): " << (double)json_total_size / pb_total_size << "\n";
    cout << "时间比 (JSON / PB): " << (double)json_time / pb_time << "\n";
}


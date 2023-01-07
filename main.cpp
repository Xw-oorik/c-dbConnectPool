#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<memory>
#include"MySqlConnet.h"
#include"Poll.h"
using namespace std;

int query()
{
    MySqlConnet conn;
    conn.connet("oorik", "033018", "testdb", "127.0.0.1", 3306);
    string sql = "insert into mytest values(4,'lilei')";//插入
    bool flg = conn.update(sql);
    cout << "flg value : " << flg << endl;
    sql = "select * from mytest";//查询
    conn.search(sql);
    while (conn.next())//打印一下
    {
        cout << conn.value(0) << ", " << conn.value(1) << endl;
    }
    return 0;
}

// 无连接池
void op1(int begin, int end) {
    for (int i = begin; i < end; ++i) {
        MySqlConnet conn;
        conn.connet("oorik", "033018", "testdb", "127.0.0.1", 3306);
        char sql[1024] = { 0 };
        sprintf(sql, "insert into mytest values(%d,'lilei')", i);
        conn.update(sql);
    }
}

//有连接池
void op2(Poll* poll,int begin, int end) {
    for (int i = begin; i < end; ++i) {
        shared_ptr<MySqlConnet> conn = poll->getConnection();
        char sql[1024] = { 0 };
        sprintf(sql, "insert into mytest values(%d,'lilei')", i);
        conn->update(sql);
    }
}
//单线程
void test1() {
#if 1
    // 非连接池, 单线程
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    op1(0, 5000);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - begin;
    cout << "非连接池, 单线程, 用时: " << length.count() << " 纳秒, "
        << length.count() / 1000000 << " 毫秒" << endl;
#else
    // 连接池, 单线程
    Poll* poll = Poll::getPoll();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    op2(poll, 0, 5000);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - begin;
    cout << "连接池, 单线程, 用时: " << length.count() << " 纳秒, "
        << length.count() / 1000000 << " 毫秒" << endl;
#endif
}
//多线程
void test2() {
#if 0
    //非连接池, 多线程
    MySqlConnet conn;
    conn.connet("oorik", "033018", "testdb", "127.0.0.1", 3306);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    thread t1(op1, 0, 1000);
    thread t2(op1, 1000, 2000);
    thread t3(op1, 2000, 3000);
    thread t4(op1, 3000, 4000);
    thread t5(op1, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - begin;
    cout << "非连接池, 多线程, 用时: " << length.count() << " 纳秒, "
        << length.count() / 1000000 << " 毫秒" << endl;

#else
    //连接池, 多线程
    Poll* poll = Poll::getPoll();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    thread t1(op2, poll, 0, 1000);
    thread t2(op2, poll, 1000, 2000);
    thread t3(op2, poll, 2000, 3000);
    thread t4(op2, poll, 3000, 4000);
    thread t5(op2, poll, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - begin;
    cout << "连接池, 多线程, 用时: " << length.count() << " 纳秒, "
        << length.count() / 1000000 << " 毫秒" << endl;
#endif
}

int main()
{
    //query();
    //test1();
    test2();
    return 0;
}
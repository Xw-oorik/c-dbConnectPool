#pragma once
#include<iostream>
#include<string>
#include<mysql.h>
#include<chrono>
using namespace std;
using namespace chrono;
//封装一下数据库连接操作的类
class MySqlConnet
{
public:
    //初始化数据库连接
    MySqlConnet();
    //释放数据库连接
    ~MySqlConnet();
    //连接数据库
    bool connet(string user, string passwd, string dbname, string ip, unsigned short port = 3306);
    //更新数据库
    bool update(string sql);
    //查询数据库
    bool search(string sql);
    //遍历查询得到的结果集
    bool next();
    //得到结果集中的字段值
    string value(int index);
    //事务操作
    bool transaction();
    //提交事务
    bool commit();
    //回滚事务
    bool rollback();
    //刷新数据库连接起始的空闲时间点
    void refreshAliveTime();
    //计算连接存活的总时长
    long long getAliveTime();
private:
    MYSQL* m_connet = nullptr; //连接句柄的指针
    MYSQL_RES* m_result = nullptr;//结果集的地址
    MYSQL_ROW m_row ;//遍历的行结构地址
    steady_clock::time_point m_alivetime;
    //把结果集内存读完了 就要释放
    void freeResult();
};

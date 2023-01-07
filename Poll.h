#pragma once
#include<iostream>
#include<queue>
#include"MySqlConnet.h"
#include<mutex>
#include<condition_variable>
#include<json/json.h>
#include <fstream>
#include <atomic>
using namespace std;

//线程安全的饿汉
class Poll
{
private:
    Poll();
public:
    Poll(const Poll& pos)=delete;
    Poll& operator=(const Poll& pos)=delete;
    ~Poll();
    static Poll* getPoll();
    //智能指针自动管理连接的释放
    shared_ptr<MySqlConnet> getConnection();//从池子里面获取连接
private:
    //建立连接需要一些属性 ip port这些
    string m_ip;
    string m_user;
    string m_dbname;
    string m_passwd;
    unsigned short m_port;
    //最大 最小连接数
    int m_maxsize;
    int m_minsize;
    //超时时长
    int m_timeout;
    //最大空闲时长
    int m_iddtime;
    //互斥量
    mutex m_mutex;
    //条件变量
    condition_variable m_cond;
    //连接池队列里面存储的都是mysql有效的连接
    queue<MySqlConnet*> m_connetionQ;
    //记录连接所创建的connection连接的总数量
    atomic_int m_connectionCount;  // 连接的总数，不会超过m_maxsize
private:
    bool parseJsonFile();//解析json文件
    //类的非静态成员函数 
    void producerConnection();//运行在独立的线程中  用来给池子里生成连接用
    void recyclerConnection();//对于超过空闲时间的连接进行回收 
    void addConnection();//添加连接
};
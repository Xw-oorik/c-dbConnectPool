#include"Poll.h"
#include<thread>
#include<json/json.h>
#include <fstream>
#include "Log.h"

Poll* Poll::getPoll()
{
    static Poll poll;
    return &poll;
}
bool Poll::parseJsonFile()//解析json文件
{
    ifstream ifs("dbconf.json");
    Json::Reader rd;
    Json::Value root;
    rd.parse(ifs,root);
    if (root.isObject())
    {
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_user = root["user"].asString();
        m_passwd = root["passwd"].asString(); 
        m_dbname = root["dbname"].asString();
        m_maxsize = root["maxsize"].asInt();
        m_minsize = root["minsize"].asInt();
        m_timeout = root["timeout"].asInt();
        m_iddtime = root["iddtime"].asInt();
        return true;
    }
    cout << "aaaa" << endl;
    return false;
}
shared_ptr<MySqlConnet> Poll::getConnection()// 给外部提供接口，从连接池中获取可用的空闲连接
{
    unique_lock<mutex> locker(m_mutex);
    while (m_connetionQ.empty())//连接为空，就阻塞等待超时时间，如果时间过了，还没唤醒
    {
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
        {
            if (m_connetionQ.empty())//表示经过了m_timeout后超时醒来队列依然为空
            {
                LOG("获取连接超时...");
                continue;
            }
        }
    }
    //对于使用完成的连接，不能直接销毁该连接，而是需要将该连接归还给连接池的队列，
    // 供之后的其他消费者使用，
    //于是我们使用智能指针，自定义其析构函数，完成放回的操作：
    shared_ptr<MySqlConnet> connptr(m_connetionQ.front(), [this](MySqlConnet* conn)
        {m_mutex.lock();
        conn->refreshAliveTime();
        m_connetionQ.push(conn);
        m_mutex.unlock();
        }
    );
    m_connetionQ.pop();
    if (m_connetionQ.empty())
    {    //连接池队列没有连接了，通知生产者去生产新连接
         m_cond.notify_all();
    }
    return connptr;
}
void Poll::producerConnection()//运行在独立的线程中，负责生产新的连接
{
    while (true)
    {  // 生产者需要访问连接队列，加锁，防止和消费者同时访问
        unique_lock<mutex> locker(m_mutex);
        while (m_connetionQ.size()>= m_minsize)
        {   // 连接池队列里有多余的连接，生产线程进入等待状态，并且释放刚刚拿到的互斥锁
            m_cond.wait(locker);
        }
        if (m_connectionCount < m_maxsize)//没有达到最大连接数，创建连接
        {
             addConnection();
        }
        //通知消费者可以消费连接
        m_cond.notify_all();
    }
}
void Poll::recyclerConnection()
{
    while (true)
    {
        // 定时检查队列超时的连接
        this_thread::sleep_for(chrono::milliseconds(m_iddtime));
        unique_lock<mutex> locker(m_mutex);
        while (m_connetionQ.size() > m_minsize)
        {
            // 队列中的连接全都空闲，检查对头空闲的时间，超过空闲时长则释放
            MySqlConnet* connt = m_connetionQ.front();
            if (connt->getAliveTime() >= m_iddtime)
            {
                m_connetionQ.pop();
                --m_connectionCount;
                delete connt;
            }
            else //队尾插入对头释放，如果对头的连接空闲时间没有超过 那么其他连接肯定也没有超过
            {
                break;
            }
        }
    }
}
void Poll::addConnection()
{
    MySqlConnet* conn = new MySqlConnet();
    conn->connet(m_user, m_passwd, m_dbname, m_ip, m_port);
    //当数据库连接建立成功 就要记录时间戳了
    conn->refreshAliveTime();
    m_connetionQ.push(conn);
    m_connectionCount++;//连接总数++

}
Poll::Poll()
{
    //加载json文件
    if (!parseJsonFile())
    {
        return;
    }
    //创建数据库连接
    for (int i = 0; i < m_minsize; ++i)
    {
        addConnection();
    }
    //创建两个线程
    //一个去生产连接 一个去销毁多余空闲连接
    //类的非静态成员函数 传的话要传地址和this
    thread producer(&Poll::producerConnection, this);
    thread recycler(&Poll::recyclerConnection, this);
    //分离
    producer.detach();
    recycler.detach();
}
Poll::~Poll()
{
    while (!m_connetionQ.empty())
    {
        MySqlConnet* conn = m_connetionQ.front();
        m_connetionQ.pop();
        delete conn;
    }
}
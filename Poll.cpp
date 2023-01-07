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
bool Poll::parseJsonFile()//����json�ļ�
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
shared_ptr<MySqlConnet> Poll::getConnection()// ���ⲿ�ṩ�ӿڣ������ӳ��л�ȡ���õĿ�������
{
    unique_lock<mutex> locker(m_mutex);
    while (m_connetionQ.empty())//����Ϊ�գ��������ȴ���ʱʱ�䣬���ʱ����ˣ���û����
    {
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
        {
            if (m_connetionQ.empty())//��ʾ������m_timeout��ʱ����������ȻΪ��
            {
                LOG("��ȡ���ӳ�ʱ...");
                continue;
            }
        }
    }
    //����ʹ����ɵ����ӣ�����ֱ�����ٸ����ӣ�������Ҫ�������ӹ黹�����ӳصĶ��У�
    // ��֮�������������ʹ�ã�
    //��������ʹ������ָ�룬�Զ�����������������ɷŻصĲ�����
    shared_ptr<MySqlConnet> connptr(m_connetionQ.front(), [this](MySqlConnet* conn)
        {m_mutex.lock();
        conn->refreshAliveTime();
        m_connetionQ.push(conn);
        m_mutex.unlock();
        }
    );
    m_connetionQ.pop();
    if (m_connetionQ.empty())
    {    //���ӳض���û�������ˣ�֪ͨ������ȥ����������
         m_cond.notify_all();
    }
    return connptr;
}
void Poll::producerConnection()//�����ڶ������߳��У����������µ�����
{
    while (true)
    {  // ��������Ҫ�������Ӷ��У���������ֹ��������ͬʱ����
        unique_lock<mutex> locker(m_mutex);
        while (m_connetionQ.size()>= m_minsize)
        {   // ���ӳض������ж�������ӣ������߳̽���ȴ�״̬�������ͷŸո��õ��Ļ�����
            m_cond.wait(locker);
        }
        if (m_connectionCount < m_maxsize)//û�дﵽ�������������������
        {
             addConnection();
        }
        //֪ͨ�����߿�����������
        m_cond.notify_all();
    }
}
void Poll::recyclerConnection()
{
    while (true)
    {
        // ��ʱ�����г�ʱ������
        this_thread::sleep_for(chrono::milliseconds(m_iddtime));
        unique_lock<mutex> locker(m_mutex);
        while (m_connetionQ.size() > m_minsize)
        {
            // �����е�����ȫ�����У�����ͷ���е�ʱ�䣬��������ʱ�����ͷ�
            MySqlConnet* connt = m_connetionQ.front();
            if (connt->getAliveTime() >= m_iddtime)
            {
                m_connetionQ.pop();
                --m_connectionCount;
                delete connt;
            }
            else //��β�����ͷ�ͷţ������ͷ�����ӿ���ʱ��û�г��� ��ô�������ӿ϶�Ҳû�г���
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
    //�����ݿ����ӽ����ɹ� ��Ҫ��¼ʱ�����
    conn->refreshAliveTime();
    m_connetionQ.push(conn);
    m_connectionCount++;//��������++

}
Poll::Poll()
{
    //����json�ļ�
    if (!parseJsonFile())
    {
        return;
    }
    //�������ݿ�����
    for (int i = 0; i < m_minsize; ++i)
    {
        addConnection();
    }
    //���������߳�
    //һ��ȥ�������� һ��ȥ���ٶ����������
    //��ķǾ�̬��Ա���� ���Ļ�Ҫ����ַ��this
    thread producer(&Poll::producerConnection, this);
    thread recycler(&Poll::recyclerConnection, this);
    //����
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
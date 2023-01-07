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

//�̰߳�ȫ�Ķ���
class Poll
{
private:
    Poll();
public:
    Poll(const Poll& pos)=delete;
    Poll& operator=(const Poll& pos)=delete;
    ~Poll();
    static Poll* getPoll();
    //����ָ���Զ��������ӵ��ͷ�
    shared_ptr<MySqlConnet> getConnection();//�ӳ��������ȡ����
private:
    //����������ҪһЩ���� ip port��Щ
    string m_ip;
    string m_user;
    string m_dbname;
    string m_passwd;
    unsigned short m_port;
    //��� ��С������
    int m_maxsize;
    int m_minsize;
    //��ʱʱ��
    int m_timeout;
    //������ʱ��
    int m_iddtime;
    //������
    mutex m_mutex;
    //��������
    condition_variable m_cond;
    //���ӳض�������洢�Ķ���mysql��Ч������
    queue<MySqlConnet*> m_connetionQ;
    //��¼������������connection���ӵ�������
    atomic_int m_connectionCount;  // ���ӵ����������ᳬ��m_maxsize
private:
    bool parseJsonFile();//����json�ļ�
    //��ķǾ�̬��Ա���� 
    void producerConnection();//�����ڶ������߳���  ����������������������
    void recyclerConnection();//���ڳ�������ʱ������ӽ��л��� 
    void addConnection();//�������
};
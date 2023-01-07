#pragma once
#include<iostream>
#include<string>
#include<mysql.h>
#include<chrono>
using namespace std;
using namespace chrono;
//��װһ�����ݿ����Ӳ�������
class MySqlConnet
{
public:
    //��ʼ�����ݿ�����
    MySqlConnet();
    //�ͷ����ݿ�����
    ~MySqlConnet();
    //�������ݿ�
    bool connet(string user, string passwd, string dbname, string ip, unsigned short port = 3306);
    //�������ݿ�
    bool update(string sql);
    //��ѯ���ݿ�
    bool search(string sql);
    //������ѯ�õ��Ľ����
    bool next();
    //�õ�������е��ֶ�ֵ
    string value(int index);
    //�������
    bool transaction();
    //�ύ����
    bool commit();
    //�ع�����
    bool rollback();
    //ˢ�����ݿ�������ʼ�Ŀ���ʱ���
    void refreshAliveTime();
    //�������Ӵ�����ʱ��
    long long getAliveTime();
private:
    MYSQL* m_connet = nullptr; //���Ӿ����ָ��
    MYSQL_RES* m_result = nullptr;//������ĵ�ַ
    MYSQL_ROW m_row ;//�������нṹ��ַ
    steady_clock::time_point m_alivetime;
    //�ѽ�����ڴ������ ��Ҫ�ͷ�
    void freeResult();
};

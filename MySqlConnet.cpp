#include"MySqlConnet.h"
#include"Log.h"
MySqlConnet::MySqlConnet()
{
    //��ʼ��mysql����
    m_connet = mysql_init(nullptr);
    //�ӿ�ʹ�õı������ó�utf8����ֹ������������
    mysql_set_character_set(m_connet, "utf8");
}
MySqlConnet:: ~MySqlConnet()
{
    if (m_connet != nullptr)
    {
        mysql_close(m_connet);
    }
    if (m_result != nullptr)
    {
        freeResult();
    }
}
bool MySqlConnet::connet(string user, string passwd, string dbname, string ip, unsigned short port)
{
    MYSQL* sql = mysql_real_connect(m_connet, ip.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, nullptr, 0);

    return sql != nullptr;
}

bool MySqlConnet::update(string sql)
{
    if (mysql_query(m_connet, sql.c_str()))
    {
        LOG("����ʧ�ܣ�" + sql);
        return false;
    }
    return true;
}

bool MySqlConnet::search(string sql)
{
    freeResult();
    if (mysql_query(m_connet, sql.c_str()))
    {
        LOG("��ѯʧ�ܣ�" + sql);
        return false;
    }
    //�ѷ������˲�ѯ���Ľ�������浽�ͻ���
    m_result = mysql_store_result(m_connet);//�õ������
    return true;
}
bool MySqlConnet::next()
{
    //���� �õ�������
    if (m_result == nullptr)return false;
    m_row = mysql_fetch_row(m_result);
    return true;
}

string MySqlConnet::value(int index)//�õ��ֶ�
{
    int colCount = mysql_num_fields(m_result);//�е�����
    if (index >= colCount || index < 0)
    {
        return "";
    }
    char* val = m_row[index];
    //���ص��ǵ�ǰ��¼�����ֶζ�Ӧ�ĳ���, mysq_fetch_lengths(m_result)
    //�ӡ�index������index���ֶγ���
    unsigned long length = mysql_fetch_lengths(m_result)[index];

    return string(val, length);
}

bool  MySqlConnet::transaction()
{
    //�ֶ��ύ
    return mysql_autocommit(m_connet, false);
}
bool  MySqlConnet::commit()
{
    return mysql_commit(m_connet);
}

bool  MySqlConnet::rollback()
{
    return mysql_rollback(m_connet);
}

void MySqlConnet::refreshAliveTime()
{
    //�õ�һ��ʱ�������
    m_alivetime = steady_clock::now();
}

long long MySqlConnet::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_alivetime;//���뼶��
    //���ǰ�����ת�ɺ��� ��������ʧ����
    milliseconds millsec = duration_cast<milliseconds>(res);//����
    return  millsec.count();
}
void MySqlConnet::freeResult()
{
    if (m_result)
    {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}
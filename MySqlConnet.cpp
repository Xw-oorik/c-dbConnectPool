#include"MySqlConnet.h"
#include"Log.h"
MySqlConnet::MySqlConnet()
{
    //初始化mysql环境
    m_connet = mysql_init(nullptr);
    //接口使用的编码设置成utf8，防止出现中文乱码
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
        LOG("更新失败：" + sql);
        return false;
    }
    return true;
}

bool MySqlConnet::search(string sql)
{
    freeResult();
    if (mysql_query(m_connet, sql.c_str()))
    {
        LOG("查询失败：" + sql);
        return false;
    }
    //把服务器端查询到的结果集保存到客户端
    m_result = mysql_store_result(m_connet);//得到结果集
    return true;
}
bool MySqlConnet::next()
{
    //遍历 得到的是行
    if (m_result == nullptr)return false;
    m_row = mysql_fetch_row(m_result);
    return true;
}

string MySqlConnet::value(int index)//得到字段
{
    int colCount = mysql_num_fields(m_result);//列的数量
    if (index >= colCount || index < 0)
    {
        return "";
    }
    char* val = m_row[index];
    //返回的是当前记录所有字段对应的长度, mysq_fetch_lengths(m_result)
    //加【index】就是index的字段长度
    unsigned long length = mysql_fetch_lengths(m_result)[index];

    return string(val, length);
}

bool  MySqlConnet::transaction()
{
    //手动提交
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
    //得到一个时间戳就行
    m_alivetime = steady_clock::now();
}

long long MySqlConnet::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_alivetime;//纳秒级的
    //我们把纳秒转成毫秒 不过会损失精度
    milliseconds millsec = duration_cast<milliseconds>(res);//毫秒
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
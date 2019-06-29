#pragma once
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <vector>

class Mysql
{
public:
    void Mysql_init(){
        mysql_init(&_mysql);
    }

    //连接数据库
    void Mysql_real_connect(
                            const char* host,
                            const char* user,
                            const char* password){
        //设置数据的连接的字符集为utf_8
        mysql_set_character_set(&_mysql, "utf8");
        mysql_real_connect(&_mysql, host, user, password, "oj_data", 3306, NULL, 0);
    }

    //将查询字符串转化为相应的正确的sql语句
    //并对数据库进行操作
    void Mysql_query(std::string sql_opt){
        int result = mysql_query(&_mysql, sql_opt.c_str());
        if(!result){
            std::string err_string = mysql_error(&_mysql);
            if(!err_string.empty()){
                std::cout << err_string << std::endl;
            }
        }
    }

    void Mysql_store_result(std::vector<std::vector<std::string>>& array){
        MYSQL_RES* result = mysql_store_result(&_mysql);
        if (!result)
            std::cout << "mysql not result!!" << std::endl;
        int num_fields = mysql_num_fields(result);  //获取字段数量
        //MYSQL_FIELD *fields = mysql_fetch_fields(result);
        //获取字段名
        //获取整条数据内容
        while (MYSQL_ROW row = mysql_fetch_row(result)){
            std::vector<std::string> temp;
            for (int i = 0; i < num_fields; i++){
                if (NULL == row[i]){
                    temp.push_back("");
                }
                else{
                    temp.push_back(row[i]);
                }   
            }
            array.push_back(temp);
        }
    }

    MYSQL Getmysql(){
        return _mysql;
    }

    //关闭数据库
    void Mysql_close(){
        mysql_close(&_mysql);
    }

private:
    MYSQL _mysql;
};


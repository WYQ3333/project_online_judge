#include "mysql_operator.hpp"

int main()
{
    Mysql t_mysql;
    t_mysql.Mysql_init();
    t_mysql.Mysql_real_connect("localhost","root", "15529302179@%W_yq");
    std::string sql = "select *from data;";
    t_mysql.Mysql_query(sql);
    std::vector<std::vector<std::string>> array;
    t_mysql.Mysql_store_result(array);
    for(size_t i = 0; i < array.size(); ++i){
        for(size_t j = 0; j < array[i].size(); ++j){
            if(array[i][j].empty())
                std::cout <<  " " << "NULL";
            else{
                std::string temp = array[i][j];
                std::cout << " " << temp;
            }
        }
        std::cout << std::endl;
    }
    return 0;
}


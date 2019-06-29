#pragma once
#include<unordered_map>
#include<map>
#include<fstream>
#include<string>
#include<vector>
#include"util.hpp"
#include"mysql_operator.hpp"

//MVC 经典的软件设计模式，20年前就有了 现在仍在使用，现代还有比如MVVM这个软件设计方式   
//M => model:负责存储数据
//V => view :负责显示界面
//C => controller:核心业务逻辑
//基于文件的方式完成题目的存储，具体组织方式：
//约定每个题目对应一个目录，目录的名字就是题目的id
//目录里包含以下几个文件：
//1)header.cpp    =>    代码矿建
//2)tail.cpp      =>    代码测试用例
//3)desc.txt      =>    题目的详细描述
//除此之外，再搞一个oj_config.cfg文件，作为一个总的入口文件，这个文件是一个行文本文件
//这一行里面包含以下几个信息：题目的id，题目的名字，题目的难度，题目对应的目录
//model要做的事情就是把我们做的配置文件中的题目信息加载起来，供服务其随时使用


struct Question{
  std::string id;    //题目的序号
  std::string name;  //题目的名字
  std::string dir;        //表示题目对应的目录,目录中包含了题目描述，题目的代码框架，题目的测试用例
  
  std::string star;       //表示题目等级
  std::string desc;       //题目的描述
  std::string header_cpp; //题目的代码框架中的代码
  std::string tail_cpp;   //题目的测试用例代码
};

class OjModel{
  private:
    //MD5,SHA1
    //上面的这两种方法是计算字符串哈希值的方法
    //计算的哈希值非常均匀，不可逆，固定长度
    std::map<std::string, Question> model_;
  public:
    //把文件上的数据加载起来,加载到内存中,加到哈希表中(数组和哈希表在工作中很常用)
    bool Load(){
      //1.先打开oj_config文件
      //2.按行读取oj_config 文件，并且解析
      //3.根据解析结果拼装成Question结构体
      Mysql t_mysql;
      Question q;
      t_mysql.Mysql_init();
      t_mysql.Mysql_real_connect("localhost","root", "15529302179@%W_yq");
      std::string sql = "select *from data;";
      t_mysql.Mysql_query(sql);
      std::vector<std::vector<std::string>> array;
      t_mysql.Mysql_store_result(array);
      for(size_t i = 0; i < array.size(); ++i){
        q.id = array[i][0];
        q.name = array[i][1];
        q.star = array[i][2];
        q.desc = array[i][3];
        q.header_cpp = array[i][4];
        q.tail_cpp = array[i][5];
        model_[q.id] = q;   
      }

      //std::ifstream file("./oj_data/oj_config.cfg");
      //if(!file.is_open()){
      //  return false;
      //}
      //std::string line;
      //while(std::getline(file,line)){
      //  std::vector<std::string> tokens;
      //  StringUtil::Split(line, "\t", &tokens);
      //  if(tokens.size() != 4 ){
      //    LOG(ERROR) << "config file format error!\n";
      //    continue;
      //  }
      //  FileUtil::Read(q.dir + "/desc.txt", &q.desc);
      //  FileUtil::Read(q.dir + "/header.cpp", &q.header_cpp);
      //  FileUtil::Read(q.dir + "/tail.cpp", &q.tail_cpp);
      //  model_[q.id] = q;       //方括号方法，如果key不存在，则创建新的键值对，如果存在，则查找对应value，用新的替换旧的
      //}
      //file.close();
      t_mysql.Mysql_close();
      LOG(INFO) << "Load" << model_.size() << "questions\n";
      return true;
    }

    bool GetAllQuestions(std::vector<Question>* questions)const{
      //遍历哈希表，获得所有的题目
      questions->clear();
      for(const auto& kv:model_){
        //auto推导出来的类型是键值对，push_back需要的类型是Questio
        questions->push_back(kv.second); 
      }
      return true;
    }

    bool GetQuestion(const std::string& id, Question* q)const{
      //q为输出型参数
      auto pos = model_.find(id);    //const成员函数中只能使用const迭代器
      if(pos == model_.end()){
        //该id没查找到
	      return false;
      }
      *q = pos->second;
      return true;
    }
};

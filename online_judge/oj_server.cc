#include"httplib.h"
#include<jsoncpp/json/json.h>
#include"compile.hpp"
#include"util.hpp"
#include"oj_model.hpp"
#include"oj_view.hpp"

int main(){
  //服务器启动只加载一次
  OjModel model;
  model.Load();
  using namespace httplib;
  Server server;
  server.Get("/all_questions",[&model](const Request& req, Response& resp){
      (void)req;
      std::vector<Question> all_questions;
      model.GetAllQuestions(&all_questions);
      std::string html;
      //如何借助all_questions得到最终的html
      OjView::RenderAllQuestion(all_questions, &html);
      resp.set_content(html, "text/html");   //生成了响应对象,组织网页
      });
  //R"()" C++11 引入的语法，原始字符串（胡忽略字符串中的转义字符)
  //\d+正则表达式，用一些特殊符号来表示字符串满足什么样的条件
  server.Get(R"(/question/(\d+))",[&model](const Request& req, Response& resp){
      Question question;
      model.GetQuestion(req.matches[1].str(),&question);
      std::string html;
      OjView::RenderQuestion(question,&html);
      resp.set_content(html, "text/html");
      });
  
  server.Post(R"(/compile/(\d+))",[&model](const Request& req, Response& resp){
      Question question;
      model.GetQuestion(req.matches[1].str(), &question);
      
      std::unordered_map<std::string, std::string> body_kv;
      UrlUtil::ParseBody(req.body, &body_kv);
      const std::string& user_code = body_kv["code"];
      Json::Value req_json;   
      req_json["code"] = user_code + question.tail_cpp;
      Json::Value resp_json; 
      Compiler::CompilerAndRun(req_json, &resp_json);
      std::string html;
      OjView::RenderResult(resp_json["stdout"].asString(), resp_json["reason"].asString(), &html);
      resp.set_content(html,"text/html");
      });
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0",9000);
  return 0;
}


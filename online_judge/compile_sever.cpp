#include<unordered_map>
#include"cpp-httplib/httplib.h"
#include"compile.hpp"
#include<jsoncpp/json/json.h>

int main(){
  //写在里面好，如果写在外面，起不到名字隔离的作用，提高了名字冲突的概率
  using namespace httplib;
  Server server;
  //Get注册了一个回调函数，这个函数的调用时机，处理Get方法的时候
  //lambda表达式：匿名函数，[](){};
  //get中的第一个参数是：路由
  server.Post("/compile",[](const Request& req, Response& resp){
      //根据具体的问题场景，根据请求，计算出响应结果
      (void)req;
      //如何从req中获取json请求，JSON如何和HTTP协议结合
      //需要的请求格式是JSON格式，而HTTP提供的是另一种键值对的格式，因此需要格式转换
      //服务器收到请求后，做的第一件事就是进行urldecode.然后解析这个数据整理成需要的JSON格式
      
      //键值对用二叉搜索树、哈希表进行表示,unordered_map是一个容器
      std::unordered_map<std::string, std::string> body_kv;
      UrlUtil::ParseBody(req.body, &body_kv);
      //在这里调用CompileAndRun
      Json::Value req_json;   //从req对象中获取到
      for(auto p : body_kv){
                                       //p的类型与解引用得到的类型是一致的
        req_json[p.first] = p.second; 
      }
      Json::Value resp_json;  //resp_json放到响应中
      Compiler::CompilerAndRun(req_json, &resp_json);
      //需要把Json对象序列化成字符串才能返回
      Json::FastWriter writer;
      resp.set_content(writer.write(resp_json),"text/plain");
      });
  //下面这个目录可以让浏览器可以访问到静态页面
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0",9092);       //传递端口号
  return 0;
}

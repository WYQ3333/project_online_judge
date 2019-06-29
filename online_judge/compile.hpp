#pragma once
#include<string>
#include<jsoncpp/json/json.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include"util.hpp"
#include<atomic>

//此代码完成在线编译模块的功能
//提供一个Compiler 类，由这个类提供一个核心的
//CompileAndRun 函数，由这个函数来完成编译+运行的功能

//本质上，在此通过使用文件完成进程间通信
//1.源代码文件,此处的name表示当前请求的名字
//    请求和请求之间，name必须是不同的
static std::string SrcPath(const std::string& name){
  return "./temp_files/" + name + ".cpp";
}
//2.编译错误文件
static std::string CompileErrorPath(const std::string& name){
  return "./temp_files/" + name + ".compile_error";
}
//3.可执行程序文件
static std::string ExePath(const std::string& name){
  return "./temp_files/" + name + ".exe";
  //windows可执行程序的格式是PE
  //Linux可执行的程序的格式是ELF
  //由于同种格式的操作系统中，不同的操作系统提供的动态库以及版本可能不同，ELF会依赖动态库(因此一个系统的可执行程序可能在另一个
  //系统中运行不同),可以通过静态库解决此问题，但也有其缺点
}
//4.标准输入文件
static std::string StdinPath(const std::string& name){
  return "./temp_files/" + name + ".stdin";
}
//5.标准输出文件
static std::string StdoutPath(const std::string& name){
  return "./temp_files/" + name + ".stdout";
}
//6.标准错误文件
static std::string StderrPath(const std::string& name){
  return "./temp_files/" + name + ".stderr";
}

class Compiler{
  public:
    //JSON格式，Json::Value 是jsoncpp中的核心类，记住这个类就可以完成序列化与反序列化的动作
    //这个类的使用方法与map非常相似,可以使用[]完成属性的操作
    static bool CompilerAndRun(const Json::Value& req,Json::Value* resp){
      //1.根据json请求对象生成源代码文件,和标准输入文件
      if(req["code"].empty()){
        (*resp)["error"] = 3;
        (*resp)["reason"] = "code empty";
        LOG(ERROR) << "code empty" << std::endl;
        return false;
      }
      //req["code"]是根据key取出vaule，value也是Json::Value，这个类型通过asString 转化成字符串
      
      const std::string& code = req["code"].asString();
      //std::string code = req["code"].asString();
      
      //通过这个函数将代码写到标准输入文件中
      std::string file_name = WriteTmpFile(code, req["stdin"].asString());
      
      //把标准输入写到文件中
      //2.调用g++进行编译(fork+程序替换)
      bool ret = Compile(file_name);
      if(!ret){
        //错误处理
        (*resp)["error"] = 1;
        std::string reason = "";
        FileUtil::Read(CompileErrorPath(file_name), &reason);
        (*resp)["reason"] = reason;
        //虽然是编译出错，但这样的错误是用户自己的错误，不是服务器的错误
        LOG(INFO) << "Compile failde!(please check yours code!!!) " << std::endl;
        return false;
      }
      //会生成一个可执行程序，（编译失败无可执行程序，需要记录编译错误，用一个文件记录，重定向）
      //3.编译成功，调用可执行程序，把我们标准输入记录到文件中，把文件中的内容重定向给可执行程序
      // 可执行程序的标准输出和标准错误也要重定向到文件中
      int sig = Run(file_name);
      if(sig != 0){
        //错误处理
        (*resp)["error"] = 2;
        (*resp)["reason"] = "Program exit by signo: " + std::to_string(sig);
        LOG(INFO) << "Program exit by signo: " << std::to_string(sig) << std::endl;
        return false;
      }

      // 4.把程序的最终结果进行返回，构造resp对象
      (*resp)["error"] = 0;
      (*resp)["reason"] = "";
      std::string str_stdout;
      FileUtil::Read(StdoutPath(file_name), &str_stdout);
      (*resp)["stdout"] = str_stdout;

      std::string str_stderr;
      FileUtil::Read(StderrPath(file_name), &str_stderr);
      (*resp)["stderr"] = str_stderr;
      LOG(INFO) << "Program " << file_name << " Done" << std::endl;
      return true;
   }
private:
    //1.把代码写到文件里，2.给这次请求分配唯一的名字，铜通过返回值返回
    //分派的名字形如：tmp_1550976161.1
    static std::string WriteTmpFile(const std::string& code, const std::string& str_stdin){
      //原子操作依赖CPU的支持
      static std::atomic_int id(0);
      ++id;
      std::string file_name = "tmp_" + std::to_string(TimeUtil::TimeStamp()) + "." + std::to_string(id);
      FileUtil::Write(SrcPath(file_name), code);
      FileUtil::Write(StdinPath(file_name), str_stdin);
      return file_name;
    }

    static bool Compile(const std::string& file_name){
      //1.先构造出编译指令：g++ file_name.cpp -o file_name.exe -std=c++11
      //必须要保证command的指针指向有效内存
      // sprintf将格式化的字符串写到数组中
      // 2创建子进程，父进程进程等待，子进程进行程序替换
      char* command[20] = {0};
      char buf[20][50] = {{0}};
      int i = 0;
      for(i = 0; i < 20; ++i){
        command[i] = buf[i];
      }

      sprintf(command[0], "%s", "g++");
      sprintf(command[1], "%s", SrcPath(file_name).c_str());
      sprintf(command[2], "%s", "-o");
      sprintf(command[3], "%s", ExePath(file_name).c_str());
      sprintf(command[4], "%s", "-std=c++11");
      command[5] = NULL;

      int ret = fork();
      if(ret > 0){
        waitpid(ret, NULL, 0);
      }
      else{
        int fd = open(CompileErrorPath(file_name).c_str(), O_WRONLY|O_CREAT, 0666); //权限是八进制的，所以要用0表示八进制
        if(fd < 0){
          LOG(ERROR) << "open Compiler file error" << std::endl;
          exit(1);
        }
        dup2(fd, 2);   //将标准错误重定向到文件
        execvp(command[0], command);
        exit(0);  //进程替换失败退出，否则不退出，如果退出，可能时command拼接错误
      }
      //代码执行到这里，不知道编译是否成功；解决办法：判断可执行文件是否存在
      //stat是系统提供的函数，功能与ls相同，ls是基于stat实现的
      struct stat st;
      ret = stat( ExePath(file_name).c_str(), &st );
      if(ret < 0){
        LOG(INFO) << "Compile failed (compile): " << file_name << std::endl;
        return false;
      }
      LOG(INFO) << "Compile " << file_name << " OK!" << std::endl;
      return true;
    }
    static int Run(const std::string& file_name){
      //1.创建子进程
      int ret = fork();
      if(ret > 0){
        int status = 0;
        waitpid(ret, &status, 0);
        return status & 0x7f;
      }
      else{
        //首先进行重定向
        //2.父进程等待，子进程进行程序替换
        int fd_stdin = open(StdinPath(file_name).c_str(), O_RDONLY);
        dup2(fd_stdin,0);
        int fd_stdout = open(StdoutPath(file_name).c_str(), O_WRONLY|O_CREAT, 0666);
        dup2(fd_stdout,1);
        int fd_stderr =open(StderrPath(file_name).c_str(), O_WRONLY|O_CREAT, 0666);
        dup2(fd_stderr,2);
        execl(ExePath(file_name).c_str(), ExePath(file_name).c_str(), NULL);
        exit(0);
      }
    }
};

#pragma once
#include<string>
#include<ctemplate/template.h>
#include"oj_model.hpp"
class OjView{
  public:
    //根据数据，生成html这个动作，通常称为网页渲染
    static void RenderAllQuestion(const std::vector<Question>& all_questions, std::string* html){
      ctemplate::TemplateDictionary dict("all_questions");
      for(const auto& question:all_questions){
        ctemplate::TemplateDictionary* table_dict = dict.AddSectionDictionary("question");
        table_dict->SetValue("id",question.id);
        table_dict->SetValue("name",question.name);
        table_dict->SetValue("star",question.star);
      }
      ctemplate::Template* tp1;
      tp1 = ctemplate::Template::GetTemplate("./template/all_questions.html", ctemplate::DO_NOT_STRIP);
      tp1->Expand(html, &dict);
    }
    static void RenderQuestion(const Question& question, std::string* html){
        ctemplate::TemplateDictionary dict("question");
        dict.SetValue("id",question.id);
        dict.SetValue("name",question.name);
        dict.SetValue("star",question.star);
        dict.SetValue("desc",question.desc);
        dict.SetValue("header",question.header_cpp);
      ctemplate::Template* tp1;
      tp1 = ctemplate::Template::GetTemplate("./template/question.html", ctemplate::DO_NOT_STRIP);
      tp1->Expand(html, &dict);
    }
    static void RenderResult(const std::string& str_stdout, const std::string& reason, std::string* html){
        ctemplate::TemplateDictionary dict("result");
        dict.SetValue("stdout",str_stdout);
        dict.SetValue("reason",reason);
        ctemplate::Template* tp1;
        tp1 = ctemplate::Template::GetTemplate("./template/result.html", ctemplate::DO_NOT_STRIP);
        tp1->Expand(html, &dict);
    }
};


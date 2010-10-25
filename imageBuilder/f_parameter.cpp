#include "f_parameter.h"
#include <QRegExp>
#include <QTextStream>
#include <stdlib.h>

using namespace std;

f_parameter::f_parameter(QString func, map<QString, QString> pars)
{
  parameters = pars;
  fname = func;
}

f_parameter::f_parameter(vector<QString> pars){
  // assume that first name is the function name
  // and that the follwing are the paramters
  // in the format of par=value, or par (assumed=true)
  if(!pars.size()){
    fname = "null";
    return;
  }
  fname = pars[0];
  QRegExp rx("(\\w+)=(\\S+)");
  //rx.setMinimal(true);
  for(uint i=1; i < pars.size(); ++i){
    if(rx.indexIn(pars[i]) != -1){
      parameters.insert(make_pair(rx.cap(1), rx.cap(2)));
    }else{
      parameters.insert(make_pair(pars[i], ""));
    }
  }
}

void f_parameter::setFunction(QString& f)
{
  fname = f;
}

QString f_parameter::function(){
  return(fname);
}

bool f_parameter::defined(QString pname){
  return(parameters.count(pname));
}

bool f_parameter::param(QString par, float& f)
{
  if(!parameters.count(par))
    return(false);
  bool ok;
  float tf = parameters[par].toFloat(&ok);
  if(ok)
    f = tf;
  return(ok);
}

bool f_parameter::param(QString par, int& i)
{
  if(!parameters.count(par))
    return(false);
  bool ok;
  int ti = parameters[par].toInt(&ok);
  if(ok)
    i = ti;
  return(ok);
}

bool f_parameter::param(QString par, unsigned int& i)
{
  int s_i;
  if(!param(par, s_i))
    return(false);
  i = abs(s_i);
  return(true);
}

bool f_parameter::param(QString par, short& s){
  if(!parameters.count(par))
    return(false);
  bool ok;
  int ts = parameters[par].toShort(&ok);
  if(ok)
    s = ts;
  return(ok);
}

bool f_parameter::param(QString par, bool& b){
  if(!parameters.count(par))
    return(false);
  if(parameters[par].contains("false", Qt::CaseInsensitive) ||
     parameters[par] == "F" || parameters[par] == "f" ||
     parameters[par] == "0"){
    b = false;
  }else{
    b = true;
  }
  return(true);
}
     

bool f_parameter::param(QString par, string& s)
{
  if(!parameters.count(par))
    return(false);
  s = parameters[par].toAscii().constData();
  return(true);
}

bool f_parameter::param(QString par, QString& str)
{
  if(!parameters.count(par))
    return(false);
  str = parameters[par];
  return(true);
}

bool f_parameter::param(QString par, QChar sep, vector<int>& ints)
{
  if(!parameters.count(par))
    return(false);
  vector<QString> words = vecArgument(parameters[par], sep);
  bool ok;
  int n;
  for(unsigned int i=0; i < words.size(); ++i){
    n = words[i].toInt(&ok);
    if(ok)
      ints.push_back(n);
  }
  if(ints.size())
    return(true);
  return(false);
}

bool f_parameter::param(QString par, QChar sep, vector<unsigned int>& ints)
{
  vector<int> s_ints;
  if(!param(par, sep, s_ints))
    return(false);
  ints.resize(s_ints.size());
  for(unsigned int i=0; i < s_ints.size(); ++i)
    ints[i] = abs(s_ints[i]);
  return(true);
}

bool f_parameter::param(QString par, QChar sep, vector<QString>& words)
{
  if(!parameters.count(par))
    return(false);
  words = vecArgument(parameters[par], sep);
  return(true);
}

vector<QString> f_parameter::vecArgument(QString& str, QChar sep)
{
  vector<QString> strings;
  QChar c;
  QString word;
  QTextStream tst(&str);
  while(!tst.atEnd()){
    tst >> c;
    if(c == sep){
      if(word.length())
	strings.push_back(word);
      word = QString("");
      continue;
    }
    word.append(c);
  }
  if(word.length())
    strings.push_back(word);
  return(strings);
}

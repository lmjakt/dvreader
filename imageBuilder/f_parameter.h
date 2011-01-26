#ifndef F_PARAMETER_H
#define F_PARAMETER_H

// a more generalised parameter class than the p_parameter

#include <QString>
#include <QChar>
#include <map>
#include <string>
#include <vector>
#include <QColor>

class f_parameter {
 public:
  f_parameter(QString fname, std::map<QString, QString> pars);
  f_parameter(std::vector<QString> words);
  f_parameter(QString line);

  void setFunction(QString& f);
  QString function();
  bool defined(QString pname);
  bool param(QString par, float& f);
  bool param(QString par, int& i);
  bool param(QString par, unsigned int& i);
  bool param(QString par, short& s);
  bool param(QString par, unsigned char& uc);
  bool param(QString par, bool& b);
  bool param(QString par, std::string& s);
  bool param(QString par, QString& str);
  bool param(QString par, QChar sep, std::vector<int>& ints);  // assumes comma seperated.
  bool param(QString par, QChar sep, std::vector<unsigned int>& ints);
  bool param(QString par, QChar sep, std::vector<float>& floats);
  bool param(QString par, QChar sep, std::vector<QString>& words);
  bool param(QString par, std::vector<QColor>& colors);  // hard-coded separators, tries to get colours as r,g,b;r,g,b;yellow, etc.. 

  std::map<QString, QString> params(){
    return(parameters);
  }

 private:
  std::map<QString, QString> parameters;
  std::vector<QString> vecArgument(QString& str, QChar sep);
  QColor getColor(QString cname);
  QString fname;
};

#endif

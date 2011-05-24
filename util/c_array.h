#ifndef C_ARRAY_H
#define C_ARRAY_H

// A container that contains a char*
// Similar to a vector, but overloads the push_back function
// to allow pushing of all sorts of data types (e.g. int,
// unsigned int etc.. 
// essentially a stream I suppose, but everything in binary
// no conversions, just typecasts.

// a problem I'm sure solved somewhere else..
// Use to serialise data structures.

// NOT really likely to be fast, just convenient

// WARNING: will fail if sizeof(..) does not return the same on different platforms

#include <string>
#include <string.h> 
#include <QString>

class c_array {
 public:
  c_array(size_t n, bool destructive);
  ~c_array();
  
  char* data();
  size_t size(){
    return(pos);
  }
  size_t capacity();

  void push(unsigned char c);
  void push(char c);
  void push(unsigned int i);
  void push(int i);
  void push(unsigned long l);
  void push(long l);
  void push(float f);
  void push(std::string s);
  // the following make no attempt to consider multibyte strings.
  // and do not write a terminating 0
  void push(QString qs);
  void push(const char* c, size_t n);

 private:
  size_t cap;
  size_t pos;
  char* dat;
  bool destructive;

  void grow();
};

#endif

#include "c_array.h"

c_array::c_array(size_t n, bool destructive)
  : cap(n), pos(0), dat(0), destructive(destructive)
{
  cap = cap > 0 ? cap : 10;
  dat = new char[ cap ];
}

c_array::~c_array()
{
  if(destructive)
    delete []dat;
}

void c_array::push(unsigned char c)
{
  push((char*)&c, 1);
}

void c_array::push(char c)
{
  push(&c, 1);
}

void c_array::push(unsigned int i)
{
  push((char*)&i, sizeof(unsigned int));
}

void c_array::push(int i)
{
  push((char*)&i, sizeof(int));
}

void c_array::push(unsigned long l)
{
  push((char*)&l, sizeof(unsigned long));
}

void c_array::push(long l)
{
  push((char*)&l, sizeof(long));
}

void c_array::push(float f)
{
  push((char*)&f, sizeof(float));
}

// This will fail if its not known if the string is
// multibyte or single byte
void c_array::push(std::string s)
{
  push(s.c_str(), s.size());
}

void c_array::push(QString qs)
{
  push(qs.toAscii().data(), qs.length());
}

void c_array::push(const char* c, size_t n)
{
  while(cap <= (pos + n))
    grow();
  memcpy((void*)(dat + pos), (void*)c, n);
}

void c_array::grow()
{
  cap *= 2;
  char* new_dat = new char[ cap ];
  memcpy((void*)new_dat, dat, pos);
  delete []dat;
  dat = new_dat;
}

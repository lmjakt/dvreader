#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <vector>

using namespace std;

void swapBytes(char* from, char* to, unsigned int wn, unsigned int ws){
  for(int i=0; i < wn; i++){        // wn is the number of words, hence the byte length is ws * wn ..
    for(int j=0; j < ws; j++){
      /// assign first byte of to as the last value of the ith word of from
      to[i * ws + j] = from[(i+1) * ws - (j + 1)];
    }
  }
}

int main(int argc, char** argv){
  int a = 1024;
  int g = -1024;
  int b[5] = {1, 2, 4, 8, 16};
  int f[5];  // assign by swapBytes.. 
  cout << a << endl;
  int c;
  swab((const void*)&a, (void*)&c, 4);
  cout << c << endl;
  int d = htonl(a);
  cout << d << endl;
  int h = htonl(g);
  cout << "g --> h" << g << "  --> " << h << endl;

  cout << "and try with swapBytes function.. " << endl;
  swapBytes((char*)&a, (char*)&d, 1, sizeof(int));
  cout << d << endl;
  swapBytes((char*)&g, (char*)&h, 1, sizeof(int));
  cout << "and again .. g --> h" << g << "  --> " << h << endl;
  
  // and with an array..
  swapBytes((char*)b, (char*)f, 5, sizeof(int));
  for(int i=0; i < 5; i++){
    cout << b[i] << " --> " << f[i] << endl;
  }

  // try the memset function with floats..
  int n = 10;
  int m = 1;
  float* fbuffer = new float[n];
  memset((void*)fbuffer, m, sizeof(float)*n);
  for(int i=0; i < n; i++){
      cout << i << "\t" << fbuffer[i] << endl;
  }

  cout << "size of a pointer is " << sizeof(void*) << endl;

  bool hello = false;
  if( (hello = 5 > 9) ){
	cout << "5 is larger than 3 and hello is now : " << hello << endl;
  }
  cout << "And at this point hello is " << hello << endl; 

  vector<int> v(10);
  for(uint i=0; i < v.size(); i++){
	v[i] = i * 2;
  }
  cout << "Address of v is   " << &v
       << "\nAddress of v[0] : " << &v[0] << endl;
  //bool a = true;
  cout << "the size of a bool is : " << endl;
  cout << sizeof(bool) << endl;

  // try some binary comparisons..
  char aa = 0;
  char bb = 1;
  char cc = 2;
  char dd = 3;

  if(aa & bb){
      cout << "aa & bb is true" << endl;
  }
  if(bb & dd){
      cout << "bb & dd is true" << endl;
  }
  if(cc & dd){
      cout << "cc & dd is true" << endl;
  }
  if(dd & cc){
      cout << "dd & cc is true" << endl;
  }
  if(dd & bb){
      cout << "dd & bb is true" << endl;
  }
  if(aa & dd){
      cout << "aa & dd is true" << endl;
  }
  if(bb & cc){
      cout << "bb & cc is true" << endl;
  }
  char ee = 0;
  //char ff = 0;
  
  ee |= bb;
  cout << "ee is now " << (int)ee << endl;
  ee |= cc;
  cout << "ee is now " << (int)ee << endl;
  char ff = bb|ee;
  cout << "ff is " << (int)ff << endl;
  ff |= bb;
  cout << "ff is still " << (int)ff << endl;

}
    

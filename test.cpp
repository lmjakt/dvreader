#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include "image/bitMask.h"
#include "image/volumeMask.h"
#include <set>
#include <limits.h>

using namespace std;

void swapBytes(char* from, char* to, unsigned int wn, unsigned int ws){
  for(int i=0; i < wn; i++){        // wn is the number of words, hence the byte length is ws * wn ..
    for(int j=0; j < ws; j++){
      /// assign first byte of to as the last value of the ith word of from
      to[i * ws + j] = from[(i+1) * ws - (j + 1)];
    }
  }
}

void print_bits(unsigned char b){
    for(int i=0; i < 8; ++i){
	cout << (bool) ((b << i) & 128);
    }
    cout << endl;
}

int main(int argc, char** argv){
    vector<uint> names;
    names.push_back(1);
    uint i = 2;
    while(i <= UINT_MAX/2){
	names.push_back(i);
	cout << names.size() << " : " << i << endl;
	i *= 2;
	//if(i > 1024)
	//   exit(1);
    }
    {
	int a;
	a = 0;
	for(uint i=0; i < names.size(); ++i){
	    cout << i << " : " << a << " |= " << names[i] << "  --> ";
	    a |= names[i];
	    cout << a << endl;
	}
    }

    exit(0);
    vector<int> testVector;
    testVector.resize(100);
    testVector.assign(100, 5);
    cout << "testVector size " << testVector.size() << endl;
    for(uint i=0; i < testVector.size(); ++i){
	cout << i << "\t" << testVector[i] << endl;
    }


    // try some set deletions to check;
    set<int> ints;
    for(int i=0; i < 1000; ++i){
	ints.insert(i);
    }
    for(set<int>::iterator it=ints.begin(); it != ints.end(); ++it){
	if((*it) == 25){
	    for(int i=24; i < 999; ++i)
		ints.erase(i);
	}
	
	cout << (*it) << endl;
    }
    exit(0);

    // try some bitwise operations. To see what we can do.
    {
	int dx = -1;
	unsigned long ul = 20;
	cout << "ul is " << ul << " dx is " << dx << "  and ul + dx is " << ul + dx << endl;
    }

    BitMask* bm = new BitMask(100);
    bm->set_bit(true, 23);
    bm->set_bit(true, 0);
    bm->set_bit(true, 98);
    bm->set_bit(true, 99);
    bm->set_bit(true, 47);
    bm->set_bit(true, 48);
    bm->set_bit(true, 49);

    bm->set_bit(true, 120);


    bm->printMask();

    bm->set_bit(false, 48);
    cout << "set 48 to false" << endl;
    bm->printMask();
    
    {
	unsigned long w, h, d;
	w = h = 30;
	d = 5;
	VolumeMask* vm = new VolumeMask(w, h, d);
	for(unsigned long z=0; z < d; ++z){
	    for(unsigned long x = 0; x < w; ++x)
		vm->setMask(true, x+z, x, z);
	}
	vm->printMask();

    }

    {
	unsigned char k = (char) (atoi(argv[1]));
	cout << "Right shifting " << (int)k << endl;
	for(int i=0; i < 8; ++i){
	    bool b = (k >> i) & 01;
	    cout << b;
	}
	cout << endl << "Left shifting " << (int)k << endl;
	for(int i=0; i < 8; ++i){
	    bool b = (k << i) & 01;
	    unsigned char d = k << i;
	    cout << (bool) ((k << i) & 128);
	}
	cout << endl;
	// and applying the k to the mask..
	unsigned char mask = 0;
	if(k > 0){
	    mask = 128;
	    mask >>= (k-1);
	}
	cout << "mask becomes " << (unsigned int)mask << endl;
	for(int i=0; i < 8; ++i){
	    cout << (bool) ((mask << i) & 128);
	}
	cout << endl;
	cout << "using print bits to print bits" << endl;
	print_bits(mask);
	mask =~ mask;
	print_bits(mask);
    }

    {
	unsigned int a = 3e9;
	unsigned int b = 2e9;
	cout << a << " + " << b << " = " << a + b << endl;
	unsigned long c = (unsigned long)a + b;
	cout << " or is it going to be : " << c << endl;
    }

    exit(0);

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
    

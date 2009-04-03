#include "bitMask.h"
#include <iostream>

BitMask::BitMask(unsigned long size){
    bit_size = size;
    mask = 0;
    if(!bit_size)
	return;
    byte_size = bit_size % 8 ? (bit_size / 8) + 1 : bit_size / 8;
    mask = new unsigned char[byte_size];
    memset((void*)mask, 0, byte_size);
}

BitMask::~BitMask(){
    delete mask;  // if 0 doesn't delete in C++ (I think)
}

bool BitMask::set_bit(bool b, unsigned long p){
    if(p > bit_size)
	return(false);
    unsigned long byte_pos = p / 8;
    unsigned char bit_pos = p % 8;
    unsigned char m = 128; // 10000000
    m >>= bit_pos;
    if(b){
	mask[byte_pos] |= m; 
	return(true);
    }
    m =~ m; // reverses
    mask[byte_pos] &= m;
    return(true);
}

bool BitMask::bit(unsigned long p){
    if(p > bit_size)
	return(false);
    unsigned long byte_pos = p / 8;
    unsigned char bit_pos = p % 8;
    unsigned char m = 128; // 10000000
    m >>= bit_pos;
    return( (m & mask[byte_pos]) );
}

void BitMask::printMask(){
    for(ulong i=0; i < byte_size; ++i)
	print_byte(mask[i]);
}

void BitMask::print_byte(unsigned char b){
    for(int i=0; i < 8; ++i)
	std::cout << (bool) ((b << i) & 128);
    std::cout << std::endl;
}

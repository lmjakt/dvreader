#ifndef BITMASK_H
#define BITMASK_H

// this defines a binary mask implemented at the bit level using binary shifts.
// This is done in order to be able to have very large masks that we can use for
// A variety of purposes.

typedef unsigned long ulong;

class BitMask {
 public:
    BitMask(unsigned long size);
    ~BitMask();
    bool set_bit(bool b, unsigned long p);
    bool bit(unsigned long p);
    void printMask();
    void zeroMask();

 private:
    unsigned char* mask;
    unsigned long bit_size;
    unsigned long byte_size;  // either bit_size / 8 or 1 + (bit_size / 8 )
    void print_byte(unsigned char b);
};

#endif

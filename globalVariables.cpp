#include <stdlib.h>
#include "globalVariables.h"
#include <string.h>

// globalVariables should have been called globalConstants.

// it's probably the case that I should be returning a const char* const
// rather than just a const char* as I believe that the user can actually
// modify the content of the const* char, but not the address ?? though
// that doesn't make much sense to me.

char* dvhome = 0;

const char* dvreader_home(){
  if(dvhome)
    return(dvhome);
  const char* home = getenv("HOME");
  if(!home)
    return("my_home_dir");
  size_t l = strlen(home);
  size_t hl = l + 10;  // length of '/.dvreader'
  dvhome = new char[ hl + 1 ];  // strlen doesn't include the initial character
  strncpy(dvhome, home, l);
  strncpy(dvhome + l, "/.dvreader", 10);
  dvhome[hl] = 0;
  return(dvhome);
}



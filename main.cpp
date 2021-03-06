//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include <QApplication>
#include <QStyle>
#include "dvReader.h"
#include "jpgView/jpgView.h"
#include "deltaViewer.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <string>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

int main(int argc, char** argv){
  // supposedly on mac the stack is increased automatically and
  // the following code fails. Hence, let's remove it

  #ifndef __APPLE__
  // some of the recursive functions require a bigger stack.
  struct rlimit stack_limit;
  if(getrlimit(RLIMIT_STACK, &stack_limit)){
    cerr << "Unable to obtain stack_limit exit" << endl;
    exit(1);
  }
  cout << "Current stack " << stack_limit.rlim_cur << "  : " << stack_limit.rlim_max << endl;
  stack_limit.rlim_cur *= 16;
  cout << "And limit set to " << stack_limit.rlim_cur << " : " << stack_limit.rlim_max << endl;

  if(setrlimit(RLIMIT_STACK, &stack_limit)){
    cerr << "Unable to increase stack limit to : " << stack_limit.rlim_cur << endl;
    exit(1);
  }
  // end of non-mac section
#endif

  // the above seems to cause a segmentation fault. consider using ulimit -s
  // don't know why, but crash happens with no stack comment.. ??

  // obtain any options that we might have.. 
  int c;
  map<string, string> opt_commands;
  vector<char*> non_options;
  int xyMargin = 32;
  while( (c = getopt(argc, argv, "-s:b:d:-c:-r:m:l:")) != -1){
    switch(c){
    case 's':
      opt_commands["find_spots"] = optarg;
      break;
    case 'd':
      opt_commands["die"] = "yes";   // useful for making projections
      break;
    case 'b':
      opt_commands["find_blobs"] = optarg;
      break;
    case 'c':
      opt_commands["colorFile"] = optarg;
      break;
    case 'r':
      opt_commands["rangeFile"] = optarg;
      break;
    case 'm':
      xyMargin = atoi(optarg);
      break;
    case 'l':
      opt_commands["max"] = optarg;
      break;
    case 1 :
      non_options.push_back(optarg);
      break;
    case '?' :
      cerr << "Unknown option : " << optopt << endl;
      break;
    default :
      abort();
    }
  }
  
  //     for(map<string, string>::iterator it=opt_commands.begin(); it != opt_commands.end(); ++it)
  // 	cout << (*it).first << " : " << (*it).second << endl;
  //     exit(1);
  
  
  // make a qapplication..
  QApplication::setStyle("cleanlooks");
  QApplication app(argc, argv);
  //    app.setStyle("plastique");
  //    app.setStyle("cde");
  const char* ifName = 0;
  if(non_options.size()){
    ifName = non_options[0];
  }
  DeltaViewer viewer(opt_commands, xyMargin, ifName);
  viewer.setContentsMargins(0, 0, 0, 0);
  app.setMainWidget(&viewer);
  viewer.show();
  return app.exec();
  
}


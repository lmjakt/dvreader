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

#include "tiffReader.h"
//#include "require.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>


using namespace std;

void i_write(unsigned int n, ofstream& out){
  out.write((char*)&n, 4);    // should be OK.. 
}

void s_write(unsigned short s, ofstream& out){
  out.write((char*)&s, 2);
}

void c_write(char c, ofstream& out){
  out.write(&c, 1);
}

void writeEntry(int tag, int format, int num, int v, ofstream& out){
  // v is either the value or the offset,, 
  s_write((unsigned short)tag, out);
  s_write((unsigned short)format, out);
  i_write((unsigned int)num, out);
  switch(format){
  case 3:
    if(num > 1){      // assume offset, don't use this function if this doesn't fit!!
      i_write((unsigned int)v, out);
    }else{
      unsigned short zero = 0;
      s_write((unsigned short)v, out);
      s_write(zero, out);
    }
    break;
  case 4:
    i_write((unsigned int)v, out);
    break;
  case 5:
    i_write((unsigned int)v, out);  // it will be the offset anyhow.. !
    break;
  default: 
    cout << "ERRRORORORORORO don't use this function if you don't know it's limitations" << endl;
  }
}
    
      

unsigned int i_read(ifstream& in){
    unsigned int n;
    in.read((char*)&n, 4);
//    char c[4];  // read into the vector
//    in.read(&c, 4); // should be it..
//    unsigned int n = *(unsigned int*)&c;  // and that should give me the thing
    return(n);
}

unsigned short s_read(ifstream& in){
    short s;
    in.read((char*)&s, 2);
    return(s);
//   char c[2];
//   in.read(&c, 2);
//   unsigned short s = *(unsigned short*)&c;
//   return(s);
}
  
unsigned char c_read(ifstream& in){
  unsigned char c;
  in.read((char*)&c, 1);
  return(c);
}

TiffReader::TiffReader(){
  // do nothing..
}

TiffReader::TiffReader(TiffReader* tr){
  indexed = false;
  values = tr->values;
  w = tr->width();
  h = tr->height();   // makes lots of assumptions..... ,, but it will do for now.. 
  fileName = string("newFile");
}

TiffReader::TiffReader(string iFile){
  indexed = false;
  fileName = iFile;
  ifstream in(iFile.c_str(), ios::binary);
  cout << "creating the in file thingy" << endl; 
//  assure(in, iFile.c_str());
  // and start reading the file.. just make sure that the 
  // byte order is amenable. Could use ntohl ,, i.e. networ to host functions.
  // if the order is in the other way. But assume not for now..
  vector<unsigned int> stripOffsets;
  vector<unsigned int> stripByteCounts;
  //  char c3;       // ?????
  cout << "Byte order is  : " << c_read(in) << c_read(in) << endl;
  cout << "Magic Number is: " << s_read(in) << endl;
  unsigned int ifdOffset = i_read(in);
  cout << "IFD offset is  : " << ifdOffset << endl;
  /// OK, now we can read through all of the IFD's, first, how many are there, and so on..
  //  cout << "Am now at read position: " << in.tellg() << endl;
  //cout << "Will move to: " << ifdOffset << " to read the input file directory" << endl;
  in.seekg(ifdOffset);
  unsigned short entryNo = s_read(in);
  cout << "number of directory entries: " << entryNo << endl;
  /// each directory entry takes up 12 bytes, no more and no less. The data if its needed is put somewhere else
  /// i.e. if its longer. Or at least it's looking like that at the moment...
  int startPosition = in.tellg();
  for(short i=0; i < entryNo; i++){          // the last directory entry is not an entry, but the pointer to the next ifd
    // cout << endl << "Reading directorey Entry #" << i << endl;
    //cout << "Seeking to " << startPosition + i*12;
    in.seekg(startPosition + i*12);
    //cout << "\t\tFile offset: " << in.tellg() << endl;
    unsigned short tagId = s_read(in);
    //cout << "\t\tTag ID:     " << tagId << endl;
    unsigned short fieldType = s_read(in);
    //cout << "\t\tField type: " << fieldType << endl;
    unsigned int valueNo = i_read(in);
    cout << "\tNumber of Values: " << valueNo << endl;
    unsigned int colourSize;
    switch(tagId){
    case 256:
      // the width.. can be short or long.. but should be just one value
      if(fieldType == 3){ // short
	w = (unsigned int)(s_read(in));
      }else{
	w = i_read(in);
      }
      cout << "\twidth : " << w << endl;
      break;
    case 257:
      // the height
      if(fieldType == 3){
	h = (unsigned int)(s_read(in));
      }else{      // a bit presumptious but..
	h = i_read(in);
      }
      cout << "\theight : " << h << endl;
      break;
    case 258:
      // bits per sample, -- 8, 8, 8 or something like that
      bytesPerSample = valueNo;
      if(valueNo > 3){            // doesn't fit into the thingy,, then
	in.seekg(i_read(in));
      }
      for(uint i=0; i < valueNo; i++){
	bitsPerSample.push_back(s_read(in));
      }
      break;
    case 259:
      compressionScheme = s_read(in);
      cout << "\tcompression : " << compressionScheme << endl;
      // compression,, if its 1 -not compressed, and so alright for me
      break;
    case 262:
      // photometric interpretation, kind of important tells me how to interpret the pixels
      photometricInterpretation = s_read(in);
      cout << "photometic interpretation : " << photometricInterpretation << endl;
      if(photometricInterpretation == 3){ indexed = true; }
      break;
    case 273:
      // The strip offsets,, tells me where the raw data is.. -- short or long.. 
      if(fieldType == 3){  // short.. 
	if(valueNo > 2){    /// doiesn;t fits into the offset thingy,
	  in.seekg(i_read(in));
	}
	for(uint i=0; i < valueNo; i++){
	  stripOffsets.push_back((unsigned int)s_read(in));
	}
      }
      if(fieldType == 4){
	if(valueNo > 1){
	  in.seekg(i_read(in));
	}
	for(uint i=0; i < valueNo; i++){
	  stripOffsets.push_back(i_read(in));
	}
      }
      break;
    case 277:
      // samples per pixels..
      bytesPerSample = s_read(in);
      cout << "Bytes per sample: " << bytesPerSample << endl;
      break;
    case 278: // short or long
      // the number of rows per strip. Am not sure to what extent I need this.. but might as well use it..
      // leave it for now.. 
      break;
    case 279:
      // the stripByteCounts.. important for the input output.. so, there goes..
      if(fieldType == 3){  // short..
	for(uint i=0; i < valueNo; i++){
	  stripByteCounts.push_back((unsigned int)s_read(in));
	}
      }
      if(fieldType == 4){
	for(uint i=0; i < valueNo; i++){
	  stripByteCounts.push_back(i_read(in));
	}
      }
      break;
    case 320:
      // the colour map. Complex stuff. Take it easey..
      // not so bad, first the green, then the red, and then the blues..
      // all in shorts  
      in.seekg(i_read(in));  // go the right place !! 
      colourSize = valueNo / 3;
      if(valueNo % 3){
	cerr << "puke.. puke,, colour map screwed up might as well crash.. !! something will go horribly wrong" << endl;
      }
      colorMap.resize(3);
      for(uint i=0; i < colorMap.size(); i++){
	//cout << "pushing back colorMap with empty thing" << endl;
//	colorMap.push_back();
	colorMap[i].resize(colourSize);
	for(uint j=0; j < colourSize; j++){
	  colorMap[i][j] = s_read(in);
	  //cout << "colorMap[" << i << "][" << j << "]\t" << colorMap[i][j] << endl;
	}
      }     // which should be it. Just remember what the RGB stands for.. and lets hope we don't crash here... 
      break;
    default :
      // do nothing now, just calm down for a moment..
      break;
    }
  }
  cout << "finished reading the directory information" << endl;
  // the offset for the next ifd.. ignore for now.. 
  in.seekg(startPosition + 12*entryNo);
  int nextOffset = i_read(in);
  cout << "\nNext offset is: " << nextOffset << endl;
  // resize the values map appropriately..
  //  values.resize(bytesPerSample);
  values.resize(3);
  int count = 0;      // number of values read in for determining where we put them.. 
  for(uint i=0; i < stripOffsets.size(); i++){
    cout << "Strip Offset : " << i << "\t: " << stripOffsets[i] << "\tbytecCount: " << stripByteCounts[i] << endl;
    in.seekg(stripOffsets[i]);
    for(int j=0; j < 3; j++){     // make rgb regardless. I like rgb it's better.. !! hmmm... 
      if(indexed){
	values[j].reserve(values.size() + stripByteCounts[i]);
      }else{
	values[j].reserve(values.size() + (stripByteCounts[i]/3));
      }
    }
//      for(int j=0; j < bytesPerSample; j++){
//        values[j].reserve(values.size() + (stripByteCounts[i]/bytesPerSample));
//      }
    for(uint j=0; j < stripByteCounts[i]; j++){
      if(indexed){
	unsigned int currentIndex = (unsigned int)(c_read(in));
	values[0].push_back((unsigned char)((colorMap[0][currentIndex])/256));
	values[1].push_back((unsigned char)((colorMap[1][currentIndex])/256));
	values[2].push_back((unsigned char)((colorMap[2][currentIndex])/256));
	//	cout << "j: " << j << "\tindex : " << color
      }else{
	values[count % bytesPerSample].push_back(c_read(in));
      }
      //int lastIndex = values[count % bytesPerSample].size() - 1;
      //cout << "i " << i << "\tchannel: " << count % bytesPerSample << "\tvalue: " << values[count % bytesPerSample][lastIndex] << endl;
      count++;
      //cout << values.size() << "\t" << values.size() % 3 << "\t" << (int)values[values.size()-1] << endl;
    }
  }
  cout << "size of values: " << values.size() << endl;
  for(uint i=0; i < values.size(); i++){
    cout << "\tsize of values[" << i << "] : " << values[i].size() << endl;
  }
  writeOut(string("test.tif"));
}

bool TiffReader::makeFromRGBFloat(float* data, uint wdth, uint hgt){
    values.resize(3);
    w = wdth;
    h = hgt;
    for(uint i=0; i < values.size(); i++)
	values[i].resize(w * h * 3);
    for(uint i=0; i < (w * h); ++i){
	for(uint j=0; j < 3; ++j){
	    float f = data[i * 3 + j];
	    f = f < 0.0 ? 0.0 : f;
	    f = f > 1.0 ? 1.0 : f;
	    values[j][i] =  (short)(255.0 * f);
	}
    }
    // ?? I don't know much else to do ? 
    return(true);
}

// throws away the alpha channel at the moment. Quick hack to give 
// me some images.. 
bool TiffReader::makeFromRGBA(unsigned char* data, uint wdth, uint hgt){
    values.resize(3);
    w = wdth;
    h = hgt;
    for(uint i=0; i < values.size(); i++)
	values[i].resize(w * h * 3);
    for(uint i=0; i < (w * h); ++i){
	for(uint j=0; j < 3; ++j){
	    values[j][i] =  data[4 * i + j];
	}
    }
    // ?? I don't know much else to do ? 
    return(true);
}

bool TiffReader::addMerge(TiffReader* tr){
  if(values[0].size() != tr->values[0].size()){
    cout << "Tiff Reader Values are of different sizes.. returning false" << endl;
    return(false);
  }
  for(uint i=0; i < values.size() && i < tr->values.size(); i++){
    for(uint j=0; j < values[i].size(); j++){
      values[i][j] += tr->values[i][j];
      if(values[i][j] > 255) { values[i][j] = 255; }
    }
  }
  return(true);
  // and that's kind of it.. Really.. 
}

int TiffReader::width(){
  return(w);
}

int TiffReader::height(){
  return(h);
}

void TiffReader::writeOut(string outFile){
  // write to an RGB format tiff file.. should be simple, but I need some functions for writing ints, and chars.. 
  // open the ofstream..
  ofstream out(outFile.c_str(), ios::binary);
//  assure(out, outFile.c_str());
  // byte Order,, 
  char byte = 'I';
  out.write(&byte, 1);
  out.write(&byte, 1);
  // the magic number 42.. 
  unsigned short magic = 42;
  s_write(magic, out);
  // the offset of the first IFD.. 
  unsigned int offset = 8;
  i_write(offset, out);
  // then a short telling how many fields in the IFD. Simplest case for a RGB file is 12,, 
  unsigned short fieldNo = 12;
  s_write(fieldNo, out);
  // then just write out the values of the different fields.. 
  // 256 -- the width  ,, 257 the height,, 
  writeEntry(256, 4, 1, w, out);
  writeEntry(257, 4, 1, h, out);
  writeEntry(258, 3, 3, 158, out);
  writeEntry(259, 3, 1, 1, out);
  writeEntry(262, 3, 1, 2, out);
  writeEntry(273, 4, 1, 182, out);
  writeEntry(277, 3, 1, 3, out);
  writeEntry(278, 4, 1, h, out);
  writeEntry(279, 4, 1, w*h*3, out);
  writeEntry(282, 5, 1, 158+8, out);
  writeEntry(283, 5, 1, 158+8+8, out);
  writeEntry(296, 3, 1, 2, out);
  unsigned int zero = 0;
  unsigned short eight = 8;
  i_write(zero, out);
  s_write(eight, out);
  s_write(eight, out);
  s_write(eight, out);
  s_write((unsigned short)zero, out);
  // s_write((unsigned short)8, 0);
  //s_write((unsigned short)8, 0);
  //s_write((unsigned short)0, 0);  // padding..
  // write the resolution in rational format..
  unsigned int seventwo = 72;
  unsigned int one = 1;
  i_write(seventwo, out);
  i_write(one, out);
  i_write(seventwo, out);
  i_write(one, out);

  // and then just write the data out,, ..
  for(uint i=0; i < values[0].size(); i++){
    for(uint j=0; j < values.size(); j++){
      // the values have been stored in short format,, now I'll have to cast them to char,, hmm.. 
      c_write((char)values[j][i], out);
      //cout << i << "\t" << j << "\t" << values[j][i] << endl;
    }
  }
  out.close();     // end of story.. ho yeahh.. 
}

//  int main(int argc, char** argv){
//    cout << "Main function, argv[1] is " << argv[1] << endl;
//    if(argc < 2){
//      exit(1);
//    }
//    TiffReader* reader = new TiffReader(string(argv[1]));
//  }
  

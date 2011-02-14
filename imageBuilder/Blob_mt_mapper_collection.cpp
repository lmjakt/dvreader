#include "Blob_mt_mapper_collection.h"
#include "blob_mt_mapper.h"
#include "stack_info.h"
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QRegExp>
#include <iostream>
#include <set>
#include "../image/blob.h"

// This should probably be divided up into all sorts of files and assorted bits and pieces,
// but for now..

using namespace std;

void ClassCriteria::insertCriteria(unsigned int m_id, Criteria criteria){
  if(blob_criteria.count(m_id)){
    cerr << "Attempt to insert duplicate criteria into a Class Criteria thingy" << endl;
    return;
  }
  cout << "ClassCriteria id:  " << blob_set_id << " inserting a criteria with mapper_id : " << m_id 
       << "  ==  " << criteria.m_id() << endl;
  blob_criteria.insert(make_pair(m_id, criteria));
}

unsigned int ClassCriteria::assess(QString& par, blob_set& bset){
  unsigned int correction_id = 0;
  for(unsigned int i=0; i < bset.size(); ++i){
    if(! blob_criteria.count( bset.bid(i))){
      cerr << "ClassCriteria::assess, no criteria set for blob mapper id : " << bset.bid(i) 
	   << "  for blob set with id : " << bset.id() << "  contains ids :";
      for(map<unsigned int, Criteria>::iterator it=blob_criteria.begin(); it != blob_criteria.end(); ++it)
	cerr << ", " << (*it).first;
      cerr << endl;
      continue;
      //      return(0);
    }
    float paramValue = getBlobParameter( bset.b(i), par );
    if(!blob_criteria[ bset.bid(i) ].within(par, paramValue))
      correction_id += bset.bid(i);
  }
  bset.setCorrectedId( bset.id() - correction_id );
  return(correction_id);
}

Blob_mt_mapper_collection::Blob_mt_mapper_collection(){
  // nothing really necessary ?
}

Blob_mt_mapper_collection::~Blob_mt_mapper_collection(){
  // destroy all mappers, blobs and associated gubbins.
  
}

// if we have already defined some mappers, we should carefully remove any with the incorrect
// 
void Blob_mt_mapper_collection::setMappers(map<unsigned int, vector<blob_set> >& blbs, map<unsigned int, vector<Blob_mt_mapper*> > maps){
  if(blobs.size() || mappers.size()){
    cerr << "Blob_mt_mapper_collections : mappers and blobs already defined. " << endl;
  }
  mappers = maps;
  blobs = blbs;
}

bool Blob_mt_mapper_collection::readCriteriaFromFile(QString& fileName){
  // do some magic to build up the correct stuffs here.. 
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    cerr << "Blob_mt_mapper_collection::readCriteriaFromFile : Unable to open file" << endl;
    return(false);
  }
  QTextStream in(&file);
  QString header;
  QStringList params;
  if(!in.atEnd())
    header = in.readLine();
  params = header.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  if(!params.size()){
    cerr << "Blob_mt_mapper_collection::readCriteriaFromFile : Unable to obtain params from first line" << endl;
    return(false);
  }
  while(!in.atEnd()){
    QString line = in.readLine();
    unsigned int super_id = 0;
    Criteria criteria = readCriteria(line, params, super_id);
    if(!criteria.m_id() || !super_id){
      cerr << "Blob_mt_mapper_collection::unable to readCriteria from line. Should give up and die." << endl;
      continue;
    }
    if(!class_criteria.count(super_id))
      class_criteria.insert(make_pair(super_id, ClassCriteria(super_id)));
    class_criteria[super_id].insertCriteria(criteria.m_id(), criteria);
    cout << "inserted a Criteria with super id " << super_id << " and mapper id : " << criteria.m_id() << endl;
  }
  // Should run some completeness checks, but leave that for later.. 
  return(true);
}

bool Blob_mt_mapper_collection::mapperDimensions(int& w, int& h, int& d){
  stack_info pos;
  map<unsigned int, vector<Blob_mt_mapper*> >::iterator it = mappers.begin();
  if(it == mappers.end())
    return(false);
  if(!(*it).second.size())
    return(false);
  pos = (*it).second[0]->position();
  w = pos.w;
  h = pos.h;
  d = pos.d;
  return(true);
}

Criteria Blob_mt_mapper_collection::readCriteria(QString& line, QStringList& params, unsigned int& super_id){
  QStringList words = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  Criteria criteria;
  if(words.size() != params.size() + 2){
    cerr << "Blob_mt_mapper_collection::readCriteria file format is not good" << endl;
    cerr << "Words.size: " << words.size() << "  params.size() " << params.size() << endl;
    return(criteria);
  }
  QStringList set_id_words = words[0].split(QRegExp(","));
  set<unsigned int> mapper_ids;
  super_id = 0;
  bool ok;
  for(int i=0; i < set_id_words.size(); ++i){
    unsigned int n = set_id_words[i].toUInt(&ok);
    if(!ok){
      super_id = 0;
      cerr << "Blob_mt_mapper_collection::readCriteria unable to obtain mapper ids" << endl;
      break;
    }
    super_id += n;
    mapper_ids.insert(n);
  }
  unsigned int mapper_id = words[1].toUInt(&ok);
  if(!ok || !mapper_ids.count(mapper_id)){
    cerr << "Blob_mt_mapper_collection::readCritera unable to obtain mapper id" << endl;
    return(criteria);
  }
  // then just make ranges using the params, and the expectation things..
  QRegExp rx("(\\d+.?\\d*)-(\\d+.?\\d*)");
  map<QString, Range> ranges;
  for(int i=2; i < words.size(); ++i){
    if(!words[i].contains(rx)){
      cerr << "Blob_mt_mapper_collection::readCriteria, unable to get range from: " << words[i].ascii() << endl;
      return(criteria);
    }
    float min = rx.cap(1).toFloat();
    float max = rx.cap(2).toFloat();
    ranges.insert(make_pair(params[i-2], Range(min, max)));
  }
  return(Criteria(mapper_id, ranges));
}

vector<blob_set> Blob_mt_mapper_collection::blobSets(){
  vector<blob_set> b;
  for(map<unsigned int, vector<blob_set> >::iterator it=blobs.begin();
      it != blobs.end(); ++it)
    b.insert(b.end(), (*it).second.begin(), (*it).second.end());
  return(b);
}

vector<blob_set> Blob_mt_mapper_collection::blobSets(vector<unsigned int> superIds){
  vector<blob_set> b;
  for(unsigned int i=0; i < superIds.size(); ++i){
    if(blobs.count(superIds[i]))
      b.insert( b.end(), blobs[superIds[i]].begin(), blobs[superIds[i]].end() );
  }
  return(b);
}

vector<blob_set> Blob_mt_mapper_collection::blobSets(vector<unsigned int> superIds, QString parName){
  vector<blob_set> b;
  for(unsigned int i=0; i < superIds.size(); ++i){
    if(blobs.count(superIds[i]) && class_criteria.count(superIds[i])){
      ClassCriteria& criteria = class_criteria[superIds[i]];
      vector<blob_set>& bsets = blobs[superIds[i]];
      for(unsigned int j=0; j < bsets.size(); ++j){
	if(!criteria.assess(parName, bsets[j]))  // returns an error code.. if it doesn't pass, hence the !
	  b.push_back(bsets[j]);
      }
    }
  }
  return(b);
}

vector<blob_set> Blob_mt_mapper_collection::blobSets(vector<QString> parNames){
  vector<unsigned int> superIds;
  for(map<unsigned int, ClassCriteria>::iterator it=class_criteria.begin(); it != class_criteria.end(); ++it)
    superIds.push_back((*it).first);
  return(blobSets(superIds, parNames));
}
  
vector<blob_set> Blob_mt_mapper_collection::blobSets(vector<unsigned int> superIds, vector<QString> parNames){
  if(!superIds.size() && !parNames.size())
    return(blobSets());
  if(superIds.size() && !parNames.size())
    return( blobSets(superIds) );
  if(!superIds.size() && parNames.size())
    return( blobSets(parNames) );  // which will actually call this function again.. but what the hell
  // and here if both have a reasonable size.. 
  vector<blob_set> b;
  for(unsigned int i=0; i < superIds.size(); ++i){
    if(blobs.count(superIds[i]) && class_criteria.count(superIds[i])){
      ClassCriteria& criteria = class_criteria[superIds[i]];
      vector<blob_set>& bsets = blobs[superIds[i]];
      for(unsigned int j=0; j < bsets.size(); ++j){
	for(unsigned int k=0; k < parNames.size(); ++k){
	  if(criteria.assess(parNames[k], bsets[j]))
	    break;
	  b.push_back(bsets[j]);
	}
      }
    }
  }
  return(b);
}

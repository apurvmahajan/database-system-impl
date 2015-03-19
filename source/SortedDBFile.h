#ifndef SORTED_DBFILE_H
#define SORTED_DBFILE_H


#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Pipe.h"
#include "BigQ.h"
#include "Defs.h"
#include "GenericDBFile.h"



struct SortInfo { 
	OrderMaker *myOrder; 
	int runLength; 
}; 

typedef struct SortInfo SortInfo;

class SortedDBFile: public GenericDBFile{

private:

	Pipe *inPipe;
	Pipe *outPipe;
	BigQ *bq;
	File* file;
	SortInfo *sInfo;
	File file_obj;
	Page page_obj;
	off_t atPageNo;
	bool mode; // mode = 0 is write and mode = 1 is write
	char *file_path;




public:
	SortedDBFile (); 

	int Create (char *fpath, fType file_type, void *startup);
	int Open (char *fpath);
	int Close ();

	void Load (Schema &myschema, char *loadpath);

	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
	void merge();
	int binarySearch(Record& fetchme, OrderMaker& queryorder, Record& literal, OrderMaker& cnforder, ComparisonEngine& cmp);

	~SortedDBFile();
};
#endif

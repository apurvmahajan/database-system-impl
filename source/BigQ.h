#ifndef BIGQ_H
#define BIGQ_H
#include <pthread.h>
#include <iostream>
#include "algorithm"
#include "vector"
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include "exception"
#include "Defs.h"



using namespace std;

class BigQ {

public:


	BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);
	~BigQ ();
};


class RecordTracker {

public : 
	Record *rec;
	int pageNo;
	int runNo;

	RecordTracker();
	~RecordTracker();

};


struct ThreadData {


Pipe *inP;
Pipe *outP;
OrderMaker *sortOrder;
int runLength;


};

typedef struct ThreadData thread_p;





#endif

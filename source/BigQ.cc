#include "BigQ.h"
#include <unistd.h>


File tmpFile;
char *filePath = "temporary.bin";

std::vector<int> indexOfPages;
std::vector<RecordTracker *> recordSorter;
std::vector<Page *> pagesOfRuns;
OrderMaker *order;
int atRun = 0;

//************************************************************************************


RecordTracker::RecordTracker() {
	rec = new Record();
	pageNo = 0;
	runNo = 0;
}


//************************************************************************************

bool heapComparison(const RecordTracker *left, const RecordTracker *right)
{
  ComparisonEngine comp;

  Record *rec1 = left->rec;
  Record *rec2 = right->rec;

  if (comp.Compare(rec1, rec2, order) < 0) {
    return false;
  } else {
    return true;
  }
}

//************************************************************************************


bool recordComparison(const RecordTracker *left, const RecordTracker *right) 
{
  ComparisonEngine comp;

  Record *rec1 = left->rec;
  Record *rec2 = right->rec;

  if (comp.Compare(rec1, rec2, order) < 0) {
    return true;
  } else {
    return false;
  }

}


//************************************************************************************

void mergePages(thread_p *threadParams)
{


  RecordTracker *recCurrent;
  RecordTracker *recNext;
  Page* tempPage = NULL;
  Record *newRecord;

  recordSorter.clear();
  tmpFile.Open(1, filePath);

	int pageVectorSize = indexOfPages.size();
	int i = 0;
	int outgoingRun = 0;
    int outgoingPage = 0;
	int no = 0;
	int pass = 0;
	
	if(pageVectorSize == 1){
		
		tempPage = new Page();
		while(tmpFile.GetPage(tempPage,i)){
			
			newRecord = new Record();
			while(tempPage->GetFirst(newRecord)) {
				
				
				threadParams->outP->Insert(newRecord);
				newRecord = new Record();	
				
			}
			tempPage->EmptyItOut();
			i++;
		}
			
	
	}
 else
 {
	while(i < pageVectorSize){
		
	tempPage = new Page();
    tmpFile.GetPage(tempPage,indexOfPages[i]);
    pagesOfRuns.push_back(tempPage);


    newRecord = new Record();
    pagesOfRuns[i]->GetFirst(newRecord);
    

    recCurrent = new RecordTracker();
    recCurrent->rec = newRecord;
    recCurrent->runNo = i;
    recCurrent->pageNo = indexOfPages[i];

    recordSorter.push_back(recCurrent);
		
		
		i++;
	}
 
 
 
  while(!recordSorter.empty())
  {
	no++;

	std::make_heap(recordSorter.begin(),recordSorter.end(),heapComparison);
    

    recCurrent = new RecordTracker();
    recCurrent = recordSorter.front(); 
    outgoingRun = recCurrent->runNo;
    outgoingPage = recCurrent->pageNo;

    std::pop_heap(recordSorter.begin(),recordSorter.end());
    recordSorter.pop_back();


    threadParams->outP->Insert(recCurrent->rec);

    recNext = new RecordTracker();
    recNext->runNo = outgoingRun;
    recNext->pageNo = outgoingPage;


    if(pagesOfRuns[outgoingRun]->GetFirst(recNext->rec)) {
      recordSorter.push_back(recNext);
      
    }
    else {
		
		if((outgoingRun+1) == (indexOfPages.size()))
		{
			pass = 1;
		}

      if(outgoingPage+2 < tmpFile.GetLength()){ 

        if((outgoingPage+1)< indexOfPages[outgoingRun+1] || pass==1) {
          
          tempPage = new Page();
          tmpFile.GetPage(tempPage,outgoingPage+1);
          pagesOfRuns[outgoingRun] = tempPage;
        
          if(pagesOfRuns[outgoingRun]->GetFirst(recNext->rec)) {
            recNext->pageNo = outgoingPage+1;
            recordSorter.push_back(recNext);
          
          }
          
        }
      } 
      
      pass = 0;

    }
  }
}
  

  tmpFile.Close();
   threadParams->outP->ShutDown();

}


//**********************************************************************************************



void transferPagesToFile() 
{

	
	Page tempPage;
	Record *tempRec = NULL;


  int pageNo = 0;

  pageNo = tmpFile.GetLength();
  
  
  if(pageNo>0){
	  
	  pageNo = pageNo-1;
  }

  indexOfPages.push_back(pageNo);
  int i = 0;
  int size = recordSorter.size();
  

	while(i < size) {
	
	
	 tempRec = recordSorter[i]->rec;


    if (!tempPage.Append(tempRec)) {


      tmpFile.AddPage(&tempPage, pageNo);


      tempPage.EmptyItOut();
      pageNo++;
	  tempPage.Append(tempRec);
    }
	
	i++;
	
}

  tmpFile.AddPage(&tempPage, pageNo);
 // cout<<"reached end of transferPagesToFile"<<endl;
 
  

}




//************************************************************************************


void *externalSort(void *threadParams){
	
	
thread_p *td = (thread_p *) threadParams;

	int numOfPages = 0;
	bool pipeHasData = 1;
	
	Record fetchRecord;
	RecordTracker *tempRecord;
	Page tempPage;
	int no = 0;

//	cout<<"printing inside external sort of bigq"<<endl;



	tmpFile.Open(0, filePath);

	while (pipeHasData) {
		while (numOfPages < td->runLength) {

			if (td->inP->Remove(&fetchRecord)) {
				no++;

				tempRecord = new (std::nothrow) RecordTracker;
				tempRecord->rec = new Record;

				tempRecord->rec->Copy(&fetchRecord);
				tempRecord->runNo = atRun;


				recordSorter.push_back(tempRecord);




				if (0 == tempPage.Append(&fetchRecord)) {

					numOfPages++;
					tempPage.EmptyItOut();
					tempPage.Append(&fetchRecord);

				}
			} else {

				pipeHasData = 0;
				
				break;
			}
		}


		atRun++;
		
		std::sort(recordSorter.begin(), recordSorter.end(), recordComparison);

		transferPagesToFile();


		
		numOfPages = 0;
	
		
		recordSorter.clear();
	
	}
	 

	tmpFile.Close();

	mergePages(td);
}	


//************************************************************************************


BigQ :: BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {



thread_p *td = new (std::nothrow) thread_p;



	td->inP = &in;
	td->outP = &out;
	td->sortOrder = &sortorder;
	td->runLength = runlen;
	order = &sortorder;

//	cout<<"printing myorder in bigq"<<endl;
	sortorder.Print();
	

	pthread_t bThread;
	pthread_create(&bThread, NULL, externalSort, (void *) td);
	//pthread_join(bThread, NULL);
	//cout<<"reaching end of bigq"<<endl;

	//delete td;
	//out.ShutDown ();
}

//************************************************************************************

	

BigQ::~BigQ () {
}

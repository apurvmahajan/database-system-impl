#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "SortedDBFile.h"
#include "Defs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "Pipe.h"
#include "BigQ.h"
#include <fstream>
#include <cassert>
#include <cstdlib>
#include "Comparison.h"


//Schema *temp_schema = NULL;
int tot = 0;

using namespace std;




SortedDBFile::SortedDBFile() {
	
	inPipe = new Pipe(100);	
	outPipe = new Pipe(100);	
	bq = NULL;
	sInfo = NULL;
	atPageNo = 0;
	mode = 0;
	
}


int SortedDBFile::Create (char *f_path, fType f_type, void *startup) {
	int open_status = file_obj.Open(0,f_path);
	file_path = f_path;

	sInfo = (SortInfo *)startup;
	char *fileHeaderName = new char[100];
	sprintf(fileHeaderName, "%s.header", f_path);
	FILE *f = fopen(fileHeaderName, "w");
	if(!f) {
		cerr << "Open file header name error!"<<endl;
		return 0;
	}
	fprintf(f, "%d\n", f_type);
	fprintf(f, "%d\n", sInfo->runLength);
	fprintf(f, "%s", sInfo->myOrder->ToString().c_str());
	//cout<<"printing myorder by calling print function of OrderMaker inside create of SortedDBFile"<<endl;
	sInfo->myOrder->Print();
	//cout<<"printing myorder inside create of SortedDBFile"<<endl<<sInfo->myOrder->ToString().c_str()<<endl;
	fclose(f);
	
	return open_status;


	
}

void SortedDBFile::Load (Schema &f_schema, char *loadpath) {

	//temp_schema = &f_schema;
	//cout<<"reached inside load of SortedDBFile" << endl;
	
	FILE *TableFile = fopen(loadpath, "r");
	Record temp_rec;
	Page temp_page;
	int page_no = 0;
	// OrderMaker ss = *(sInfo->myOrder);
	// int le = sInfo->runLength;
	Record temp;
	//cout<<"printing myorder inside load of SortedDBFile"<<endl;
	sInfo->myOrder->Print();
	
	if(bq==NULL) bq = new BigQ(*inPipe,*outPipe,*(sInfo->myOrder),sInfo->runLength);


	
	while(temp_rec.SuckNextRecord(&f_schema, TableFile) == 1) {
		
		inPipe->Insert (&temp_rec);
		
	}

	//cout<<"all the data has been pushed to inpipe for BigQ"<<endl;
	inPipe->ShutDown ();
	fclose(TableFile);
	
	
	while(outPipe->Remove(&temp_rec)) {
		//temp_rec.Print(&f_schema);
		
	if(temp_page.Append(&temp_rec) == 0) {
			
			file_obj.AddPage(&temp_page,page_no);
			page_no++;
			temp_page.EmptyItOut();
			temp_page.Append(&temp_rec);			
			
			
			
		}
	}
	
		file_obj.AddPage(&temp_page,page_no);
		
	//cout<<"end of load in SortedDBFile"<<endl;
	
	
	
}

int SortedDBFile::Open (char *f_path) {


	
	int open_status = file_obj.Open(1, f_path);
	file_path = f_path;
	sInfo = new SortInfo();
	int len;
	OrderMaker so;
	//cout << "reached in open of SortedDBFile"<<endl;
	char *fileHeaderName = new char[100];
	sprintf(fileHeaderName, "%s.header", f_path);
	FILE *f = fopen(fileHeaderName,"r");
	int runlen;
	OrderMaker *o = new OrderMaker;
	fscanf(f, "%d", &runlen); //since the first line is the filetype
	fscanf(f, "%d", &runlen);
	sInfo->runLength = runlen;
	int attNum;
	fscanf(f, "%d", &attNum);
	o->numAtts = attNum;
	for(int i = 0;i<attNum;i++) {
		int att;
		int type;
		if(feof(f)) {
			cerr << "Retrieve ordermaker from file error"<<endl;
			return 0;
		}
		fscanf(f, "%d %d", &att, &type);
		o->whichAtts[i] = att;
		if(0 == type) {
			o->whichTypes[i] = Int;
		} else if(1==type) {
			o->whichTypes[i] = Double;
		} else
			o->whichTypes[i] = String;
	}
	sInfo->myOrder = o;
	

	//cout<<"printing order inside open of SortedDBFile"<<endl;
	sInfo->myOrder->Print();
	return open_status;
	
	
}

void SortedDBFile::MoveFirst () {
	
	lseek (file_obj.GetFilDes(), PAGE_SIZE, SEEK_SET);
	atPageNo = 0;
	page_obj.EmptyItOut();
	

}

int SortedDBFile::Close () {

	if(mode == 1) merge();
	
	int length = file_obj.Close();

	
}

void SortedDBFile::Add (Record &rec) {
	
	
	if(mode == 0) {
		mode = 1;
		delete inPipe;
		delete outPipe;
		delete bq;
		inPipe = new Pipe(100);	
		outPipe = new Pipe(100);
		bq = new BigQ(*inPipe,*outPipe,*(sInfo->myOrder),sInfo->runLength);
		
	}

		tot++;
		
		inPipe->Insert (&rec);

	
}


void SortedDBFile::merge(){


	//cout<<"total records in pipe ="<<tot<<endl;
	Record rec_file;
	Record rec_pipe;
	Record final_rec;
	Page tempPage;
	ComparisonEngine comp;
	File temp_file;
	int appendStatus;
	int count = 0, count1 = 0, count2 = 0;
	char *tempFileName = new char[100];
	sprintf(tempFileName, "%s.temp", file_path);
	
	off_t page_no = 0;

	temp_file.Open(0,tempFileName);

	mode = 0;

	inPipe->ShutDown();
	this->MoveFirst();

	//cout<<"value returned from getnext"<<endl;
	if(this->GetNext(rec_file)==0){

		goto end;

	}
	outPipe->Remove(&rec_pipe);
	//rec_pipe.Print(temp_schema);

	// cout<<"printing sort order inside merge function of SortedDBFile"<<endl;
	// sInfo->myOrder->Print();


	while(1){
			//count++;
		int comparisonRes = comp.Compare(&rec_file, &rec_pipe, sInfo->myOrder);
		// cout<<"comparison result = "<<comparisonRes<<endl;
		// cout<<"***************************"<<endl;
	if (comparisonRes <= 0) {
    	
    	count1++;
    	final_rec.Copy(&rec_file);
		if(this->GetNext(rec_file) == 0){
			break;
		}
  } else {
   		
   		//count2++;
   		final_rec.Copy(&rec_pipe);
   		if(outPipe->Remove(&rec_pipe) == 0){

   			break;
   		}
}

		//final_rec.Print(temp_schema);


		appendStatus = tempPage.Append(&final_rec);
	
   		if(appendStatus == 0) {
			
			temp_file.AddPage(&tempPage,page_no);
			page_no++;
			tempPage.EmptyItOut();
			tempPage.Append(&final_rec);			
			
			
			
		}

	//	cout<<"reached here"<<endl;
 	
}

//count++;

// cout<<"total records = "  <<count<<endl;
// cout<<"records from file = "<<count1<<endl;
// cout<<"records from pipe ="<<count2<<endl;


		if(tempPage.Append(&final_rec) == 0) {
			
			temp_file.AddPage(&tempPage,page_no);
			page_no++;
			tempPage.EmptyItOut();
			tempPage.Append(&final_rec);			
			
			
			
		}



	while(this->GetNext(rec_file)){

		if(tempPage.Append(&rec_file) == 0) {
			
			temp_file.AddPage(&tempPage,page_no);
			page_no++;
			tempPage.EmptyItOut();
			tempPage.Append(&rec_file);			
			
			
			
		}

	}

end:
	while(outPipe->Remove(&rec_pipe)) {
		
		if(tempPage.Append(&rec_pipe) == 0) {
			
			temp_file.AddPage(&tempPage,page_no);
			page_no++;
			tempPage.EmptyItOut();
			tempPage.Append(&rec_pipe);			
			
			
			
		}
	}




	temp_file.AddPage(&tempPage,page_no);
	file_obj.Close();
	temp_file.Close();
	remove(file_path);
	rename(tempFileName, file_path);
	file_obj.Open(1, file_path);
	MoveFirst();

	// cout<<"reached the end of merge"<<endl;


}

int SortedDBFile::GetNext (Record &fetchme) {


	if(mode == 1) merge();
	

	if(page_obj.GetNoOfRecords() == 0) {
	
		
		if(file_obj.GetPage(&page_obj, atPageNo) == 0)
		{
				return 0;

		}
				
				atPageNo++;
		
	}

	

	if(page_obj.GetFirst(&fetchme) == 0){
		
		
		//cout<<"inside second IF of getnext"<<endl;
		
		if((atPageNo+1) < file_obj.GetLength()) {
			
			file_obj.GetPage(&page_obj, atPageNo);
			page_obj.GetFirst(&fetchme);
			
		}
		
		else return 0;
		
	}

	//fetchme.Print(temp_schema);
	
	//cout<<"reached end of GetNext of SortedDBFile"<<endl;
	return 1;
	

}

int SortedDBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	
	cout << "##################### SortedDBFile :: GetNext ##########################" << endl;
	OrderMaker queryorder, cnforder;
	OrderMaker::queryOrderMaker(*(sInfo->myOrder), cnf, queryorder, cnforder);
	ComparisonEngine compObj;
	if(!binarySearch(fetchme, queryorder, literal, cnforder, compObj)) return 0;
	do {
		if(compObj.Compare(&fetchme, &queryorder, &literal, &cnforder)) return 0;
		if(compObj.Compare(&fetchme, &literal, &cnf)) return 1;
	} while(GetNext(fetchme));

	return 0;
	

	//while(GetNext(fetchme)){
	//	if(comp.Compare (&fetchme, &literal, &cnf)==1)
	//		return 1;
	//}
	//return 0;
}


int SortedDBFile::binarySearch(Record& fetchme, OrderMaker& queryorder, Record& literal, OrderMaker& cnforder, ComparisonEngine& compObj) {

	cout << "##################### SortedDBFile :: binarySearch #####################" << endl;;
	if (!GetNext(fetchme)) return 0;
	int result = compObj.Compare(&fetchme, &queryorder, &literal, &cnforder);
	if (result > 0) return 0;
	else if (result == 0) return 1;
 
	off_t low=atPageNo, high=file_obj.GetLength()-2, mid=(low+high)/2;
	for (; low<mid; mid=(low+high)/2) {
		file_obj.GetPage(&page_obj, mid);
		if(!GetNext(fetchme)) cout<<"ERROR: empty page found"<<endl;
		result = compObj.Compare(&fetchme, &queryorder, &literal, &cnforder);
		if (result<0) low = mid;
		else if (result>0) high = mid-1;
		else high = mid;   
	}

	file_obj.GetPage(&page_obj, low);
	do {    
		if (!GetNext(fetchme)) return 0;
		result = compObj.Compare(&fetchme, &queryorder, &literal, &cnforder);
	} while (result<0);
         
	return result==0;
}



SortedDBFile::~SortedDBFile(){

}

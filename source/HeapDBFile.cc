#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "HeapDBFile.h"
#include "Defs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;



HeapDBFile::HeapDBFile () {
	atPageNo = 0;

}

int HeapDBFile::Create (char *f_path, fType f_type, void *startup) {
	
int open_status = file_obj.Open(0, f_path);

	char *fileHeaderName = new char[100];
	sprintf(fileHeaderName, "%s.header", f_path);
	FILE *f = fopen(fileHeaderName, "w");
	if(!f) {
		cerr << "Open file header name error!"<<endl;
		return 0;
	}
	fprintf(f, "%d\n", f_type);
	fclose(f);

return open_status;
	
}

void HeapDBFile::Load (Schema &f_schema, char *loadpath) {
	


	FILE *TableFile = fopen(loadpath, "r");
	Record temp_rec;
	Page temp_page;
	int counter = 0;
	int page_no = 0;
	
	while(temp_rec.SuckNextRecord(&f_schema, TableFile) == 1) {
		
		
		counter++;
		
		if(counter % 10000 == 0) {
			
			cerr<<counter<<"\n";
			
				
		}
		temp_rec.Print(&f_schema);
		
		if(temp_page.Append(&temp_rec) == 0) {
			
			file_obj.AddPage(&temp_page,page_no);
			page_no++;
			temp_page.EmptyItOut();
			temp_page.Append(&temp_rec);			
			
			
			
		}
		
		
	}
	
		
		file_obj.AddPage(&temp_page,page_no);
		
	

	//cout<<" no of pages = " << page_no<<"\n";
	
}

int HeapDBFile::Open (char *f_path) {
	
	int open_status = file_obj.Open(1, f_path);
	page_obj.EmptyItOut();
	lastPageAdded = 0;
	atPageNo = 0;
	isPageDirty = false;
	return open_status;
	
}

void HeapDBFile::MoveFirst () {
	
	lseek (file_obj.GetFilDes(), PAGE_SIZE, SEEK_SET);
	atPageNo = 0;
	page_obj.EmptyItOut();
	
}

int HeapDBFile::Close () {

	if(isPageDirty){
		file_obj.AddPage(&page_obj,++lastPageAdded);

	}
	
	int length = file_obj.Close();
	

	//cout<<"current length of file in pages ="<<length<<"\n";

	
}

void HeapDBFile::Add (Record &rec) {
	
	
	

	if(!isPageDirty){
		off_t file_length = file_obj.GetLength();
		page_obj.EmptyItOut();

		if(file_length>1){
			file_obj.GetPage(&page_obj, file_length-2);		

		}

		lastPageAdded--;

	if(page_obj.Append(&rec)==0) {
		
		file_obj.AddPage(&page_obj,file_length-2);
		page_obj.EmptyItOut();
		page_obj.Append(&rec);
		lastPageAdded = file_length-2;

		
	}

	isPageDirty = true;


	}

	else {

	if(page_obj.Append(&rec)==0) {

		file_obj.AddPage(&page_obj,++lastPageAdded);
		page_obj.EmptyItOut();
		page_obj.Append(&rec);

		
	} 
	
}
	
	
	
	
}

int HeapDBFile::GetNext (Record &fetchme) {
	

	if(isPageDirty) {

		file_obj.AddPage(&page_obj,++lastPageAdded);
		isPageDirty = false;
		page_obj.EmptyItOut();
	}
	

	if(page_obj.GetNoOfRecords() == 0) {
	
	
		if(file_obj.GetPage(&page_obj, atPageNo) == 0)
				return 0;
				
				atPageNo++;
		
	}

	

	if(page_obj.GetFirst(&fetchme) == 0){
		
		
		
		
		if((atPageNo+1) < file_obj.GetLength()) {
			
			file_obj.GetPage(&page_obj, atPageNo);
			page_obj.GetFirst(&fetchme);
			
		}
		
		else return 0;
		
	}
	
	
	
	
	return 1;
	
}

int HeapDBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	
	
		ComparisonEngine comp;
	
		while(GetNext(fetchme)){
		if(comp.Compare (&fetchme, &literal, &cnf)==1)
			return 1;
	}

  return 0;
	
	
	
}

HeapDBFile::~HeapDBFile(){

}
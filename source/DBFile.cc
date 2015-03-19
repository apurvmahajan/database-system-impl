#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "GenericDBFile.h"
#include "HeapDBFile.h"
#include "SortedDBFile.h"

using namespace std;





DBFile::DBFile () {
	gdb = NULL;
}

int DBFile::Create (char *f_path, fType f_type, void *startup) {
if(f_type==heap){

		this->gdb= new HeapDBFile();

	}
	else if(f_type==sorted){

		this->gdb= new SortedDBFile();	

	}

	if(gdb!=NULL){

		return gdb->Create(f_path,f_type,startup);		

	}	
	

	
}

void DBFile::Load (Schema &f_schema, char *loadpath) {
	
	//cout<<"reached in load of dbfile" << endl;
	gdb->Load(f_schema,loadpath);
	
	
}

int DBFile::Open (char *f_path) {
	
	char meta_path[50];

	sprintf(meta_path,"%s.header",f_path);

	FILE *metadata =  fopen(meta_path,"r");;
	//cout << "opened";

	fType type;

	fscanf(metadata,"%d",&type);
	fclose(metadata);
//cout << type << endl;

	if(type == heap){
	
		this->gdb = new HeapDBFile();
		

	}
	else if(type == sorted){
		//cout<<"inside open of DBFile"<<endl;
		this-> gdb = new SortedDBFile();
		

	}


	return this->gdb->Open(f_path);

	
}

void DBFile::MoveFirst () {
	
	gdb->MoveFirst();

}

int DBFile::Close () {
	
 return gdb->Close();

	
}

void DBFile::Add (Record &rec) {
	
	
	gdb->Add(rec);
	
		
	
}

int DBFile::GetNext (Record &fetchme) {
	
	return gdb->GetNext(fetchme);
	

}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	

	return gdb->GetNext(fetchme,cnf,literal);
	
	
}

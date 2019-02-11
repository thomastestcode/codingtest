#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <iterator>
#include <vector>
#include <thread>             
#include <mutex>              
#include <condition_variable>
#include <unistd.h>
#define NUM_THREAD 4

using namespace std;
ifstream inputFile;
ofstream outputFile;
string itemList[10000];
int lineCount = 0;
mutex mtxRead, mtxWrite, mtxOut;
condition_variable cv;
int iPreIdxRead = -1, iPreIdxWrite = 0;
int iReadReady = 0;

string SelectionSort(vector<int> vect)
{
	stringstream ss, ss1;
	string s;
	int n = vect.size();
	for (int j = 0; j < n - 1; ++j) {
		int min = j;
		for (int i = j + 1; i < n; ++i) {
			if (vect.at(min) > vect.at(i)) {
				min = i;
			}
		}
		if (min != j)
			swap(vect.at(j), vect.at(min));
	}
	copy(vect.begin(), vect.end(), ostream_iterator<int>(ss, ","));
	s = ss.str();
	s = s.substr(0, s.length() - 1);
	return(s);
}

string BubbleSort(vector<int> vect)
{
	stringstream ss;
	string s;
	size_t n;
	n = vect.size();
	bool swapp = true;
	while (swapp) {
		swapp = false;
		for (size_t i = 0; i < n - 1; i++) {
			if (vect[i] > vect[i + 1]) {
				vect[i] += vect[i + 1];
				vect[i + 1] = vect[i] - vect[i + 1];
				vect[i] -= vect[i + 1];
				swapp = true;
			}
		}
	}
	// Insert ',' between numeric characters within vector
	copy(vect.begin(), vect.end(), ostream_iterator<int>(ss, ","));
	s = ss.str();
	s = s.substr(0, s.length() - 1);
	return(s);
}

void WriteFile(string itemWrt, int iIdxWrt)
{
	unique_lock<mutex> lckWrite(mtxWrite);
	while(iIdxWrt != iPreIdxWrite) cv.wait(lckWrite);
  // Setting up mutext to protected shared output file resource
	{
		  mtxOut.lock();
		  outputFile << itemWrt << endl;
		  mtxOut.unlock();
			cv.notify_all();
	}
}

//Consumer
void SortProcess(char* sortMet)
{
	char a[2];
	int n, i, j, iThreadIdxRead = 0;
	int iThreadIdxWrite;
	vector<int> A;
	string s = "";
	
	for(;;)
	{
		// Setting up mutext to protected shared Item List buffer resource
		if(mtxRead.try_lock())
		{
			if((iPreIdxRead + 1) < lineCount)
			{
				iThreadIdxRead = iPreIdxRead + 1;
				iThreadIdxWrite = iThreadIdxRead;
				iPreIdxRead ++;
			}
			else {
				mtxRead.unlock();
		  	cv.notify_all();
				break;
			} 
			iReadReady = 0;
		 	//Create lock key
		  n = itemList[iThreadIdxRead].length();
			for (i = 0; i < n; i++)
		  {
		  	a[0] = itemList[iThreadIdxRead][i];
				a[1] = '\0';
		  	// Sleep for a second when hit a blank space
				if (a[0] == ' ')
		  	{
		  		mtxRead.unlock();
		  		cv.notify_all();
					iReadReady = 1;
		  		this_thread::sleep_for(chrono::seconds(1));
		  	  unique_lock<mutex> lckRead(mtxRead);
				  mtxRead.try_lock();
				  cv.wait(lckRead, [] {return iReadReady == 1; });
		  	}
		  	else
		  	{
					// Split each character of the item into integer vector
		  		A.push_back(atoi(&a[0]));
		  	}
		  } 
		  mtxRead.unlock();
			cv.notify_all();
			iReadReady =1;
		} 
		else { 
		    unique_lock<mutex> lckRead(mtxRead);
				cv.wait(lckRead, [] {return iReadReady == 1; });
				iReadReady = 0;
				continue;
		}
  	if (strcmp(sortMet, "selection") == 0) s = SelectionSort(A);
		else if (strcmp(sortMet, "bubble") == 0) s= BubbleSort(A);
    A.clear();
		WriteFile(s,iThreadIdxWrite);
	  iPreIdxWrite++;
	} // End For 
} // End SortProcess

int ReadFile(char* cInput, char* cOutput)
{
	try {
     inputFile.open(cInput);
	   outputFile.open(cOutput);
	   while (getline(inputFile, itemList[lineCount]))
	   {
			//THe amount of characters in each item will nerver excedd 100 chars
		  if (itemList[lineCount].length() > 100)
		  {
			cout << "The Amount in this Item exceed 100 chars!" << endl;
			return(-1);
		  }
		  lineCount++;
	   }
	   // The amount of lines in the input file will nerver execeed 10000 lines
		 if ((lineCount -1) > 10000)
	   {
		 cout << "The Amount in line exceed 10000 lines!" << endl;
		 return(-1);
	   }
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
		return(-1);
	}
	return(0);
}

int main(int argc, char *argv[])
{
	//Producer
	thread workerThread[4];
	string item = "", itemOut = "";
	string line;
	//
	//Check all passed arguments
	//
	if (argc < 4)
	{
		cout << "Executed command or argurment is invalid" << endl;
		cout << "Syntax: ./ProducerConsumer inputfile OutputFile AlgorithmToSort (eg selection or bubble)" << endl;
	}
 	if ((strcmp(argv[3], "selection") != 0) && (strcmp(argv[3], "bubble") != 0))
	{
		cout << "Sort Method is invalid !!!" << endl;
		return (0);
	}
	//
	//  Read input file in Main process
	//
	if(ReadFile (argv[1], argv[2]) == -1)
	{
	  inputFile.close();
	  outputFile.close();
		return(0);
	}
	//
	// Create 4 Worker Thread to process the sort
	//
  for(int i=0; i<NUM_THREAD; i++)
	{
		workerThread[i] = thread(SortProcess, ref(argv[3]));
	}
	for(int i=0; i<NUM_THREAD; i++)
	{
	  workerThread[i].join();
	}
  inputFile.close();
	outputFile.close();
	return(0);
}
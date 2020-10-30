#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <pthread.h>
//for mingw under windows
//#include "mingw.thread.h"
//#include "mingw.mutex.h"
using namespace std;

fstream finput, foutput;
int **arr_input;
int lines_input = 0;
int num_thread;
vector<thread> threads;
vector<string> json_strings;

void read_csv()
{
    //get total lines in input file
    finput.open("input.csv", ios::in);
    string str;
    while(getline(finput, str)) {
        lines_input++;
    }
    arr_input = new int*[lines_input];
    for(int i=0; i<lines_input; ++i) arr_input[i] = new int[20];
    finput.close();

    //read input
    finput.open("input.csv", ios::in);
    for(int i=0; i<lines_input; ++i){
        int cnt = 0;
        char *substr;
        const char delimiters[] = "|\n";
        if(getline(finput, str)){
            char *cstr = (char*)str.c_str();
            substr = strtok(cstr, delimiters);
            while(substr){
                arr_input[i][cnt++] = atoi(substr);
                substr = strtok(NULL, delimiters);
            }
        }
        else break;
    }
    finput.close();
}

void thread_make(int size, int offset)
{
    printf("> Thread[%d] started.\n", offset/size);
    double s, e, ss, ee, t=0;
    s = clock();
    int c = 0;
    int **arr_p = &arr_input[offset];
    /*
    int **arr_p = new int*[size];
    for(int i=0; i<size; ++i) arr_p[i] = new int[20];
    for(int i=0; i<size; ++i) arr_p[i] = arr_input[offset+i];
    */
    for(int i=0; i<size; ++i, ++c){
        ss = clock();
        string str = "    {\n";
        for(int j=0; j<19; ++j){
            str += ("        \"col_" + to_string(j+1) + "\":" + to_string(arr_p[i][j]) + ",\n");
        }
        str += ("        \"col_" + to_string(20) + "\":" + to_string(arr_p[i][19]) + "\n");
        if(i != lines_input-1) str += "    },\n";
        else str += "    }\n";
        json_strings[offset+i] = str;
        ee = clock();
        //printf("%f\n", (ee-ss)/CLOCKS_PER_SEC);
        t+=((ee-ss)/CLOCKS_PER_SEC);
    }
    e = clock();
    t = t/c;
    printf("> Thread[%d] ended with total time %fsec. (avg %fsec/run, with %druns)\n", offset/size, (e - s) / CLOCKS_PER_SEC, t, c);
}

void make_json()
{
    for(int i=0; i<lines_input; ++i){
        json_strings.push_back("");
    }

    //generate output via multi-threads
    int size = lines_input/num_thread;
    for(int i=0; i<num_thread; ++i){
        threads.push_back(thread(thread_make, size, i*size));
    }
    for(int i=0; i<threads.size();++i){
        threads[i].join();
    }
    threads.clear();  
    if(lines_input%num_thread != 0){
        //something left
        thread tmp_thread(thread_make, lines_input%num_thread, num_thread*size);
        tmp_thread.join();
    }
}

void write_json()
{
    //write ouput file
    foutput.open("output.json", ios::out | ios::trunc);
    foutput << "[\n";
    for(int i=0; i<json_strings.size(); ++i){
        foutput << json_strings[i];
    }
    foutput << "]";
    foutput.close();
}

int main(int argc, char *argv[])
{
    double START, END, MID;
    START = clock();
    //default number of threads = 4
    num_thread = (argc > 1) ? atoi(argv[1]) : 4;

    //read input.csv
    cout << "> Start reading input.csv.\n";
    read_csv();
    MID = clock();
    cout << "> Finish reading with " << (MID - START) / CLOCKS_PER_SEC << "sec used.\n";

    //generate string for json
    cout << "> Start making output strings.\n";
    make_json();
    END = clock();
    cout << "> Finish making with " << (END - MID) / CLOCKS_PER_SEC << "sec used.\n";
    
    //write output.json
    cout << "> Start writing output.json.\n";
    write_json();
    MID = clock();
    cout << "> Finish writing with " << (MID - END) / CLOCKS_PER_SEC << "sec used.\n";

    //free memory
    for(int i=0; i<lines_input; ++i) delete [] arr_input[i];
    delete [] arr_input;

    END = clock();
    cout << "> Ended with total time: " << (END - START) / CLOCKS_PER_SEC << "sec.\n";
    return 0;
}
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

ifstream finput;
ofstream foutput;
fstream ftmp;

vector<int> arr_input;
vector<int> arr_output;
vector<vector<int> >arr_runs;
vector<fstream> runs;

int page_size, page_num, last_page_size, run_size, out_buffer_size;

void init(string str_input)
{
    page_num = 0;
    last_page_size = page_size;

    cout << "========================================\n";
    cout << " Initialize with:\n";
    cout << " page_size > " << page_size << "\n";
    cout << " run_size > " << run_size << "\n";
    cout << " out_buffer_size > " << out_buffer_size << "\n";
    cout << "========================================\n";

    finput.open(str_input, ios::in);
    foutput.open("output.txt", ios::out | ios::trunc);
    if(!finput || !foutput) cout<<"[ERR]File can not open.\n";

    //make sure ./tmp/ is exist and empty (only under linux system)
    DIR *dir;   
    if ((dir=opendir("./tmp/")) == NULL)   
    {   
        system("mkdir tmp/");
    }  
    else
    {
        system("rm -r tmp/");
        system("mkdir tmp/");
    }
}

void free()
{
    finput.close();
    foutput.close();
    runs.clear();
    system("rm -r tmp/");
}

//functions for 1st stage(divide)
void write_tmp(int index)
{
    //write the divided (and sorted) data in binary file
    ftmp.open("./tmp/page_" + to_string(index), ios::out | ios::binary | ios::trunc);
    if(!ftmp) cout << "[ERR]Something goes wrong.(@write_tmp())\n";
    for(int i=0; i<arr_input.size(); ++i){
        int a = arr_input[i];
        ftmp.write(reinterpret_cast<const char *>(&a), sizeof(int));
    }
    ftmp.close();
}

void divide()
{
    //divide data (in input.txt) in to several binary file 
    int cline = 0;
    string str_tmp;
    while (getline(finput, str_tmp))
    {
        arr_input.push_back(stoi(str_tmp));
        if(cline++ == page_size-1){
            cline = 0;
            sort(arr_input.begin(), arr_input.end());
            write_tmp(page_num++);
            arr_input.clear();
        }
    }
    if(!arr_input.empty()){
        last_page_size = arr_input.size();
        sort(arr_input.begin(), arr_input.end());
        write_tmp(page_num++);
    }
}


//functions for 2nd stage(merge)
vector<int> read_tmp(int index, int offset)
{
    //read from binary, start from offset^th for in_page_size number 
    int cline = 0;
    int a[2];
    vector<int> v_tmp;
    offset *= 4; //sizeof(int) = 4 bytes
    runs[index].seekg(offset, ios::beg); //shift
    while(runs[index].read((char*)(&a), sizeof(int))){
        //cout << "get " << a[0] << endl;
        if(cline++ == run_size){
            //full
            break;
        }
        v_tmp.push_back(a[0]);
    }
    return v_tmp;
}

void merge()
{
    int k[page_num];
    bool b = true;
    for(int i=0; i<page_num; ++i){
        runs.push_back(fstream("./tmp/page_" + to_string(i), ios::in | ios::binary));
        if(!runs[i]) cout<<"[ERR]Something goes wrong.(@merge())\n";
        arr_runs.push_back(read_tmp(i, 0));
        k[i] = 0;
    }
    
    /*
    for(int i=0;i<page_num;++i){
        for(int j=0; j<arr_runs[i].size();++j){
            cout << arr_runs[i][j] <<", ";
        }
        cout << arr_runs[i].size() <<endl;
    }
    */
    while(b){
        int min = INT32_MAX;
        int index_min = -1;
        for(int i=0; i<page_num; ++i){
            //cout << i << "-" << k[i] << ": ";
            if(i == page_num-1){
                if(k[i] < last_page_size){
                    if(arr_runs[i][k[i]%run_size] <= min){
                        min = arr_runs[i][k[i]%run_size];
                        index_min = i;
                    }
                }
            }
            else if(k[i] < /*(i == page_num-1) ? last_page_size : */page_size){
                //cout << arr_runs[i][k[i]%run_size];
                if(arr_runs[i][k[i]%run_size] <= min){
                    min = arr_runs[i][k[i]%run_size];
                    index_min = i;
                }
            }
            //cout << endl;
        }
        if(index_min != -1){
            //min found
            //cout << min << " @ " << index_min << endl;
            arr_output.push_back(min);
            if(k[index_min]%run_size == run_size-1){
                arr_runs[index_min] = read_tmp(index_min, ++k[index_min]);
                //cout << "read " << index_min << " from " << k[index_min] << endl;
            }
            else{
                k[index_min]++;
            }
            if(arr_output.size() == out_buffer_size){
                //flush
                for(int j=0; j<arr_output.size(); ++j){
                    foutput << arr_output[j] << "\n";
                }
                arr_output.clear();
            }
        }
        else{
            //cout << "min not found\n";
            break;
        }
        b = false;
        for(int i=0; i<page_num; ++i){
            //cout << "k[" << i << "] = " << k[i] <<endl;
            if(i == page_num-1){
                if(k[i] < last_page_size){
                    b=true;
                } 
            }
            else if(k[i] < page_size) {
                b=true;
            }
        }
   }
   //end flush
    for(int j=0; j<arr_output.size(); ++j){
        foutput << arr_output[j] << "\n";
    }
    arr_output.clear();
}


int main(int argc, char*argv[])
{
    double START,END,INTER;
    START = clock();
    int input_size, input_scale;
    string str_input = "input.txt";
    if(argc > 3){
        //self define
        page_size = atoi(argv[1]);
        run_size = atoi(argv[2]);
        out_buffer_size = atoi(argv[3]); 
    }
    else if(argc > 2){
        input_scale = atoi(argv[1]);
        input_size = atoi(argv[2]);
        switch(input_scale){
            case 1:
                //Kb
                page_size = ((input_size+1)/2) * 100;
                run_size = 100;
                out_buffer_size = (input_size+1)*100;
                break;
            case 2:
                //Mb
                page_size = ((input_size+1)/2) * 100000;
                run_size = 100000;
                out_buffer_size = (input_size+1)*100000;
                break;
            case 3:
                //Gb
                page_size = ((input_size+1)/2) * 100000000;
                run_size = 100000000;
                out_buffer_size = (input_size+1)*100000000;
                break;
            default:
                cout << "Input file is too big!\n";
                exit(0);       
        }
    }
    else if(argc > 1){
        //alternertive input
        str_input = argv[1];
        page_size = 10000000;
        run_size = 2000000;
        out_buffer_size = 5000000;
    }
    else{
        //default
        page_size = 10000000;
        run_size = 2000000;
        out_buffer_size = 5000000;
    }
    init(str_input);
    cout << "> Start dividing\n";
    divide();
    INTER = clock();
    cout << "> Succeeded with time usage: " << (INTER - START) / CLOCKS_PER_SEC << " sec\n";
    cout << "========================================\n";
    
    cout << "> Start merging( " << page_num << " way )\n";
    //cout << last_page_size << endl;
    cout << "> Estimate ram usage: " << (run_size/1024 * page_num + out_buffer_size/1024) * 4 << " Kbytes\n";
    merge();
    END = clock();
    cout << "> Succeeded with time usage: " << (END - INTER) / CLOCKS_PER_SEC << " sec\n";
    cout << "========================================\n";
    
    free();
    END = clock();
    cout << "> Ended with total time: " << (END - START) / CLOCKS_PER_SEC << " sec\n";
    return 0;
}

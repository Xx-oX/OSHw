#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
using namespace std;

fstream file;
//random engine and distribution setup, <random>
random_device rd;
default_random_engine gen = default_random_engine(rd());
uniform_int_distribution<int> dis(-2147483648, 2147483647);

 /*******************
      KiB -> 1024
      MiB -> 1048576
      GiB -> 1073741824
      int -> 4
    *******************/

void gen_Gb(int x, int mode)
{ 
    mode = pow(1024, mode);
    for(int i=0; i<x; ++i)
    {
        for(int j=0; j<mode/10/*roughly 10bytes a num*/; ++j)
        {
            file<<dis(gen)<<"\n";
        }
        /*for accurate generating, write in binary form (not UTF-8 readable)
        for(int j=0; j<mode; ++j)
        {
            int a = dis(gen);
            file.write(reinterpret_cast<const char *>(&a), sizeof(int));
        }
        */
    }
}

int main(int argc, char *argv[])
{
    double START, END;
    START = clock();
    file.open("input.txt", ios::out | ios::binary |ios::trunc);
    //defult 1Kb
    int m = (argc > 1) ? atoi(argv[1]) : 1;
    int x = (argc > 2) ? atoi(argv[2]) : 1;
    string str;
    switch(m){
        case 1: str = "Kb"; break;
        case 2: str = "Mb"; break;
        case 3: str = "Gb"; break;
        default: str = "Overflow"; break;
    }
    if(m > 3){
        cout << "========== Argument is too big! ==========\n";

    }
    else{
        cout << "> Start generating " << x << str << "(roughly) of random number\n";
        gen_Gb(x, m);
    }
    file.close();
    END = clock();
    cout << "> Ended with total time: " << (END - START) / CLOCKS_PER_SEC << "sec\n";
    return 0;
}
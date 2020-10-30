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

void generate(int x)
{ 
    for(int i=0; i<x; ++i){
        for(int j=0; j<19; ++j){
            file << dis(gen) << "|";
        }
        file << dis(gen) << "\n";
    }
}

int main(int argc, char *argv[])
{
    double START, END;
    START = clock();
    file.open("input.csv", ios::out | ios::binary |ios::trunc);
    int x = (argc > 1) ? atoi(argv[1]) : 4888888; //roughly 1Gb
    cout << "> Start generating!!\n";
    generate(x);
    file.close();
    END = clock();
    cout << "> Ended with total time: " << (END - START) / CLOCKS_PER_SEC << "sec.\n";
    return 0;
}
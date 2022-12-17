#include <iostream>
#include <fstream>
using namespace std;

int main()
{
    const char *path="./data/test.txt";
    ofstream outfile(path);
    outfile << "Fuck";
    outfile.close();
}

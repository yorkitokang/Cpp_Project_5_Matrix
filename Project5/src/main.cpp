#include "matrix.hpp"
//#include "matrixImproved.hpp"
using namespace std;
int main()
{
    //Basic test
    Mat<int> matI1(3,3,1,'z');
    Mat<int> matI2(3,3,1,'d');
    Mat<int> matI4(0,0,1,'d');
    Mat<int> matI5(3,3,2,'z');
    Mat<int> matI6 = matI1;
    matI1.print();//should output zero matrix
    matI2.print();
    matI4.print();
    matI5.print();
    Mat<float> matF1(3,3,1);
    matI1.setValue(1,1,1,1);

    Mat<double> matD1(10,10,2,'d');
    Mat<double> matD2(10,10,2,'d');
    Mat<double> matD3 = matD1 * matD2;
    Mat<double> matD4(matD1);
    matD4.setValue(0,0,0,2);
    matD1.print();
    matD2.print();
    matD3.print();
    matD4.print();

    // TODO:Specialize test

    // TODO:Region of Interest test
}
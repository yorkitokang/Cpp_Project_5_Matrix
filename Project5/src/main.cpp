#include "matrix.hpp"
#define TEST(a,b) cout << "Test the " << a << " function of " << b << " class" << endl;
//#include "matrixImproved.hpp"
using namespace std;
int main()
{
    // Run test
    // Mat<int> matI1(3,3,1,'z');
    // Mat<int> matI2(3,3,1,'d');
    // Mat<int> matI4(0,0,1,'d');
    // Mat<int> matI5(3,3,2,'z');
    // Mat<int> matI6 = matI1;
    // matI1.print();//should output zero matrix
    // matI2.print();
    // matI4.print();
    // matI5.print();
    // Mat<float> matF1(3,3,1);
    // matI1.setValue(1,1,1,1);

    // TODO:Constructor Test
    TEST("Constructor","Mat<double>");
    Mat<double> matD1;//zero constructor
    Mat<double> matD2(3,4,2);//blank mat
    Mat<double> matD3(3,5,2,'z');//zero mat
    Mat<double> matD4(5,4,2,'z');//zero mat
    Mat<double> matD5(5,5,2,'d');//diag mat
    Mat<double> matD6(3,3);//zero mat one channel
    matD4.print();
    matD5.print();
    matD5.print("matD552-diag.txt");
    // TODO: Operator test

    // TODO: Soft copy test

    // TODO: Different type test

    // TODO: Specialize test

    // TODO: Speed test

    // TODO: Region of Interest test
}
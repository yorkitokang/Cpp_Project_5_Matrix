#include "matrix.hpp"
#define TEST(a,b) cout << "Test the " << a << " function of " << b << " class" << endl;
#define SIZE 6400
#include <chrono>
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
    // TEST("Constructor","Mat<double>");
    // Mat<double> matD1;//zero constructor
    // Mat<double> matD2(3,4,2);//blank mat
    // Mat<double> matD3(3,5,2,'z');//zero mat
    // Mat<double> matD4(5,4,2,'z');//zero mat
    // Mat<double> matD5(5,5,2,'d');//diag mat
    // Mat<double> matD6(3,3);//zero mat one channel
    // matD4.print();
    // matD5.print();
    // matD5.print("matD552-diag.txt");
    // Mat<double> matD7(5,5,2);
    // matD7.read("../data/matD552-diag.txt");
    // matD7.print();

    // TODO: Operator test
    // Mat<double> matD1(3,3,2,'r');
    // Mat<int> matI1(3,3,2,'r');
    // Mat<float> matF1(3,3,2,'r');
    // Mat<char> matC1(3,3,2,'r');
    // matD1.print("matD332-rand");
    // matI1.print();
    // matF1.print();
    // matC1.print();

    // TODO: ROI test
    // Mat<double> matD1(3,3,1,'d');
    // Mat<double> matD2(matD1);
    // Mat<double> matD3 = matD1;
    // Mat<double> matD4;
    // matD4.ROI(matD1,2,2,1,1,1,0);
    // matD1.print();
    // matD2.print();
    // matD3.print();
    // matD4.print();
    // matD1.setValue(0,1,0,2);
    // matD1.print();
    // matD2.print();
    // matD3.print();
    // matD4.print();
    // {
    //     Mat<double> matD5;
    //     matD5.ROI(matD1,2,2,1,1,1,0);
    // }
    // Mat<double> matD6;

    // TODO: Different type test


    Mat<double> mat1D(SIZE,SIZE,1);
    Mat<double> mat2D(SIZE,SIZE,1);
    double* li = new double[SIZE*SIZE];
    double* li2= new double[SIZE*SIZE];
    time_t t;
    srand((unsigned)time(&t));
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
            {
                li[i * SIZE + j] = 0 + 2 * 1.0 * rand() / RAND_MAX * (1 - 0);
            }
    }
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
            {
                li2[i * SIZE + j] = 0 + 2 * 1.0 * rand() / RAND_MAX * (1 - 0);
            }
    }
    mat1D.fill(li);
    // mat1D.print();
    mat2D.fill(li2);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    Mat<double> mat3D = mat2D*mat1D;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout<< "Time elapsed size " <<SIZE << ": " <<std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count() << "ms" <<std::endl;

    // Mat<int> mat1D(SIZE,SIZE,1);
    // Mat<int> mat2D(SIZE,SIZE,1);
    // int* li = new int[SIZE*SIZE];
    // int* li2= new int[SIZE*SIZE];
    // for(int i = 0 ; i < SIZE*SIZE;i++)
    //     li[i] = iRand(1,10);
    // for(int i = 0 ; i < SIZE*SIZE;i++)
    // {
    //     li2[i] = iRand(1,10);
    //     cout << li2[i];
    // }
    // mat1D.fill(li);
    // mat2D.fill(li2);
    // Mat<int> mat3D(SIZE,SIZE,1);
    // mat1D.print();
    // mat2D.print();
    // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // mat3D = mat1D*mat2D;
    // std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    // std::cout<< "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count() << "ms" <<std::endl;

    // TODO: Speed test

    // TODO: Region of Interest test
}
#include <iostream>
#include <string>
#include <cstdlib>
template <typename T> std::string type_name();
using namespace std;
void test_size_t(size_t a,size_t b)
{
    cout << "success" << endl;
    printf("%ld\n",a);
    cout << typeid(a).name() << endl;
    size_t c = a + b;
    printf("c : %ld\n",c);
    cout << "c++ :" << c << endl;
    cout << typeid(a).name() << endl;
}

int main()
{
    int a = 3;
    int b = -4;
    size_t d = -10;
    cout << d << endl;
    cout << typeid(a).name() << endl;
    test_size_t(a,b);
}
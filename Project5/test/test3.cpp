#include <random>

using namespace std;

template<typename T>
T add(T one,T two)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> dis(0.0,1.0);

    T random_number = dis(gen);
    return random_number;
}
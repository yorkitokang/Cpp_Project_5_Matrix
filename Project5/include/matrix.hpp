#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef WITH_AVX2
#include <immintrin.h>
#endif

#ifdef WITH_NEON
#include <arm_neon.h>
#endif
/*
Because of linking advise, Put all the implementations in c.
*/

template<typename T>
class Mat
{
private:
    size_t rows;
    size_t cols;
    size_t channels;
    T * data; // rows*cols*channels
    T * parent_ptr;
    u_int16_t *ref_count; //for soft copy

public:

    /*
        Constructors and Destructors
    */

    //Default:
    Mat();
    /*
        Constructor
        @param rows: number of rows
        @param cols: number of columns
        @param channels: number of channels
        @param data: pointer to data
        @param parent_ptr: pointer to parent
        @param ref_count: pointer to reference count
        @paran type: default b = blank matrix
                    d = diag matrix
                    r = random matrix
    */
    Mat(size_t rows = 0, size_t cols = 0, size_t channels = 1,char type = 'b');
    //Copy Constructor
    Mat(const Mat & other);
    //Destructor
    ~Mat();
    //Function
    Mat<T> operator+(const Mat<T> & other) const;
    Mat<T> operator+(const T arg) const;
    friend Mat<T> operator+(T const arg, const Mat<T> & other);
    Mat<T> operator-(const Mat<T>& other) const;
    Mat<T> operator-(const T arg;) const;
    friend Mat<T> operator-(T const arg, const Mat<T> & other);
    Mat<T> operator*(const Mat<T>& other) const;
    Mat<T> operator*(const T arg) const;
    friend Mat<T> operator*(T const arg, const Mat<T> & other);
    Mat<T> operator/(const Mat<T>& other) const;
    Mat<T> operator/(const T arg) const;
    friend Mat<T> operator/(T const arg, const Mat<T> & other);

    Mat<T>& operator=(const Mat<T> & other);
    Mat<T>& operator+=(const Mat<T> & other);
    Mat<T>& operator-=(const Mat<T> & other);
    Mat<T>& operator*=(const Mat<T> & other);
    Mat<T>& operator/=(const T arg);
    


    void print();
};

template<>
class Mat<int>
{

}

/*
Implementation
*/

template<typename T>
Mat<T>::Mat():rows(0),cols(0),channels(0),data(nullptr){}

template<typename T>
Mat<T>::Mat(size_t rows = 0, size_t cols = 0,size_t channels = 0, char type = 'b'):rows(rows),cols(cols), channels(channels)
{
    *(ref_count) = 1;
    data = new T[rows * cols * channels * sizeof(T)];
    if(type == 'b') return;

    if(type == 'z')
    {
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for(size_t i = 0; i < rows * cols * channels; i++)
        {
            data[i] = T(0);
        }
        return;
    }

    if(type == 'd')
    {
#ifdef _OPENMP
#pragma omp parallel for
#endif
        if(rows != cols)
        {
            std::cerr << "Matrix Creation Error: trying to create a nonsquare diag matrix";
        } else {
            for(size_t i = 0; i < rows * cols * channels; i++)
            {
                data[i] = T(0);
            }
            for(size_t i = 0; i < rows * cols * channels; i++)
            {
                data[i] = T(1);
            }
        }
        return;
    }

    if(type == 'r')
    {
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for(size_t i = 0; i < rows * cols * channels; i++)
        {
            data[i] = T(0);
        }
        return;
    }
}

template <typename T>
Mat<T>::Mat(const Mat<T> & Mat)
{
    cols = Mat.cols;
    rows = Mat.rows;
    channels = Mat.channels;
    data = Mat.data;
    ref_count = Mat.ref_count;
    *(ref_count) ++;
}

template<typename T>
Mat<T>::~Mat()
{
    //soft copy
    *(ref_count) -= 1;
    if(*(ref_count) == 0 && data != nullptr)
    {
        delete[] data;
    }
}

template<typename T>
Mat<T>& Mat<T>::operator=(const Mat<T> & other)
{
    //subsitute current object
    cols = other.cols;
    rows = other.rows;
    channels = other.channels;
    parent_ptr = other.parent_ptr;

    //reduce current object ref count
    *ref_count--;
    if((*ref_count) == 0 && ref_count != nullptr)
    {
        delete[] data;
    }

    //subsitute current object
    data = other.data;
    ref_count = other.ref_count;
    ref_count ++;
    return *this;
}

//Add
template<typename T>
Mat<T> Mat<T>::operator+(const Mat<T> & other) const
{
    //Check memory allocation
    if(data == nullptr || other.data == nullptr)
    {
        throw std::runtime_error("Mat::operator+: Matrix is not allocated");
    }

    //Check arguments
    if(rows!= other.rows || cols!= other.cols || channels!= other.channels)
    {
        throw std::runtime_error("Mat::operator+: Size mismatch");
    }

    //Do the addition
    Mat<T> result(rows, cols, channels);

#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif

#ifdef WITH_AVX2 //Intel Acceletration
    //TODO: Intel Acceletration
#elif defined WITH_NEON //AMD Acceletration
    //TODO: ARM Acceletration
#else //Normal
    for(size_t i=0; i<rows*cols*channels; i++)
    {
        result.data[i] = data[i] + other.data[i];
    }
#endif
}

template<typename T>
Mat<T> operator+(const T arg)
{
    
}

template<typename T>
Mat<T> operator+(T const arg, const Mat<T> & other)
{
    return other + arg;
}

Mat<T> operator-(const Mat<T>& other);
Mat<T> operator-(const T arg;);
friend Mat<T> operator-(T const arg, const Mat<T> & other);
Mat<T> operator*(const Mat<T>& other);
Mat<T> operator*(const T arg);
friend Mat<T> operator*(T const arg, const Mat<T> & other);
Mat<T> operator/(const Mat<T>& other);
Mat<T> operator/(const T arg);
friend Mat<T> operator/(T const arg, const Mat<T> & other);

template<typename T>
Mat<T>& Mat<T>::operator+=(const Mat<T> & other)
{

}

template<typename T>
Mat<T>& Mat<T>::operator-=(const Mat<T> & other)
{

}

template<typename T>
Mat<T>& Mat<T>::operator*=(const Mat<T> & other)
{

}

template<typename T>
Mat<T>& Mat<T>::operator/=(const T arg)
{

}

template<typename T>
Mat<T> operator+(const Mat<T> & other)
{
    
}

template<typename T>
Mat<T> operator-(Mat<T>& other)
{

}

template<typename T>
Mat<T> operator*(Mat<T>& other)
{
    
}

void print()
{

}

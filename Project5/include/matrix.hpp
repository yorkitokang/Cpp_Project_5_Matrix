#include <iostream>
#include <fstream>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef WITH_AVX2
#include <immintrin.h>
#endif

#ifdef WITH_NEON
#include <arm_neon.h>
#endif

#define RESULT(i,j,c) result.data[c*(result.rows)*(result.cols) + i*(result.cols) + j]
#define THIS(i,j,c) this->data[c*(this->rows)*(this->cols) + i*(this->cols) + j]
#define OTHER(i,j,c) other.data[c*(other.rows)*(other.cols) + i*(other.cols) + j]


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
    int *ref_count; //for soft copy

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

    //add operator
    Mat<T> operator+(const Mat<T> & other) const;
    Mat<T> operator+(const T arg) const;
    friend Mat<T> operator+(T const arg, const Mat<T> & other);
    //subtract operator
    Mat<T> operator-(const Mat<T>& other) const;
    Mat<T> operator-(const T arg) const;
    friend Mat<T> operator-(T const arg, const Mat<T> & other);
    //multiply operator
    Mat<T> operator*(const Mat<T>& other) const;
    Mat<T> operator*(const T arg) const;
    friend Mat<T> operator*(T const arg, const Mat<T> & other);
    //division operator
    Mat<T> operator/(const Mat<T>& other) const;
    Mat<T> operator/(const T arg) const;

    //copy operator
    Mat<T>& operator=(const Mat<T> & other);
    //add equal
    Mat<T>& operator+=(const Mat<T> & other);
    //subtract equal
    Mat<T>& operator-=(const Mat<T> & other);
    //multiply equal
    Mat<T>& operator*=(const Mat<T> & other);
    //divide equal
    Mat<T>& operator/=(const T arg);

    //hard copy
    Mat<T>& clone(const Mat<T> & other);
    //print
    void print();

    //set value
    void setValue(const size_t row, const size_t col, const size_t channel, T value);
    // std::ostream operator<<();
};

/*
Implementation
*/

template<typename T>
inline Mat<T>::Mat()
{
    rows = 0;
    cols = 0;
    channels = 0;
    ref_count = nullptr;
    parent_ptr = nullptr;
    data = nullptr;
}

template<typename T>
inline Mat<T>::Mat(size_t rows , size_t cols ,size_t channels , char type ):rows(rows),cols(cols), channels(channels)
{
    int ref_init = 1;
    parent_ptr = nullptr;
    ref_count = &ref_init;
    data = new T[rows * cols * channels];
    if(type == 'b')
    {
        std::cout << "create blank matrix" << std::endl;
        return;
    }
    if(type == 'z')
    {
        std::cout << "create zero matrix" << std::endl;
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
        std::cout << "create diag matrix" << std::endl;

        if(rows != cols)
        {
            fprintf(stderr,"Math Error: creating a nonsquare diag matrix\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
            exit(1);
        } else {
             size_t i;
#ifdef _OPENMP
#pragma omp parallel for
#endif
            for(i = 0; i < rows * cols * channels; i++)
            {
                data[i] = T(0);
            }
// #ifdef _OPENMP
// #pragma omp parallel for
// #endif
            for(i = 0; i < rows * cols * channels;)//BUG:In openMP you cannot delete the increment part, thus delete it
            {
                data[i] = T(1);
                if(i % cols == cols-1 && i !=0)
                {
                    data[i+1] = T(1);
                    i++;
                }
                i += cols +1;
            }
        }
        return;
    }

    if(type == 'r')
    {
        fprintf(stderr,"Type Error: creating unsupported random matrix\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
}

template <typename T>
inline Mat<T>::Mat(const Mat<T> & Mat)
{
    std::cout << "Mat<T>::Mat(const Mat<T> & Mat)" << std::endl;
    cols = Mat.cols;
    rows = Mat.rows;
    channels = Mat.channels;
    data = Mat.data;
    ref_count = Mat.ref_count;
    (*ref_count) ++;
}

template<typename T>
inline Mat<T>::~Mat()
{
    std::cout << "Mat<T>::~Mat()" << std::endl;
    //soft copy
    (*ref_count) -= 1;
    if((*ref_count) == 0 && data != nullptr)
    {
        delete[] data;
    }
}

template<typename T>
inline Mat<T>& Mat<T>::operator=(const Mat<T> & other)
{
    std::cout << "Mat<T>::operator=(const Mat<T> & other)" << std::endl;

    //subsitute current object
    cols = other.cols;
    rows = other.rows;
    channels = other.channels;
    parent_ptr = other.parent_ptr;

    //reduce current object ref count
    (*ref_count)--;
    if((*ref_count) == 0 && ref_count != nullptr)
    {
        delete[] data;
    }

    //subsitute current object
    data = other.data;
    ref_count = other.ref_count;
    (*ref_count) ++;
    return *this;
}

//Add
template<typename T>
inline Mat<T> Mat<T>::operator+(const Mat<T> & other) const
{
    std::cout << "Mat<T>::operator+(const Mat<T> & other)" << std::endl;

    //Check
    if(this->data == nullptr || other.data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    if(this->rows!= other.rows || this->cols!= other.cols || this->channels!= other.channels)
    {
        fprintf(stderr,"Math Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    //Do the addition
    Mat<T> result(rows, cols, channels);

#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
    {
        result.data[i] = data[i] + other.data[i];
    }
    return result;
}

template<typename T>
inline Mat<T> Mat<T>::operator+(const T arg) const
{
    std::cout << "operator+(const T arg)" << std::endl;
    //Check
    if(this->data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    Mat<T> result(rows,cols,channels);
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
    {
        result.data[i] = data[i] + arg;
    }
    return result;
}

template<typename T>
inline Mat<T> operator+(T const arg, const Mat<T> & other)
{
    if(other.data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    return other + arg;
}

template<typename T>
inline Mat<T> Mat<T>::operator-(const Mat<T>& other) const
{
    std::cout << "Mat<T>::operator-(const Mat<T>& other)" << std::endl;

    //Check memory allocation
    if(data == nullptr || other.data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    //Check arguments
    if(rows!= other.rows || cols!= other.cols || channels!= other.channels)
    {
        fprintf(stderr,"Math Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    //Do the addition
    Mat<T> result(rows, cols, channels);
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
        result.data[i] = data[i] - other.data[i];
    return result;
}

template<typename T>
inline Mat<T> Mat<T>::operator-(const T arg) const
{
    std::cout << "Mat<T>::operator-(const T arg)" << std::endl;

    if(this->data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    Mat<T> result(rows, cols, channels);
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
        result.data[i] = data[i] - arg;
    return result;
}

template<typename T>
inline Mat<T> operator-(T const arg, const Mat<T> & other)
{
    std::cout << "operator-(T const arg, const Mat<T> & other)" << std::endl;
    //Check
    if(other.data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    return other + arg;
}

template<typename T>
inline Mat<T> Mat<T>::operator*(const Mat<T>& other) const
{
    std::cout << "Mat<T> operator*(const Mat<T> & other)" << std::endl;

    //Check arguments
    if(cols!= other.rows || channels!= other.channels)
    {
        fprintf(stderr,"Math Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    if(this->data == nullptr || other.data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    Mat<T> result(rows,other.cols,channels,'z');
    for(size_t ch = 0; ch < channels; ch++)
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
        for (size_t r = 0; r < rows; r++)
            for (size_t k = 0; k < cols; k++)
                for (size_t c = 0; c < other.cols; c++)
                    RESULT(r,c,ch) += THIS(r,k,ch) * OTHER(k,c,ch);

    return result;
}

template<typename T>
inline Mat<T> Mat<T>::operator*(const T arg) const
{
    std::cout << "Mat<T>::operator*(const T arg)" << std::endl;
    if(this->data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    Mat<T> result(rows, cols, channels);
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
        result.data[i] = data[i] * arg;
    return result;
}

template<typename T>
inline Mat<T> operator*(T const arg, const Mat<T> & other)
{
    std::cout << "operator*(T const arg, const Mat<T> & other)" << std::endl;
    if(other->data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    return other * arg;
}

template<typename T>
inline Mat<T> Mat<T>::operator/(const Mat<T>& other) const
{
    std::cout << "operator/(const Mat<T>& other)" << std::endl;

    //Check
    if(this->data == nullptr || other.data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    if(this->rows!= other.rows || this->cols!= other.cols || this->channels!= other.channels)
    {
        fprintf(stderr,"Math Error: size not match\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    Mat<T> result(rows, cols, channels);
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
    {
        if(other.data[i] == T(0))
        {
            fprintf(stderr,"Math Error: divided by zero\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
            exit(1);
        }
        result.data[i] = data[i] / other.data[i];
    }
    return result;
}

template<typename T>
inline Mat<T> Mat<T>::operator/(const T arg) const
{
    std::cout << "Mat<T>::operator/(const T arg)" << std::endl;

    //Check
    if(this->data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    if(arg == 0)
    {
        fprintf(stderr,"Math Error: devivded by zero\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }

    Mat<T> result(rows, cols, channels);
#ifdef _OPENMP //OpenMP support
#pragma omp parallel for
#endif
    for(size_t i=0; i<rows*cols*channels; i++)
        result.data[i] = data[i] / arg;
    return result;
}

template<typename T>
inline Mat<T>& Mat<T>::operator+=(const Mat<T> & other)
{
    *this = *this + other;
    return *this;
}

template<typename T>
inline Mat<T>& Mat<T>::operator-=(const Mat<T> & other)
{
    *this = *this - other;
    return *this;
}

template<typename T>
inline Mat<T>& Mat<T>::operator*=(const Mat<T> & other)
{
    *this = *this * other;
    return *this;
}

template<typename T>
inline Mat<T>& Mat<T>::operator/=(const T arg)
{
    *this = *this / arg;
    return *this;
}

template<typename T>
inline void Mat<T>::print()
{
    if(this->data == nullptr)
    {
        fprintf(stderr,"Math Error: creating a nonsquare diag matrix\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
// #ifdef _OPENMP //OpenMP support
// #pragma omp parallel for
// #endif
    for(int i = 0; i < channels; i++)
    {
        for(int j = 0; j < rows;j++)
        {
            for(int k = 0; k < cols; k++)
            {
                std::cout << this->data[i*rows*cols + j *(cols) +k] << " " ;
            }
            std::cout  << std::endl;
        }
        std::cout << std::endl;
    }
}

template <typename T>
inline void Mat<T>::setValue(const size_t row,const size_t col,const size_t channel,const T value)
{
    this->data[channel*rows*cols + row*cols + col] = value;
}

template<typename T>
inline Mat<T>& Mat<T>::clone(const Mat<T>& other)
{
    this->channels = other.channels;
    this->rows = other.rows;
    this->cols = other.cols;
    parent_ptr = other.parent_ptr;

    //reduce current object ref count
    *ref_count--;
    if((*ref_count) == 0 && ref_count != nullptr)
    {
        delete[] data;
    }

    this->data = new T[other.cols*other.rows*other.channels*sizeof(T)];
    for(int i = 0; i < other.channels * other.rows * other.cols; i++)
    {
        this->data[i] = other.data[i];
    }

    return *this;
}

/*
    specialized class: Int
*/

// template<>
// class Mat<int>
// {
// #ifdef WITH_AVX2 //Intel Acceletration
//     //TODO: Intel Acceletration
// #elif defined WITH_NEON //AMD Acceletration
//     //TODO: ARM Acceletration
// #endif
// };

/*
    specialized class: float
*/

/*
    specialized class: double
*/
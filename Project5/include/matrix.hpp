#include <iostream>
#include <fstream>
#include <random>
#include <cstring>

#ifdef _OPENMP
#include <omp.h>
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
    size_t rows;
    size_t cols;
    size_t channels;
    T * data; // rows*cols*channels
    T * parent_ptr;
    int *ref_count; //for soft copy
public:

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
    template<typename U>
    friend Mat<U> operator+(U const arg, const Mat<U> & other);

    //subtract operator
    Mat<T> operator-(const Mat<T>& other) const;
    Mat<T> operator-(const T arg) const;
    template<typename U>
    friend Mat<U> operator-(U const arg, const Mat<U> & other);

    //multiply operator
    Mat<T> operator*(const Mat<T>& other) const;
    Mat<T> operator*(const T arg) const;
    template<typename U>
    friend Mat<U> operator*(U const arg, const Mat<U> & other);

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

    //getter
    void print();
    void print(const char * filename);
    void shape();
    size_t getRows();
    size_t getCols();
    size_t getChannels();
    T * getData();
    T * getParent();
    int * getRef();
    T& operator()(size_t row, size_t col, size_t channel);

    //setter
    void fill();
    void fill(T* content);
    void read(const char * filename);
    void setValue(const size_t row, const size_t col, const size_t channel, T value);
    //ROI
    void ROI(Mat<T> & parent,size_t rows, size_t cols, size_t channels, size_t row_offset, size_t col_offset, size_t channel_offset);

    static Mat<T>& hadamard(Mat<T>& lhs, Mat<T>& rhs);

};

/*
Implementation
*/
template<typename T>
inline Mat<T>::Mat(size_t rows , size_t cols ,size_t channels , char type ):rows(rows),cols(cols), channels(channels)
{
    int ref_init = 1;
    parent_ptr = nullptr;
    ref_count = &ref_init;
    if(rows == 0 || cols == 0 || channels  <= 0)
    {
        data = nullptr;
    } else
    {
        data = new T[rows * cols * channels];
    }

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

    fprintf(stderr,"Parameter: can't recognize the type code of matrix \n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
    exit(1);
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
        if(parent_ptr == nullptr)
        {
            delete[] data;
        } else{
            delete[] parent_ptr;
        }
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
        if(parent_ptr == nullptr)
        {
            delete[] data;
        } else{
            delete[] parent_ptr;
        }
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
        for (size_t r0 = 0; r0 < rows; r0 += 16)
            for (size_t k0 = 0; k0 < cols; k0 += 16)
                for (size_t r = 0; r < 16; r++)
                    for (size_t k = 0; k < 16; k++)
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
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
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

template<typename T>
inline void Mat<T>::print(const char * filename)
{
    std::string path("../data/");
    path.append(filename);
    char *cstr = new char[path.length() + 1];
    strcpy(cstr, path.c_str());
    std::ofstream outfile(cstr);
    for(int i = 0; i < rows*cols*channels; i++)
        outfile << data[i] << " ";
    outfile.close();
    delete [] cstr;
}

template<typename T>
inline void Mat<T>::shape()
{
    std::cout << "(" << this->rows << "," << this->cols << "," << this->channels << ")" << std::endl;
}

template<typename T>
T& Mat<T>::operator()(size_t row, size_t col, size_t channel)
{
    return data[channel*rows*cols + row * cols + col];
}

template<typename T>
void Mat<T>::fill()
{
    if(this->data == nullptr)
    {
        fprintf(stderr,"NUll Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    for(int i = 0; i < rows*cols*channels;i++)
        std::cin >> data[i];
}

template<typename T>
void Mat<T>::fill(T * content)
{
    if(content == nullptr)
    {
        fprintf(stderr,"NUll Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    this->data = content;
}

template<typename T>
void Mat<T>::read(const char * filename)
{
    std::ifstream input_file(filename);

    if (!input_file.is_open()) {
        fprintf(stderr,"Error opening input file: file not exist\n FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__) ;
        exit(1);
    }
    T value;
    int i = 0;
    while(input_file >> value)
    {
        data[i] = value;
        i++;
        if(i > rows*cols*channels)
        {
            fprintf(stderr,"Error reading input file: Index out of bond\n FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__) ;
            exit(1);
        }
    }
    input_file.close();
}

template<typename T>
Mat<T>& Mat<T>::hadamard(Mat<T>& lhs, Mat<T>& rhs)
{
    if(lhs.getChannels() != rhs.getChannels() || lhs.getRows() != rhs.getRows()|| lhs.getCols() != rhs.getCols())
    {
        fprintf(stderr,"Math Error: size not match\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    if(lhs.getData() == nullptr || rhs.getData() == nullptr)
    {
        fprintf(stderr,"NUll Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    size_t rows = lhs.getRows();
    size_t cols = lhs.getCols();
    size_t channels = lhs.getChannels();
    Mat<T> temp(rows,cols,channels);
    T * lhsD;
    T * rhsD;
    for(int i = 0; i < rows*cols*channels; i++)
        temp = lhsD[i] * rhsD[i];

    return *temp;
}

template <typename T>
inline void Mat<T>::setValue(const size_t row,const size_t col,const size_t channel,const T value)
{
    this->data[channel*rows*cols + row*cols + col] = value;
}

template <typename T>
void ROI(Mat<T> & parent,size_t rows, size_t cols, size_t channels, size_t row_offset, size_t col_offset, size_t channel_offset)
{
    if(rows+row_offset > parent.getRows() || cols+col_offset > parent.getCols() || channels+channel_offset > parent.getChannels())
    {
        fprintf(stderr,"Index Out of Bond: ROI is either too big or not in the parent region\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    if(parent.getData() == nullptr )
    {
        fprintf(stderr,"Null Pointer Error: parent matrix is a null matrix\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    this->rows = rows;
    this->cols = cols;
    this->channels = channels;
    (*ref_count)--;
    if((*ref_count) == 0 && data != nullptr)
    {
        if(parent_ptr == nullptr)
        {
            delete[] data;
        } else{
            delete[] parent_ptr;
        }
    }

    ref_count = other.getRef();
    (*ref_count)++;
    data = other.getData()+ channel_offset*parent.getCols()*parent.getRows() + row_offset * parent.getCols() + col_offset();
    parent_ptr = other.getParent();

}

template <typename T>
size_t Mat<T>::getRows()
{
    return rows;
}

template <typename T>
size_t Mat<T>::getCols()
{
    return cols;
}

template <typename T>
size_t Mat<T>::getChannels()
{
    return channels;
}

template <typename T>
T* Mat<T>::getData()
{
    return this->data;
}

template <typename T>
T* Mat<T>::getParent()
{
    return this->parent_ptr;
}

template <typename T>
int* Mat<T>::getRef()
{
    return this->ref_count;
}
# C++ Project - A Matrix Class

> Student Name: Yaozhong Kang
>
> SID: 12110225

[toc]

## Part 1 - Analysis

> **Requirement Table**

- [x] Design a class for matrices, and the class should contain the data of a matrix and related
  information such the number of rows, the number of columns, the number of channels, etc.
- [x] The class should support different data types. It means that the matrix elements can be
  unsigned char , short , int , float , double , etc.
- [x] Do not use memory hard copy if a matrix object is assigned to another. Please carefully handle
  the memory management to avoid memory leaks and to release memory multiple times.
- [x] Implement some frequently used operators including but not limited to = , == , + , - , * , etc.
  Surely the matrix multiplication in Project 4 can be included.
- [x] Implement region of interest (ROI) to avoid memory hard copy.
- [ ] [Optional] Test your program on X86 and ARM platforms, and describe the differences.

The basic compositions of a class are the member variables, constructors, and functions.

### Design member var 

As implemented in Project 3 and Project 4, we need at least three member variables: `size_t rows` `size_t cols` and a 1-D type value array   `T* data`. To meet the requirement for doing soft copy when copying one matrix instance to another, inspired by the solution in `cv::Mat` , we create a pointer pointing to an int variable, `int * ref_count`, to record the times one object matrix been copied. Also, we have to implement ROI. Thus, I added `T* parent_ptr` pointing to the root matrix.

### Design constructors

There should be constructors which can create blank, zero, diagonal or random matrices. The problem appears when you want to create a random matrix of data T. Searching from web, we need to implement a class for **randomization**, and that may not conclude all the situations as well. Thus, if the user want to create a random matrix, he has to randomize the data and use`Mat<T>::fill(T* data)` to fill the randomized data into the matrix. The process is dangerous because if the data length is not as long, error exists.

A copy constructor should be implemented as well. For soft copy need, we have to **do ref_count increment** in copy constructor.

### Design member functions and static functions

There should be operator functions, the multiplication method can be inherited from Project 4. After reading my fellow Wenqian Yan's report, stunned and amazed, I was taught that the compiler would help us to do **SIMD** and **OMP**. Customizing the **SIMD** method may cause the opposite effect leading to the decrement of computing rate. At first, I wanted to write some **specialized classes** of `Mat<T>` to do **SIMD** individually depending on the specific data type.

Another problem come when you try to **do the operation between matrices of different type**. That operation is not allowed. However, you can print the data of your matrix in a file and read it by a new matrix of specific data type.

### Dynamic memory management

Because we're doing soft copy and a matrix may have a parent matrix because of it may be created by **ROI**, when destructing, copying a matrix or doing **ROI** we have do some assertions.

## Part 2 - Code & Test

[My Repo](https://github.com/yorkitokang/Cpp_Project_5_Matrix/tree/master/Project5)

### 2.1 Structure of Mat

```c++
template<typename T>
class Mat
{
    size_t rows;
    size_t cols;
    size_t channels;
    T * data; // rows*cols*channels
    int *ref_count; //for soft copy
public:
    Mat(size_t rows = 0, size_t cols = 0, size_t channels = 1,char type = 'b');
    //Copy Constructor
    Mat(const Mat & other);
    //Destructor
    ~Mat();
    
    bool operator==(Mat<T>& other);
    bool operator!=(Mat<T>& other);

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
```

### 2.2 Constructor

#### 2.2.1 Normal Constructor

At first, we Initialize the member vars.

```c++
int ref_init = 1;
parent_ptr = nullptr;
ref_count = &ref_init;
```

Check the arguments:

```c++
if(rows == 0 || cols == 0 || channels  <= 0)
{
    data = nullptr;
} else
{
    data = new T[rows * cols * channels];
}
```

Depending on the parameter, construct blank matrix, zero matrix or diagonal matrix:

```c++
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
```

When we want to do OMP, we must enter the increment method of for loop and shouldn't edit `i` in the content of for loop. That property is quite annoying.

#### 2.2.2 Copy constructor

Notice that we're doing soft copy, so that we let the pointer point to the destination.

```c++
inline Mat<T>::Mat(const Mat<T> & Mat)
{
    cols = Mat.cols;
    rows = Mat.rows;
    channels = Mat.channels;
    data = Mat.data;
    ref_count = Mat.ref_count;
    (*ref_count) ++;
}
```

#### 2.2.3 Destructor

When destructing an matrix object, we have to judge that if it is the last object sharing the same data, either the data is shared because of soft copy or ROI.

Thus, this structure would also be used in ROI and `operator=`.

```c++
inline Mat<T>::~Mat()
{
    //soft copy
    (*ref_count) -= 1;
    if((*ref_count) == 0 && data != nullptr)
    {
		delete[] data;
    }
}
```

### 2.3 Operators

#### 2.3.1 Copy Operator

We have to careful that we we're copying, we are first destructing the original object.

```c++
inline Mat<T>& Mat<T>::operator=(const Mat<T> & other)
{
    std::cout << "Mat<T>::operator=(const Mat<T> & other)" << std::endl;

    //subsitute current object
    cols = other.cols;
    rows = other.rows;
    channels = other.channels;


    //reduce current object ref count
    (*ref_count)--;
    if((*ref_count) == 0 && ref_count != nullptr)
    {
        delete[] data;
        delete ref_count;
    }

    //subsitute current object
    data = other.data;
    ref_count = other.ref_count;
    (*ref_count) ++;
    return *this;
}
```

To test the copy operator, we debug it and see the  value of `ref_count`: 

```c++
Mat<double> matD1(3,3,1,'d');
Mat<double> matD2(matD1);
Mat<double> matD3 = matD1;
matD1.print();
matD2.print();
matD3.print();
matD1.setValue(0,1,0,2.0);
matD1.print();
matD2.print();
matD3.print();
```

Result:

![image-20221218163336182](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218163336182.png)

![image-20221218163848236](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218163848236.png)

![image-20221218163933015](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218163933015.png)

![image-20221218164003099](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218164003099.png)

We can see that soft copy succeeded.

#### 2.3.2 Normal Operator (example: *)

Because we've done the matrix acceleration in Project 4, and I was told that the compiler would automatically use the **SIMD** method to improve the speed, I will not implement **SIMD** for integer, float and double multiplication. If we want to do **SIMD** for type class `Mat<T>`,  we need to specialize the matrix for the above three data type.

Following the instructions of Wenqian Yan, I chose to implement the blocked rkc method:

```c++
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
```

**Time Elapse** (double)

Time is limited thus I didn't do the comparison between the normal implementation and the optimized implemetation.

| size | time optimized(ms) |
| ---- | ------------------ |
| 100  | 2                  |
| 200  | 21                 |
| 400  | 75                 |
| 800  | 471                |
| 1600 | 3395               |
| 3200 | 26164              |
| 6400 | 204395             |

![image-20221218205439316](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218205439316.png)

![image-20221218205452010](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218205452010.png)

![image-20221218205507578](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218205507578.png)

![image-20221218205520532](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218205520532.png)

![image-20221218205536957](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218205536957.png)

![image-20221218205742886](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218205742886.png)

#### 2.3.3 Friend Operator (example: *)

We have to take care that friend operators should use a different typename.

```c++
template<typename U>
inline Mat<U> operator*(U const arg, const Mat<U> & other)
{
    std::cout << "operator*(U const arg, const Mat<U> & other)" << std::endl;
    if(other->data == nullptr)
    {
        fprintf(stderr,"Null Pointer Error:\n  FILE:%s-->LINE:%d-->%s\n",__FILE__,__LINE__,__func__);
        exit(1);
    }
    return other * arg;
}
```

### 2.4 Region Of Interest

```c++
void Mat<T>::ROI(Mat<T> & parent,size_t rows, size_t cols, size_t channels, size_t row_offset, size_t col_offset, size_t channel_offset)
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
        delete[] data;
        delete ref_count;
    }

    ref_count = parent.getRef();
    (*ref_count)++;
    data = parent.getData()+ channel_offset*parent.getCols()*parent.getRows() + row_offset * parent.getCols() + col_offset;
}
```

To implement ROI, we are actually soft copying the original matrix, however pointing to the specific position will cause a problem.

Because the data is read in sequence, and the data is one dimensional. We're actually taking a slice but not a block.

To test ROI, we follow the soft copy test method and debug it.

```c++
Mat<double> matD1(3,3,1,'d');
Mat<double> matD2(matD1);
Mat<double> matD3 = matD1;
Mat<double> matD4;
matD4.ROI(matD1,2,2,1,1,1,0);
matD1.print();
matD2.print();
matD3.print();
matD4.print();
matD1.setValue(0,1,0,2);
matD1.print();
matD2.print();
matD3.print();
matD4.print();
{
    Mat<double> matD5;
    matD5.ROI(matD1,2,2,1,1,1,0);
}
Mat<double> matD6;
```

![image-20221218175218358](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218175218358.png)



![image-20221218174301824](/home/yorkitokang/.config/Typora/typora-user-images/image-20221218174301824.png)

From test we can see that `*ref_count` was reduced from 5 to 4 after the ROI matrix destruct.

However, the output of ROI is `1 0 0 0` not `1 0 0 1`, which we intended to have.

---

Another way to achieve ROI is to do hard copy, and that is not elegant and memory-wasting.

```c++
template <typename T>
void Mat<T>::ROI(Mat<T> & parent,size_t rows, size_t cols, size_t channels, size_t row_offset, size_t col_offset, size_t channel_offset)
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
        delete[] data;
        delete ref_count;
    }
    data = new T[rows*cols*channels];
    ref_count = new int(1);
    (*ref_count)++;
    for(int i = 0; i < channels; i += parent.getCols()*parent.getRows())
        for(int j = 0; j < rows; j+= parent.getCols())
            for(int k = 0; k < cols; k++)
                data[i*rows*cols+j*cols+k] = parent.getData()[(channel_offset+i)*parent.getCols()*parent.getRows()+(row_offset+j)*parent.getCols()+(cols_offset+k)];
}
```

And we get the right result but I think it is not the best solution. Since I don't know what ROI is used for, i think I will keep the both methods.

### 2.5 File

#### 2.5.1 Read

1. You can read from standard input from command line using `void Mat<T>::fill()`
2. You can read from an array using `void Mat<T>::fill(T * content)`
3. You can read from a file using `void Mat<T>::read(const char * filename)`

#### 2.5.2 Write

1. You can print the content of the matrix on screen using `void print()`

2. You can print the content to a file of a specific name using`void print(const char * filename)`

   

### 2.6 Static method: hadamard mul

The aim of writing this method is because every good class should contain some static stuff so that your code look abundant and professional.

```c++
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
```

## Part 3 - Difficulties & Solutions

The first difficulty is to solve the memory management problem. It is more of an experience issue. Many times the memory would leaks out of nowhere or the pointers would be repeatedly freed. The solutions is obtained by repeatedly debugging and find out where may cause the problem. However, I'm still not so familiar with how the compiler work that when creating small matrices and do the multiplication the memory would be freed twice but when doing huge matrix multiplications the problem won't happen. That's pretty annoying and took me a lot of time to debug but still didn't achieved any progress.

The second difficulty is ROI. At first I didn't figure out that simply implementing as doing soft copy is a intuitively easy but actually hard-to-achieve process. Hard copy is quiet easy, but is slow and memory-wasting. The memory has got to be two dimensional if we use pointers, which make things  complicated.

Actually, I didn't solve the difficulties quiet well. There still much to improve. However, I've learned too much pain and experience. So that's all for it.
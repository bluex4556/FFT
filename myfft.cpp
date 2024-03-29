#include <complex>
#include <iostream>
#include <valarray>
#include "mpi.h"
 
const double PI = 3.1415;
 
typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;
 
// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;
 
    // divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray  odd = x[std::slice(1, N/2, 2)];
 
    // conquer
    fft(even);
    fft(odd);
 
    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}
 
// inverse fft (in-place)
void ifft(CArray& x)
{
    // conjugate the complex numbers
    x = x.apply(std::conj);
 
    // forward fft
    fft( x );
 
    // conjugate the complex numbers again
    x = x.apply(std::conj);
 
    // scale the numbers
    x /= x.size();
}
 
int main( int argc, char *argv[])
{
    int p; //Number of processors
    int id; // Rank
    
    MPI_Init ( &argc, &argv);
    
    Complex test[] = { 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    Complex pdata[4];

    // CArray data(test, 8);

    MPI_Scatter(
        test,
        4,
        MPI_DOUBLE_COMPLEX,
        pdata,
        4,
        MPI_DOUBLE_COMPLEX,
        0,
        MPI_COMM_WORLD);
  
    CArray data(pdata,4);

    ifft(data);
    
    for(int i=0;i<4;i++)
        pdata[i] = data[i] ;

    Complex finaldata[8];
    // MPI_Gather(data)
    
    MPI_Scatter(
        pdata,
        4,
        MPI_DOUBLE_COMPLEX,
        finaldata,
        4,
        MPI_DOUBLE_COMPLEX,
        0,
        MPI_COMM_WORLD);
    if(id==0)
    {
        std::cout << std::endl << "ifft" << std::endl;
        for (int i = 0; i < 8; ++i)
        {
            std::cout << data[i] << std::endl;
        }
    }
    MPI_Finalize();
    return 0;
    
}
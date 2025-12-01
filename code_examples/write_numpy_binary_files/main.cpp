#include <iostream>
#include <string>
#include <vector>

#include <armadillo>
#include "npy.hpp"

// Build:
// g++ -O2 main.cpp -o main -I./include -larmadillo



// Helper function to write an arma::cx_mat to a numpy binary file
void write_cx_mat_to_npy_file(arma::cx_mat& A, std::string filename)
{
    // Prepare a libnpy view pointing to the Armadillo memory
    npy::npy_data_ptr<std::complex<double>> d;
    d.data_ptr = reinterpret_cast<const std::complex<double>*>(A.memptr());

    // The shape info is given in vector<unsigned long> format
    d.shape = { 
        static_cast<unsigned long>(A.n_rows), 
        static_cast<unsigned long>(A.n_cols)
    };

    // Set Fortran order to ensure that the numpy file will use 
    // the same element ordering as in the Armadillo matrix
    d.fortran_order = true;

    // Write to file
    npy::write_npy(filename, d);
}



// Helper function to write an arma::cx_cube to a numpy binary file
void write_cx_cube_to_npy_file(arma::cx_cube& C, std::string filename)
{
    // Prepare a libnpy view pointing to the Armadillo memory
    npy::npy_data_ptr<std::complex<double>> d;
    d.data_ptr = reinterpret_cast<const std::complex<double>*>(C.memptr());

    // The shape info is given in vector<unsigned long> format
    d.shape = {
        static_cast<unsigned long>(C.n_rows),
        static_cast<unsigned long>(C.n_cols),
        static_cast<unsigned long>(C.n_slices)
    };

    // Set Fortran order to ensure that numpy uses the same 
    // element ordering in the Armadillo cube.
    d.fortran_order = true;

    // Write to file
    npy::write_npy(filename, d);
}



// Main program to test the two helper functions above
int main() {

    // Create an example complex matrix
    arma::cx_mat A(2, 3);  // (n_rows, n_cols)

    for (size_t j = 0; j < A.n_cols; ++j) 
    {
        for (size_t i = 0; i < A.n_rows; ++i) 
        {
            double real = double(i) + 0.1 * double(j);
            double imag = 0.5 * double(i) - 0.2 * double(j);
            A(i,j) = std::complex<double>(real, imag);
        }
    }

    // Write matrix to file
    write_cx_mat_to_npy_file(A, "matrix_A.npy");


    // Create an example complex cube
    arma::cx_cube C(2, 3, 6);  // (n_rows, n_cols, n_slices)

    for (arma::uword k = 0; k < C.n_slices; ++k)
    {
        for (arma::uword j = 0; j < C.n_cols; ++j)
        {
            for (arma::uword i = 0; i < C.n_rows; ++i)
            {
                double real = double(i) + 0.1*double(j) + 0.01*double(k);
                double imag = 0.5*double(i) - 0.2*double(j) + 0.02*double(k);
                C(i,j,k) = std::complex<double>(real, imag);
            }
        }
    }

    // Write to file
    write_cx_cube_to_npy_file(C, "cube_C.npy");

    return 0;
}

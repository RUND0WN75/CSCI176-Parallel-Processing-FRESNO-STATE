/////////////////////////////////////////////////////////////////////////////////////////////
//
//  Description: [from svd_house_db_sequential.cpp] """This program performs a Singular Value Decomposition of an n x m matrix. 
//               these matrices have the added condition that the n >= m at the moment.
//               To perform this decomp, it makes use of the known Householder Bidiagonalization
//               method. 
//               In short, it iteratively subtracts smaller and smaller sub-matrices from the original
//               matrix and also from an Identity matrix. The result is similar to a QR decomposition
//               in that we are left with a diagonal matrix, and an upper triangular matrix, except the
//               diagonal matrix has the main and sub-diagonal still present.""""
//  Addition: This file has integrated parallelism. Any matrix operation performed is performed in 
//              parallel. This is done through combining a cache coherent matrix operation design
//              along with openMP threading. Because of the cache coherent design, we are able to implement
//              cyclical thread assignment per row without worrying about resource sharing management
//
//
//  Some tools in the code require features from c++ 17 or later, ensure you are 
//  using the correct version
//
//  to compile: g++ -std=c++17 -fopenmp svd_house_db_parallel.cpp
//  to run: ./a.out thread_num
//
//
//  Do note that the matrices generated will be a factor of the thread number squared.
/////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <omp.h>
using namespace std;

typedef vector<vector<double>> matrix;
int max_threads;

// Helper function to make a Householder vector
pair<vector<double>, double> make_house_vec(const vector<double>& x) {
    int n = x.size();
    vector<double> v(x);
    double beta = 0.0;
    double sigma = 0.0;
    v[0] = 1.0;
    //Sigma is sum of square of all points in vector 
    // execept the first element
    #pragma omp parallel for reduction(+:sigma)
    for (int i = 1; i < n; i++) {
        sigma += x[i] * x[i];
    }
    #pragma omp barrier

    if (sigma == 0.0) {
        beta = 0.0;
    } 
	else {
        //grab full magnitude
        double mu = sqrt(x[0] * x[0] + sigma);
        if (x[0] <= 0.0) {
            v[0] = x[0] - mu;
        } 
		else {
            v[0] = -sigma / (x[0] + mu);
        }
        beta = (2.0 * v[0] * v[0]) / (sigma + v[0] * v[0]);
		for (int i = 1; i < n; i++) {
            v[i] = x[i] / v[0];
        }
        // v[0] /= v[0]; //<-not super reliable, occasionally get 0.9889 like values
        v[0] = 1.0; // hard set to 1

    }
    return make_pair(v, beta);
}

// Helper function to make a full Householder matrix
matrix full_house(int n, int col, const vector<double>& v, double beta) {
    //Make a matrix
    matrix Q(n, vector<double>(n, 0.0));
    //make [i][i] = 1.0, makes an identity matrix
    for (int i = 0; i < n; i++) {
        Q[i][i] = 1.0;
    }
    //Subtract (beta * outer_product) from matrix
    #pragma omp parallel num_threads(max_threads)
    {
        int i = omp_get_thread_num() + col;
        while(i < n){
            for (int i = col; i < n; i++) {
                for (int j = col; j < n; j++) {
                    Q[i][j] -= beta * v[i-col] * v[j-col];
                    if(abs(Q[i][j]) < 1e-6){
                        Q[i][j] = 0.0;
                    }
                }
            }
            i+=max_threads;
        }
    }
    #pragma omp barrier
    return Q;
}

void print_matrix(matrix& A){
    int m = A.size();
    int n = A[0].size();
    
    cout << "[";
    for(int i = 0; i < m; i++){
        if(i != 0) cout << " ";
        cout << "[";
        for(int j = 0; j < n; j++){
            if(abs(A[i][j]) < 1e-6){
                A[i][j] = 0.0;
            }
            cout << left << setw(10) << A[i][j];
            if(j < n - 1){
                cout << " ";
            }
        }
        cout << "]";
        if(i < m-1){
            cout << endl;
        }
    }
    cout << "]" << endl;
}
matrix dot(matrix& A, matrix& B){
    int m = A.size();
    int n = B[0].size();
    vector<vector<double>> mat(m, vector<double>(n));

    #pragma omp parallel num_threads(max_threads)
    {
        int i = omp_get_thread_num();
        while(i < m){
            vector<double> row = A[i];
            for (int j = 0; j < n; j++) {
                double temp = row[j];
                vector<double> column = B[j];   
                for(int k = 0; k < n; k++){
                    mat[i][k] += temp * column[k];
                    if(abs(mat[i][k]) < 1e-6){
                        mat[i][k] = 0.0;
                    }
                }
            }
            i += max_threads; // num threads 
        }
    }
    #pragma omp barrier
    return mat;
}



void update_matrix(vector<vector<double>>& A, vector<double> v, double beta, int col) {
    int m = A.size();
    int n = A[0].size();

    int matrix_rows = m - col;
    int matrix_columns = n-col;
    vector<vector<double>> mat(matrix_rows, vector<double>(matrix_columns));
    #pragma omp parallel num_threads(max_threads)
    {   
        int i = omp_get_thread_num();
        while( i < matrix_rows){
            float temp;
            vector<double> row;
            for (int j = 0; j < matrix_rows; j++) {
                temp = -(beta * v[i] * v[j]);
                if (i == j) {
                    temp += 1.0;
                }
                
                row = A[col+j];
                for(int k = 0; k < matrix_columns; k++){
                    mat[i][k] += temp * row[col+k];
                }
            } 
            i += max_threads;//num threads
        }
    }
    #pragma omp barrier
    #pragma omp parallel num_threads(max_threads)
    {
        int i = omp_get_thread_num();
        while(i < matrix_rows){
            for(int j = 0; j < matrix_columns; j++){
                if(abs(mat[i][j]) < 1e-6){
                    mat[i][j] = 0;
                }
                A[i+col][j+col] = mat[i][j];
            }
            i += max_threads;
        }
    }
    #pragma omp barrier
}

void update_matrix_of_diag(vector<vector<double>>& A, vector<double> v, double beta, int col){
    int m = A.size();
    int n = A[0].size();

    int col_1 = col+1;
    int matrix_rows = m-col;
    int matrix_columns = n-col_1;
    vector<vector<double>> mat(matrix_rows, vector<double>(matrix_columns));
    #pragma omp parallel num_threads(max_threads)
    {
        int i = omp_get_thread_num()+col;
    
        int i_m;
        int j_m;
        int k_m;
        double temp;
        double out_temp;
    // for(int i = col; i < m; i++){
        while(i < matrix_rows){
            vector<double> row = A[i];
            i_m = i-col;
            for(int j = col_1; j < n; j++){
                temp = row[j];
                j_m = j-col_1; 
                for(int k = col_1; k < n; k++){
                    k_m = k-col_1;
                    out_temp = -(beta * v[j_m] * v[k_m]);
                    if(j==k) out_temp += 1.0; //modify according to identity matrix
                    mat[i_m][k_m] += (temp * out_temp);
                }
            }
            i+= max_threads;
        }
    }
    #pragma omp barrier
    #pragma omp parallel num_threads(max_threads)
    {
        int i = omp_get_thread_num();
        while(i < matrix_rows){
            for(int j = 0; j < matrix_columns; j++){
                if(abs(mat[i][j]) < 1e-6){
                    mat[i][j] = 0;
                }
                A[i+col][j+col+1] = mat[i][j];
            }
            i+= max_threads;
        }
    }
    #pragma omp barrier
}

void print_vector(vector<double>& vec){
    cout << " [";
    for(int i = 0; i < vec.size(); i++){
        if(abs(vec[i]) < 1e-6){
            vec[i] = 0.0;
        }
        cout << left << setw(5) << vec[i];
        if(i < vec.size()-1){
            cout << " ";
        }
    }
    cout << "]" << endl;
}


// Main function for computing the bidiagonalization of a matrix A
void house_bidiag_explicit_UV(matrix& A, matrix& U, matrix& Vt) {
    int m = A.size();
    int n = A[0].size();
    U = matrix(m, vector<double>(m, 0.0));
    Vt = matrix(n, vector<double>(n, 0.0));

    //Make U and Vt identity
    for (int i = 0; i < m; i++) {
        U[i][i] = 1.0;
    }
    for (int i = 0; i < n; i++) {
        Vt[i][i] = 1.0;
    }


    for (int col = 0; col < n; col++) {
        vector<double> colData(m - col);
        for (int i = col; i < m; i++) {
            colData[i - col] = A[i][col];
        }
        auto [v, beta] = make_house_vec(colData);
        update_matrix(A, v, beta, col);
        auto Q = full_house(m, col, v, beta);
        if (col <= n-2) {
            int row_size = (n-col) - 1; //minus to keep off diag
            vector<double> colData((n-col) - 1);
            for (int i = 1; i <= row_size; i++) { // start at one so i + col will start off the diag
                colData[i-1] = A[col][i + col]; 
            } 
            auto [v, beta] = make_house_vec(colData);
            update_matrix_of_diag(A, v, beta, col);
            auto P = full_house(n, col+1, v, beta);
            Vt = dot(P, Vt);
        }        
    }
}

matrix generate_random_matrix(int n, int m) {
    vector<vector<double>> mat(n, vector<double>(m));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            mat[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return mat;
}

// compile with g++ -std=c++17 -fopenmp svd_multi_thread.cpp -o mu.out
int main(int argc, char** argv) {
    max_threads = stoi(argv[1]);
    // int m = stoi(argv[2]);


    for(int i = 3; i < 50; i++){
        for(int j = 2; j < i; j++){
        // Example usage
        // matrix A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
        matrix A = generate_random_matrix(i, j);
        matrix U, Vt;
        long start = clock();
        house_bidiag_explicit_UV(A, U, Vt);
        start = clock() - start;
        cout << i << "x"<< j << " Time in seconds: " << (float)start/(CLOCKS_PER_SEC) << endl;
        }
    }
    return 0;
}
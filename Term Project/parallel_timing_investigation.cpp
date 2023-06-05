/////////////////////////////////////////////////////////////////////////////////////////////
//// compile with g++ -std=c++17 -fopenmp svd_multi_thread.cpp -o mu.out
//
//  Description: This program attempts to cull the overhead from using extra threads
//              during matrix operations. Typically, we see a speedup from parallelizing these
//              functions, however because of the nature of the householder bidiagonalization
//              method, our matrices iteratively shrink, resulting in more cost than gain from
//              new threads.
//
//
//  Some tools in the code require features from c++ 17 or later, ensure you are 
//  using the correct version
//
//  to compile: g++ -std=c++17 -fopenmp parallel_timing_investigation.cpp
//  to run: ./a.out thread_num
//
//
//  Do note that the matrices generated will be a factor of the thread number squared.
/////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <omp.h>
using namespace std;

typedef vector<vector<double>> matrix;
int max_threads;
void force_cache_clear(){
    const size_t bigger_than_cachesize = 10 * 1024 * 1024;
    long *p = new long[bigger_than_cachesize];
        for(int i = 0; i < bigger_than_cachesize; i++)
    {
        p[i] = rand();
    }
    delete [] p;
}
pair<vector<double>, double> make_house_vec(const vector<double>& x) {
    int n = x.size();
    vector<double> v(x);
    double beta = 0.0;
    double sigma = 0.0;
    v[0] = 1.0;
    for (int i = 1; i < n; i++) {
        sigma += x[i] * x[i];
    }
    if (sigma == 0.0) {
        beta = 0.0;
    } 
	else {
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
        v[0] = 1.0; // hard set to 1

    }
    return make_pair(v, beta);
}

// Helper function to make a full Householder matrix
matrix full_house(int n, int col, const vector<double>& v, double beta) {
    matrix Q(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        Q[i][i] = 1.0;
    }

    if( max_threads*(0.5*max_threads) > Q.size()){
        for (int i = col; i < n; i++) {
            for (int j = col; j < n; j++) {
                Q[i][j] -= beta * v[i-col] * v[j-col];
                if(abs(Q[i][j]) < 1e-6){
                    Q[i][j] = 0.0;
                }
            }
        }
    }
    else{
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
    }
    return Q;
}

matrix dot(matrix& A, matrix& B){
    int m = A.size();
    int n = B[0].size();
    vector<vector<double>> mat(m, vector<double>(n));

    if( max_threads*(0.5*max_threads) > A.size()){
        // cout << "Seq" << endl;
        for (int i = 0; i < m; i++) {
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
        }
    }
    else{
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
    }
    return mat;
}



void update_matrix(vector<vector<double>>& A, vector<double> v, double beta, int col) {
    int m = A.size();
    int n = A[0].size();

    int matrix_rows = m - col;
    int matrix_columns = n-col;
    vector<vector<double>> mat(matrix_rows, vector<double>(matrix_columns));
    if( max_threads*(0.5*max_threads) > (matrix_rows*matrix_columns)*0.5){
        for (int i = 0; i < matrix_rows; i++) {
            for (int j = 0; j < matrix_rows; j++) {
                float temp = -(beta * v[i] * v[j]);
                if (i == j) {
                    temp += 1.0;
                }
                
                vector<double> row = A[col+j];
                for(int k = 0; k < matrix_columns; k++){
                    mat[i][k] += temp * row[col+k];
                }
            } 
        }
        for(int i = 0; i < matrix_rows; i++){
            for(int j = 0; j < matrix_columns; j++){
                if(abs(mat[i][j]) < 1e-6){
                    mat[i][j] = 0;
                }
                A[i+col][j+col] = mat[i][j];
            }
        }
    }else{

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
            // for(int i = 0; i < matrix_rows; i++){
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

}

void update_matrix_of_diag(vector<vector<double>>& A, vector<double> v, double beta, int col){
    int m = A.size();
    int n = A[0].size();

    int col_1 = col+1;
    int matrix_rows = m-col;
    int matrix_columns = n-col_1;
    vector<vector<double>> mat(matrix_rows, vector<double>(matrix_columns));
    if(max_threads*(0.5*max_threads) > (matrix_rows*matrix_columns)*0.5){
        int i_m;
        int j_m;
        int k_m;
        double mod;
        double out_temp;
        for(int i = col; i < m; i++){
            vector<double> row = A[i];
            i_m = i-col;
            for(int j = col_1; j < n; j++){
                double temp = row[j];
                j_m = j-col_1; 
                for(int k = col_1; k < n; k++){
                    k_m = k-col_1;
                    out_temp = -(beta * v[j_m] * v[k_m]);
                    if(j==k) out_temp += 1.0; //modify according to identity matrix
                    mat[i_m][k_m] += (temp * out_temp);
                }
            }
        }
    }
    else{
        
        #pragma omp parallel num_threads(max_threads)
        {
            int i = omp_get_thread_num()+col;
        
            int i_m;
            int j_m;
            int k_m;
            double temp;
            double out_temp;
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
    }

    if(max_threads*(0.5*max_threads) > (matrix_rows*matrix_columns)*0.5){
        for(int i = 0; i < matrix_rows; i++){
            for(int j = 0; j < matrix_columns; j++){
                if(abs(mat[i][j]) < 1e-6){
                    mat[i][j] = 0;
                }
                A[i+col][j+col+1] = mat[i][j];
            }
        }
    }
    else{
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

        U = dot(U, Q);


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
int main(int argc, char** argv) {
    max_threads = stoi(argv[1]);
    force_cache_clear();
    long start;
    int size;
    for(int i = 3; i < 7; i++){
        for(int j = 2; j < i; j++){
            matrix A = generate_random_matrix(i*(max_threads*max_threads), j*(max_threads*max_threads));
            matrix U, Vt;
            start = clock();
            house_bidiag_explicit_UV(A, U, Vt);
            start = clock() - start;
            size = i * (max_threads*max_threads) * j * (max_threads*max_threads); 
            cout <<  i * (max_threads*max_threads) << "x"<< j * (max_threads*max_threads) << " Size: " << size << " Time in seconds: " << (float)start/(CLOCKS_PER_SEC) << endl;
        }
    }
    return 0;
}

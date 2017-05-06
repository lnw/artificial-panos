#ifndef ARRAY2D_HH
#define ARRAY2D_HH

#include <vector>
#include <iostream>
//#include <climits>
//#include <cassert>
#include <cmath>

using namespace std;

template <typename T> class array2D : public vector<T> {

public:
  int m, n; 

  array2D(int m, int n, const vector<T>& A) : vector<T>(A.begin(), A.end()), m(m), n(n) { assert(A.size()==m*n); }
  array2D(int m, int n, const T& zero = 0) : vector<T>(m*n,zero), m(m), n(n) {}

  template <typename S> array2D(const array2D<S>& A) : vector<T>(A.begin(),A.end()), m(A.m), n(A.n) {}

  T& operator()(int i, int j)       { return (*this)[i*n+j]; }
  T  operator()(int i, int j) const { return (*this)[i*n+j]; }

  void transpose(){
    int x(m); m=n; n=x;
    array2D<T> A(m,n);
    for(int i=0; i<m; i++){
      for(int j=0; j<n; j++){
        A(j,i) = (*this)(i,j);
      }
    }
    *this = A;
  }

  friend ostream& operator<<(ostream& S, const array2D& A)
  {
    vector< vector<T> > VV(A.m, vector<T>(A.n));
    for(int i=0;i<A.m;i++) 
      for(int j=0;j<A.n;j++)
        VV[i][j] = A[i*A.n+j];

    S << VV;
    return S;
  }
};

#endif

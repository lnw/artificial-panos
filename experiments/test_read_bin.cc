
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "../auxiliary.hh"
#include "../array2D.hh"

using namespace std;


int16_t endian_swap(int16_t in){
  unsigned char c[2];
  memcpy(c, &in, 2);
  return (int16_t)(c[0] << 8 | c[1]);
}

array2D<int16_t> read_tile(char const * FILENAME, const int dim){
  array2D<int16_t> A(dim,dim);
  const int size = dim*dim;
  int16_t size_test;

  ifstream ifs(FILENAME, ios::in | ios::binary);
  ifs.read(reinterpret_cast<char *>(&(A(0,0))), size * sizeof(size_test));
  ifs.close();

  for (int i=0; i<dim; i++){
    for (int j=0; j<dim; j++){
      A(i,j) = endian_swap(A(i,j));
    }
  }
  return A;
}


int main(int ac, char **av) {

  //const int size = 1201;
  const int size = 3601;
  array2D<int16_t> A = read_tile("N49E008.hgt", size);
  //A.transpose();
  cout << size;
  for (int i=0;i<size;i++) cout << " " << i+1;
  cout << endl;
  for (int i=0;i<size;i++){
    cout << size - i << " ";
    for (int j=0;j<size;j++){
      cout << A(i,j) << " ";
    }
    cout << endl;
  }

//   cout << A(0,0) << endl;
//   cout << A(0,1) << endl;
//   cout << A(0,2) << endl;
//   cout << A(0,3) << endl;
//   cout << A(0,4) << endl;
//
//   cout << A(0,30) << endl;
//   cout << A(0,31) << endl;
//   cout << A(0,32) << endl;
//   cout << A(0,33) << endl;
//   cout << A(0,34) << endl;

  return 0;
}



#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "auxiliary.hh"
#include "2Darray.hh"

using namespace std;


int16_t swap_bytes(int16_t in){
  char c[2];
  memcpy(c, &in, 2);
  return (int16_t)(c[0] << 8 | c[1]);
}



int main(int ac, char **av) {

//  const double deg2rad = M_PI/180;
//  const double rad2deg = 180/M_PI;
//  
//  const double scene_direction = 270*deg2rad; // angle in rad, east is 0
//  const double scene_width = 60*deg2rad; // angle in rad
//  const double range = 50000; // in m
// 
//  const int view_width = 1000; // pixels  
//  const int view_height = 500; // pixels  

  cout << swap_bytes(int16_t(0)) << " (0)" << endl;
  cout << swap_bytes(int16_t(1)) << " (256)" << endl;
  cout << swap_bytes(int16_t(255)) << " (65...)" << endl;



  std::vector<int16_t> myData;
  int size = 1201 * 1201;
  myData.resize(size);
  int16_t size_test;

  cout << sizeof(size_test) << endl;
  cout << size * sizeof(size_test) << endl;

  ifstream ifs("N58E005.hgt", ios::in | ios::binary);

  ifs.read(reinterpret_cast<char *>(&(myData[0])), size * sizeof(size_test));
  ifs.close();

  cout << sizeof(myData) << endl;

  for(int i=0; i<1202; i++)
    cout << swap_bytes(myData[i]) << " ";
  cout << endl;

  return 0;
}





class circ_360 {
  int ii;

public:
  circ_360(int i=0): ii((i+360)%360){}
//  circ_360(const int& i): ii(i) {}
//  circ_360& operator=(circ_360 i)
//    {
//        std::swap(n, other.n);
//        std::swap(s1, other.s1);
//        return *this;
//    }

  operator int(){return ii;}
  
  circ_360 operator*(const int s) const { return circ_360(ii*s); }
  circ_360 operator+(const circ_360& y) const { return circ_360(ii+y.ii); }
  circ_360 operator-(const circ_360& y) const { return circ_360(ii-y.ii); }
  circ_360& operator+=(const circ_360& y){ ii+=y.ii; ii=ii%360; return *this; }
  circ_360& operator-=(const circ_360& y){ ii-=y.ii; ii=(ii+360)%360; return *this; }
  circ_360& operator*=(const int s)  { ii*=s; ii=ii%360; return *this;}
  circ_360 operator-() const {circ_360 y(360-ii); return y;}

  friend std::ostream& operator<<(std::ostream &s, const circ_360& x){ s << x.ii; return s; }
};

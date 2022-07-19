
using namespace std;

// Vector data struct and its computation operations
struct Vec3D {
  double x, y, z;
  inline Vec3D(): x(0.0), y(0.0), z(0.0) {}
  inline Vec3D(double xin, double yin, double zin): x(xin), y(yin), z(zin) {}
  inline Vec3D(const Vec3D & vecin): x(vecin.x), y(vecin.y), z(vecin.z) {}
  inline double operator*(Vec3D b){ return ((this->x * b.x) + (this->y * b.y) + (this->z * b.z)); }
  inline Vec3D operator-(Vec3D b){ return Vec3D((this->x - b.x), (this->y - b.y), (this->z - b.z)); }
  inline Vec3D operator+(Vec3D b){ return Vec3D((this->x + b.x), (this->y + b.y), (this->z + b.z)); }
  inline Vec3D operator-(){ return Vec3D(-this->x, -this->y, -this->z); }
  inline Vec3D operator*(double b){ return Vec3D(this->x*b, this->y*b, this->z*b); }
  inline bool operator==(Vec3D b){ return (this->x==b.x) && (this->y==b.y) && (this->z==b.z); }
//  friend ostream &operator<<( ostream &output, const Vec3D a )
//      { output << a.x << endl << a.y << endl << a.z; return output; }
};

inline Vec3D operator*(double b, Vec3D a){ return Vec3D(a.x*b, a.y*b, a.z*b); }


inline double norm(Vec3D a){
  return sqrt(pow(a.x,2) + pow(a.y,2) + pow(a.z,2));
}

// Matrix data struct and its computation operations
struct Mat {
  Vec3D r1, r2, r3;
  inline Mat(Vec3D r1in, Vec3D r2in, Vec3D r3in): r1(r1in), r2(r2in), r3(r3in) {}
  inline Mat(double r11, double r12, double r13, double r21, double r22, double r23, double r31, double r32, double r33): r1(r11, r12, r13), r2(r21, r22, r23), r3(r31, r32, r33) {}
  inline Vec3D operator*(Vec3D b){ return Vec3D(r1*b, r2*b, r3*b); }
//  inline Vec3D operator-(Vec3D b){ return Vec3D((this->x - b.x), (this->y - b.y), (this->z - b.z)); }
//  inline Vec3D operator-(){ return Vec3D(-this->x, -this->y, -this->z); }
  inline Mat operator*(double b){ return Mat(this->r1*b, this->r2*b, this->r3*b); }
//  inline bool operator==(Vec3D b){ return (this->x==b.x) && (this->y==b.y) && (this->z==b.z); }
//  friend ostream &operator<<( ostream &output, const Mat a )
//      { output << a.r1.x << ", " << a.r1.y << ", " << a.r1.z << endl
//               << a.r2.x << ", " << a.r2.y << ", " << a.r2.z << endl
//               << a.r3.x << ", " << a.r3.y << ", " << a.r3.z; return output; }
};

const static int vec_size = sizeof(struct Vec3D);
// Union for EEPROM write and read
union VecUnion{
  Vec3D vec_var {};
  unsigned char vec_char[vec_size];
}vec_union;

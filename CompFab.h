

#ifndef voxelizer_CompFab_h
#define voxelizer_CompFab_h

#define EPSILON 1e-9

#include <cmath>

namespace CompFab
{
    
    //Data Types
    typedef struct Vec3Struct
    {
        
        Vec3Struct();
        Vec3Struct(double x, double y, double z);

        union
        {
            double m_pos[3];
            struct { double m_x,m_y,m_z; };
        };
        
        inline double & operator[](unsigned int index) { return m_pos[index]; }
        inline const double & operator[](unsigned int index) const { return m_pos[index]; }
        inline void operator+=(const Vec3Struct &a)
        {
            m_x += a.m_x;
            m_y += a.m_y;
            m_z += a.m_z;
        }
        
        void normalize();
        
    }Vec3;

    //Data Types
    typedef struct Vec3iStruct
    {
        
        Vec3iStruct();
        Vec3iStruct(double x, double y, double z);
        union
        {
            int m_pos[3];
            struct {int m_x,m_y,m_z;};
        };
        
        inline int & operator[](unsigned int index) { return m_pos[index]; }
        inline const int & operator[](unsigned int index) const { return m_pos[index]; }
        
    }Vec3i;

    //Data Types
    typedef struct Vec2fStruct
    {
        
        Vec2fStruct();
        Vec2fStruct(double x, double y);
        
        union
        {
            float m_pos[2];
            struct { float m_x,m_y; };
        };
        
        inline float & operator[](unsigned int index) { return m_pos[index]; }
        inline const float & operator[](unsigned int index) const { return m_pos[index]; }
        
    }Vec2f;

    
    //NOTE: Ray direction must be normalized
    typedef struct RayStruct
    {
        
        RayStruct();
        RayStruct(Vec3 &origin, Vec3 &direction);
        
        Vec3 m_origin;
        Vec3 m_direction;
        
    } Ray;
    
    typedef struct TriangleStruct
    {
        
        TriangleStruct(Vec3 &v1, Vec3 &v2,Vec3 &v3);
        
        Vec3 m_v1, m_v2, m_v3;
        
    }Triangle;
    
    //Some useful operations
    //Compute v1 - v2
    Vec3 operator-(const Vec3 &v1, const Vec3 &v2);
	Vec3 operator-(const Vec3 &v1, double v);
    
    Vec3 operator+(const Vec3 &v1, const Vec3 &v2);


    
    //Cross Product
    Vec3 operator%(const Vec3 &v1, const Vec3 &v2);
    
    //Dot Product
    double operator*(const Vec3 &v1, const Vec3 &v2);

	//Length of the segment between two vertices
	double  operator/(const Vec3 &v1, const Vec3 &v2);
    
    
   
}



#endif

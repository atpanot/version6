#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include "CompFab.h"
#include "Mesh.h"
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


// http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.1/
// https://www.youtube.com/watch?v=K8SyO72Kx-k
// http://stackoverflow.com/questions/1114914/add-library-to-visual-studio-2008-c-project
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PROGRAM INPUT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//  Name of the .obj file containing a triangular esh needs to be in the same folder
char* filename = "onecube.obj";

// In every axes we rotate the object rotation times uniformly
unsigned rotations = 3;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~GLOBAL VARIABLE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//  Triangle list (global)
typedef std::vector<CompFab::Triangle> TriangleList;
TriangleList g_triangleList;

//  Global: The surface of every triangle
std::vector<double> entropy;
//  Global: The surface of every triangle
std::vector<double> triangleArea;
//  Global: for every orientation the projected visible surface of the object counted on pixels
std::vector<float> projected_surface;
//  Global: for every orientation the visible surface of the object counted on square units
std::vector<float> surface;
//  Global: for every orientation maximum z_value - minimum z_value
std::vector<float> maxz;
//  Global: for every orientation histogram z_value
std::vector<float> histz;
// Global: for every orientation the length of the Silhouette(the outerior length of the object as it projects on the screen)
std::vector<float>siLen;
//  Global: the list that stores the object for visualizing opengl purposes
GLuint listName;
//  Global: Is it the first time that the scene is displayed
bool firstTime = true;
//  Counter for the total surface in square units of the object
double totalArea = 0.0;
//  Maximum projected area seen so far in pixels
double maxProjArea = 0.0;

//  Initial Width and height  of the window in pixels used in main
int WIDTH = 1000;
int HEIGHT = 1000;

//Number of bins we use for the histogram parameter 8
int bins = 10;







/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HELPER FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

//  Writes the output file with the scores for every rotation
void writecsv() {
	double angle_rotation = 360. / rotations ;
	std::ofstream file_results; 
	file_results.open("results.csv");
	file_results << "rotate z, rotate y, a1: projected visisble surface, a2: visible surface, a8: max depth, a7: hist depth, a4: silhouette length, a3: viewpoint entropy \n";
	for (unsigned z = 0; z < rotations; z++) {
		for (unsigned y = 0; y < rotations; y++) {
			file_results << angle_rotation * z << ", ";
			file_results << angle_rotation * y << ", ";
			file_results << 1.0 * projected_surface[ z*rotations + y ] / (WIDTH * HEIGHT) << ", ";
			file_results << surface[ z*rotations + y ] / totalArea << ", ";
			file_results << maxz[ z*rotations + y ] << ", ";
			file_results << histz[ z*rotations + y ] << ", ";
			file_results << siLen[ z*rotations + y ]/(HEIGHT*WIDTH) << ", ";
			file_results << entropy[ z*rotations + y ] << "\n ";

			//file_results << projected_surface[ z*rotations + y ] / maxProjArea << ", ";
			//file_results << surface[ z*rotations + y ] / totalArea << "\n";
		}
	}
	file_results.close();
}
// If the pixel i j is on the boundary
bool isOnBoundary( int i, int j) {
	if( i == 0 || j == 0 || i == WIDTH -1 || j == HEIGHT - 1) {
		return true;
	}
	return false;
}
// If the pixel i j is on the corner
bool isCorner( int i, int j) {
	if( (i == 0 && j == 0) || (i == WIDTH - 1 && j == 0) || (i == 0 && j == HEIGHT - 1) || (i == WIDTH - 1 && j == HEIGHT - 1)) {
		return true;
	}
	return false;
}

// left side	value[j][i - 1] != 1
// right side	value[j][i + 1] != 1
// up side		value[j - 1][i] != 1
// down side	value[j + 1][i] != 1 


char shape2( float** value, int i, int j ) {
	int num_touch = 0;
	if (value[j][i] != 1) {
		std:: cout<< "Why am I here\n";
	}
	if(!isOnBoundary( i, j)) {

		// touches left side
		if( value[j][i - 1] != 1 ) {
			
			// touches right side
			if ( value[j][i + 1] != 1 ) {
				
				// touches up side
				if ( value[j - 1][i] != 1 ) {
					
					// touches down side
					if ( value[j + 1][i] != 1 ) {
						
						//std::cout<<"It touches four sides!!!!\n";
						return 'o';
					}
					// does not touch down side
					else{
						//std::cout<<"It touches left right up!!!!\n";
						return 'g';
					}
				}

				// does not touch up side
				else {

					// touches down side
					if ( value[j + 1][i] != 1 ) {
						//std::cout<<"It touches left right down!!!!\n";
						return 'g';
					}

					// does not touch down side
					else {
						//std::cout<<"It touches left right!!!!\n";
						return '2';
					}
				}
			}
			// does not touch right side
			else {
				// touch up side
				if ( value[j - 1][i] != 1 ) {
					// touches down side
					if ( value[j + 1][i] != 1 ) {
						
						//std::cout<<"It touches left up and down sides!!!!\n";
						return 'g';
					}
					// does not touch down side
					else{
						//std::cout<<"It touches left and up!!!!\n";
						return 'r';
					}
				}
				// does not touch up side
				else {
					// touches down side
					if ( value[j + 1][i] != 1 ) {
						
						//std::cout<<"It touches left and down sides!!!!\n";
						return 't';
					}
					// does not touch down side
					else{
						//std::cout<<"It touches left!!!!\n";
						return 'g';
					}
				}	
			}
		}

		// does not touch left side
		else {

			// touch right side 
			if(value[j][i + 1] != 1) {

				// touch up side
				if(value[j - 1][i] != 1) {

					// touch down side
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches right up down!!!!\n";
						return 'g';
					}

					// not touch down side
					else {
						//std::cout<<"It touches right up!!!!\n";
						return 'b';
					}
				}

				// not touch up side
				else {

					// touch down side
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches right down!!!!\n";
						return 'l';
					}

					// not touch down side
					else {
						//std::cout<<"It touches right!!!!\n";
						return 'g';
					}
				}
			}

			// does not touch right side
			else
			{

				// touches up side
				if(value[j - 1][i] != 1) {

					// touches botton
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches up and bottom !!!!\n";
						return '2';
					}

					// does not touch botton
					else {
						//std::cout<<"It touches up!!!!\n";
							return 'g';
					}
				}

				// does not touch up side
				else {

					// touches bottom
					if ( value[j + 1][i] != 1  ) {
						//std::cout<<"It touches down!!!!\n";
						return 'g';
					}

					// does not touch bottom
					else {
						// std::cout<<"Touches nothing !!!!\n"
						return 'o';
					}

				}
			}
		}
	}

	// it is a border pixel
	else {

		// it is a corner pixel
		if(isCorner( i, j)) {

			// it is a top left corner pixel
			if( j == 0 && i == 0 ) {

				// touches right
				if (value[j][i + 1] != 1) {

					// touches bottom
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches right and down!!!!\n";
						return 't';
					}

					// does not touch bottom
					else {
						//std::cout<<"It touches right!!!!\n";
						return 'g';
					}
				}

				// does not touch right
				else {

					// touches bottom
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches bottom!!!!\n";
						return 'g';
					}

					// does not touch bottom
					else {
						//std::cout<<"It touches nothing!!!!\n";
						return 'o';
					}
				}
			}

			// it is a top right corner pixel
			if( j == 0 && i == WIDTH -1 ) {

				// touches left
				if (value[j][i - 1] != 1) {

					// touches bottom
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches left and down!!!!\n";
						return 't';
					}

					// does not touch bottom
					else {
						//std::cout<<"It touches left!!!!\n";
						return 'g';
					}
				}

				// does not touch left
				else {

					// touches bottom
					if(value[j + 1][i] != 1) {
						//std::cout<<"It touches bottom!!!!\n";
						return 'g';
					}

					// does not touch bottom
					else {
						//std::cout<<"It touches nothing!!!!\n";
						return 'o';
					}
				}
			}

			// it is a bottom left corner pixel
			if( j == HEIGHT - 1 && i == 0 ) {

				// touches right
				if (value[j][i + 1] != 1) {

					// touches top
					if(value[j - 1][i] != 1) {
						std::cout<<"It touches right and top!!!!\n";
						return 'l';
					}

					// does not touch top
					else {
						std::cout<<"It touches right!!!!\n";
						return 'g';
					}
				}

				// does not touch right
				else {

					// touches top
					if(value[j - 1][i] != 1) {
						std::cout<<"It touches top!!!!\n";
						return 'g';
					}

					// does not touch top
					else {
						//std::cout<<"It touches nothing!!!!\n";
						return 'o';
					}
				}
			}

			// it is a bottom right corner pixel
			if( j == HEIGHT - 1  && i == WIDTH -1 ) {

				// touches left
				if (value[j][i - 1] != 1) {

					// touches top
					if(value[j - 1][i] != 1) {
						std::cout<<"It touches left and top!!!!\n";
						return 'r';
					}

					// does not touch top
					else {
						std::cout<<"It touches left!!!!\n";
						return 'g';
					}
				}

				// does not touch left
				else {

					// touches top
					if(value[j - 1][i] != 1) {
						std::cout<<"It touches top!!!!\n";
						return 'g';
					}

					// does not touch top
					else {
						//std::cout<<"It touches nothing!!!!\n";
						return 'o';
					}
				}
			}
		}

		// it is a side pixel
		else {

			// top side
			if ( j == 0 ) {

				// touches bottom
				if (value[j + 1][i] != 1) {
						std::cout<<"It touches bottom!!!!\n";
						return 'g';
				}

				// does not touch bottom
				else {
					//std::cout<<"It touches nothing!!!!\n";
					return 'o';
				}
			}

			// bottom side 
			if ( j == HEIGHT - 1 ) {

				// touches top
				if (value[j - 1][i] != 1 ) {
						std::cout<<"It touches top!!!!\n";
						return 'g';
				}

				// does not touch top
				else {
					//std::cout<<"It touches nothing!!!!\n";
					return 'o';
				}
			}

			// left side
			if ( i == 0 ) {

				// touches right
				if (value[j][i + 1] != 1 ) {
						std::cout<<"It touches right!!!!\n";
						return 'g';
				}

				// does not dtouch right
				else {
					//std::cout<<"It touches nothing!!!!\n";
					return 'o';
				}
			}

			// right side
			if ( i == WIDTH - 1 ) {

				// touches left
				if (value[j][i - 1] != 1 ) {
						std::cout<<"It touches left!!!!\n";
						return 'g';
				}

				// does not touch left
				else {
					//std::cout<<"It touches nothing!!!!\n";
					return 'o';
				}
			}
		}
	}
	std::cout<<"You should not be here!!!!\n";
}
//  Returns the integer that is nearest to the double x
float roundf(GLdouble x)
{
	return 0.5 >= (x - floor(x)) ? floor(x + 0.5f) : ceil(x - 0.5f);
}

//  http://en.wikipedia.org/wiki/Centroid
//  Computes and returns the centroid of a triangle with vertices v1, v2, v3
CompFab::Vec3 findCentroid(const CompFab::Vec3 &v1, const CompFab::Vec3 &v2, const CompFab::Vec3 &v3)
{
	CompFab::Vec3 v4;
	v4[0] = (v1[0] + v2[0] + v3[0]) / 3;
	v4[1] = (v1[1] + v2[1] + v3[1]) / 3;
	v4[2] = (v1[2] + v2[2] + v3[2]) / 3;
	return v4;

}

//  Computes and returns the surface of a triangle with vertices v1, v2, ve in square units
double triangleSurface(const CompFab::Triangle &t)
{


	double  side1 , side2, side3;  //sides of the triangle


	//  Computing the lengths of the three sides of the triangle

	side1 = t.m_v1/t.m_v2;  //side1

	side2 = t.m_v1/t.m_v3;  //side2

	side3 = t.m_v2/t.m_v3;  //side3


	//  Computing the area of a triangle using Heron' s formula
	//  www.mathopenref.com/heronsformula.html
	//  improved stable type from en.wikipedia.org/wiki/Heron%27s_formula
	double triangle_A;
	if(side1 > side2){
		if(side3 > side1){
			//side3 > side1 > side2
			triangle_A = 0.25 * sqrt((side3 + (side1 + side2)) * (side2 - (side3 - side1)) * (side2 + (side3 - side1)) * (side3 + (side1 - side2)));
		}
		else{
			if(side3 > side2){
				//side1 > side3 > side2
				triangle_A = 0.25 * sqrt((side1 + (side3 + side2)) * (side2 - (side1 - side3)) * (side2 + (side1 - side3)) * (side1 + (side3 - side2)));
			}
			else{
				//side1 > side2 > side3
				triangle_A = 0.25 * sqrt((side1 + (side2 + side3)) * (side3 - (side1 - side2)) * (side3 + (side1 - side2)) * (side1 + (side2 - side3)));
			}
		}
	}
	else{
		if(side3 > side2){
			//side3 > side2 > side1
			triangle_A = 0.25 * sqrt((side3 + (side2 + side1)) * (side1 - (side3 - side2)) * (side1 + (side3 - side2)) * (side3 + (side2 - side1)));
		}
		else{
			if(side3 > side1){
				//side2 > side3 > side1
				triangle_A = 0.25 * sqrt((side2 + (side3 + side1)) * (side1 - (side2 - side3)) * (side1 + (side2 - side3)) * (side2 + (side3 - side1)));
			}
			else{
				//side2 > side1 > side3
				triangle_A = 0.25 * sqrt((side2 + (side1 + side3)) * (side3 - (side2 - side1)) * (side3 + (side2 - side1)) * (side2 + (side1 - side3)));
			}	
		}	
	}
	return triangle_A;
}

//  Loads the mesh from the .obj with name filename and puts it in a unit square centered in (0, 0, 0), that spans values fron -0.5 to 0.5
//  The triangles are stored in the g_triangleList, and the surface areas of the triangle sin the triangleArea vector
//  the .obj file needs to be in the same folder as the main.cpp
bool loadMesh(char *filename)
{
	g_triangleList.clear();

	Mesh *tempMesh = new Mesh(filename, true);

	CompFab::Vec3 v1, v2, v3;

	//copy triangles to global list
	for(unsigned int tri =0; tri<tempMesh->t.size(); ++tri)
	{
		//read the vertices of the triangle
		v1 = tempMesh->v[tempMesh->t[tri][0]];
		v2 = tempMesh->v[tempMesh->t[tri][1]];
		v3 = tempMesh->v[tempMesh->t[tri][2]];
		v1 = v1 - 0.5;
		v2 = v2 - 0.5;
		v3 = v3 - 0.5;

		CompFab::Triangle t = CompFab::Triangle(v1,v2,v3);

		//  add the triangle to the global list with triangles
		g_triangleList.push_back(t);


		//  compute the area of a triangle using Heron' s formula
		double triangle_A = triangleSurface(t);


		//  put the triangle area in the global triangle Area vector
		triangleArea.push_back(triangle_A);

		//  add the triangle surface area to the total area
		totalArea += triangle_A;
	}

	return true;

}



// The function is not in used
double  computeThreshold ( float * pixels_z ) {

	double max_z = 0.;
	double min_z = 1.;
	for(int j = 0; j < WIDTH * HEIGHT; j++) {

		//If the pixel has something drawn on it the depth is not the farthest away
		if(max_z < pixels_z[j] && pixels_z[j] != 1) {
			max_z = pixels_z[j];
		}
		if(min_z > pixels_z[j]) {
			min_z = pixels_z[j] ;
		}
	}
	return (max_z - min_z) * 0.0105 ;
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PARAMETER FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

//  Parameter 3  Viewpoint Entropy
void viewpointEntropy( int rotatey, int rotatez)  {
	// for the current rotation of the object initialize the total entropy
	entropy.push_back(0);
}


//  Parameter 4  Silhouette length
void computeSiLen( int rotatey, int rotatez)  {
	

	// array  that stores the z-value of every pixel
	float * pixels_z = new float [WIDTH * HEIGHT];
	glReadPixels(0,  0,  WIDTH,  HEIGHT,  GL_DEPTH_COMPONENT,  GL_FLOAT,  pixels_z);

	
	
	// From 1D[HEIGHT*WIDTH] to equivalent 2D [HEIGHT][WIDTH], j in HEIGHT i in WIDTH pixel
	float **tdim_z = new float* [HEIGHT];
	char **rgb_z = new char* [HEIGHT];

	for(int j= 0 ; j < HEIGHT ; j++) {
		tdim_z[j] = new float[WIDTH];
		rgb_z[j] = new char[WIDTH];
	}

	// everything set to empty
	for(int j = 0 ; j < HEIGHT ; j++) {
		for(int i = 0 ; i < WIDTH ; i++) { 
			tdim_z [j][i] = pixels_z[j*WIDTH + i];	
			// initialize rgb to out-empty
			rgb_z [j][i] = 'o';		
		}
	}




	// update the value of the rgb_z according to what shape the filled pixels around them have
	for(int j = 0 ; j < HEIGHT ; j++) {
		for(int i = 0 ; i < WIDTH ; i++) { 
			// if the pixel does not have any object
			if(tdim_z[j][i] == 1){
				rgb_z [j][i] = shape2(tdim_z, i, j);
			}
		}
	}

	//std::cout<<"length "<<length<< "\n";
	bool in_line = false; // true if we are already on a continuous line
	double line_length = 0; // the length of the line counted on pixels
	char first = 'o'; // the rgb value fo the first pixel on the line
	char last = 'o'; // the rgb of the last pixel on the line
	double length2 = 0; // counter for the total silhouette length
	// for every pixel-line of the image
	for(int j = 0 ; j < HEIGHT ; j++) {
		for(int i = 0 ; i < WIDTH ; i++) { 
			// if the rgb color is not empty(something touches the pixel) and we are not already on a line initiate a new line
			if(!in_line && rgb_z[j][i] != 'o'){
				in_line = true; // we are in line
				first = rgb_z[j][i]; // set the rgb value of the first pixel of the line
				line_length++; // augment the line length by 1 pixel
				last = rgb_z[j][i]; // se the rgb value of the last pixel of the line

			}
			// if we already are in a line and there is still a pixel that touches something on the same line update the line values
			else if(in_line && rgb_z[j][i] != 'o') {
				line_length++; // augment the length line y one pixel
				last = rgb_z[j][i]; // update the rgb color of the last pixel
	
			}
			// if we were in a line and the current pixel does not touch the object that means that the line finished at the previous pixel, update the line values
			// do the same if we are at the end of the screen
			else if( (in_line && rgb_z[j][i] == 'o' )|| i == WIDTH -1 ) {
				in_line = false; // we are not any more on the line
				rgb_z[j][i] = 'o'; // MAYBE THIS NEEDS TO BE REMOVED
				// update the silhouette length ccording to the length of that line
				// if the line is greater than one pixel in length
				if (line_length > 1) {
					// set every rgb to empty of that pixel of that line, so that to not double counted on the second pass
					for( int k = 1; k <= line_length; k++) {
						rgb_z[j][ i - k ] = 'o';
					}
					// if the line starts and ends with a pixel that touch just one surface then
					if(first == 'g' && last == 'g'){
						// add length of line
						length2 += line_length;
					}
					// if only one od the endpoints of the line have one touching surface, while the remaining have two or more
					else if( first == 'g' || last == 'g') {
						// add sqrt of line
						length2 += sqrt(line_length*line_length+1.0);
					}
					// else if the first touches bottom left and the last touches right bottom
					else if( first == 't' || last == 'l') {
						// add length of line
						length2 += line_length;
					}
					// or the opposite
					else if( first == 'r' || last == 'b') {
						// add length of line
						length2 += line_length;
					}
					// we should not be here
					else{
						std::cout<<"Should not be here 3 "<<first<<" "<<last<< "\n";
					}
				}
				// else if the length is of size one to not count it 
				else { 

				}
				line_length = 0;
			}
		}
	}

	// update the variables to count the vertical lines on the screen
	in_line = false;
	line_length = 0;
	first = 'o';
	last = 'o';

	// for every color of the image
	for(int i = 0 ; i < WIDTH ; i++) {
		for(int j = 0 ; j < HEIGHT ; j++) { 
			// a new line starts
			if(!in_line && rgb_z[j][i] != 'o'){
				in_line = true;
				first = rgb_z[j][i];
				line_length++;
				last = rgb_z[j][i];

			}
			// we are already in a line
			else if(in_line && rgb_z[j][i] != 'o') {
				line_length++;
				last = rgb_z[j][i];
	
			}
			// the line finishes
			else if( (in_line && rgb_z[j][i] == 'o' )|| j == HEIGHT -1 ) {
				in_line = false;
				rgb_z[j][i] = 'o';
				// if the line length is greater than 1 update the silhouette length similarly to the horizontal case
				if (line_length > 1) {

					//std::cout<<"length: "<<line_length<<"\n";
					if(first == 'g' && last == 'g'){
						// add length of line
						length2 += line_length;
					}
					else if( first == 'g' || last == 'g') {
						// add sqrt of line
						length2 += sqrt(line_length*line_length+1.0);
					}
					else if( first == 'b' || last == 'l') {
						// add length of line
						length2 += line_length;
					}
					else if( first == 'r' || last == 't') {
						// add length of line
						length2 += line_length;
					}
					else{
						std::cout<<"Should not be here 4 "<<first<<" "<<last<< "\n";
					}
				}
				// if the line is of size less than 2
				else { 
					// if the length is 1
					if(line_length > 0){
						// if the pixel touches at 1 or 3 sides
						if( first == 'g') {
						//add one
							length2 += 1;
						}
						// if the pixel touches at 2 sides MAYBE CORRECT THE FOUR SIDES CASE AND THE TWO PARALLEL SIDE CASE
						else {
						//add sqrt(2)
							length2 += sqrt(2.0);
						}
					}
				}
				line_length = 0;
			}
		}
	}
	

	// for the current rotation of the object initialize the total silhouette length to result
	siLen.push_back(length2);



	// delete the dynamically allocated memory
	delete[] pixels_z;
	for(int j= 0 ; j < HEIGHT ; j++){
		delete []tdim_z[j];
		delete []rgb_z [j];
	}
	delete[]tdim_z;
	delete[]rgb_z;


}

//  Parameter 8 depth distribution
void computeDepthHist( int rotatey, int rotatez, int bins )  {
	// for the current rotation of the object initialize the histz value to zero 0
	histz.push_back(0);

	// array  that stores the z-value of every pixel
	float * pixels_z = new float [WIDTH * HEIGHT];
	glReadPixels(0,  0,  WIDTH,  HEIGHT,  GL_DEPTH_COMPONENT,  GL_FLOAT,  pixels_z);



	// the range of every bin
	double range_bin = 1./bins; 
	// the histogram
	double* hist = new double[bins+1];
	std::fill_n(hist, bins+1, 0);
	// the normalization factor the total amount of pixels
	int norm_f = 0;
	double total = 0;
	for(int j = 0; j < WIDTH * HEIGHT; j++) {

		// hist[(int)(ceil((pixels_z[j]*bins))-1)]++;
		// we do not want to count the ones 
		hist[(int)(floor(pixels_z[j]*bins))]++;
		
	}
	
	// compute the normalizetion factor
	for(int j = 0; j < bins; j++) {
		norm_f += hist[j];
	}
	if(norm_f == 0){
		std::cout<<"The norm factor is zero possible mistake\n";
		exit(2);
	}
	// divide by the normalization factor after compute the square value and the integral
	for(int j = 0; j < bins; j++) {
		hist[j] = hist[j] / norm_f;
		// which normalization?
		//hist[j] = hist[j] / range_bin;
		hist[j] = hist[j] * hist[j];
		total += (hist[j] * range_bin);
	}

	// finally compute the 8th parameter
	histz[rotatey + rotatez*rotations] = 1 - total;

	delete[] hist;
	delete[] pixels_z;

}

/* It computes the projected visible surface and stores the results in the vector projected_surface updates the global variable maxProjArea
returns the threshold value according to the max min depth value and computes the max depth value*/
double computeProjectedVisSur( int rotatey, int rotatez )  {

	//for the current rotation of the object initialize the total projected visible surface to zero 0
	projected_surface.push_back(0);

	//array  that stores the z-value of every pixel
	float * pixels_z = new float [WIDTH * HEIGHT];
	glReadPixels(0,  0,  WIDTH,  HEIGHT,  GL_DEPTH_COMPONENT,  GL_FLOAT,  pixels_z);

	double max_z = 0.;
	double min_z = 1.;

	for(int j = 0; j < WIDTH * HEIGHT; j++){
		//If the pixel has something drawn on it the depth is not the farthest away
		if(pixels_z[j]!= 1){
			projected_surface[rotatey + rotatez*rotations]++;
		}
		if(max_z < pixels_z[j] && pixels_z[j] != 1) {
			max_z = pixels_z[j];
		}
		if(min_z > pixels_z[j] /* && pixels_z[j] != 0*/) {
			min_z = pixels_z[j] ;
		}
	}


	maxz.push_back(0);
	maxz[rotatey + rotatez*rotations] = max_z - min_z;

	double threshold = (max_z - min_z) * 0.0105 ;
	std::cout << "threshold:" <<threshold<<"\n";
	//update the value for the maximum projected area
	maxProjArea = maxProjArea > projected_surface[rotatey + rotatez*rotations] ? maxProjArea : projected_surface[rotatey + rotatez*rotations];

	delete[] pixels_z;
	return threshold;

}



/* It computes the visible surface and stores the results in the vector surface */
void computeVisSur ( int rotatey, int rotatez, double threshold )  {


	//  for the current rotation initialize the totaql visible surface to zero 0
	surface.push_back(0);

	//  array stores the z-value of every pixel
	float * pixels_z = new float [WIDTH * HEIGHT];
	glReadPixels(0,  0,  WIDTH,  HEIGHT,  GL_DEPTH_COMPONENT,  GL_FLOAT,  pixels_z);

	//  Variables used to store the projection modelview viewport matrices
	GLdouble projection[16];
	GLdouble modelview[16];
	GLint viewport[4];

	//  Read the matrices: projecttion, modelview and viewport
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetIntegerv( GL_VIEWPORT, viewport );


	//std::ostringstream ss;
	//ss << (rotatey + rotatez*rotations);
	//std::ofstream myfile;
	//myfile.open( ("test_back_"+ ss.str() +".obj").c_str() );
	//myfile<< "o TestObj\n";

	//  For every triangle of the mesh
	for (unsigned j= 0; j< g_triangleList.size(); j++){

		CompFab::Vec3 C = findCentroid( g_triangleList[j].m_v1, g_triangleList[j].m_v2, g_triangleList[j].m_v3);  //  Find the Centroid of the triangle

		GLdouble * screen_coords = new GLdouble[3];  //  Store the pixel values

		gluProject( C.m_x, C.m_y, C.m_z, modelview, projection,viewport, &screen_coords[0], &screen_coords[1], &screen_coords[2]);

		//  Compute the subscript of the one dimensional array (the pixel_z) given the pixel on the x and y axes found by the gluProject
		int subs = (int)( roundf( screen_coords[1] ) * WIDTH + roundf (screen_coords[0] ) );

		//  If the z value of the Centroid of the triangle is close to the z value of that pixel on the screen 
		if(( abs( pixels_z[subs] - screen_coords[2] ) < threshold ) /*|| ( screen_coords[2] != 1 && pixels_z[subs] == 1 )*/ ){
			surface[rotatey + rotatez*rotations] += triangleArea[j];
			//  myfile<<"v "<<g_triangleList[j].m_v1.m_x<<" "<<g_triangleList[j].m_v1.m_y<<" "<<g_triangleList[j].m_v1.m_z<<"\n";
			//  myfile<<"v "<<g_triangleList[j].m_v2.m_x<<" "<<g_triangleList[j].m_v2.m_y<<" "<<g_triangleList[j].m_v2.m_z<<"\n";
			//  myfile<<"v "<<g_triangleList[j].m_v3.m_x<<" "<<g_triangleList[j].m_v3.m_y<<" "<<g_triangleList[j].m_v3.m_z<<"\n";
		}  //  end if the triangle is visible

		delete [] screen_coords;
	}  //  end for every triangle in the mesh

	//myfile.close();
	delete[] pixels_z;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WINDOW FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

//  Initialize the window using opengl
static void init (void)
{
	//  read the mesh and compute the area of every triangle
	loadMesh(filename);
	//  enable the depth buffer
	glEnable(GL_DEPTH_TEST);
	//  enable the depth test
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	//  the depth range is from 0 to 1 as the default values
	glDepthRange(0.0f, 1.0f);
	//  Make the opengl list
	listName = glGenLists (1);
	glNewList (listName, GL_COMPILE);
	glShadeModel(GL_SMOOTH);
	//  Add all the triangles from the mesh
	glBegin (GL_TRIANGLES);
	for (unsigned i = 0; i<g_triangleList.size(); i++){
		glColor3f (1.0, 0.0, 0.0);  /*  current color red  */
		glVertex3f (g_triangleList[i].m_v1.m_x, g_triangleList[i].m_v1.m_y, g_triangleList[i].m_v1.m_z);
		glColor3f (0.0, 1.0, 0.0);  
		glVertex3f (g_triangleList[i].m_v2.m_x, g_triangleList[i].m_v2.m_y, g_triangleList[i].m_v2.m_z);
		glColor3f (0.0, 0.0, 1.0);  
		glVertex3f (g_triangleList[i].m_v3.m_x, g_triangleList[i].m_v3.m_y, g_triangleList[i].m_v3.m_z);
	}
	glEnd ();
	glEndList ();

}

void display(void)
{	

	GLuint rotatey, rotatez;
	double angle_rotation = 360. / rotations ; 
	if(firstTime){
		for (rotatez = 0; rotatez < rotations ; rotatez++) {
			for (rotatey = 0; rotatey < rotations; rotatey++){

				//  Set the clear color to black
				glClearColor(0,0,0,0);

				//  Set the clear value of the z buffer to 1
				glClearDepth(1.0f);

				//  Clear the color and the depth buffer
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glLoadIdentity();

				//  Rotate  the model
				glRotatef(angle_rotation * rotatez,   0,  0, 1);
				glRotatef(angle_rotation * rotatey,   0,  1, 0);

				//  Draw the contents of the opengl list, that is the object
				glCallList (listName);

				glFinish();
				
				viewpointEntropy(rotatey, rotatez);
				computeSiLen( rotatey, rotatez); // Fourth parameter
				double threshold = computeProjectedVisSur( rotatey, rotatez );  // First parameter &seventh parameter
				computeVisSur( rotatey, rotatez, threshold ) ;  //  Second parameter
				computeDepthHist( rotatey, rotatez, bins ); //Eighth parameter

			}

		}
 		writecsv();
		std::cout << "Finish Computations\n";
		firstTime = false;
	}
	glFlush ();
}


void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	WIDTH = w;
	HEIGHT = h;
	//  TODO: Change the value of the firstTime to recompute for the new window size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// It always fits into that since it is resided to fit in the unit cube and given that with any rotation can be at most the length of the diameter ~1.73
	// http://en.wikipedia.org/wiki/Unit_cube
	glOrtho(-1,  1,  -1,  1,  -1,  1);
	glMatrixMode(GL_MODELVIEW); 
	// Default values for the  gluLookAt(0, 0, 0, 0, 0,-1, 0, 1, 0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	  case 27:
		  exit(0);
	}
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MAIN FUNCTION PROGRAM STARTS FROM HERE:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, char** argv)
{
	//  Initialize the window
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE|GLUT_DEPTH);
	//  Initialize window size with width pixels and height pixels
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);
	//  Initialize window with the given name and attach the functions to that 
	glutCreateWindow("5asaP");
	init ();
	glutReshapeFunc (reshape);
	glutKeyboardFunc (keyboard);
	glutDisplayFunc (display);
	glutMainLoop();



 

	return 0;
}




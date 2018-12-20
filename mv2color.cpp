#include "mv2color.hpp"
#include "colorcode.h"

#include "opencv2/opencv.hpp"

#include <iostream>
#include <stdexcept>
#include <math.h>

using namespace std;
using namespace cv;

const float Mv2Color::mvMaxRatio = 0.05;
const bool useMillbury = false;

void Mv2Color::mv2HSV(Point2f mv, float* pix, Size imageSize) 
{
	const float scale = scaleForImageSize(imageSize);	

	Point3f pol;
	// hue: 0..360°
	pol.x = 180 + atan2(-mv.y, -mv.x) * 180.0 / M_PI;

	// saturation: 0..1.0
	pol.y = sqrt((pow(mv.x, 2))+(pow(mv.y, 2)));
	pol.y /= scale;
	
	// value: 0..1.0
	// y-values in saturation are faded to black
	pol.z = (pol.y > 1.0)? 1 / pol.y: 1.0;

	if ( pol.y > 1.0 )
	{
		cout << "mv value " << mv << " rad: " << pol.y*scale << " exceeds maximum of " << scale << endl;
	}
	
	// range checks
	pol.y = (pol.y > 1.0)? 1.0: pol.y;
	pol.y = (pol.y < 0.0)? 0.0: pol.y;
	pol.z = (pol.z > 1.0)? 1.0: pol.z;
	pol.z = (pol.z < 0.0)? 0.0: pol.z;
	
	pix[0] = pol.x;
	pix[1] = pol.y;
	pix[2] = pol.z;
	
	//~ cout << mv << ":" << pol << scale << endl;
}

float Mv2Color::scaleForImageSize(Size imageSize)
{
	return round(mvMaxRatio * sqrt(pow(imageSize.width,2)+pow(imageSize.height,2)));
}

Vec3b Mv2Color::mv2BGR(Point2f mv, Size imageSize)
{
	Mat_<Vec3f> bgrMat;

	if (useMillbury)
	{
		uint8_t pix[3];
		const float scale = scaleForImageSize(imageSize);	
		
		computeColor(-mv.x/scale, -mv.y/scale, pix);

		Vec3f pixfv(pix[0], pix[1], pix[2]);
		bgrMat = Mat_<Vec3f>(pixfv);
		
		return bgrMat(0);
	}
	
	float pixf[3];
	
	// get HSV
	mv2HSV(mv, pixf, imageSize);
	Vec3f pixfv(pixf[0], pixf[1], pixf[2]);
	
	// convert to BGR
	Mat_<Vec3f> hsvMat(pixfv);
	cvtColor(hsvMat, bgrMat, CV_HSV2BGR);
	
	// scale
	bgrMat *= 255;

	return bgrMat(0);
}

void Mv2Color::test(Size size, Size imageSize)
{
	int _mvMax = (imageSize.width > imageSize.height)? imageSize.width : imageSize.height;
	_mvMax *= mvMaxRatio;
	Size mvMax(_mvMax, _mvMax);

	Mat image(size.height, size.width, CV_32FC3);

	// shift mv(0,0) to center of image
	Point2f shift((size.width-1)/2.0, (size.height-1)/2.0);
	// scale 
	Point2f scale((2.0 * sqrt(2) * mvMax.width) / (size.width-1), (2.0 * sqrt(2) * mvMax.height) / (size.height-1));
	cout << "shift: " << shift << "\tscale: " << scale << "\tmv_max: " << imageSize << endl;

	for (int i = 0; i < size.width; i++) 
	{		
		for (int j = 0; j < size.height; j++) 
		{
			Point2f mv(i,j);
			mv -= shift;
			mv.x *= scale.x;
			mv.y *= scale.y;
			float* pix = &(image.at<Vec3f>(j,i)[0]);

			mv2HSV(mv, pix, imageSize);
		}
	}

	// convert 32bit HSV to 32bit BGR
	Mat iBGR32(size.height, size.width, CV_32FC3);
	cvtColor(image, iBGR32, CV_HSV2BGR);

	// convert 32bit BGR to 16bit BGR
	Mat iBGR16(size.height, size.width, CV_16UC3);
	iBGR32.convertTo(iBGR16, CV_16U, 255.0*255.0);

	// convert 32bit BGR to 8bit BGR
	Mat iBGR8(size.height, size.width, CV_8UC3);
	iBGR32.convertTo(iBGR8, CV_8U, 255.0);

	// resize
	Mat iResize8(1000, 1000, CV_8UC3);
	resize(iBGR8, iResize8, iResize8.size(), 0, 0, INTER_NEAREST);

	Mat iResize16(1000, 1000, CV_16UC3);
	resize(iBGR16, iResize16, iResize16.size(), 0, 0, INTER_NEAREST);

	// save as png
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	try {
		imwrite("mv2color_chart8.png", iResize8, compression_params);
		imwrite("mv2color_chart16.png", iResize16, compression_params);
	}
	catch (runtime_error& ex) {
		cerr << "Exception converting image to PNG format: " << ex.what() << endl;
	}	
}


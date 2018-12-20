#include "oflowFB.hpp"

#include "colorcode.h"
#include "mv2color.hpp"

#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <fstream>

using namespace cv;
using namespace std;

OflowFB::OflowFB(VideoCapture cap, int winSize)
	: winSize(winSize)
{
	this->cap = cap;
	cout << "\twindows size: " << winSize << "x" << winSize << endl;
}

int OflowFB::calc(bool skip)
{
	Mat frame;
	cap >> frame;
	if (frame.empty()) 
	{
		return -1;
	}

	cvtColor(frame, gray, CV_BGR2GRAY);
	
	if ( prevGray.data && !skip )
	{
		calcOpticalFlowFarneback(prevGray, gray, flow, 0.5, 3, winSize, 3, 5, 1.1, 0);
	}
	
	swap(prevGray, gray);
	
	return flow.rows * flow.cols;
}

int OflowFB::process(string filename, int frames, bool analyse)
{
	ofstream extract;

	mvCount = flow.rows * flow.cols;
	mvCountZero = 0;
	mvSum = Point2f(0,0);

	if ( !filename.empty() )
	{
		ostringstream target;
		target << filename << ".dat";
		extract.open(target.str().c_str(), ofstream::out);
	}

	// analyse and extract
	if ( analyse || extract.is_open() )
	{			
		for ( int i=0; i< flow.cols; i++ )
		//~ for ( int i=0; i< flow.cols; i += winSize )
		{
			for ( int j=0; j< flow.rows; j++ )
			//~ for ( int j=0; j< flow.rows; j += winSize )
			{
				const Point2f& mv = -flow.at<Point2f>(j,i);
				if ( analyse )
				{
					mvSum += mv;
					if ( mv == Point2f(0,0) ) mvCountZero++;
				}
				if ( extract.is_open() )
				{
					extract << i << "\t" << j << "\t" << mv.x << "\t" << mv.y << endl;
				}
			}
		}
	}
	extract.close();
		
	// extract flow and gray image
	if ( !filename.empty() && flow.data )
	{
		Mat image(flow.rows, flow.cols, CV_8UC3, Scalar(0,0,0));

		for (int i=0; i< flow.cols; i++) 
		{
			for (int j=0; j< flow.rows; j++)
			{
				const Point2f& mv = -flow.at<Point2f>(j,i);
				image.at<Vec3b>(j,i) = Mv2Color::mv2BGR(mv, gray.size());
				//~ uchar* pix = &(image.at<Vec3b>(i,j)[0]);
				//~ computeColor(mv.x, mv.y, pix);
			}
		}

		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(9);
		try {
			ostringstream target;
			target << filename << "_" << Mv2Color::scaleForImageSize(gray.size()) << "_flow.png";
			Rect cropped(0,0,gray.cols,gray.rows);
			imwrite(target.str().c_str(), image, compression_params);
			imwrite(filename + "_gray.png", gray, compression_params);
		}
		catch (runtime_error& ex) {
			fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
			return EXIT_FAILURE;
		}
	}

    return EXIT_SUCCESS;
}

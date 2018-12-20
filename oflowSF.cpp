#include "oflowSF.hpp"

#include <cstdlib>
#include <exception>
#include <fstream>

using namespace cv;
using namespace std;

OflowSF::OflowSF(VideoCapture cap, int blockSize, int searchArea)
	: blockSize(blockSize)
	, searchArea(searchArea)
{
	this->cap = cap;
	
	cout << "\tblock size: " << blockSize << "x" << blockSize << endl;
	cout << "\tsearch area: " << searchArea << "x" << searchArea << endl;
}

int OflowSF::calc(bool skip)
{
	Mat frame;
	cap >> frame;
	if (frame.empty()) 
	{
		return -1;
	}

	frame.copyTo(gray);
	
	if ( prevGray.data && !skip )
	{
		calcOpticalFlowSF(prevGray, gray, flow, 1, blockSize, searchArea);
	}
	
	swap(prevGray, gray);
	
	return flow.rows * flow.cols;
}

int OflowSF::process(string filename, int frames, bool analyse)
{
	mvCountZero = 0;
	mvSum = Point2f(0,0);

	ofstream extract;

	if (!filename.empty()) 
	{
		extract.open(filename.c_str(), ofstream::out);
	}

	if (analyse || extract.is_open())
	{
		Mat cflow;
		for (int i=0; i< flow.cols; i++) 
		{
			for (int j=0; j< flow.rows; j++)
			{
				const Point2f& mv = flow.at<Point2f>(j,i);
				if (analyse)
				{
					mvCount++;
					mvSum += mv;
					if ((mv.x == 0.0) && (mv.y == 00)) mvCountZero++;
				}
				if (extract.is_open()) 
				{
					extract << i << "\t" << j << "\t" << mv.x << "\t" << mv.y << endl;
				}
			}
		}
	}
	extract.close();


	return EXIT_SUCCESS;
}

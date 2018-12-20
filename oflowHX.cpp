#include "oflowHX.hpp"

#include "mv2color.hpp"
#include "colorcode.h"

#include <stdexcept>
#include <fstream>

using namespace cv;
using namespace std;

const int mbscale = 16;

OflowHX::OflowHX(VideoCapture cap, int blockSize)
	: blockSize(blockSize)
{
	this->cap = cap;
	float capWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	float capHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	this->velSize = Size(ceil(capWidth / blockSize), ceil(capHeight / blockSize));

	cout << "\tblock size: " << blockSize << "x" << blockSize << endl;
	cout << "\tflow size: " << velSize << endl;
	cout << "\tINFO: this algorithm expects motion vector file" << endl;
}

int OflowHX::calc(bool skip)
{
	Mat frame;
	cap >> frame;
	if (frame.empty()) 
	{
		return -1;
	}

	cvtColor(frame, gray, CV_BGR2GRAY);

	return EXIT_SUCCESS;
}

int OflowHX::process(string filename, int frames, bool analyse)
{
	mvCount = 0;
	mvCountZero = 0;
	mvSum = Point2f(0,0);

	ifstream extract;

	if ( !filename.empty() )
	{
		ostringstream target;
		target << filename << ".dat";
		extract.open(target.str().c_str(), ifstream::in);
		
		if ( !extract.is_open() )
		{
			cout << "failed reading: " << target.str() << endl;
			return EXIT_FAILURE;
		}
	}

	if ( analyse && extract.is_open() )
	{
		Mat image(velSize.height, velSize.width, CV_8UC3, Scalar(255,255,255));
		
		string line;
		while ( getline(extract, line) )
		{
			int i,j;
			Point2f mv;

			stringstream(line) >> i >> j >> mv.x >> mv.y;
			i /= mbscale;
			j /= mbscale;

			if ( analyse )
			{
				mvCount++;
				mvSum += mv;
				if ( mv == Point2f(0,0) ) mvCountZero++;
			}
			
			if ( (i >= 0) && (i < velSize.width) && (j >= 0) && (j < velSize.height) )
			{
				image.at<Vec3b>(j,i) = Mv2Color::mv2BGR(mv, gray.size());
			}
			else 
			{
				cout << "invalid index: " << i << ":" << j << endl;
			}
		}
		
		Mat iResize(velSize.height*blockSize, velSize.width*blockSize, CV_8UC3);
		resize(image, iResize, iResize.size(), 0, 0, INTER_NEAREST);
				
		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(9);

		try {
			ostringstream target;
			target << filename << "_" << Mv2Color::scaleForImageSize(gray.size()) << "_flow.png";
			Rect cropped(0,0,gray.cols,gray.rows);
			imwrite(target.str().c_str(), iResize(cropped), compression_params);
			//~ imwrite(filename + "_flow.png", iResize(cropped), compression_params);
			imwrite(filename + "_grey.png", gray, compression_params);
		}
		catch (runtime_error& ex) {
			cerr << "Exception converting image to PNG format: " << ex.what() << endl;
			return EXIT_FAILURE;
		}
	}
	extract.close();

	return EXIT_SUCCESS;
}

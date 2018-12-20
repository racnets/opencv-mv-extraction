#include "oflowBM.hpp"
#include "colorcode.h"
#include "mv2color.hpp"

#include <stdexcept>
#include <fstream>

using namespace cv;
using namespace std;

OflowBM::OflowBM(VideoCapture cap, int blockSize, int shiftSize, float searchRadius)
	: blockSize(blockSize)
	, shiftSize(shiftSize)
{
	this->cap = cap;
	int capWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int capHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	// as per https://github.com/opencv/opencv/blob/2.4/modules/legacy/src/optflowbm.cpp:78
	this->velSize = Size((capWidth - blockSize + shiftSize) / shiftSize, (capHeight - blockSize + shiftSize) / shiftSize);

	if (searchRadius <= 0)
	{
		//set to max
		this->searchArea = Size(capWidth,capHeight);
	}
	else if (searchRadius < 1.0)
	{
		//set to fraction
		this->searchArea = Size(capWidth*searchRadius, capHeight*searchRadius);
	}
	else 
	{
		// set direct
		this->searchArea = Size(searchRadius, searchRadius);
	}
	
	cout << "\tblock size: " << blockSize << "x" << blockSize<< endl;
	cout << "\tshift size: " << shiftSize << "x" << shiftSize << endl;
	cout << "\tsearch area: " << searchArea << endl;
	cout << "\tvector map size: " << velSize << endl;
	
}

int OflowBM::calc(bool skip)
{
	if ( !cap.isOpened() )
	{
		return -1;
	} 

	Mat frame;
	cap >> frame;
	if ( frame.empty() )
	{
		return -1;
	}

	// split channels
	if (gray.empty()) {
		cvtColor(frame, gray, CV_BGR2GRAY);
		prevGray = gray.clone();
	} else {
		prevGray = gray.clone();
		cvtColor(frame, gray, CV_BGR2GRAY);			
	}

	velX = Mat::zeros(velSize, CV_32FC1);
	velY = Mat::zeros(velSize, CV_32FC1);

	// convertion for legacy call
	IplImage _curr = gray;
	IplImage _prev = prevGray;
	CvMat cvvelx = velX;
	CvMat cvvely = velY;

	cvCalcOpticalFlowBM(&_curr, &_prev, cvSize(blockSize, blockSize), cvSize(shiftSize, shiftSize), searchArea, 0, &cvvelx, &cvvely);

	return velSize.height * velSize.width;
}

int OflowBM::process(string filename, int frames, bool analyse)
{
	mvCount = velSize.height * velSize.width;
	mvCountZero = 0;
	mvSum = Point2f(0,0);

	ofstream extract;

	if (!filename.empty()) 
	{
		ostringstream target;
		target << filename << ".dat";
		extract.open(target.str().c_str(), ofstream::out);
	}

	if (analyse || extract.is_open())
	{
		for (int i=0; i< velSize.width; i++) 
		{
			for (int j=0; j< velSize.height; j++) 
			{
				const Point2f mv(velX.at<float>(j,i), velY.at<float>(j,i));
				//~ const Point2f mv(velX.at<float>(j,i), velY.at<float>(j,i));
				if (analyse) 
				{				
					mvSum += mv;
					
					if ( mv == Point2f(0,0) ) mvCountZero++;
					if ( abs(mv.x) >= searchArea.width || abs(mv.y) >= searchArea.height )
					{
						cout << "mv " << mv << "at: " << Point2f(i,j) << " is in saturation! Increase search radius!" << endl;
					}
				}
				if (extract.is_open()) 
				{
					extract << i*shiftSize << "\t" << j*shiftSize << "\t" << mv.x << "\t" << mv.y << endl;
				}
			}
		}
	}
	extract.close();

	// extract flow and gray image
	if ( !filename.empty() )
	{
		Mat image(velSize.height, velSize.width, CV_8UC3, Scalar(0,0,0));

		for (int i=0; i< velSize.width; i++) 
		{
			for (int j=0; j< velSize.height; j++)
			{				
				//~ uchar* pix = &(image.at<Vec3b>(j,i)[0]);
				//~ computeColor(velX.at<float>(j,i), velY.at<float>(j,i), pix);
				//~ const Point2f mv(-velX.at<float>(j,i), -velY.at<float>(j,i));
				const Point2f mv(velX.at<float>(j,i), velY.at<float>(j,i));
				image.at<Vec3b>(j,i) = Mv2Color::mv2BGR(mv, gray.size());
			}
		}

		Mat iResize(gray.rows, gray.cols, CV_8UC3);
		resize(image, iResize, iResize.size(), 0, 0, INTER_NEAREST);

		//~ Mat iResizeInt(gray.rows, gray.cols, CV_8UC3);
		//~ resize(image, iResizeInt, iResizeInt.size(), 0, 0, INTER_LINEAR);

		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(9);
		try {
			ostringstream target;
			target << filename << "_" << Mv2Color::scaleForImageSize(gray.size()) << "_flow.png";
			imwrite(target.str().c_str(), iResize, compression_params);
			//~ imwrite(filename + "_flowI.png", iResizeInt, compression_params);
			imwrite(filename + "_grey.png", gray, compression_params);
		}
		catch (runtime_error& ex) {
			fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

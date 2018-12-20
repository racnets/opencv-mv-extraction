#include "oflowHS.hpp"

#include "colorcode.h"
#include "mv2color.hpp"

#include <cstdlib>
#include <stdexcept>
#include <fstream>

using namespace cv;
using namespace std;

const TermCriteria OflowHS::termcrit(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 10, 0.0001);

OflowHS::OflowHS(VideoCapture cap)
{
	this->cap = cap;
	this->velSize = Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
}

int OflowHS::calc(bool skip)
{
	Mat frame;
	cap >> frame;
	if (frame.empty()) 
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

	velX = Mat::zeros(velSize.height, velSize.width, CV_32FC1);
	velY = Mat::zeros(velSize.height, velSize.width, CV_32FC1);

	// legacy
	IplImage _curr = gray;
	IplImage _prev = prevGray;
	CvMat cvvelx = velX;
	CvMat cvvely = velY;

	cvCalcOpticalFlowHS(&_prev, &_curr, 0, &cvvelx, &cvvely, 1.0, termcrit);

	return velSize.height * velSize.width;
}

int OflowHS::process(string filename, int frames, bool analyse)
{
	ofstream extract;
	
	mvCount = velSize.width * velSize.height;
	mvCountZero = 0;
	mvSum = Point2f(0,0);

	if ( !filename.empty() )
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
				const Point2f mv(-velX.at<float>(j,i), -velY.at<float>(j,i));				
				if (analyse) 
				{
					mvSum += mv;
					if ( mv == Point2f(0,0) ) mvCountZero++;
				}
				if (extract.is_open()) 
				{
					extract << i << "\t" << j << "\t" << mv.x << "\t" << mv.y << endl;
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
			const Point2f mv(-velX.at<float>(j,i), -velY.at<float>(j,i));
				image.at<Vec3b>(j,i) = Mv2Color::mv2BGR(mv, gray.size());
			}
		}

		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(9);
		try {
			ostringstream target;
			target << filename << "_" << Mv2Color::scaleForImageSize(gray.size()) << "_flow.png";
			imwrite(target.str().c_str(), image, compression_params);
			imwrite(filename + "_grey.png", gray, compression_params);
		}
		catch (runtime_error& ex) {
			fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
			return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}

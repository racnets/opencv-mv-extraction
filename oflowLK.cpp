#include "oflowLK.hpp"
#include "colorcode.h"
#include "mv2color.hpp"

#include <cstdlib>
#include <stdexcept>
#include <fstream>

using namespace cv;
using namespace std;

const TermCriteria OflowLK::termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03);

OflowLK::OflowLK(VideoCapture cap, int vectorSize, int winSize, int subWinSize, double reInitThreshold)
	: reInitThreshold(reInitThreshold)
	, needToInit(true)
{
	this->cap = cap;
	this->winSize = Size(winSize, winSize);
	this->subPixWinSize = Size(subWinSize,subWinSize);

	int capWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int capHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	maxVectors = ceil(capWidth / vectorSize) * ceil(capHeight / vectorSize);

	cout << "\tmax vector count: " << maxVectors << endl;
	cout << "\twindows size: " << winSize << "x" << winSize << endl;
	cout << "\tsub window size: " << subWinSize << "x" << subWinSize << endl;
	cout << "\tre-initialize features if lower then " << reInitThreshold << " remaining features" << endl;
}

int OflowLK::calc(bool skip)
{
	Mat frame, image;
	cap >> frame;
	if (frame.empty()) 
	{
		return -1;
	}

	frame.copyTo(image);
	cvtColor(image, gray, CV_BGR2GRAY);
	
	if(needToInit)
	{
		// automatic initialization
		goodFeaturesToTrack(gray, points[1], maxVectors, 0.01, 10, Mat());
		if(!points[1].empty())
		{
			cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
			needToInit = false;
			minFeatureThreshold = points[1].size() * reInitThreshold;
		}		

		// save for mv-calculation
		points[2] = points[1];
		points[0] = points[1];
	} 
	else if(!points[0].empty())
	{
		vector<float> err;
		vector<uchar> status;

		// first frame
		if(prevGray.empty())
			gray.copyTo(prevGray);
		
		// calculate
		calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 3, termcrit, 0, 0.001);
		
		// save for mv-calculation
		points[2] = points[1];
		
		// resize points - keep only re-found features 
		size_t i, k;
		for( i = k = 0; i < points[1].size(); i++ )
		{
			if(!status[i])
				continue;

			points[1][k++] = points[1][i];
		}
		points[1].resize(k);
		
		if (points[1].size() < minFeatureThreshold)
			needToInit = true;
	}
	
	swap(points[1], points[0]);
	swap(prevGray, gray);
	
	return points[1].size();
}

int OflowLK::process(string filename, int frames, bool analyse)
{
	mvCount = 0;
	mvCountZero = 0;
	mvSum = Point2f(0,0);

	ofstream extract;
	
	if ( !filename.empty() )
	{
		ostringstream target;
		target << filename << ".dat";
		extract.open(target.str().c_str(), ofstream::out);
	}

	if (analyse || extract.is_open())
	{
		for( size_t i = 0; i < points[1].size(); i++ )
		{
			Point2f mv = points[1][i] - points[2][i];
			if (analyse)
			{
				mvSum += mv;
				if ((mv.x == 0.0) && (mv.y == 0.0)) mvCountZero++;
			}
			if (extract.is_open()) 
			{
				extract << points[1][i].x << "\t" << points[1][i].y << "\t" << mv.x << "\t" << mv.y << endl;
			}
		}
		mvCount = points[1].size();
	}
	extract.close();

	// extract flow and gray image
	if ( !filename.empty() && !points[1].empty() && !points[2].empty() )
	{
		Mat image(prevGray.rows, prevGray.cols, CV_8UC3, Scalar(255,255,255));
		Mat mvs(prevGray.rows, prevGray.cols, CV_32FC3, Scalar(0.0, 0.0, 0.0));

		for( size_t i = 0; i < points[1].size(); i++ )
		{
			if ( (points[1][i].x < 0) || (points[1][i].y < 0) ) 
			{
				// don't draw pixels outside the frame border
				continue;
			}

			Point2f mv = points[1][i] - points[2][i];
			mvs.at<Vec3f>(points[1][i].y, points[1][i].x)[0] += mv.x;
			mvs.at<Vec3f>(points[1][i].y, points[1][i].x)[1] += mv.y;
			mvs.at<Vec3f>(points[1][i].y, points[1][i].x)[2]++;

			//~ uchar* pix = &(image.at<Vec3b>( points[1][i].y, points[1][i].x )[0]);
			//~ computeColor(mv.x, mv.y, pix);
			image.at<Vec3b>( points[1][i].y, points[1][i].x ) = Mv2Color::mv2BGR(mv, gray.size());

		}
		
		// flow for window size
		Mat flowI(prevGray.rows, prevGray.cols, CV_8UC3, Scalar(255,255,255));

		for (int i=0; i < flowI.cols; i++) 
		{
			for (int j=0; j < flowI.rows; j++)
			{
				Point3f mvSum(0.0, 0.0, 0.0);
				for (int x = i-winSize.width/2; x < i+winSize.width/2; x++) 
				{
					for (int y = j-winSize.height/2; y < j+winSize.height/2; y++)
					{
						if ( (x > 0) && (x < flowI.cols) && (y > 0) && (y < flowI.rows) )
						{
							mvSum += (Point3f)mvs.at<Vec3f>(y, x);
						}
					}
				}
				if (mvSum.z > 1)
				{
					mvSum.x /= mvSum.z;
					mvSum.y /= mvSum.z;
				}

				//~ uchar* pix = &(flowI.at<Vec3b>(i, j)[0]);
				//~ computeColor(mvSum.x, mvSum.y, pix);
				flowI.at<Vec3b>(j,i) = Mv2Color::mv2BGR(Point2f(mvSum.x,mvSum.y), prevGray.size());
			}
		}

		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(9);
		try {
			ostringstream target;
			target << filename << "_" << Mv2Color::scaleForImageSize(gray.size()) << "_flow.png";
			imwrite(target.str().c_str(), flowI, compression_params);
			imwrite(filename + "_flowI.png", image, compression_params);
			imwrite(filename + "_grey.png", prevGray, compression_params);
		}
		catch (runtime_error& ex) {
			fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

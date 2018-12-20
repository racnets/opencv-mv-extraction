/*
 * OflowFB.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  based strongly on https://github.com/kipr/opencv/blob/master/samples/cpp/fback.cpp
 * 
 *  Author: racnets
 */

#ifndef OFLOW_FB_HPP_
#define OFLOW_FB_HPP_

#include "oflow.hpp"

#include "opencv2/opencv.hpp"

class OflowFB : public Oflow {
	private:
		int winSize;

	public:
		OflowFB(cv::VideoCapture cap, int winSize = 15);
		int calc(bool skip = false);
		int process(std::string filename, int frames, bool analyse = false);
};

#endif /* OFLOW_FB_HPP_ */

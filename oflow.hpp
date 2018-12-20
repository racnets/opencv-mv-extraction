/*
 * oflow.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  Author: racnets
 */

#ifndef OFLOW_HPP_
#define OFLOW_HPP_

#include "opencv2/opencv.hpp"

#include <string>

class Oflow {
	protected:
		cv::VideoCapture cap;
		cv::Mat gray, prevGray, flow;

		int mvCount, mvCountZero;
		cv::Point2f mvSum;
	public:
		virtual int calc(bool skip) = 0;
		virtual int process(std::string filename, int frames, bool analyse) = 0;
		int log(int frames);
};

#endif /* OFLOW_HPP_ */

/*
 * oflowLK.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  based strongly on https://github.com/kipr/opencv/blob/master/samples/cpp/lkdemo.cpp
 * 
 *  Author: racnets
 */

#ifndef OFLOW_LK_HPP_
#define OFLOW_LK_HPP_

#include "oflow.hpp"

#include "opencv2/opencv.hpp"

class OflowLK : public Oflow {
	private:
		static const cv::TermCriteria termcrit;
	
		cv::Size winSize;
		cv::Size subPixWinSize;
		int maxVectors;
		double reInitThreshold;
		size_t minFeatureThreshold;
		bool needToInit;
		cv::vector<cv::Point2f> points[3];

	public:
		OflowLK(cv::VideoCapture cap, int maxVectors, int winSize, int subWinSize = 10, double reInitThreshold = 0.5);
		int calc(bool skip = false);
		int process(std::string filename, int frames, bool analyse = false);
};

#endif /* OFLOW_LK_HPP_ */

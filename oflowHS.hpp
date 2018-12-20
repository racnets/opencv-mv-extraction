/*
 * oflowLK.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  Author: racnets
 */

#ifndef OFLOW_HS_HPP_
#define OFLOW_HS_HPP_

#include "oflow.hpp"

#include "opencv2/legacy/legacy.hpp"
#include "opencv2/opencv.hpp"

class OflowHS : public Oflow {
	private:
		static const cv::TermCriteria termcrit;
	
		cv::Size velSize;
		cv::Mat velX, velY;

	public:
		OflowHS(cv::VideoCapture cap);
		int calc(bool skip = false);
		int process(std::string filename, int frames, bool analyse = false);
};

#endif /* OFLOW_HS_HPP_ */

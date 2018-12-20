/*
 * oflowHX.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  Author: racnets
 */

#ifndef OFLOW_HX_HPP_
#define OFLOW_HX_HPP_

#include "oflow.hpp"

#include "opencv2/legacy/legacy.hpp"
#include "opencv2/opencv.hpp"

class OflowHX : public Oflow {
	private:
		int blockSize;

		cv::Size velSize;
		cv::Mat velX, velY;

	public:
		OflowHX(cv::VideoCapture cap, int blockSize);
		int calc(bool skip = false);
		int process(std::string filename, int frames, bool analyse = false);
};

#endif /* OFLOW_HX_HPP_ */

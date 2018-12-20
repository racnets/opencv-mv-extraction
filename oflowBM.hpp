/*
 * oflowLK.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  Author: racnets
 */

#ifndef OFLOW_BM_HPP_
#define OFLOW_BM_HPP_

#include "oflow.hpp"

#include "opencv2/legacy/legacy.hpp"
#include "opencv2/opencv.hpp"

class OflowBM : public Oflow {
	private:
		int blockSize, shiftSize;

		cv::Size velSize, searchArea;
		cv::Mat velX, velY;

	public:
		OflowBM(cv::VideoCapture cap, int blockSize, int shiftSize, float searchArea);
		int calc(bool skip = false);
		int process(std::string filename, int frames, bool analyse = false);
};

#endif /* OFLOW_BM_HPP_ */

/*
 * OflowSF.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  based strongly on https://github.com/kipr/opencv/blob/master/samples/cpp/simpleflow_demo.cpp
 * 
 *  Author: racnets
 */

#ifndef OFLOW_SF_HPP_
#define OFLOW_SF_HPP_

#include "oflow.hpp"

#include "opencv2/opencv.hpp"

class OflowSF : public Oflow {
	private:
		int blockSize, searchArea;

	public:
		OflowSF(cv::VideoCapture cap, int blockSize = 2, int searchArea = 4);
		int calc(bool skip = false);
		int process(std::string filename, int frames, bool analyse = false);
};

#endif /* OFLOW_SF_HPP_ */

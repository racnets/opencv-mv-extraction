/*
 * oflow.hpp
 *
 *  Created on: 22.11.2018
 *	
 *  Author: racnets
 */

#ifndef MV2COLOR_HPP_
#define MV2COLOR_HPP_

#include "opencv2/opencv.hpp"

#include <string>

class Mv2Color {
	private:
		static const float mvMaxRatio;
	public:
		static void mv2HSV(cv::Point2f mv, float* pix, cv::Size imageSize);
		static cv::Vec3b mv2BGR(cv::Point2f mv, cv::Size imageSize);
		static void test(cv::Size size, cv::Size imageSize);
		static float scaleForImageSize(cv::Size imageSize);

};

#endif /* MV2COLOR_HPP_ */

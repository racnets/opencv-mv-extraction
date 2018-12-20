/*
 * main.cpp
 *
 * Created on:  27.10.2018
 * Last edited: 27.10.2018
 *
 * Author: racnets
 */
#include "oflow.hpp"
#include "oflowBM.hpp"
#include "oflowLK.hpp"
#include "oflowHS.hpp"
#include "oflowFB.hpp"
#include "oflowSF.hpp"
#include "oflowHX.hpp"
#include "mv2color.hpp"

#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <string>
#include <sys/stat.h>  //mkdir

#include "opencv2/opencv.hpp"
#include "opencv2/legacy/legacy.hpp"

#include "logger.h"

using namespace cv;
using namespace std;

const char *srcFilename = NULL;
const char *logFilename = NULL;
const char *extractDirName = NULL;

enum Algos { BM, HS, LK, FB, SF, HX, TP};
Algos algo = BM;
int verbose = 0;
int analyse = 0;
int extract = 0;
int block_size = 16;
int shift_size = 16;
float search_area = 7;
int skip_frames = 0;
int trunc_frames = INT_MAX;


static void print_usage(const char *prog){
	cout << "Usage: " << prog << " -i INPUT [-f FILE] aso" << endl <<
	" general" << endl <<
	"  -i IN     --input IN       \"pipe:N\" for numbered piped input" << endl <<
	"  -f FILE   --file FILE      log file to write to" << endl <<
	"  -k N      --seek N         seek/skip N frames " << endl <<
	"  -t N      --trunc N        truncate after N frames " << endl <<
	"  -a        --analyse        analyse the motion data" << endl <<
	"  -e DIR    --extract DIR    extract the motion vector data for each frame to directory\n"
	"  -c ALGO   --compute ALGO   analyse using given algorithm (default: BM)" << endl <<
	"  -v        --verbose        be verbose" << endl <<
	"  -b        --block          block size (default: " << block_size << ")" << endl <<
	"  -d        --shift          shift size (default: " << shift_size << ")" << endl <<
	"  -s        --search         search area (default; " << search_area << ")" << endl;
	exit(EXIT_SUCCESS);
}

static void parse_opts(int argc, char* argv[])
{
	//forbid error message
	opterr=0;
	
	static const struct option lopts[] = {
		{ "input",   required_argument, 0, 'i' },
		{ "file",    required_argument, 0, 'f' },
		{ "seek",    required_argument, 0, 'k' },
		{ "trunc",   required_argument, 0, 't' },
		{ "block",   required_argument, 0, 'b' },
		{ "compute", required_argument, 0, 'c' },
		{ "extract", required_argument, 0, 'e' },
		{ "shift",   required_argument, 0, 'd' },
		{ "search",  required_argument, 0, 's' },
		{ "analyse", no_argument,       0, 'a' },
		{ "verbose", no_argument,       0, 'v' },
		{ NULL, 0, 0, 0 },
	};
	int opt;
	while ((opt = getopt_long(argc, argv, "i:f:k:b:c:d:e:s:t:hav", lopts, NULL)) != -1) {
		switch (opt) {
			case 'i':
				srcFilename = optarg;
				break;
			case 'f':
				logFilename = optarg;
				break;
			case 'k':
				skip_frames = atoi(optarg);
				break;
			case 't':
				trunc_frames = atoi(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'a':
				analyse = 1;
				break;
			case 'b':
				block_size = atoi(optarg);
				break;
			case 'c':
				if (string(optarg) == "BM") algo = BM;
				else if (string(optarg) == "HS") algo = HS;
				else if (string(optarg) == "LK") algo = LK;
				else if (string(optarg) == "FB") algo = FB;
				else if (string(optarg) == "SF") algo = SF;
				else if (string(optarg) == "HX") algo = HX;
				else if (string(optarg) == "TP") algo = TP;
				else cerr << "unrecognized parameter for \"-c\": " << optarg << endl;
				break;		
			case 'd':
				shift_size = atoi(optarg);
				break;
			case 'e':
				extract = 1;
				extractDirName = optarg;
				break;
			case 's':
				search_area = atof(optarg);
				break;
			default:
				cerr << "unrecognized argument." << endl;
				print_usage(argv[0]);
		}
	}
}

int main(int argc, char* argv[]) {
	std::cout << "opencv-mv-extraction" << std::endl;

	// argument parsing
	if (argc < 2) {
		print_usage(argv[0]);
	}
	parse_opts(argc, argv);

	if ( algo == TP )
	{
		cout << "generate mv to color conversion chart" << endl;
		Mv2Color::test(Size(block_size,block_size), Size(search_area,search_area));
		cout << "done" << endl;
		exit(EXIT_SUCCESS);				
	}
	
	// check
	if (srcFilename == NULL) {
		cout << "no input file!" << endl;
		exit(EXIT_FAILURE);
	}
	
	// setup logging
	if (setupLogger(logFilename) == EXIT_FAILURE) {
		if (logFilename != NULL) {
			cout << "error setting up log file: " << endl;
		}
	}
	
	// setup extract folder
	if (extract && (extractDirName != NULL)) {
		mkdir(extractDirName, 0777);
	}
		
	// open video file
	cout << "reading file: " << srcFilename << endl;
	VideoCapture vf(srcFilename);
	if ( !vf.isOpened() ) 
	{
		cerr << "error opening file: " << srcFilename << endl;
		exit(EXIT_FAILURE);
	}
	if ( skip_frames > 0 )
	{
		cout << "skip " << skip_frames << " frames" << endl;
	}
	if ( trunc_frames < INT_MAX )
	{
		cout << "truncate after " << trunc_frames << " frames" << endl;
	}
	
	// setup
	Oflow* oFlowAlgo;
	cout << "setup algorithm ";
	switch (algo) 
	{
		case BM:
			cout << "BM - Block-matching algorithm" << endl;
			oFlowAlgo = new OflowBM(vf, block_size, shift_size, search_area);
			cout << "done" << endl;
			break;
		case HS:
			cout << "HS - Horn–Schunck method" << endl;
			oFlowAlgo = new OflowHS(vf);
			cout << "done" << endl;
			break;
		case LK:
			cout << "LK - Lucas-Kanade method with pyramids" << endl;
			oFlowAlgo = new OflowLK(vf, shift_size, search_area);
			cout << "done" << endl;
			break;
		case FB:
			cout << "FB - Farneback’s algorithm" << endl;
			oFlowAlgo = new OflowFB(vf, shift_size);
			cout << "done" << endl;
			break;
		case SF:
			cout << "SF - SimpleFlow" << endl;
			oFlowAlgo = new OflowSF(vf, block_size, search_area);
			cout << "done" << endl;
			break;
		case HX:
			cout << "HX - H.264 Extraction" << endl;
			oFlowAlgo = new OflowHX(vf, block_size);
			cout << "done" << endl;
			break;
		default:;
			cout << "no algorithm defined/found to be used" << endl;
			exit(EXIT_SUCCESS);
	}

	float start = (float)getTickCount();
	int frames = 0;
	for (;;)
	{
		bool skip = frames < skip_frames;

		if ( oFlowAlgo->calc(skip) < 0 ) break;

		if ( !skip )
		{
			// extract, analyse
			ostringstream target;
			target << extractDirName << "/frame_" << frames;
			if ( extract ) oFlowAlgo->process(target.str(), frames, analyse);
			else if ( analyse ) oFlowAlgo->process("", frames, analyse);
		}
		
		if ( verbose || ( logFilename != NULL )) 
		{
			oFlowAlgo->log(frames);
		}
		
		// truncate
		if (frames >= trunc_frames) break;
		
		frames++;
	}
	
    float time = (getTickCount() - start) / getTickFrequency();
    cout << "got " << frames << " frames in: " << time << " seconds" << endl;
	cout << frames/time << "fps" << endl;

	vf.release();
	exit(EXIT_SUCCESS);

	//~ 
	//~ Mat frame, prev, curr;
	//~ for (;;)
	//~ {
		//~ double mvX = 0;
		//~ double mvY = 0;
		//~ int mvAmountZero = 0;
		//~ int mvAmountNotZero = 0;
		//~ if (verbose || analyse || extract) {
			//~ ofstream ofs;
			//~ if (extract) {
				//~ stringstream extractFileName;
				//~ extractFileName << extractDirName << "/frame_" << frames << ".dat";
				//~ ofs.open(extractFileName.str().c_str(), ofstream::out);
			//~ }
			//~ for (int i=0; i< velSize.width; i++) {
				//~ for (int j=0; j< velSize.height; j++) {
					//~ mvX += velx.at<float>(j,i);
					//~ mvY += vely.at<float>(j,i);
					//~ if (analyse) {
						//~ if ((mvX == 0.0) && (mvY == 0.0)) {
							//~ mvAmountZero++;
						//~ } else {
							//~ mvAmountNotZero++;
						//~ }
					//~ }
					//~ if (verbose) {
						//~ cout << i << "x" << j << ": " << velx.at<float>(j,i) << "x" << vely.at<float>(j,i) << endl;
					//~ }
					//~ if (extract) {
						//~ ofs << i << "\t" << j << "\t" << velx.at<float>(j,i) << "\t" << vely.at<float>(j,i) << endl;
					//~ }
				//~ }
			//~ }
			//~ if (verbose) {
				//~ cout << "sum: " << mvX << " x " << mvY << endl;
			//~ }
			//~ if (extract) {
				//~ ofs.close(); 
			//~ }
		//~ }
		//~ // logging 
		//~ doLogging(frames, mvAmountZero+mvAmountNotZero, mvAmountZero, mvAmountNotZero, mvX, mvY);
//~ 
//~ //		break;
//~ //              calcOpticalFlowSF(frame, frame,
//~ //                              flow,
//~ //                              3, 2, 4, 4.1, 25.5, 18, 55.0, 25.5, 0.35, 18, 55.0, 25.5, 10);
//~ //              calcOpticalFlowSF(prev, curr, _flow, 1, 2, 8);
//~ //              CvMat prev = frame;
//~ //              CvMat curr = frame;
//~ //              IplImage *prev = cvCreateImage(cvSize(vf_width, vf_height), IPL_DEPTH_8U, 3);
//~ //              IplImage *curr = cvCreateImage(cvSize(vf_width, vf_height), IPL_DEPTH_8U, 3);
//~ //              cvSetZero(prev);
//~ //              cvSetZero(curr);
//~ 
//~ 
//~ //              int rows = int(ceil (double (vf_height) / block_size));
//~ //              int cols = int(ceil (double (vf_width) / block_size));
//~ //                std::cout << "calcOpticalFlowSF : %lf sec\n" << (getTickCount() - start) / getTickFrequency() << std::endl;
//~ //		cout << "calcOpticalFlowBM : %lf sec\n" << (getTickCount() - start) / getTickFrequency() << endl;
	//~ }
    //~ time = (getTickCount() - start) / getTickFrequency();
    //~ cout << "got " << frames << " frames in: " << time << " seconds" << endl;
	//~ cout << frames/time << "fps" << endl;
//~ 
	//~ vf.release();
	//~ exit(EXIT_SUCCESS);
}

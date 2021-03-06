//
// Created by chuguanyi on 18-3-21.
//

#include <vector>
#include <iostream>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <fstream>

#include "CameraModel.h"
#include "glm.h"
#include "ModelWarpper.h"
#include "Solver.h"
#include "Preprocess.h"

using namespace std;
using namespace cv;


int main(int argc, char** argv) {

	VideoCapture video_im("E:/cat_videoorgin.avi");
	ModelWarpper cube;
	cube.LoadObj("E:/cat.obj");
	//instrinct
	Matx33d K;
	K << 832, 0, 320,
		0, 832, 240,
		0, 0, 1;
	cube.Init(K);
	double pose_parameter_tr[6] = { 6, 9, 80.1646,2.21238, -0.0546741, -0.0897579 };
	cv::Mat frame;
	cv::Mat last_frame;
	Solver solver;
	solver.init(pose_parameter_tr);
	int frame_id = 0;
	Histogram f, g;
	while (video_im.read(frame))
	{
		vector<vec3> support_points;
		vec3 ti(pose_parameter_tr);
		vec3 ri(pose_parameter_tr + 3);
		mat4 T = GetTranformation(ri, ti);
		cube.DisplayGL(T);
		auto rendered = cube.render_.getRenderedImg();
		Mat mask = rendered > 1;
		if (frame_id == 0)
			UpdateHistogram(frame, mask, f, g, true);
		else
			UpdateHistogram(last_frame, mask, f, g, false);
		Mat Segment = SegmentByHistogram(frame, f, g);

		SampleVertices(support_points, rendered, cube);
		if (support_points.empty())
			break;
		Mat distance_map;

		Preprocess(Segment, distance_map);

		solver.Optimize(pose_parameter_tr, support_points, K, distance_map);

		vec3 tj(pose_parameter_tr);
		vec3 rj(pose_parameter_tr + 3);
		frame = cube.DrawOn(frame, rj, tj, Scalar(0, 255, 0));


		imshow("result", frame);
		waitKey(1);
		frame_id++;
		last_frame = frame;
	}

	return 0;
}


#pragma once

#include <openpose/headers.hpp>

struct ImageInfo
{
	struct Node { float x, y, confidence; };

	// send
	size_t frameNumber;
	cv::Mat inputImage;
	bool needOpenposeProcess = true;

	// recieve
	cv::Mat outputImage;
	std::map<size_t, std::vector<Node>> people;
};

class OpenPoseEvent
{
public:
	OpenPoseEvent() {}
	virtual ~OpenPoseEvent() {}
	virtual int init() { return 0; };
	virtual void exit() {};
	virtual int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) = 0;
	virtual int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) = 0;
	virtual void recieveErrors(const std::vector<std::string>& errors) { (void)errors; }
};
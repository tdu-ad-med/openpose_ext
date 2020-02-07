#pragma once

#include <OpenPoseWrapper/OpenPoseEvent.h>

class VideoOpenPoseEvent : public OpenPoseEvent
{
private:
	std::string videoPath;
	cv::VideoCapture cap;
public:
	VideoOpenPoseEvent(const std::string& videoPath) : videoPath(videoPath){}
	virtual ~VideoOpenPoseEvent() {};
	int init() override final
	{
		cap.open(videoPath);
		if (!cap.isOpened())
		{
			std::cout << "can not open \"" << videoPath << "\"" << std::endl;
			return 1;
		}
		return 0;
	}
	void exit() override final
	{
		cap.release();
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (!cap.isOpened())
		{
			std::cout << "can not open \"" << videoPath << "\"" << std::endl;
			return 1;
		}
		imageInfo.needOpenposeProcess = true;
		imageInfo.frameNumber = (size_t)cap.get(cv::CAP_PROP_POS_FRAMES);
		imageInfo.frameSum = (size_t)cap.get(cv::CAP_PROP_FRAME_COUNT);
		imageInfo.frameTimeStamp = (size_t)cap.get(cv::CAP_PROP_POS_MSEC);
		cap.read(imageInfo.inputImage);
		if (imageInfo.inputImage.empty()) exit();
		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	void recieveErrors(const std::vector<std::string>& errors) override final
	{
		for (auto error : errors)
			std::cout << error << std::endl;
	}
};
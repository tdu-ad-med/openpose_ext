#pragma once

#include <functional>
#include <vector>
#include <string>

#include <OpenPoseWrapper/OpenPoseEvent.h>

class PreviewOpenPoseEvent : public OpenPoseEvent
{
private:
	std::string windowTitle;
	std::vector<std::function<void(int)>> keyboardEventListener;
	std::vector<std::function<void(int, int, int)>> mouseEventListener;
public:
	PreviewOpenPoseEvent(std::string windowTitle = "result") : windowTitle(windowTitle){}
	virtual ~PreviewOpenPoseEvent() {};
	int init() override final { return 0; }
	void exit() override final {}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// �E�B���h�E�̕\��
		cv::imshow(windowTitle, imageInfo.outputImage);

		// �}�E�X�C�x���g�̃R�[���o�b�N�֐��̎w��
		cv::setMouseCallback(windowTitle, [](int event, int x, int y, int flags, void* userdata) {
			// �}�E�X�C�x���g�̃��X�i�[�̔���
			for (auto&& func : *((std::vector<std::function<void(int, int, int)>>*)userdata)) func(event, x, y);
		}, (void*)(&mouseEventListener));

		// �L�[���͂̎擾
		int key = cv::waitKey(1);
		if (key == 0x1b)
		{
			exit();
			return 0;
		}

		// �L�[�C�x���g�̃��X�i�[�̔���
		if (key != -1)
		{
			for (auto&& func : keyboardEventListener) func(key);
		}
		return 0;
	}
	void recieveErrors(const std::vector<std::string>& errors) override final
	{
		for (auto&& error : errors)
			std::cout << error << std::endl;
	}
	void addMouseEventListener(const std::function<void(int, int, int)>& func)
	{
		mouseEventListener.push_back(func);
	}
	void addKeyboardEventListener(const std::function<void(int)>& func)
	{
		keyboardEventListener.push_back(func);
	}
};
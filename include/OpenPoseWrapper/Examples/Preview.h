#pragma once

#include <functional>
#include <vector>
#include <string>

class Preview
{
private:
	const std::string windowTitle;
	std::vector<std::function<void(int)>> keyboardEventListener;
	std::vector<std::function<void(int, int, int)>> mouseEventListener;

public:
	Preview(const std::string windowTitle = "result") : windowTitle(windowTitle){}
	virtual ~Preview() {};

	int preview(const cv::Mat& input)
	{
		// �E�B���h�E�̕\��
		cv::imshow(windowTitle, input);

		// �}�E�X�C�x���g�̃R�[���o�b�N�֐��̎w��
		cv::setMouseCallback(windowTitle, [](int event, int x, int y, int flags, void* userdata) {
			// �}�E�X�C�x���g�̃��X�i�[�̔���
			for (auto&& func : *((std::vector<std::function<void(int, int, int)>>*)userdata)) func(event, x, y);
		}, (void*)(&mouseEventListener));

		// �L�[���͂̎擾
		int key = cv::waitKey(1);

		// �L�[�C�x���g�̃��X�i�[�̔���
		if (key != -1)
		{
			for (auto&& func : keyboardEventListener) func(key);
		}

		return key;
	}
	
	// �}�E�X�C�x���g�̃R�[���o�b�N�֐��o�^
	void addMouseEventListener(const std::function<void(int, int, int)>& func)
	{
		mouseEventListener.push_back(func);
	}

	// �L�[�C�x���g�̃R�[���o�b�N�֐��o�^
	void addKeyboardEventListener(const std::function<void(int)>& func)
	{
		keyboardEventListener.push_back(func);
	}
};
#pragma once

#include <OpenPoseWrapper/MinimumOpenPose.h>

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
	// �E�B���h�E��̃}�E�X���W
	cv::Point mouse;

	Preview(const std::string windowTitle = "result") : windowTitle(windowTitle), mouse(0, 0)
	{
		// �}�E�X���W���X�V
		addMouseEventListener([&](int event, int x, int y) {
			if (event == cv::EVENT_MOUSEMOVE) { mouse.x = x; mouse.y = y; }
		});
	}
	
	virtual ~Preview() {};

	/**
	 * �w�肳�ꂽ�摜���E�B���h�E�Ƃ��ĕ\������
	 * @param input �\������摜
	 * @param delay �~���b�P�ʂ̑ҋ@���� (0���w�肷��ƃL�[���͂�����܂Œ�~����)
	 * @withoutWaitKey true�ɂ����delay�ɂ��ҋ@���Ԃ�0�b�ɂȂ邪�AaddKeyboardEventListener�֐��̌��ʂ��Ȃ��Ȃ�
	 * @return �Ō�ɓ��͂��ꂽ�L�[�ԍ����A��
	 * @note
	 * OpenCV�̎d�l��A�����̃E�B���h�E���������ꂽ��ԂŃL�[���͂����Ă��A�ǂ̃E�B���h�E�ɑ΂��Ă̑���ł��邩�����ł��Ȃ��B
	 * �܂��A������Preview�N���X�𐶐����A����炷�ׂ�withoutWaitKey��true�ɐݒ肷��ƁA�L�[�C�x���g���ǂ̃E�B���h�E�ɑ΂��đ��M����邩�͗\�z�ł��Ȃ��B
	 * ���̂��߁A1�̃E�B���h�E�̂�withoutWaitKey��true�ɐݒ肵�A����ȊO�̃E�B���h�E��false�ɐݒ肷�邱�Ƃňꎞ�I�ɂ��̖�������ł���B
	 */
	int preview(const cv::Mat& input, uint32_t delay = 1, bool withoutWaitKey = false)
	{
		// �E�B���h�E�̕\��
		cv::imshow(windowTitle, input);

		// �}�E�X�C�x���g�̃R�[���o�b�N�֐��̎w��
		cv::setMouseCallback(windowTitle, [](int event, int x, int y, int flags, void* userdata) {
			// �}�E�X�C�x���g�̃��X�i�[�̔���
			for (auto&& func : *((std::vector<std::function<void(int, int, int)>>*)userdata)) func(event, x, y);
		}, (void*)(&mouseEventListener));

		if (withoutWaitKey) return 0;

		// �L�[���͂̎擾
		int key = cv::waitKey(delay);

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

	// ���N���b�N���̃R�[���o�b�N�֐��o�^
	void onClick(const std::function<void(int, int)>& func)
	{
		addMouseEventListener([func](int event, int x, int y) {
			if (event == cv::EVENT_LBUTTONDOWN) { func(x, y); }
		});
	}
};
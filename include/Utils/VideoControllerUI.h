#pragma once

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Video.h>
#include <Utils/Preview.h>

class VideoControllerUI
{
private:
	Preview uiWindow;
	cv::Mat ui;
	cv::Point mouse;
	bool isClicked;

public:
	VideoControllerUI() : uiWindow("Video Controll Panel"), ui(120, 360, CV_8UC3), mouse(0, 0), isClicked(false)
	{
		uiWindow.addMouseEventListener([&](int event, int x, int y) {
			if (event == cv::EVENT_LBUTTONDOWN) { isClicked = true; }
			mouse.x = x; mouse.y = y;
		});
	}

	virtual ~VideoControllerUI() {};

	/**
	 * �E�B���h�E�ɃV���[�g�J�b�g�L�[��ǉ�����
	 * @param previewWindow �V���[�g�J�b�g�L�[��ǉ�����Preview�̃C���X�^���X
	 * @note
	 * Preview��withoutKeyWait��false�Ɏw�肳��Ă���E�B���h�E���w�肵�Ă��������B
	 * �ȉ��͂��̊֐��Œǉ������V���[�g�J�b�g�ꗗ�ł��B
	 * - 'j' : 30�t���[���߂�
	 * - 'k' : 30�t���[���i��
	 * - �X�y�[�X�L�[ : �Đ� / �ꎞ��~
	 */
	void addShortcutKeys(Preview& previewWindow, Video& video)
	{
		previewWindow.addKeyboardEventListener([&](int key) {
			// J�L�[��30�t���[���߂�
			if ('j' == key) video.seekRelative(-31);

			// K�L�[��30�t���[���i��
			if ('k' == key) video.seekRelative(29);

			// �X�y�[�X�L�[�� �Đ� / �ꎞ��~
			if (32 == key)
			{
				if (video.isPlay()) video.pause();
				else video.play();
			}
		});
	}

	/**
	 * UI�E�B���h�E�̉�ʍX�V
	 * @param video Video�̃C���X�^���X
	 */
	void showUI(Video& video)
	{
		// �Đ����̎擾
		auto videoInfo = video.getInfo();
		double progress = 0;
		if (videoInfo.frameSum >= 2)
		{
			progress = (int)videoInfo.frameNumber / (double)(videoInfo.frameSum - 1);
		}

		// ��ʂ̏�����
		ui = cv::Mat(120, 360, CV_8UC3, cv::Scalar(255, 255, 255));

		// �ꎞ�ϐ�
		cv::Rect area;
		cv::Point points[3];

		// 30�t���[���߂�{�^���̕\��
		area = { 90, 0, 60, 60 };
		if (area.contains(mouse))
		{
			cv::rectangle(ui, { area.x + 2, area.y + 2, 56, 56 }, { 240, 240, 240 }, -1);
			if (isClicked) video.seekRelative(-31);
		}
		points[0] = cv::Point(area.x + 40, area.y + 15);
		points[1] = cv::Point(area.x + 25, area.y + 30);
		points[2] = cv::Point(area.x + 40, area.y + 45);
		cv::fillConvexPoly(ui, points, 3, { 180, 0, 120 });
		cv::rectangle(ui, { area.x + 20, area.y + 15, 5, 30 }, { 180, 0, 120 }, -1);

		// �Đ� / �ꎞ��~ �{�^���̕\��
		area.x += 60;
		if (area.contains(mouse))
		{
			cv::rectangle(ui, { area.x + 2, area.y + 2, 56, 56 }, { 240, 240, 240 }, -1);
			if (isClicked)
			{
				if (video.isPlay()) video.pause();
				else video.play();
			}
		}
		if (video.isPlay())
		{
			cv::rectangle(ui, { area.x + 18, area.y + 15, 10, 30 }, { 180, 0, 120 }, -1);
			cv::rectangle(ui, { area.x + 32, area.y + 15, 10, 30 }, { 180, 0, 120 }, -1);
		}
		else
		{
			points[0] = cv::Point(area.x + 20, area.y + 15);
			points[1] = cv::Point(area.x + 40, area.y + 30);
			points[2] = cv::Point(area.x + 20, area.y + 45);
			cv::fillConvexPoly(ui, points, 3, { 180, 0, 120 });
		}
		
		// 30�t���[���i�ރ{�^���̕\��
		area.x += 60;
		if (area.contains(mouse))
		{
			cv::rectangle(ui, { area.x + 2, area.y + 2, 56, 56 }, { 240, 240, 240 }, -1);
			if (isClicked) video.seekRelative(29);
		}
		points[0] = cv::Point(area.x + 20, area.y + 15);
		points[1] = cv::Point(area.x + 35, area.y + 30);
		points[2] = cv::Point(area.x + 20, area.y + 45);
		cv::fillConvexPoly(ui, points, 3, { 180, 0, 120 });
		cv::rectangle(ui, { area.x + 35, area.y + 15, 5, 30 }, { 180, 0, 120 }, -1);

		// �Đ��ʒu��\���v���O���X�o�[��\��
		area = { 0, 82, 360, 16 };
		cv::rectangle(ui, { area.x + 30, area.y + (area.height / 2) - 2, area.width - 60, 4 }, { 180, 180, 180 }, -1);
		if (area.contains(mouse))
		{
			double m_progress = (double)(mouse.x - (area.x + 30)) / (double)(area.width - 60);
			m_progress = (m_progress < 0.0) ? 0.0 : (m_progress > 1.0) ? 1.0 : m_progress;
			int frame = (m_progress * (double)videoInfo.frameSum);
			frame = (frame < 0) ? 0 : (frame >= videoInfo.frameSum) ? (videoInfo.frameSum - 1) : frame;
			cv::circle(ui, { area.x + 30 + (int)((double)(area.width - 60) * m_progress), area.y + (area.height / 2) }, 8, { 240, 160, 200 }, -1);
			if (isClicked) video.seekAbsolute(frame);
		}
		cv::circle(ui, { area.x + 30 + (int)((double)(area.width - 60) * progress), area.y + (area.height / 2) }, 8, { 180, 0, 120 }, -1);

		// �E�B���h�E�ɕ`��
		uiWindow.preview(ui, 0, true);

		// �N���b�N��Ԃ����ɖ߂�
		isClicked = false;
	}
};
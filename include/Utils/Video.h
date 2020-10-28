#pragma once

#include <OpenPoseWrapper/MinimumOpenPose.h>

class Video
{
private:
	cv::VideoCapture videoCapture;
	bool play_, needUpdate;
	cv::Mat buffer;

public:
	struct FrameInfo{
		size_t frameNumber, frameSum, frameTimeStamp;
	};
	Video() : play_(true), needUpdate(false) {}
	virtual ~Video() {};

	// ����t�@�C�����J��
	int open(const std::string& videoPath)
	{
		// ����t�@�C�����J��
		videoCapture.open(videoPath);

		// �G���[�̊m�F
		if (!videoCapture.isOpened())
		{
			std::cout << videoPath << "���J���܂���ł����B" << std::endl;
			return 1;
		}

		return 0;
	}

	// ����̎��̃t���[�����擾����
	cv::Mat next()
	{
		cv::Mat ret;

		// ������J���Ă��Ȃ��ꍇ�͏������I��
		if (!videoCapture.isOpened()) return ret;

		// �ꎞ��~��Ԃ��A��ʂ��X�V����K�v���Ȃ���ΈȑO�擾�����t���[����Ԃ�
		if ((!play_) && (!needUpdate)) return buffer.clone();
		needUpdate = false;

		// ���̃t���[�����擾
		videoCapture.read(ret);
		buffer = ret;

		return ret.clone();
	}

	// ����̍Đ���Ԃ��擾
	FrameInfo getInfo() const
	{
		FrameInfo ret;

		// ���݂̍Đ��ʒu(�t���[���P��)
		ret.frameNumber = (size_t)videoCapture.get(cv::CAP_PROP_POS_FRAMES);
		
		// �S�t���[���̖���
		ret.frameSum = (size_t)videoCapture.get(cv::CAP_PROP_FRAME_COUNT);
		
		// ���݂̍Đ��ʒu(�~���b�P��)
		ret.frameTimeStamp = (size_t)videoCapture.get(cv::CAP_PROP_POS_MSEC);

		return ret;
	}

	// ������Đ�����
	void play() { play_ = true; }
	
	// ������ꎞ��~����
	void pause() { play_ = false; }

	// ���悪�Đ���Ԃ��ǂ���
	bool isPlay() const { return (videoCapture.isOpened() && (play_)); }

	// ����̍Đ��ʒu���w��̃t���[���ԍ��܂ňړ�����
	void seekAbsolute(long long frame)
	{
		if (videoCapture.isOpened())
		{
			size_t frameSum = (size_t)videoCapture.get(cv::CAP_PROP_FRAME_COUNT);
			if (frame < 0) frame = 0;
			if (frame >= frameSum) frame = frameSum - 1;
			videoCapture.set(CV_CAP_PROP_POS_FRAMES, (double)frame);
			needUpdate = true;
		}
	}

	// ����̍Đ��ʒu���w�肵���t���[�������ړ�����
	void seekRelative(long long frame)
	{
		if (videoCapture.isOpened())
		{
			long long frameNumber = (size_t)videoCapture.get(cv::CAP_PROP_POS_FRAMES);
			seekAbsolute(frameNumber + frame);
		}
	}
};
#pragma once

#include <openpose/headers.hpp>

struct ImageInfo
{
	struct Node { float x, y, confidence; };

	// openpose�ɓ��͂���f�[�^
	size_t frameNumber = 0;  // �t���[���ԍ�
	size_t frameSum = 0;  // ���t���[����
	size_t frameTimeStamp = 0;  // �t���[���̃^�C���X�^���v(�~���b)
	cv::Mat inputImage;  // ���͉摜
	bool needOpenposeProcess = true;  // ���i���o�����邩�ǂ���

	// openpose����󂯎��f�[�^
	cv::Mat outputImage;  // ���i�̃v���r���[�摜
	std::map<size_t, std::vector<Node>> people;  // ���i�f�[�^
};

class OpenPoseEvent
{
public:
	OpenPoseEvent() {}
	virtual ~OpenPoseEvent() {}

	// openpose�N������ɌĂяo�����
	virtual int init() { return 0; };

	// openpose���I�����钼�O�ɌĂяo�����
	virtual void exit() {};

	// openpose�Ƀf�[�^����͂���^�C�~���O�ŌĂяo�����
	// imageInfo�̓��͗p�̃����o�ϐ������̊֐��ŏ���������
	// openpose���~������Ƃ��͈�����exit()���Ă�
	virtual int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) = 0;

	// openpose����f�[�^���󂯎��^�C�~���O�ŌĂяo�����
	// imageInfo�Ɍ��ʂ��i�[�����
	// openpose���~������Ƃ��͈�����exit()���Ă�
	virtual int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) = 0;

	// openpose���G���[�𔭐������^�C�~���O�ŌĂяo������
	// errors�ɃG���[���e���i�[�����
	virtual void recieveErrors(const std::vector<std::string>& errors) { (void)errors; }
};
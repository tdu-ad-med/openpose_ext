/*
���̃T���v���ł́A��ʏ�Ɉ����������̏�����l�̐l���ǂ̕����Ɉړ����������J�E���g���܂��B
example08_CountLine.cpp�Ƃ̈Ⴂ�͓��͂ɓ���ł͂Ȃ�Web�J�������g�p���Ă���_�ł��B
��͌��ʂ� openpose_ext/build/bin/media �̒��ɋL�^�J�n�����̃t�@�C�����ŕۑ�����܂��B
*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Video.h>
#include <Utils/Preview.h>
#include <Utils/VideoControllerUI.h>
#include <Utils/PlotInfo.h>
#include <Utils/SqlOpenPose.h>
#include <Utils/Tracking.h>
#include <Utils/PeopleCounter.h>
#include <time.h>

int main(int argc, char* argv[])
{
	/*
	Windows�ŋN�������s����ꍇ
		cv::VideoCapture webcam(0);
	�̍s��
		cv::VideoCapture webcam(cv::CAP_DSHOW + 0);
	�ɏ���������Ǝ��邩������Ȃ��ł��B
	*/

	// web�J�������J��
	cv::VideoCapture webcam(0);
	if (!webcam.isOpened()) {
		std::cout << "failed to open the web camera" << std::endl;
		return 0;
	}

	// ���ݎ������擾���� (�t�@�C�����Ɏg�p����)
	time_t timer = time(NULL); tm ptm;
	localtime_s(&ptm, &timer);
	char time_c_str[256] = { '\0' }; strftime(time_c_str, sizeof(time_c_str), "%Y-%m-%d_%H-%M-%S", &ptm);
	std::string time(time_c_str);
	std::cout << time << std::endl;

	// �o�͂��� SQL �t�@�C���̃t���p�X��^��J�n�����ɂ���
	std::string sqlPath = R"(media/webcam_)" + time + R"(.sqlite3)";

	// OpenPose �̏�����������
	MinOpenPose openpose(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// ������v���r���[���邽�߂̃E�B���h�E�𐶐�����
	Preview preview("result");

	// SQL �̓ǂݏ������s���N���X�̏�����
	SqlOpenPose sql;
	sql.open(sqlPath, 300);

	// ���i���g���b�L���O����N���X
	Tracking tracker(
		0.5f,  // �֐߂̐M���l�����̒l�ȉ��ł���ꍇ�́A�֐߂����݂��Ȃ����̂Ƃ��ď�������
		5,     // �M���l��confidenceThreshold���傫���֐߂̐������̒l�����ł���ꍇ�́A���̐l�����Ȃ����̂Ƃ��ď�������
		10,    // ��x�g���b�L���O���O�ꂽ�l�����̃t���[�������o�߂��Ă��Ĕ�������Ȃ��ꍇ�́A�����������̂Ƃ��ď�������
		50.0f  // �g���b�L���O���̐l��1�t���[���i�񂾂Ƃ��A�ړ����������̒l�����傫���ꍇ�͓���l���̌�₩��O��
	);

	// �ʍs�l���J�E���g����N���X
	PeopleCounter count(
		250, 0,    // �����̎n�_���W (X, Y)
		250, 500,  // �����̏I�_���W (X, Y)
		10         // �����̑���
	);

	// ���悪�I���܂Ń��[�v����
	uint64_t frameNumber = 0;
	while (true)
	{
		// ����̎��̃t���[����ǂݍ���
		cv::Mat image;
		if (!webcam.read(image)) { break; }

		// �f�����I�������ꍇ�̓��[�v�𔲂���
		if (image.empty()) break;

		// �p������
		MinOpenPose::People people = openpose.estimate(image);

		// ���ʂ� SQL �ɕۑ�
		sql.writeBones(frameNumber, frameNumber, people);

		// �g���b�L���O
		auto tracked_people = tracker.tracking(people, sql, frameNumber).value();

		// �ʍs�l�̃J�E���g
		count.update(tracker, frameNumber);

		// �ʍs�l�̃J�E���g�󋵂��v���r���[
		count.drawInfo(image, tracker);

		// �p������̌��ʂ� image �ɕ`�悷��
		plotBone(image, tracked_people, openpose);

		// �l��ID�̕`��
		plotId(image, tracked_people);  // �l��ID�̕`��

		// ��ʂ��X�V����
		int ret = preview.preview(image);

		// Esc�L�[�������ꂽ��I������
		if (0x1b == ret) break;

		frameNumber += 1;
	}

	return 0;
}
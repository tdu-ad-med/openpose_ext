/*

���̃T���v���ł́A��ʏ�Ɉ����������̏�����l�̐l���ǂ̕����Ɉړ����������J�E���g���܂��B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Video.h>
#include <Utils/Preview.h>
#include <Utils/VideoControllerUI.h>
#include <Utils/PlotInfo.h>
#include <Utils/SqlOpenPose.h>
#include <Utils/Tracking.h>
#include <Utils/PeopleCounter.h>

int main(int argc, char* argv[])
{
	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath = R"(media/video.mp4)";

	// ���o�͂��� SQL �t�@�C���̃t���p�X
	std::string sqlPath = videoPath + ".sqlite3";

	// OpenPose �̏�����������
	MinOpenPose openpose(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// OpenPose �ɓ��͂��铮���p�ӂ���
	Video video;
	video.open(videoPath);

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
		10, 10,    // �����̎n�_���W (X, Y)
		300, 200,  // �����̏I�_���W (X, Y)
		10         // �����̑���
	);

	// ���悪�I���܂Ń��[�v����
	while (true)
	{
		// ����̎��̃t���[����ǂݍ���
		cv::Mat image = video.next();

		// �f�����I�������ꍇ�̓��[�v�𔲂���
		if (image.empty()) break;

		// �t���[���ԍ��Ȃǂ̏����擾����
		Video::FrameInfo frameInfo = video.getInfo();

		// SQL�Ɏp�����L�^����Ă���΁A���̒l���g��
		auto peopleOpt = sql.read(frameInfo.frameNumber);
		MinOpenPose::People people;
		if (peopleOpt)
		{
			people = peopleOpt.value();
		}

		// SQL�Ɏp�����L�^����Ă��Ȃ���Ύp��������s��
		else
		{
			// �p������
			people = openpose.estimate(image);

			// ���ʂ� SQL �ɕۑ�
			sql.write(frameInfo.frameNumber, frameInfo.frameTimeStamp, people);
		}

		// �g���b�L���O
		auto tracked_people = tracker.tracking(people, sql, frameInfo.frameNumber).value();

		// �ʍs�l�̃J�E���g
		count.update(tracker, frameInfo.frameNumber);

		// �ʍs�l�̃J�E���g�󋵂��v���r���[
		count.drawInfo(image, tracker);

		// �p������̌��ʂ� image �ɕ`�悷��
		plotBone(image, tracked_people, openpose);

		// �l��ID�̕`��
		plotId(image, tracked_people);  // �l��ID�̕`��

		// ��ʂ��X�V����
		preview.preview(image);
	}

	return 0;
}
/*

OpenPose �̏����͖c��ł��B
������������x�� OpenPose �ɓ��͂���͔̂�����ł��B
���̂��� OpenPose �ł̉�͌��ʂ��t�@�C���Ƃ��ē��o�͂ł���T���v����p�ӂ��܂����B

���̃T���v���́A1�x�ڂ̎��s�ł� OpenPose �œ�����������A��͌��ʂ��t�@�C���Ƃ��ĕۑ����܂��B
2�x�ڈȍ~�̎��s�ł�1�x�ڂ̎��s�Ő������ꂽ�t�@�C����ǂݍ���ō��i�f�[�^���擾���܂��B

�܂��A��͌��ʂ�ۑ�����t�@�C�����́A���͂��铮��̖��O�̖�����".sqlite3"���t���܂��B
���Ƃ��� "aaa.mp4" �Ƃ����������͂����ꍇ�A���̓���t�@�C���Ɠ����ꏊ�� "aaa.mp4.sqlite3" �Ƃ����t�@�C������������܂��B

���̃t�@�C���̌`���� sqlite3 �Ȃ̂� DB Browser (SQLite) �ȂǂŊJ�����Ƃ��ł��܂��B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Video.h>
#include <Utils/Preview.h>
#include <Utils/VideoControllerUI.h>
#include <Utils/PlotInfo.h>
#include <Utils/SqlOpenPose.h>

int main(int argc, char* argv[])
{
	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath = R"(media/video.mp4)";

	// ���o�͂��� SQL �t�@�C���̃t���p�X
	std::string sqlPath = videoPath + ".sqlite3";

	// MinimumOpenPose �̏�����������
	MinOpenPose openpose(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// OpenPose �ɓ��͂��铮���p�ӂ���
	Video video;
	video.open(videoPath);

	// ������v���r���[���邽�߂̃E�B���h�E�𐶐�����
	Preview preview("result");

	// SQL �̓ǂݏ������s���N���X�̏�����
	SqlOpenPose sql;
	sql.open(sqlPath, 300);

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

		// �p������̌��ʂ� image �ɕ`�悷��
		plotBone(image, people, openpose);

		// ��ʂ��X�V����
		preview.preview(image);
	}

	return 0;
}
#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/Video.h>
#include <OpenPoseWrapper/Examples/Preview.h>
#include <OpenPoseWrapper/Examples/PlotInfo.h>
#include <OpenPoseWrapper/Examples/SqlOpenPose.h>
//#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
//#include <OpenPoseWrapper/Examples/PeopleCounterOpenPoseEvent.h>

int main(int argc, char* argv[])
{
	// ���͂���f���t�@�C���̃t���p�X
	// ���� : �t�@�C���`����CP932�ł͂Ȃ��ꍇ�A�t�@�C���p�X�ɓ��{�ꂪ�������Ă���Ə�肭�����Ȃ��\��������
	std::string videoPath = R"(media/video.mp4)";
	videoPath = R"(D:\�v���o\Dropbox\Dropbox\SDK\openpose\video\58.mp4)";
	if (argc == 2) videoPath = argv[1];

	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = videoPath + ".sqlite3";

	// openpose�̃��b�p�[�N���X
	MinOpenPose mop;

	// �����ǂݍ��ރN���X
	Video video;
	video.open(videoPath);

	// �v���r���[�E�B���h�E�𐶐�����N���X
	Preview preview("result");

	// ���i�Ȃǂ�\������N���X
	PlotInfo plot(true, true, true);

	// SQL�t�@�C���̓ǂݍ��݁A�������݂��s���N���X
	SqlOpenPose sql;
	sql.open(sqlPath, 300);

	while (true)
	{
		// ����̎��̃t���[����ǂݍ���
		cv::Mat frame = video.next();

		// �t���[���ԍ��Ȃǂ̏����擾����
		Video::FrameInfo frameInfo = video.getInfo();

		// �t���[�����Ȃ��ꍇ�͏I������
		if (frame.empty()) break;

		// SQL�Ɏp�����L�^����Ă���΁A���̒l���g��
		auto peopleOpt = sql.read(frameInfo.frameNumber);
		MinOpenPose::People people;
		if (peopleOpt) { people = peopleOpt.value(); }

		// SQL�Ɏp�����L�^����Ă��Ȃ���Ύp��������s��
		else
		{
			// �p������
			people = mop.estimate(frame);

			// ���ʂ�SQL�ɕۑ�
			sql.write(frameInfo.frameNumber, frameInfo.frameTimeStamp, people);
		}

		// �f���̏�ɍ��i��`��
		plot.bone(frame, mop, people);  // ���i��`��
		plot.id(frame, people);  // �t���[�����[�g�ƃt���[���ԍ��̕`��
		plot.frameInfo(frame, video);  // �t���[�����[�g�ƃt���[���ԍ��̕`��

		// �v���r���[
		int ret = preview.preview(frame);

		// Esc�L�[�������ꂽ��I������
		if (0x1b == ret) break;
	}

	/*
	// SQL���o�͋@�\�̒ǉ�
	auto sql = mop.addEventListener<SqlOpenPoseEvent>(sqlPath, 300);

	// ����ǂݍ��ݏ����̒ǉ�
	auto video = mop.addEventListener<VideoOpenPoseEvent>(videoPath);

	// ���i�̃g���b�L���O�����̒ǉ�
	auto tracker = mop.addEventListener<TrackingOpenPoseEvent>(sql);

	// �l���J�E���g�����̒ǉ�
	(void)mop.addEventListener<PeopleCounterOpenPoseEvent>(tracker, 579, 578, 1429, 577, 100.0, true);

	// �o�͉摜�ɍ��i���Ȃǂ�`�悷�鏈���̒ǉ�
	(void)mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);

	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	preview = mop.addEventListener<PreviewOpenPoseEvent>("result");

	custom->setParams(video, tracker, preview);
	*/

	return 0;
}
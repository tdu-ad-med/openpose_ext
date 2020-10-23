#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/Video.h>
#include <OpenPoseWrapper/Examples/Preview.h>
#include <OpenPoseWrapper/Examples/PlotInfo.h>
#include <OpenPoseWrapper/Examples/SqlOpenPose.h>
#include <OpenPoseWrapper/Examples/Tracking.h>
#include <OpenPoseWrapper/Examples/PeopleCounter.h>

using People = MinOpenPose::People;
using Person = MinOpenPose::Person;
using Node = MinOpenPose::Node;

int main(int argc, char* argv[])
{
	// ���͂���f���t�@�C���̃t���p�X
	// ���� : ���̃v���O�����̃t�@�C���`����CP932�ł͂Ȃ��ꍇ�A�t�@�C���p�X�ɓ��{�ꂪ�������Ă���Ə�肭�����Ȃ��\��������
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
	PlotInfo plot;

	// SQL�t�@�C���̓ǂݍ��݁A�������݂��s���N���X
	SqlOpenPose sql;
	sql.open(sqlPath, 300);

	// ���i���g���b�L���O����N���X
	Tracking tracker(
		0.5f,  // �֐߂̐M���l�����̒l�ȉ��ł���ꍇ�́A�֐߂����݂��Ȃ����̂Ƃ��ď�������
		5,  // �M���l��confidenceThreshold���傫���֐߂̐������̒l�����ł���ꍇ�́A���̐l�����Ȃ����̂Ƃ��ď�������
		10,  // ��x�g���b�L���O���O�ꂽ�l�����̃t���[�������o�߂��Ă��Ĕ�������Ȃ��ꍇ�́A�����������̂Ƃ��ď�������
		50.0f  // �g���b�L���O���̐l��1�t���[���i�񂾂Ƃ��A�ړ����������̒l�����傫���ꍇ�͓���l���̌�₩��O��
	);

	// �ʍs�l���J�E���g����N���X
	PeopleCounter count(200, 250, 500, 250, 100);

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
		People people;
		if (peopleOpt) { people = peopleOpt.value(); }

		// SQL�Ɏp�����L�^����Ă��Ȃ���Ύp��������s��
		else
		{
			// �p������
			people = mop.estimate(frame);

			// ���ʂ�SQL�ɕۑ�
			sql.write(frameInfo.frameNumber, frameInfo.frameTimeStamp, people);
		}

		// �g���b�L���O
		auto tracked_people = tracker.tracking(people, sql, frameInfo.frameNumber).value();

		// �ʍs�l�̃J�E���g
		count.update(tracker, frameInfo.frameNumber);

		// �ʍs�l�̃J�E���g�󋵂��v���r���[
		count.drawInfo(frame, tracker);

		// �f���̏�ɍ��i��`��
		plot.bone(frame, mop, tracked_people);  // ���i��`��
		plot.id(frame, tracked_people);  // �t���[�����[�g�ƃt���[���ԍ��̕`��
		plot.frameInfo(frame, video);  // �t���[�����[�g�ƃt���[���ԍ��̕`��

		// �v���r���[
		int ret = preview.preview(frame, 1);

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
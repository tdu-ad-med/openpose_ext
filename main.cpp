#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/Video.h>
#include <OpenPoseWrapper/Examples/Preview.h>
#include <OpenPoseWrapper/Examples/PlotInfo.h>
//#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
//#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
//#include <OpenPoseWrapper/Examples/PeopleCounterOpenPoseEvent.h>

int main(int argc, char* argv[])
{
	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath = R"(media/video.mp4)";
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

	while (true)
	{
		// ����̎��̃t���[����ǂݍ���
		cv::Mat frame = video.next();

		// �t���[�����Ȃ��ꍇ�͏I������
		if (frame.empty()) break;

		// �p������
		auto people = mop.estimate(frame);

		// �f���̏�ɍ��i��`��
		plot.plotBone(frame, mop, people);
		plot.plotFrameInfo(frame, video);

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
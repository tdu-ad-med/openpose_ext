#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Video.h>
#include <Utils/Preview.h>
#include <Utils/VideoControllerUI.h>
#include <Utils/PlotInfo.h>
#include <Utils/SqlOpenPose.h>
#include <Utils/Tracking.h>
#include <Utils/PeopleCounter.h>
#include <Utils/Vector.h>

using People = MinOpenPose::People;
using Person = MinOpenPose::Person;
using Node = MinOpenPose::Node;

int main(int argc, char* argv[])
{
	// ���͂���f���t�@�C���̃t���p�X
	// ���� : ���̃v���O�����̃t�@�C���`����CP932�ł͂Ȃ��ꍇ�A�t�@�C���p�X�ɓ��{�ꂪ�������Ă���Ə�肭�����Ȃ��\��������
	std::string videoPath = R"(C:\Users\�ēc��\Videos\guest002-2020-08-26_09-00-55.mp4)";

	// �R���\�[�������ɓ���̃t�@�C���p�X���w�肳�ꂽ�ꍇ�͂��̃p�X��D�悷��
	if (argc == 2) videoPath = argv[1];

	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = videoPath + ".sqlite3";

	// openpose�̃��b�p�[�N���X
	MinOpenPose openpose;

	// �����ǂݍ��ރN���X
	Video video;
	video.open(videoPath);

	// �v���r���[�E�B���h�E�𐶐�����N���X
	Preview preview("result");

	// ��ʂ̃N���b�N�ŃR���\�[���ɍ��W���o�͂���
	preview.onClick([](int x, int y) { std::cout << x << ", " << y << std::endl; });

	// �t���[�����[�g�Ȃǂ�\������N���X
	plotFrameInfo plotFrameInfo;

	// SQL�t�@�C���̓ǂݍ��݁A�������݂��s���N���X
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
	PeopleCounter count(200, 250, 500, 250, 100);

	// �ˉe�ϊ�������N���X
	vt::ScreenToGround screenToGround;

	// �J�����̘c�݂�␳����ݒ�
	screenToGround.setCalibration(
		// �J�����L�����u���[�V�������s�������̃J�����̉𑜓x, �o�͉摜�̊g�嗦
		1920, 1080, 0.5,
		// �J���������p�����[�^�̏œ_�����ƒ��S���W(fx, fy, cx, cy)
		1222.78852772764, 1214.377234799321, 967.8020317677116, 569.3667691760459,
		// �J�����̘c�݌W��(k1, k2, k3, k4)
		-0.08809225804249926, 0.03839093574614055, -0.060501971675431955, 0.033162385302275665
	);

	// �J�����̉f�����A�n�ʂ��ォ�猩���悤�ȉf���Ɏˉe�ϊ�����
	screenToGround.setParams(
		// �J�����̉𑜓x
		1280, 960,
		// �J�����̐�����p(deg)�A�J�����̒n�ʂ���̍���(m)
		53.267, 1.0,
		// �J�����Ɏʂ��Ă���n�ʂ̔C�ӂ�4�_
		147, 44,
		86, 393,
		492, 529,
		635, 199
	);

	// ����Đ��̃R���g���[����UI�ōs����悤�ɂ���N���X
	VideoControllerUI videoController;
	videoController.addShortcutKeys(preview, video);

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
			people = openpose.estimate(frame);

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
		plotBone(frame, tracked_people, openpose);  // ���i��`��
		plotId(frame, tracked_people);  // �l��ID�̕`��
		plotFrameInfo(frame, video);  // �t���[�����[�g�ƃt���[���ԍ��̕`��

		// �f�����ォ�猩���悤�Ɏˉe�ϊ�
		frame = screenToGround.translateMat(frame, 0.3f, true);

		// �v���r���[
		int ret = preview.preview(frame, 10);

		// ����Đ��R���g���[���[�̕\��
		videoController.showUI(video);

		// Esc�L�[�������ꂽ��I������
		if (0x1b == ret) break;
	}

	return 0;
}
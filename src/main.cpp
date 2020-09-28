#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PeopleCounterOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	std::shared_ptr<VideoOpenPoseEvent> video;
	std::shared_ptr<TrackingOpenPoseEvent> tracker;
	std::shared_ptr<SqlOpenPoseEvent> sql;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode;

	int checkError()
	{
		if ((!video) || (!tracker))
		{
			std::cout
				<< "VideoOpenPoseEvent, TrackingOpenPoseEvent�̂����ꂩ�����w��ł��B\n"
				<< "setParams�֐��Ő������l���w�肵�Ă��������B"
				<< std::endl;
			return 1;
		}
		return 0;
	}
public:
	CustomOpenPoseEvent() : previewMode(0) {}
	virtual ~CustomOpenPoseEvent() {}
	int init() override final
	{
		if (checkError()) return 1;

		// people_with_normalized_tracking�e�[�u�����Đ���
		if (sql->deleteTableIfExist(u8"people_with_normalized_tracking")) return 1;
		if (sql->createTableIfNoExist(u8"people_with_normalized_tracking", u8"frame INTEGER, people INTEGER, x REAL, y REAL")) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"frame", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"people", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"frame", u8"people", true)) return 1;

		// �J�����L�����u���[�V�����̐ݒ�
		screenToGround.setCalibration(
			1858.0, 1044.0, 0.5,  // �J�����L�����u���[�V�����ɗp�����摜�̉𑜓x(w, h), �o�͉摜�̊g�嗦
			1057, 1057, 935, 567,  // �J���������p�����[�^�̏œ_�����ƒ��S���W(fx, fy, cx, cy)
			0.0, 0.0, 0.0, 0.0  // �J�����̘c�݌W��(k1, k2, k3, k4)
		);

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (checkError()) return 1;

		// �J�����̐�����p
		double cam_h_fov = 112;

		// ����
		imageInfo.outputImage = imageInfo.outputImage.clone();

		// �ˉe�ϊ�
		screenToGround.setParams(
			imageInfo.outputImage.cols, imageInfo.outputImage.rows, cam_h_fov, 6.3,
			346, 659,
			1056, 668,
			990, 202,
			478, 292
		);
		
		if (previewMode == 1) imageInfo.outputImage = screenToGround.onlyFlatMat(imageInfo.outputImage); // �v���r���[
		if (previewMode == 2) imageInfo.outputImage = screenToGround.translateMat(imageInfo.outputImage, 0.3f); // �v���r���[

		screenToGround.drawAreaLine(imageInfo.outputImage, previewMode);  // �ˉe�ϊ��Ɏg�p����4�_�͈̔͂�`��

		// people_with_normalized_tracking�e�[�u���̍X�V
		if (!sql->isDataExist("people_with_normalized_tracking", "frame", imageInfo.frameNumber))
		{
			SQLite::Statement insertQuery(*sql->database, u8"INSERT INTO people_with_normalized_tracking VALUES (?, ?, ?, ?)");
			for (auto&& currentPerson = tracker->latestPeople.begin(); currentPerson != tracker->latestPeople.end(); currentPerson++)
			{
				auto&& position = TrackingOpenPoseEvent::getJointAverage(currentPerson->second);
				auto normal = screenToGround.translate(vt::Vector4{ position.x, position.y });
				if (sql->bindAllAndExec(insertQuery, imageInfo.frameNumber, currentPerson->first, normal.x, normal.y)) return 1;
			}
		}

		return 0;
	}
	void setParams(
		const std::shared_ptr<VideoOpenPoseEvent> videoTmp,
		const std::shared_ptr<TrackingOpenPoseEvent> trackingTmp,
		const std::shared_ptr<PreviewOpenPoseEvent> previewTmp = nullptr
	)
	{
		video = videoTmp;
		tracker = trackingTmp;
		sql = trackingTmp->sql;
		preview = previewTmp;

		if (checkError()) return;

		if (!preview) return;

		// �}�E�X�C�x���g����
		preview->addMouseEventListener([&](int event, int x, int y) {
			switch (event)
			{
				// �}�E�X�ړ���
			case cv::EVENT_MOUSEMOVE:
				mouse.x = x; mouse.y = y;
				break;

				// ���N���b�N��
			case cv::EVENT_LBUTTONDOWN:
				std::cout << x << ", " << y << std::endl;
				break;

				// �E�N���b�N��
			case cv::EVENT_RBUTTONDOWN:
				break;

				// ���̑�
			default:
				break;
			}
			});

		// �L�[�C�x���g����
		preview->addKeyboardEventListener([&](int key) {
			switch (key)
			{
				// J�L�[��30�t���[���߂�
			case 'j':
				video->seekRelative(-30);
				break;

				// K�L�[��30�t���[���i��
			case 'k':
				video->seekRelative(30);
				break;

				// A�L�[�ŉ�ʕ\���ؑ�
			case 'a':
				previewMode = (previewMode + 1) % 3;
				break;

				// �X�y�[�X�L�[�œ���̍Đ�/�ꎞ��~
			case 32:
				if (video->isPlay()) video->pause();
				else video->play();
				break;

				// ���̑�
			default:
				break;
			}
			});
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;



	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath = R"(media\checker.mp4)";
	if (argc == 2) videoPath = argv[1];

	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = videoPath + ".sqlite3";



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



	// openpose�̋N��
	auto start = std::chrono::high_resolution_clock::now();
	int ret = mop.startup();
	double time = (1.0 / 1000.0) * (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();



	// �N������I���܂łɗv�������Ԃ̕\��
	std::cout << "time score : " << time << " sec." << std::endl;



	return ret;
}

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

#include <Utils/Tracking.h>
class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	std::shared_ptr<TrackingOpenPoseEvent> tracking;
	std::shared_ptr<VideoOpenPoseEvent> video;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	op::PeopleLineCounter peopleLineCounter;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode;
public:
	CustomOpenPoseEvent() : previewMode(0) {}
	virtual ~CustomOpenPoseEvent() {}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if ((!tracking) || (!video))
		{
			std::cout
				<< "TrackingOpenPoseEvent��������VideoOpenPoseEvent�����w��ł��B\n"
				<< "setParams�֐��Ő������l���w�肵�Ă��������B"
				<< std::endl;
			return 1;
		}

		op::PeopleList& people = tracking->people;

		// �l���J�E���g
		peopleLineCounter.setLine(579, 578, 1429, 577, 100.0);  // �J�E���g�̊���̍��W�ݒ�
		peopleLineCounter.updateCount(people);  // �J�E���g�̍X�V
		peopleLineCounter.drawJudgeLine(imageInfo.outputImage);  // ����̕`��
		peopleLineCounter.drawPeopleLine(imageInfo.outputImage, people, true);  // �l�X�̎n�_�ƏI�_�̕`��

		// �J�E���g��\��
		gui::text(imageInfo.outputImage, std::string("up : ") + std::to_string(peopleLineCounter.getUpCount()), { 20, 200 });
		gui::text(imageInfo.outputImage, std::string("down : ") + std::to_string(peopleLineCounter.getDownCount()), { 20, 230 });

		// �ˉe�ϊ�
		screenToGround.setParams(
			imageInfo.outputImage.cols, imageInfo.outputImage.rows, 33.3, 6.3,
			492, 436,
			863, 946,
			1335, 644,
			905, 242
		);
		screenToGround.drawAreaLine(imageInfo.outputImage);  // �ˉe�ϊ��Ɏg�p����4�_�͈̔͂�`��

		if (previewMode == 1) imageInfo.outputImage = screenToGround.perspective(imageInfo.outputImage);

		return 0;
	}
	void setParams(
		const std::shared_ptr<TrackingOpenPoseEvent> trackingTmp,
		const std::shared_ptr<VideoOpenPoseEvent> videoTmp,
		const std::shared_ptr<PreviewOpenPoseEvent> previewTmp = nullptr
	)
	{
		tracking = trackingTmp;
		video = videoTmp;
		preview = previewTmp;

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
				previewMode = (previewMode + 1) % 2;
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
	std::string videoPath =
		R"(G:\�v���o\Dropbox\Dropbox\SDK\openpose\video\IMG_1533.mp4)";
	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = std::regex_replace(videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";


	// �g���b�L���O�����̒ǉ�
	auto tracking = mop.addEventListener<TrackingOpenPoseEvent>();
	// sql���o�͋@�\�̒ǉ�
	auto sql = mop.addEventListener<SqlOpenPoseEvent>(sqlPath, 60);
	// ����ǂݍ��ݏ����̒ǉ�
	auto video = mop.addEventListener<VideoOpenPoseEvent>(videoPath);
	// �o�͉摜�ɕ����Ȃǂ�`�悷�鏈���̒ǉ�
	(void)mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);
	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();
	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	auto preview = mop.addEventListener<PreviewOpenPoseEvent>("result");


	custom->setParams(tracking, video, preview);


	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

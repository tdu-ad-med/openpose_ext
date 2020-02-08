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
public:
	CustomOpenPoseEvent() {}
	virtual ~CustomOpenPoseEvent() {}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		op::PeopleList& people = tracking->people;

		// �l���J�E���g
		peopleLineCounter.setLine(579, 578, 1429, 577, 100.0);
		peopleLineCounter.update(people);
		peopleLineCounter.drawLine(imageInfo.outputImage);

		// �J�E���g��\��
		gui::text(imageInfo.outputImage, std::string("up : ") + std::to_string(peopleLineCounter.getUpCount()), { 20, 200 });
		gui::text(imageInfo.outputImage, std::string("down : ") + std::to_string(peopleLineCounter.getDownCount()), { 20, 230 });

		// �g���b�L���O�̎n�_�ƏI�_�����Ԓ�����`��
		for (size_t index : people.getCurrentIndices())
		{
			auto firstTree = people.getFirstTree(index);
			auto currentTree = people.getCurrentTree(index);
			if ((!firstTree.isValid()) || (!currentTree.isValid())) continue;

			// �����̕`��
			cv::line(imageInfo.outputImage, { (int)firstTree.average().x, (int)firstTree.average().y }, { (int)currentTree.average().x, (int)currentTree.average().y }, cv::Scalar{
				(double)((int)((std::sin((double)index * 463763.0) + 1.0) * 100000.0) % 120 + 80),
				(double)((int)((std::sin((double)index * 1279.0) + 1.0) * 100000.0) % 120 + 80),
				(double)((int)((std::sin((double)index * 92763.0) + 1.0) * 100000.0) % 120 + 80)
			}, 2.0);

			// id�̕`��
			gui::text(imageInfo.outputImage, std::to_string(index), { (int)currentTree.average().x, (int)currentTree.average().y }, gui::CENTER_CENTER, 0.5);
		}

		return 0;
	}
	void setParams(
		std::shared_ptr<TrackingOpenPoseEvent>& trackingTmp,
		std::shared_ptr<VideoOpenPoseEvent>& videoTmp,
		std::shared_ptr<PreviewOpenPoseEvent>& previewTmp
	)
	{
		tracking = trackingTmp;
		video = videoTmp;
		preview = previewTmp;

		// �}�E�X�C�x���g����
		preview->addMouseEventListener([&](int event, int x, int y) {
			switch (event)
			{
			// �}�E�X�ړ���
			case cv::EVENT_MOUSEMOVE:
				break;

			// ���N���b�N��
			case cv::EVENT_LBUTTONDOWN:
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
			std::cout << key << std::endl;

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

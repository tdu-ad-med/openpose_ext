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
	op::PeopleList& people;
	op::PeopleLineCounter peopleLineCounter;
	cv::Point mouse;
public:
	CustomOpenPoseEvent(std::shared_ptr<TrackingOpenPoseEvent>& trackingTmp) : people(trackingTmp->people) {}
	virtual ~CustomOpenPoseEvent() {};
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		/* �l���J�E���g */
		peopleLineCounter.setLine(579, 578, 1429, 577, 100.0);  // ����̍��W���X�V(�n�_��x���W, �n�_��y���W, �I�_��x���W, �I�_��y���W)
		peopleLineCounter.update(people);  // �J�E���g�̍X�V
		peopleLineCounter.drawLine(imageInfo.outputImage);  // ����̕`��
		// ������̃J�E���g��\��
		gui::text(imageInfo.outputImage, std::string("up : ") + std::to_string(peopleLineCounter.getUpCount()), { 20, 200 });
		// �������̃J�E���g��\��
		gui::text(imageInfo.outputImage, std::string("down : ") + std::to_string(peopleLineCounter.getDownCount()), { 20, 230 });

		/* �g���b�L���O�̎n�_�ƏI�_�����Ԓ�����`�� */
		for (size_t index : people.getCurrentIndices()) {  // ���݂̃t���[���ɉf���Ă��邷�ׂĂ̐l��ID��z��Ŏ擾���A�l�������[�v
			auto firstTree = people.getFirstTree(index);  // ID�Ԗڂ̐l���ŏ��ɉf�����Ƃ��̍��i���擾
			auto currentTree = people.getCurrentTree(index);  // ID�Ԗڂ̐l�̌��݂̃t���[���̍��i���擾
			if ((!firstTree.isValid()) || (!currentTree.isValid())) continue;  // �擾�������i���L���Ȓl�ł��邩�m�F
			cv::line(imageInfo.outputImage, { (int)firstTree.average().x, (int)firstTree.average().y }, { (int)currentTree.average().x, (int)currentTree.average().y }, cv::Scalar{
				(double)((int)((std::sin((double)index * 463763.0) + 1.0) * 100000.0) % 120 + 80),
				(double)((int)((std::sin((double)index * 1279.0) + 1.0) * 100000.0) % 120 + 80),
				(double)((int)((std::sin((double)index * 92763.0) + 1.0) * 100000.0) % 120 + 80)
				}, 2.0);
		}

		cv::setMouseCallback("result", [](int event, int x, int y, int flags, void* userdata) {
			if (event == cv::EVENT_MOUSEMOVE)
			{
				((cv::Point*)userdata)->x = x;
				((cv::Point*)userdata)->y = y;
				std::cout << x << ", " << y << std::endl;
			}
		}, (void*)(&mouse));

		return 0;
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;


	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath =
		R"(C:\Users\�ēc��\Documents\VirtualUsers\17ad105\Videos\IMG_1533.mp4)";
	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = std::regex_replace(videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";


	// �g���b�L���O�����̒ǉ�
	auto tracking = mop.addEventListener<TrackingOpenPoseEvent>();
	// sql���o�͋@�\�̒ǉ�
	auto sql = mop.addEventListener<SqlOpenPoseEvent>(sqlPath, false, 60);
	// ����ǂݍ��ݏ����̒ǉ�
	(void)mop.addEventListener<VideoOpenPoseEvent>(videoPath);
	// �o�͉摜�ɕ����Ȃǂ�`�悷�鏈���̒ǉ�
	(void)mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, true);
	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	(void)mop.addEventListener<CustomOpenPoseEvent>(tracking);
	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	(void)mop.addEventListener<PreviewOpenPoseEvent>("result");


	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

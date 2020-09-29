#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <Utils/Gui.h>

class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	cv::Mat image;

public:
	int init() override final
	{
		// 500x500�̍��F�̉摜�𐶐�
		image = cv::Mat::zeros(500, 500, CV_8UC3);

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// �摜�����
		imageInfo.inputImage = image;

		// ���i���o�����Ȃ�
		imageInfo.needOpenposeProcess = false;

		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		gui::text(imageInfo.outputImage, "Press 'A'", { 20, 20 });
		gui::text(imageInfo.outputImage, "Press 'Space'", { 20, 50 });
		return 0;
	}
	void registPreview(const std::shared_ptr<PreviewOpenPoseEvent> preview)
	{
		// �}�E�X�C�x���g����
		preview->addMouseEventListener([&](int event, int x, int y) {
			switch (event)
			{
			// �}�E�X�ړ���
			case cv::EVENT_MOUSEMOVE:
				gui::text(image, "A", { x, y });
				break;

			// ���N���b�N��
			case cv::EVENT_LBUTTONDOWN:
				break;

			// �E�N���b�N��
			case cv::EVENT_RBUTTONDOWN:
				break;
			}
		});

		// �L�[�C�x���g����
		preview->addKeyboardEventListener([&](int key) {
			switch (key)
			{
			// A�L�[�ŐF�𔽓]
			case 'a':
				cv::bitwise_not(image, image);
				break;

			// �X�y�[�X�L�[�ŉ摜��������
			case 32:
				image = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);
				break;
			}
		});
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;

	// �����Œ�`�����C�x���g���X�i�[��o�^����
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	auto preview = mop.addEventListener<PreviewOpenPoseEvent>("example05_MouseAndKeyboard");

	// �����Œ�`�����C�x���g���X�i�[��PreviewOpenPoseEvent�̃C���X�^���X��n��
	custom->registPreview(preview);

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

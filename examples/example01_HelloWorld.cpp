#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <Utils/Gui.h>

class CustomOpenPoseEvent : public OpenPoseEvent
{
public:
	int init() override final
	{
		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// 500x500�̍��F�̉摜�𐶐�
		cv::Mat image = cv::Mat::zeros(500, 500, CV_8UC3);

		// �摜�ɕ�����`��
		gui::text(image, "Hello World", { 20, 20 });

		// openpose�̓��͗p�̕ϐ��ɑ��
		imageInfo.inputImage = image;

		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// �摜�ɕ�����`��
		gui::text(imageInfo.outputImage, "ABC123", { 20, 50 });

		return 0;
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;

	// �����Œ�`�����C�x���g���X�i�[��o�^����
	mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	// Esc�ŏI������
	mop.addEventListener<PreviewOpenPoseEvent>("example01_HelloWorld");

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

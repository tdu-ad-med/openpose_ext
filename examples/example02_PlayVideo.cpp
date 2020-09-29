#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>

class CustomOpenPoseEvent : public OpenPoseEvent
{
public:
	int init() override final
	{
		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		return 0;
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;

	// ����ǂݍ��ݏ����̒ǉ�
	mop.addEventListener<VideoOpenPoseEvent>(R"(media/video.mp4)");

	// �o�͉摜�ɍ��i���Ȃǂ�`�悷�鏈���̒ǉ�
	mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);

	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	mop.addEventListener<PreviewOpenPoseEvent>("example02_PlayVideo");

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

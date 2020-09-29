#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
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

	// SQL���o�͋@�\�̒ǉ�
	// �w�肳�ꂽ�p�X��openpose�̉�͌��ʂ�ۑ�����
	// ���Ƀt�@�C�������݂��Ă���ꍇ��openpose�̉�͍͂s�킸�A�t�@�C���̃f�[�^��p����
	// �ȉ��̗�ł́A300�t���[�����Ƃ�media/video.mp4.sqlite3�։�͌��ʂ��X�V����
	mop.addEventListener<SqlOpenPoseEvent>(R"(media/video.mp4.sqlite3)", 300);

	// ����ǂݍ��ݏ����̒ǉ�
	mop.addEventListener<VideoOpenPoseEvent>(R"(media/video.mp4)");

	// �o�͉摜�ɍ��i���Ȃǂ�`�悷�鏈���̒ǉ�
	mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);

	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	mop.addEventListener<PreviewOpenPoseEvent>("example03_OutputSQL");

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

/*

openpose_ext��OpenPose�����V���v���Ɉ�����悤�ɂ������C�u�����ł��B
���̃��C�u�����ł́A���̂悤�ȗ���Ńv���O���������s���܂��B

	1. OpenPose�ɓ��͂���摜����������
	2. OpenPose�ŉ摜���獜�i����͂���
	3. OpenPose�ɂ���ĉ�͂��ꂽ�f�[�^���󂯎��
	4. 1�ɖ߂�

���̓��A1��3�������Œ�`���ăv���O������݌v���܂��B

�ȉ��̃T���v����openpose_ext��p�����ŏ����̃v���O�����ł��B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <Utils/Gui.h>

// OpenPoseEvent
class CustomOpenPoseEvent : public OpenPoseEvent
{
public:
	// ���������ɌĂяo�����֐�
	int init() override final
	{
		// �ŏ��ɂ��̊֐����Ă΂��
		// ����ɏ������I�������ꍇ��0��Ԃ�
		// �G���[�Ȃǂ��N�������ꍇ�ɂ�1��Ԃ�
		return 0;
	}

	// OpenPose�ɓ��͂���摜����������֐�
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// openpose�̉摜�������s����O�ɂ��̊֐����Ă΂��
		// ������imageInfo.inputImage�ɉ摜���������ނƁA���ꂪopenpose�̓��͉摜�Ƃ��ď��������
		// �v���O�������I�����������ꍇ�ɂ́A������exit�֐����Ăяo��
		// OpenPoseEvent�������o�^����Ă���ꍇ�́A�o�^�������Ԃł��̊֐����Ă΂��
		// ����ɏ������I�������ꍇ��0��Ԃ�
		// �G���[�Ȃǂ��N�������ꍇ�ɂ�1��Ԃ�
		return 0;
	}

	// OpenPose�ɂ���ĉ�͂��ꂽ�f�[�^���󂯎��֐�
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// openpose�̉摜�������s��ꂽ��ɂ��̊֐����Ă΂��
		// ������imageInfo�����͌��ʂ̃f�[�^�𓾂邱�Ƃ��ł���
		// �v���O�������I�����������ꍇ�ɂ́A������exit�֐����Ăяo��
		// OpenPoseEvent�������o�^����Ă���ꍇ�́A�o�^�������ԂƋt�̏��Ԃł��̊֐����Ă΂��
		// ����ɏ������I�������ꍇ��0��Ԃ�
		// �G���[�Ȃǂ��N�������ꍇ�ɂ�1��Ԃ�
		return 0;
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;

	// �����Œ�`�����C�x���g���X�i�[��o�^����
	// addEventListener�̓e���v���[�g�����Ɏw�肵���N���X��shared_ptr�ŃC���X�^���X�����A�����Ԃ�
	// addEventListener�Ɉ������w�肵���ꍇ�A�e���v���[�g�����Ɏw�肵���N���X�̃R���X�g���N�^�����Ƃ��ēW�J�����
	mop.addEventListener<CustomOpenPoseEvent>();

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;


	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath =
		R"(C:\Users\�ēc��\Documents\VirtualUsers\17ad105\Videos\58.mp4)";
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
	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	(void)mop.addEventListener<PreviewOpenPoseEvent>("result");


	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

/*

openpose_ext �ł͉摜�����łȂ�����̏������\�ł��B
���̂��߁A����̎p��������s���T���v����p�ӂ��܂����B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Video.h>
#include <Utils/Preview.h>
#include <Utils/VideoControllerUI.h>
#include <Utils/PlotInfo.h>

int main(int argc, char* argv[])
{
	// MinimumOpenPose �̏�����������
	MinOpenPose openpose(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// OpenPose �ɓ��͂��铮���p�ӂ���
	// "media/video.mp4" �͓��͂��铮��t�@�C���̃p�X���w�肷��
	Video video;
	video.open("media/video.mp4");

	// ������v���r���[���邽�߂̃E�B���h�E�𐶐�����
	Preview preview("result");

	// ����̃X�L�b�v�Ȃǂ��ł���悤�ɂ���
	VideoControllerUI videoControllUI;
	videoControllUI.addShortcutKeys(preview, video);  // �V���[�g�J�b�g�L�[�̒ǉ�

	// ���悪�I���܂Ń��[�v����
	while (true)
	{
		// ����̎��̃t���[����ǂݍ���
		cv::Mat image = video.next();

		// �f�����I�������ꍇ�̓��[�v�𔲂���
		if (image.empty()) break;

		// OpenPose �Ŏp�����������
		auto people = openpose.estimate(image);

		// �p������̌��ʂ� image �ɕ`�悷��
		plotBone(image, people, openpose);

		// ��ʂ��X�V����
		int ret = preview.preview(image);

		// �Đ��̑����ʂ�\������
		videoControllUI.showUI(video);

		// Esc�L�[�������ꂽ��I������
		if (0x1b == ret) break;
	}

	return 0;
}
/*

openpose_ext �� OpenPose �����V���v���Ɉ�����悤�ɂ������C�u�����ł��B
���̃��C�u�����ł́A���̂悤�ȗ���Ńv���O���������s���܂��B

	1. OpenPose �ɉ摜����͂���
	2. OpenPose ���獜�i�f�[�^���Ԃ����

�܂��A�摜�̑���͂����悻 OpenCV �ōs���Ă��܂��B
���� cv:: �ƕt���Ă���֐�����ϐ����� OpenCV �ł��B

�ȉ��̃T���v���v���O������ openpose_ext ��p�����ŏ����̃v���O�����ł��B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/PlotInfo.h>

int main(int argc, char* argv[])
{
	// OpenPose �̏�����������
	MinOpenPose mop(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// OpenPose �ɓ��͂���摜��p�ӂ���
	// "media/human.jpg" �͓��͂���摜�t�@�C���̃p�X���w�肷��
	cv::Mat image = cv::imread("media/human.jpg");

	// OpenPose �Ŏp�����������
	auto people = mop.estimate(image);

	// �p������̌��ʂ� image �ɕ`�悷��
	plotBone(image, people, mop);

	// �ł����������摜��\������
	cv::imshow("result", image);

	// �L�[���͂�����܂őҋ@����
	cv::waitKey(0);

	return 0;
}
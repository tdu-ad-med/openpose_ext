/*

�v���O�������ɏo�Ă��� MinimumOpenPose �� OpenPose �����V���v���Ɉ�����悤�ɂ������C�u�����ł��B
�ȉ��̃T���v���v���O������ MinimumOpenPose ��p�����ŏ����̃v���O�����ł��B

�܂��A�摜�̑���͂����悻 OpenCV �ōs���Ă��܂��B
���� cv:: �ƕt���Ă���֐�����ϐ����� OpenCV �Œ�`����Ă�����̂ł��B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/PlotInfo.h>

int main(int argc, char* argv[])
{
	// MinimumOpenPose �̏�����
	MinOpenPose openpose(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// OpenPose �ɓ��͂���摜��p�ӂ���
	cv::Mat image = cv::imread("media/human.jpg");

	// OpenPose �Ŏp�����������
	auto people = openpose.estimate(image);

	// �p������̌��ʂ� image �ɕ`�悷��
	plotBone(image, people, openpose);

	// �ł����������摜��\������
	cv::imshow("result", image);

	// �L�[���͂�����܂őҋ@����
	cv::waitKey(0);

	return 0;
}
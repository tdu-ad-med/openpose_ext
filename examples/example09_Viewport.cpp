/*

�����ł́A�n�ʂ��ォ�猩���悤�ɉ摜��ϊ�����v���O�������Љ�܂��B
���̃v���O������ OpenPose �ŕ��s�O�Ղ����߂��ۂɁA�n�ʂ��ォ�猩���悤�ȋO�Ղɕϊ��������Ƃ��Ȃǂɖ𗧂��܂��B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Vector.h>

int main(int argc, char* argv[])
{
	// ���͉摜
	cv::Mat before = cv::imread("media/checker.png");

	// �ˉe�ϊ�������N���X
	vt::ScreenToGround screenToGround;

	// �J�����̘c�݂�␳����ݒ�
	screenToGround.setCalibration(
		// �J�����L�����u���[�V�������s�������̃J�����̉𑜓x, �o�͉摜�̊g�嗦
		1920, 1080, 0.5,
		// �J���������p�����[�^�̏œ_�����ƒ��S���W(fx, fy, cx, cy)
		1222.78852772764, 1214.377234799321, 967.8020317677116, 569.3667691760459,
		// �J�����̘c�݌W��(k1, k2, k3, k4)
		-0.08809225804249926, 0.03839093574614055, -0.060501971675431955, 0.033162385302275665
	);

	// �J�����̉f�����A�n�ʂ��ォ�猩���悤�ȉf���Ɏˉe�ϊ�����
	screenToGround.setParams(
		// �J�����̉𑜓x
		1280, 960,
		// �J�����Ɏʂ��Ă���n�ʂ̔C�ӂ�4�_ (����A�E��A�E���A����)
		461, 334,
		1001, 243,
		1056, 669,
		348, 656,
		// ��L��4�_�̂����A1�_�ڂ���2�_�ڂ܂ł̒����ƁA2�_�ڂ���3�_�ڂ܂ł̒��� (�P�ʂ͔C��)
		100.0, 100.0
	);

	// �f�����ォ�猩���悤�Ɏˉe�ϊ�
	cv::Mat after = screenToGround.translateMat(before, 0.3f, true);

	// ��ʏ�̔C�ӂ̓_��n�ʏ�̃��[�g���P�ʂł̍��W�ɕϊ�����
	auto point = screenToGround.translate({ 1001, 243 });
	std::cout
		<< "x: " << point.x << " (%), "
		<< "y: " << point.y << " (%)"
		<< std::endl;

	// �ł����������摜��\������
	cv::imshow("before", before);
	cv::imshow("after", after);

	// �L�[���͂�����܂őҋ@����
	cv::waitKey(0);

	return 0;
}
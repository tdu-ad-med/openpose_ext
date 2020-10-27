/*

MinimumOpenPose �ł� MinOpenPose::People �Ƃ����f�[�^�^�ō��i���������܂��B
���̃T���v���ł� MinOpenPose::People �̈������ɂ��Ă�������܂��B

*/


#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/PlotInfo.h>
#include <iostream>

int main(int argc, char* argv[])
{
	// MinimumOpenPose �̏�����������
	MinOpenPose openpose(op::PoseModel::BODY_25, op::Point<int>(-1, 368));

	// OpenPose �ɓ��͂���摜��p�ӂ���
	cv::Mat image = cv::imread("media/human.jpg");

	// OpenPose �Ŏp�����������
	MinOpenPose::People people = openpose.estimate(image);

	// ������ OpenPose ���� people ���Ԃ��ꂽ�B
	// people �� ��ʂɉf��l���ׂĂ̍��i���������Ă���B
	// ���̂��߁A����͉f���Ă���l���̐��������[�v����B

	for (auto person_itr = people.begin(); person_itr != people.end(); person_itr++)
	{
		// 1�l���̍��i�����擾����
		MinOpenPose::Person person = person_itr->second;

		// 1�l���̍��i���̒��ɂ͊֐߂̍��W���z��Ŋi�[����Ă���B
		// (BODY_25 ���f�����g�p���Ă���ꍇ��25�̊֐ߍ��W�������Ă���)
		// ���̂��߁A���x�͊֐߂̐��������[�v����B

		for (MinOpenPose::Node node : person)
		{
			// �֐�1���̏�� node �ϐ��Ɋi�[�����B
			// �֐߂̏��ɂ͉�ʏ�� XY ���W�ƐM���l���i�[����Ă���B
			// �M���l�Ƃ́u�֐߂ł���\���v�̂悤�Ȓl�ł���A�Ⴏ��ΒႢ�قǐ��x���Ⴂ�B
			// �����ł́A����3�̒l���R���\�[���ɏo�͂���B
			std::cout
				<< "x: " << node.x << ", "  // �֐߂̉�ʏ��X���W
				<< "y: " << node.y << ", "  // �֐߂̉�ʏ��X���W
				<< "confidence: " << node.confidence  // �֐߂̐M���l
				<< std::endl;
		}
	}

	// �p������̌��ʂ� image �ɕ`�悷��
	plotBone(image, people, openpose);

	// �ł����������摜��\������
	cv::imshow("result", image);

	// �L�[���͂�����܂őҋ@����
	cv::waitKey(0);

	/**
	 
	�⑫

	MinOpenPose::People �� MinOpenPose::Person �Ȃǂ̒��g���������ꍇ�͐錾���`���m�F����Ɨǂ��ł��B
	�������̃v���O������ Visual Studio �Ŏ��s���Ă���ꍇ�� People �̕������E�N���b�N���āu�錾�ֈړ��v��u��`�ֈړ��v�Ȃǂ��I���ł��܂��B
	(�����̃V���[�g�J�b�g�L�[�� F12 �� Ctrl+F12 �ł��B)
	����ɂ��A�ϐ���֐��A�N���X�̒��g���ǂ��Ȃ��Ă���̂����ȒP�Ɋm�F���ɍs�����Ƃ��ł��܂��B

	����̗�ł���� MinOpenPose::People �̐��̂� std::map<size_t, Person> �ł��邱�Ƃ��킩��܂��B
	�܂� MinOpenPose::Person �̐��̂� std::vector<Node> �ł��B
	MinOpenPose::Node �̐��̂͒P�Ȃ�\���̂ł��B

	��:�@std:: ����n�܂��Ă���֐�����N���X���� C++ �̕W�����C�u�����ł��B


	*/

	return 0;
}
/*

openpose_ext �ł͉摜�� OpenCV �� cv::Mat ��p���ď��������Ă��܂��B
���̉摜�ɕ�����}�`���������݂����ꍇ���o�Ă���Ǝv���̂ŁA�}�`��\���T���v����p�ӂ��܂����B

*/

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Gui.h>

int main(int argc, char* argv[])
{
	// 500x500�̔��F�̉摜�𐶐�����
	cv::Mat image = cv::Mat(500, 500, CV_8UC3, { 255, 255, 255 });

	// "Hello"�Ƃ���������(100, 50)�̈ʒu�ɍ�������_�Ƃ���0.7�̃T�C�Y�ŐԐF�ŕ\������
	gui::text(image, "Hello", { 100, 50 }, gui::LEFT_TOP, 0.7f, { 0, 0, 255 });

	// "World"�Ƃ���������(200, 70)�̈ʒu�ɍ�������_�Ƃ���2.0�̃T�C�Y�ŐF�ŕ\������
	gui::text(image, "World", { 200, 70 }, gui::LEFT_TOP, 2.0f, { 255, 0, 0 });

	// �~�`
	cv::circle(image, { 250, 250 }, 100, { 0, 255, 0 }, -1);
	cv::circle(image, { 270, 270 }, 100, { 100, 100, 100 }, 5);

	// ��`
	cv::rectangle(image, { 350, 400, 100, 200 }, { 0, 0, 255 }, 10);

	// ����
	cv::line(image, { 20, 700 }, { 300, 300 }, { 255, 0, 0 }, 6);

	// �ł����������摜��\������
	cv::imshow("result", image);

	// �L�[���͂�����܂őҋ@����
	cv::waitKey(0);

	return 0;
}
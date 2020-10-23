/*

�v���O�����̎��s���Ƀ}�E�X��L�[�{�[�h�Ȃǂ��g���ĉ�ʂ𑀍삷��T���v����p�ӂ��܂����B
�}�E�X���ړ����邱�Ƃŉ�ʂɊG��`�����Ƃ��ł��܂��B
�܂��A�X�y�[�X�L�[�ŉ�ʂ����Z�b�g�ł��܂��B

*/

#include <Utils/Preview.h>

int main(int argc, char* argv[])
{
	// �摜���v���r���[���邽�߂̃E�B���h�E�𐶐�����
	Preview preview("result");

	// 500x500�̔��F�̉摜�𐶐�����
	cv::Mat image = cv::Mat(500, 500, CV_8UC3, { 255, 255, 255 });

	// �����_����p���ă}�E�X����C�x���g���X�i�[�̓o�^���ł���
	preview.addMouseEventListener([&](int event, int x, int y) {
		// �}�E�X���������Ƃ�
		if (cv::EVENT_MOUSEMOVE == event)
		{
			// �}�E�X�̈ʒu�ɉ~��`��
			cv::circle(image, { x, y }, 2, { 0, 0, 0 }, -1);
		}

		// ���N���b�N�������ꂽ�Ƃ�
		if (cv::EVENT_LBUTTONDOWN == event)
		{
			// �������Ȃ�
		}

		// �E�N���b�N�������ꂽ�Ƃ�
		if (cv::EVENT_RBUTTONDOWN == event)
		{
			// �������Ȃ�
		}

		// OpenCV �̃}�E�X�C�x���g�͑��ɂ����邪�A�����ł͊�������
	});

	// �����_����p���ăL�[�{�[�h����C�x���g���X�i�[�̓o�^���ł���
	preview.addKeyboardEventListener([&](int key) {
		// �X�y�[�X�L�[�������ꂽ�Ƃ�
		if (32 == key)
		{
			// ��ʂ����Z�b�g����
			image = cv::Mat(500, 500, CV_8UC3, { 255, 255, 255 });
		}

		// A�L�[�������ꂽ�Ƃ�
		if ('a' == key)
		{
			// �������Ȃ�
		}

		// B�L�[�������ꂽ�Ƃ�
		if ('b' == key)
		{
			// �������Ȃ�
		}

		// OpenCV �̃L�[�C�x���g�͑��ɂ����邪�A�����ł͊�������
	});

	// Esc���������܂Ŗ������[�v����
	while (true) {
		// ��ʂ��X�V����
		int key = preview.preview(image, 33);

		// Esc�������ꂽ�烋�[�v�𔲂���
		if (0x1b == key) break;
	}

	return 0;
}
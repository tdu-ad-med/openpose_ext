#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <Utils/Vector.h>
#include <Utils/Gui.h>

class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	cv::Mat image;
	vt::ScreenToGround screenToGround;

public:
	uint8_t previewMode = 0;

	int init() override final
	{
		// �摜��ǂݍ���
		image = cv::imread(R"(media/checker.png)", 1);

		// �J�����L�����u���[�V�����œ���ꂽ�p�����[�^�̐ݒ�
		screenToGround.setCalibration(
			// �J�����L�����u���[�V�����ɗp�����摜�̉𑜓x(w, h), �o�͉摜�̊g�嗦
			1858.0, 1044.0, 0.5,
			// �J���������p�����[�^�̏œ_�����ƒ��S���W(fx, fy, cx, cy)
			1057, 1057, 935, 567,
			// �J�����̘c�݌W��(k1, k2, k3, k4)
			0.0, 0.0, 0.0, 0.0
		);

		// �B�e�ʒu�Ȃǂ̎w��
		screenToGround.setParams(
			// �摜�̉𑜓x
			image.cols, image.rows,
			// �J�����̐�����p(deg)�ƒn�ʂ���̍���(m)
			60.0, 6.3,
			// ��ʏ�̒n�ʂ̍��W1
			346, 659,
			// ��ʏ�̒n�ʂ̍��W2
			1056, 668,
			// ��ʏ�̒n�ʂ̍��W3
			990, 202,
			// ��ʏ�̒n�ʂ̍��W4
			478, 292
		);

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// openpose�ɉ摜�����
		imageInfo.inputImage = image;

		// ���i���o�����Ȃ�
		imageInfo.needOpenposeProcess = false;

		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		auto output = imageInfo.outputImage;

		// �w�肵��4�_�͈̔͂ɐ���`��
		screenToGround.drawAreaLine(output, 0);

		// �摜�ɕ�����`��
		gui::text(output, "Press 'A'", { output.cols/2, output.rows/2}, gui::CENTER_CENTER, 3.0);

		// ���჌���Y�̘c�݂�����
		if (previewMode == 1)
		{
			output = screenToGround.onlyFlatMat(output);
		}

		// �n�ʂ��ォ�猩���悤�Ɏˉe�ϊ�
		if (previewMode == 2)
		{
			output = screenToGround.translateMat(output, 0.3f);
		}

		imageInfo.outputImage = output;

		return 0;
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;

	// �����Œ�`�����C�x���g���X�i�[��o�^����
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	auto preview = mop.addEventListener<PreviewOpenPoseEvent>("example04_Viewport");

	// �v���r���[�E�B���h�E�ɃL�[�C�x���g��ǉ�
	preview->addKeyboardEventListener([&](int key)
	{
		// 'A'�L�[�������ꂽ�ꍇ
		if (key == 'a')
		{
			// �\�����[�h��؂�ւ���
			custom->previewMode = (custom->previewMode + 1) % 3;
		}
	});

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

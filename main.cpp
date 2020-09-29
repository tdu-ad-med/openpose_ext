#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PeopleCounterOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	cv::Mat image;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode = 0;

public:
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
			112.0, 6.3,
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
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final {
		// openpose�ɉ摜�����
		imageInfo.inputImage = image;

		// ���i���o�����Ȃ�
		imageInfo.needOpenposeProcess = false;

		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// ����
		imageInfo.outputImage = imageInfo.outputImage.clone();

		if (previewMode == 1) imageInfo.outputImage = screenToGround.onlyFlatMat(imageInfo.outputImage);
		if (previewMode == 2) imageInfo.outputImage = screenToGround.translateMat(imageInfo.outputImage, 0.3f);

		return 0;
	}
	void registPreview(const std::shared_ptr<PreviewOpenPoseEvent> preview)
	{
		if (!preview) return;
		this->preview = preview;

		// �}�E�X�C�x���g����
		preview->addMouseEventListener([&](int event, int x, int y) {
			if (event == cv::EVENT_MOUSEMOVE)   { mouse.x = x; mouse.y = y; }
			if (event == cv::EVENT_LBUTTONDOWN) { std::cout << x << ", " << y << std::endl; }
		});

		// �L�[�C�x���g����
		preview->addKeyboardEventListener([&](int key) {
			if (key == 'a') { previewMode = (previewMode + 1) % 3; }
		});
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;

	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();

	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	auto preview = mop.addEventListener<PreviewOpenPoseEvent>("result");

	custom->registPreview(preview);

	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

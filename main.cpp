#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PeopleCounterOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

struct InputInfo {
	double x1, y1, x2, y2, x3, y3, x4, y4;
	double h; std::string path;

	InputInfo(
		double x1, double y1, double x2, double y2,
		double x3, double y3, double x4, double y4,
		double h, std::string path
	) : x1(x1), y1(y1), x2(x2), y2(y2), x3(x3), y3(y3), x4(x4), y4(y4), h(h), path(path) {}
};

class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	cv::Mat image;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode = 0;
	std::vector<InputInfo> inputInfo;
	size_t index = 0;

public:
	int init() override final
	{
		inputInfo.push_back(InputInfo(
			702, 792,
			822, 527,
			611, 502,
			362, 679,
			0.835, R"(C:\Users\0214t\Downloads\Gmail\1.JPG)"
		));
		inputInfo.push_back(InputInfo(
			258, 768,
			1068, 755,
			972, 462,
			359, 431,
			0.835, R"(C:\Users\0214t\Downloads\Gmail\2.JPG)"
		));
		inputInfo.push_back(InputInfo(
			291, 506,
			778, 898,
			979, 581,
			529, 390,
			0.835, R"(C:\Users\0214t\Downloads\Gmail\3.JPG)"
		));
		inputInfo.push_back(InputInfo(
			360, 200,
			659, 829,
			1108, 469,
			666, 122,
			0.535, R"(C:\Users\0214t\Downloads\Gmail\4.JPG)"
		));

		// �摜��ǂݍ���
		image = cv::imread(inputInfo[index].path, 1);

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
		auto img = imageInfo.outputImage.clone();

		// �J�����L�����u���[�V�����œ���ꂽ�p�����[�^�̐ݒ�
		screenToGround.setCalibration(
			// �J�����L�����u���[�V�����ɗp�����摜�̉𑜓x(w, h), �o�͉摜�̊g�嗦
			//1858.0, 1044.0, 0.5,
			1280, 960, 0.5,
			// �J���������p�����[�^�̏œ_�����ƒ��S���W(fx, fy, cx, cy)
			662.3344753306436, 661.0571574290184, 647.0302662540407, 485.9472740856155,  // auto calib(circle)
			// �J�����̘c�݌W��(k1, k2, k3, k4)
			-0.21123964017450284, 0.44489235639980457, -1.2246104163807603, 0.9463553619297811
		);

		// �B�e�ʒu�Ȃǂ̎w��
		screenToGround.setParams(
			// �摜�̉𑜓x
			image.cols, image.rows,
			// �J�����̐�����p(deg)�ƒn�ʂ���̍���(m)
			112.0, inputInfo[index].h,
			// ��ʏ�̒n�ʂ̍��W1
			inputInfo[index].x1, inputInfo[index].y1,
			// ��ʏ�̒n�ʂ̍��W2
			inputInfo[index].x2, inputInfo[index].y2,
			// ��ʏ�̒n�ʂ̍��W3
			inputInfo[index].x3, inputInfo[index].y3,
			// ��ʏ�̒n�ʂ̍��W4
			inputInfo[index].x4, inputInfo[index].y4
		);

		//check();

		if (previewMode == 0) {
			screenToGround.drawAreaLine(img, 0);
		}
		if (previewMode == 1) {
			img = screenToGround.onlyFlatMat(img);
			screenToGround.drawAreaLine(img, 1);
		}
		if (previewMode == 2) {
			img = screenToGround.translateMat(img, 0.3f, true);
		}

		imageInfo.outputImage = img;

		return 0;
	}
	void check() {
		// �B�e�ʒu�Ȃǂ̎w��
		screenToGround.setParams(
			// �摜�̉𑜓x
			image.cols, image.rows,
			// �J�����̐�����p(deg)�ƒn�ʂ���̍���(m)
			112.0, inputInfo[index].h,
			// ��ʏ�̒n�ʂ̍��W1
			inputInfo[index].x1, inputInfo[index].y1,
			// ��ʏ�̒n�ʂ̍��W2
			inputInfo[index].x2, inputInfo[index].y2,
			// ��ʏ�̒n�ʂ̍��W3
			inputInfo[index].x3, inputInfo[index].y3,
			// ��ʏ�̒n�ʂ̍��W4
			inputInfo[index].x4, inputInfo[index].y4
		);

		std::string str = "";
		str += "================\n";
		str += inputInfo[index].path + "\n";
		str += "calibration: " + std::string(screenToGround.fisheyeToFlat.is_init ? "ON" : "OFF") + "\n";
		auto s = screenToGround.translate(vt::Vector4(inputInfo[index].x3, inputInfo[index].y3)) * 100.0;
		s.x = std::abs(s.x); s.y = std::abs(s.y);
		if (s.x < s.y) std::swap(s.x, s.y);
		str += "real: " + std::to_string(146.5) + "(cm) x " + std::to_string(88.5) + "(cm)\n";
		str += "size: " + std::to_string(s.x) + "(cm) x " + std::to_string(s.y) + "(cm)\n";
		double x = 100.0 * std::abs(s.x - 146.5) / 146.5;
		double y = 100.0 * std::abs(s.y - 88.5) / 88.5;
		str += "diff: " + std::to_string(x) + "(%) x " + std::to_string(y) + "(%)\n";

		std::cout << str << std::endl;
	}
	void registPreview(const std::shared_ptr<PreviewOpenPoseEvent> preview)
	{
		if (!preview) return;
		this->preview = preview;

		// �}�E�X�C�x���g����
		preview->addMouseEventListener([&](int event, int x, int y) {
			if (event == cv::EVENT_MOUSEMOVE)   { mouse.x = x; mouse.y = y; }
			if (event == cv::EVENT_LBUTTONDOWN) { std::cout << x << ", " << y << "," << std::endl; }
		});

		// �L�[�C�x���g����
		preview->addKeyboardEventListener([&](int key) {
			if (key == 'a') { previewMode = (previewMode + 1) % 3; }
			if (key == 'n') {
				index = (index + 1) % inputInfo.size();
				image = cv::imread(inputInfo[index].path, 1);
			}
			if (key == 'p') {
				index = (index + inputInfo.size() - 1) % inputInfo.size();
				image = cv::imread(inputInfo[index].path, 1);
			}
			if (key == 32) {
				screenToGround.fisheyeToFlat.is_init = !screenToGround.fisheyeToFlat.is_init;
			}

			check();
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

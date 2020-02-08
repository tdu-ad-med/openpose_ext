#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

#include <Utils/Tracking.h>
class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	std::shared_ptr<TrackingOpenPoseEvent> tracking;
	std::shared_ptr<VideoOpenPoseEvent> video;
	std::shared_ptr<SqlOpenPoseEvent> sql;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	op::PeopleLineCounter peopleLineCounter;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode;

	int checkError()
	{
		if ((!tracking) || (!video) || (!sql) || (!preview))
		{
			std::cout
				<< "TrackingOpenPoseEvent, VideoOpenPoseEvent, SqlOpenPoseEvent, PreviewOpenPoseEvent�̂����ꂩ�����w��ł��B\n"
				<< "setParams�֐��Ő������l���w�肵�Ă��������B"
				<< std::endl;
			return 1;
		}
		return 0;
	}
public:
	CustomOpenPoseEvent() : previewMode(0) {}
	virtual ~CustomOpenPoseEvent() {}
	int init() override final
	{
		if (checkError()) return 1;

		// people_with_tracking�e�[�u�����Đ���
		sql->database->exec(u8"DROP TABLE IF EXISTS people_with_tracking");
		if (sql->createTableIfNoExist(
			u8"people_with_tracking",
			u8"frame INTEGER, people INTEGER, x REAL, y REAL",
			u8"frame"
		)) return 1;

		// people_with_normalized_tracking�e�[�u�����Đ���
		sql->database->exec(u8"DROP TABLE IF EXISTS people_with_normalized_tracking");
		if (sql->createTableIfNoExist(
			u8"people_with_normalized_tracking",
			u8"frame INTEGER, people INTEGER, x REAL, y REAL",
			u8"frame"
		)) return 1;

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (checkError()) return 1;

		// �l���J�E���g
		peopleLineCounter.setLine(579, 578, 1429, 577, 100.0);  // �J�E���g�̊���̍��W�ݒ�
		peopleLineCounter.updateCount(tracking->people);  // �J�E���g�̍X�V
		peopleLineCounter.drawJudgeLine(imageInfo.outputImage);  // ����̕`��
		peopleLineCounter.drawPeopleLine(imageInfo.outputImage, tracking->people, true);  // �l�X�̎n�_�ƏI�_�̕`��

		// �J�E���g��\��
		gui::text(imageInfo.outputImage, std::string("up : ") + std::to_string(peopleLineCounter.getUpCount()), { 20, 200 });
		gui::text(imageInfo.outputImage, std::string("down : ") + std::to_string(peopleLineCounter.getDownCount()), { 20, 230 });

		// �ˉe�ϊ�
		screenToGround.setParams(
			imageInfo.outputImage.cols, imageInfo.outputImage.rows, 33.3, 6.3,
			492, 436,
			863, 946,
			1335, 644,
			905, 242
		);
		screenToGround.drawAreaLine(imageInfo.outputImage);  // �ˉe�ϊ��Ɏg�p����4�_�͈̔͂�`��
		if (previewMode == 1) imageInfo.outputImage = screenToGround.perspective(imageInfo.outputImage); // �v���r���[

		// people_with_tracking�Apeople_with_normalized_tracking�e�[�u���̍X�V
		try
		{
			SQLite::Statement pwtQuery(*sql->database, u8"INSERT INTO people_with_tracking VALUES (?, ?, ?, ?)");
			SQLite::Statement pwntQuery(*sql->database, u8"INSERT INTO people_with_normalized_tracking VALUES (?, ?, ?, ?)");
			for (auto&& index : tracking->people.getCurrentIndices())
			{
				auto tree = tracking->people.getCurrentTree(index);
				if (tree.frameNumber != imageInfo.frameNumber) continue;
				auto position = tree.average();

				pwtQuery.reset();
				pwtQuery.bind(1, (long long)imageInfo.frameNumber);
				pwtQuery.bind(2, (long long)index);
				pwtQuery.bind(3, (double)position.x);
				pwtQuery.bind(4, (double)position.y);
				(void)pwtQuery.exec();

				auto normal = screenToGround.translate(vt::Vector4{ position.x, position.y });
				pwntQuery.reset();
				pwntQuery.bind(1, (long long)imageInfo.frameNumber);
				pwntQuery.bind(2, (long long)index);
				pwntQuery.bind(3, (double)normal.x);
				pwntQuery.bind(4, (double)normal.y);
				(void)pwntQuery.exec();
			}
		}
		catch (const std::exception& e)
		{
			std::cout << u8"error : " << __FILE__ << u8" : L" << __LINE__ << u8"\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}
	void setParams(
		const std::shared_ptr<TrackingOpenPoseEvent> trackingTmp,
		const std::shared_ptr<VideoOpenPoseEvent> videoTmp,
		const std::shared_ptr<SqlOpenPoseEvent> sqlTmp,
		const std::shared_ptr<PreviewOpenPoseEvent> previewTmp
	)
	{
		tracking = trackingTmp;
		video = videoTmp;
		sql = sqlTmp;
		preview = previewTmp;

		if (checkError()) return;

		// �}�E�X�C�x���g����
		preview->addMouseEventListener([&](int event, int x, int y) {
			switch (event)
			{
			// �}�E�X�ړ���
			case cv::EVENT_MOUSEMOVE:
				mouse.x = x; mouse.y = y;
				break;

			// ���N���b�N��
			case cv::EVENT_LBUTTONDOWN:
				std::cout << x << ", " << y << std::endl;
				break;

			// �E�N���b�N��
			case cv::EVENT_RBUTTONDOWN:
				break;

			// ���̑�
			default:
				break;
			}
		});

		// �L�[�C�x���g����
		preview->addKeyboardEventListener([&](int key) {
			switch (key)
			{
			// J�L�[��30�t���[���߂�
			case 'j':
				video->seekRelative(-30);
				break;

			// K�L�[��30�t���[���i��
			case 'k':
				video->seekRelative(30);
				break;

			// A�L�[�ŉ�ʕ\���ؑ�
			case 'a':
				previewMode = (previewMode + 1) % 2;
				break;

			// �X�y�[�X�L�[�œ���̍Đ�/�ꎞ��~
			case 32:
				if (video->isPlay()) video->pause();
				else video->play();
				break;

			// ���̑�
			default:
				break;
			}
		});
	}
};

int main(int argc, char* argv[])
{
	// openpose�̃��b�p�[�N���X
	MinimumOpenPose mop;


	// ���͂���f���t�@�C���̃t���p�X
	std::string videoPath =
		R"(G:\�v���o\Dropbox\Dropbox\SDK\openpose\video\IMG_1533.mp4)";
	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = std::regex_replace(videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";


	// �g���b�L���O�����̒ǉ�
	auto tracking = mop.addEventListener<TrackingOpenPoseEvent>();
	// sql���o�͋@�\�̒ǉ�
	auto sql = mop.addEventListener<SqlOpenPoseEvent>(sqlPath, 60);
	// ����ǂݍ��ݏ����̒ǉ�
	auto video = mop.addEventListener<VideoOpenPoseEvent>(videoPath);
	// �o�͉摜�ɕ����Ȃǂ�`�悷�鏈���̒ǉ�
	(void)mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);
	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();
	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	auto preview = mop.addEventListener<PreviewOpenPoseEvent>("result");


	custom->setParams(tracking, video, sql, preview);


	// openpose�̋N��
	int ret = mop.startup();

	return ret;
}

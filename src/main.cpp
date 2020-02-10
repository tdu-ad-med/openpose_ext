#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

#include <Utils/Tracking.h>
class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	op::PeopleList people{
		5,         // NUMBER_NODES_TO_TRUST
		0.5f,   // CONFIDENCE_THRESHOLD
		10,      // NUMBER_FRAMES_TO_LOST
		50.0f  // DISTANCE_THRESHOLD
	};
	std::shared_ptr<VideoOpenPoseEvent> video;
	std::shared_ptr<SqlOpenPoseEvent> sql;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	op::PeopleLineCounter peopleLineCounter;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode;

	int checkError()
	{
		if ((!video) || (!sql))
		{
			std::cout
				<< "VideoOpenPoseEvent, SqlOpenPoseEvent�̂����ꂩ�����w��ł��B\n"
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
		if (sql->deleteTableIfExist(u8"people_with_tracking")) return 1;
		if (sql->createTableIfNoExist(u8"people_with_tracking", u8"frame INTEGER, people INTEGER, x REAL, y REAL")) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_tracking", u8"frame", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_tracking", u8"people", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_tracking", u8"frame", u8"people", true)) return 1;

		// people_with_normalized_tracking�e�[�u�����Đ���
		if (sql->deleteTableIfExist(u8"people_with_normalized_tracking")) return 1;
		if (sql->createTableIfNoExist(u8"people_with_normalized_tracking", u8"frame INTEGER, people INTEGER, x REAL, y REAL")) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"frame", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"people", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"frame", u8"people", true)) return 1;

		// people_count�e�[�u�����쐬
		if (sql->deleteTableIfExist(u8"people_count")) return 1;
		if (sql->createTableIfNoExist(u8"people_count", u8"frame INTEGER PRIMARY KEY, up INTEGER, down INTEGER")) return 1;
		if (sql->createIndexIfNoExist(u8"people_count", u8"frame", true)) return 1;

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (checkError()) return 1;

		// �g���b�L���O
		people.addFrame(imageInfo);
		//people.addFrame(imageInfo, std::dynamic_pointer_cast<Database>(sql));

		// �l���J�E���g
		peopleLineCounter.setLine(579, 578, 1429, 577, 100.0);  // �J�E���g�̊���̍��W�ݒ�
		peopleLineCounter.updateCount(people);  // �J�E���g�̍X�V
		peopleLineCounter.drawJudgeLine(imageInfo.outputImage);  // ����̕`��
		peopleLineCounter.drawPeopleLine(imageInfo.outputImage, people, true);  // �l�X�̎n�_�ƏI�_�̕`��

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
		if (previewMode == 1) imageInfo.outputImage = screenToGround.perspective(imageInfo.outputImage, 0.3f); // �v���r���[

		// people_with_tracking�Apeople_with_normalized_tracking�e�[�u���̍X�V
		SQLite::Statement pwtQuery(*sql->database, u8"INSERT INTO people_with_tracking VALUES (?, ?, ?, ?)");
		SQLite::Statement pwntQuery(*sql->database, u8"INSERT INTO people_with_normalized_tracking VALUES (?, ?, ?, ?)");
		for (auto&& index : people.getCurrentIndices())
		{
			auto tree = people.getCurrentTree(index);
			if (tree.frameNumber != imageInfo.frameNumber) continue;
			auto position = tree.average();
			auto normal = screenToGround.translate(vt::Vector4{ position.x, position.y });
			if (sql->bindAllAndExec(pwtQuery, imageInfo.frameNumber, index, position.x, position.y)) return 1;
			if (sql->bindAllAndExec(pwntQuery, imageInfo.frameNumber, index, normal.x, normal.y)) return 1;
		}

		// people_count�e�[�u���̍X�V
		if (peopleLineCounter.isChanged())
		{
			SQLite::Statement pcQuery(*sql->database, u8"INSERT INTO people_count VALUES (?, ?, ?)");
			if (sql->bindAllAndExec(pcQuery, imageInfo.frameNumber, peopleLineCounter.getUpCount(), peopleLineCounter.getDownCount())) return 1;
		}

		return 0;
	}
	void setParams(
		const std::shared_ptr<VideoOpenPoseEvent> videoTmp,
		const std::shared_ptr<SqlOpenPoseEvent> sqlTmp,
		const std::shared_ptr<PreviewOpenPoseEvent> previewTmp = nullptr
	)
	{
		video = videoTmp;
		sql = sqlTmp;
		preview = previewTmp;

		if (checkError()) return;

		if (!preview) return;

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
		R"(C:\Users\�ēc��\Documents\VirtualUsers\17ad105\Videos\IMG_1533.mp4)";
	// ���o�͂���sql�t�@�C���̃t���p�X
	std::string sqlPath = std::regex_replace(videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";


	// sql���o�͋@�\�̒ǉ�
	auto sql = mop.addEventListener<SqlOpenPoseEvent>(sqlPath, 300);
	// ����ǂݍ��ݏ����̒ǉ�
	auto video = mop.addEventListener<VideoOpenPoseEvent>(videoPath);
	// �o�͉摜�ɕ����Ȃǂ�`�悷�鏈���̒ǉ�
	(void)mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);
	// �����Œ�`�����C�x���g���X�i�[�̓o�^
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();
	// �o�͉摜�̃v���r���[�E�B���h�E�𐶐����鏈���̒ǉ�
	auto preview = mop.addEventListener<PreviewOpenPoseEvent>("result");


	custom->setParams(video, sql, preview);


	// openpose�̋N��
	auto start = std::chrono::high_resolution_clock::now();
	int ret = mop.startup();
	double time = (1.0 / 1000.0) * (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	std::cout << "time score : " << time << " sec." << std::endl;

	return ret;
}

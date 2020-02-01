#include <MinOpenPose.h>
#include <regex>
#include <Database.h>

int createTable(Database& db)
{
	db->exec("DROP TABLE IF EXISTS test");
	std::string row_title = "frame INTEGER, people INTEGER";
	for (int i = 0; i < 25; i++)
	{
		row_title += ", joint" + std::to_string(i) + "x REAL";
		row_title += ", joint" + std::to_string(i) + "y REAL";
		row_title += ", joint" + std::to_string(i) + "confidence REAL";
	}
	try { db->exec("CREATE TABLE test (" + row_title + ")"); }
	catch (const std::exception & e) {
		std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int addRow(Database& db, std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>>& results)
{
	// SQL���̐���
	std::string row = "?";
	for (int colIndex = 0; colIndex < 76; colIndex++) row += ", ?";
	row = "INSERT INTO test VALUES (" + row + ")";
	// SQL���̒l���m�肷��
	try
	{
		SQLite::Statement query(*db, row);
		for (auto result : *results)
		{

			// ��ʓ��ɉf���Ă���l�������[�v����
			for (int peopleIndex = 0; peopleIndex < result->poseKeypoints.getSize(0); peopleIndex++)
			{
				query.reset();
				query.bind(1, (long long)result->frameNumber);
				query.bind(2, peopleIndex);
				for (int nodeIndex = 0; nodeIndex < result->poseKeypoints.getSize(1); nodeIndex++)
				{
					query.bind(3 + nodeIndex * 3 + 0, result->poseKeypoints[{peopleIndex, nodeIndex, 0}]);
					query.bind(3 + nodeIndex * 3 + 1, result->poseKeypoints[{peopleIndex, nodeIndex, 1}]);
					query.bind(3 + nodeIndex * 3 + 2, result->poseKeypoints[{peopleIndex, nodeIndex, 2}]);
				}
				// SQL�������s����
				(void)query.exec();

			}
		}
	}
	catch (const std::exception & e) {
		std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
		return 1;
	}
	return 0;
}

struct Node
{
	float x, y, confidence;
};

std::unique_ptr<std::vector<std::vector<Node>>> getPoseKeypoints(Database& db, size_t frameNumber)
{
	auto result = std::make_unique<std::vector<std::vector<Node>>>();
	SQLite::Statement query(*db, "SELECT * FROM test WHERE frame=?");
	query.bind(1, (long long)frameNumber);
	while (query.executeStep())
	{
		std::vector<Node> nodes;
		for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
		{
			nodes.push_back(Node{
				query.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
				query.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
				query.getColumn(2 + nodeIndex * 3 + 2).getDouble()
			});
		}
		result->push_back(nodes);
	}
	return result;
}

int main(int argc, char* argv[])
{
	// Web�J�����ɐڑ��J�n
	//cv::VideoCapture cap(0);
	//if (!cap.isOpened()) return -1;
	std::string videoPath = R"(C:/Users/�ēc��/Documents/VirtualUsers/17ad105/Videos/IMG_1533.mp4)";
	std::string sqlPath = std::regex_replace(videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";
	cv::VideoCapture cap(videoPath);
	Database db = createDatabase(sqlPath);
	bool isWriteMode = false;
	if (isWriteMode)
	{
		if (createTable(db)) return 1;
	}

	// OpenPose�̋N��
	MinimumOpenPose mop;
	mop.startup(isWriteMode);
	
	// OpenPose�̃C�x���g���[�v����
	while (true)
	{
		// OpenPose �̏�����Ԃ̊m�F
		switch (mop.getProcessState())
		{
		case MinimumOpenPose::ProcessState::WaitInput: // ���͑ҋ@
		{
			auto frame = std::make_unique<cv::Mat>();
			cap.read(*frame);
			mop.pushImage(std::move(frame)); // OpenPose �ɉ摜��n��
			if (cv::waitKey(1) == 0x1b) {
				mop.shutdown();
				return 0;
			}
		}
		break;
		case MinimumOpenPose::ProcessState::Processing: // ������
			break;
		case MinimumOpenPose::ProcessState::Finish: // �����̏I��
		{
			auto results = mop.getResultsAndReset(); // �o�͂��ꂽ�f�[�^�̎擾
			if (!static_cast<bool>(results)) break;

			if (isWriteMode)
			{
				if (addRow(db, results)) return 1;
			}

			for (auto result : *results)
			{
				if (!isWriteMode)
				{
					try
					{
						auto keyPoints = getPoseKeypoints(db, result->frameNumber);
						std::cout << result->frameNumber << std::endl;
					}
					catch (const std::exception & e)
					{
						std::cout << e.what() << std::endl;
						return 1;
					}
				}
				cv::imshow("result", result->cvOutputData);
			}
		}
		break;
		case MinimumOpenPose::ProcessState::Shutdown: // �N�����Ă��Ȃ�
			break;
		case MinimumOpenPose::ProcessState::Error: // �G���[����
		{
			auto errors = mop.getErrors();
			for (auto error : errors) std::cout << error << std::endl;
			mop.resetErrors();
			return -1;
		}
		break;
		}
	}

	return 0;
}

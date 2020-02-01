#include <MinOpenPose.h>
#include <regex>
#include <Database.h>

int main(int argc, char* argv[])
{
	// Web�J�����ɐڑ��J�n
	//cv::VideoCapture cap(0);
	//if (!cap.isOpened()) return -1;
	std::string videoPath = R"(C:/Users/�ēc��/Documents/VirtualUsers/17ad105/Videos/IMG_1533.mp4)";
	std::string sqlPath = std::regex_replace(videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";
	cv::VideoCapture cap(videoPath);
	Database db = createDatabase(sqlPath);
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

	// OpenPose�̋N��
	MinimumOpenPose mop;
	mop.startup();
	
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
				return 1;
			}
		}
		break;
		case MinimumOpenPose::ProcessState::Processing: // ������
			break;
		case MinimumOpenPose::ProcessState::Finish: // �����̏I��
		{
			auto results = mop.getResultsAndReset(); // �o�͂��ꂽ�f�[�^�̎擾
			if (!static_cast<bool>(results)) break;
			
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
					cv::imshow("result", result->cvOutputData);

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

#pragma once

#include <OpenPoseWrapper/OpenPoseEvent.h>
#include <Utils/Database.h>
#include <regex>

class SqlOpenPoseEvent : public OpenPoseEvent
{
private:
	bool writeMode;
	std::string videoPath;
	std::string sqlPath;
	cv::VideoCapture cap;
	Database database;
	std::unique_ptr<SQLite::Transaction> upTransaction;
public:
	SqlOpenPoseEvent(const std::string& videoPath, bool writeMode) :
		videoPath(videoPath), writeMode(writeMode)
	{
		sqlPath = std::regex_replace(this->videoPath, std::regex(R"(\.[^.]*$)"), "") + ".sqlite3";
	}
	virtual ~SqlOpenPoseEvent() {};
	int init() override final
	{
		// ����t�@�C���̃I�[�v��
		cap.open(videoPath);
		if (!cap.isOpened())
		{
			std::cout << "can not open \"" << videoPath << "\"" << std::endl;
			return 1;
		}

		// sql�̐���
		try
		{
			database = createDatabase(sqlPath);
			upTransaction = std::make_unique<SQLite::Transaction>(*database);
			if (writeMode)
			{
				database->exec("DROP TABLE IF EXISTS people");
				std::string row_title = "frame INTEGER, people INTEGER";
				for (int i = 0; i < 25; i++)
				{
					row_title += ", joint" + std::to_string(i) + "x REAL";
					row_title += ", joint" + std::to_string(i) + "y REAL";
					row_title += ", joint" + std::to_string(i) + "confidence REAL";
				}
				database->exec("CREATE TABLE people (" + row_title + ")");
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}
	void exit() override final
	{
		cap.release();
		if (upTransaction && writeMode) upTransaction->commit();
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (!cap.isOpened())
		{
			std::cout << "can not open \"" << videoPath << "\"" << std::endl;
			return 1;
		}
		imageInfo.needOpenposeProcess = true;
		imageInfo.frameNumber = (size_t)cap.get(CV_CAP_PROP_POS_FRAMES);
		cap.read(imageInfo.inputImage);
		if (imageInfo.inputImage.empty())
		{
			exit();
			return 0;
		}

		if (!writeMode)
		{
			try
			{
				imageInfo.needOpenposeProcess = false;
				SQLite::Statement query(*database, "SELECT * FROM people WHERE frame=?");
				query.bind(1, (long long)imageInfo.frameNumber);
				while (query.executeStep())
				{
					std::vector<ImageInfo::Node> nodes;
					for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
					{
						nodes.push_back(ImageInfo::Node{
							(float)query.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
							(float)query.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
							(float)query.getColumn(2 + nodeIndex * 3 + 2).getDouble()
							});
					}
					imageInfo.people.emplace_back(std::move(nodes));
				}
			}
			catch (const std::exception& e)
			{
				std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
				return 1;
			}
		}

		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		// �v���r���[�̕\��
		if (!writeMode)
		{
			for (auto person : imageInfo.people)
			{
				for (auto node : person)
				{
					if (node.confidence != 0.0f)
					{
						cv::circle(
							imageInfo.outputImage,
							cv::Point{ (int)node.x, (int)node.y },
							3, cv::Scalar{ 255, 0, 0 }, -1
						);
					}
				}
			}
		}
		cv::imshow("result", imageInfo.outputImage);
		if (cv::waitKey(1) == 0x1b) exit();

		if (writeMode)
		{
			// sql���̐���
			std::string row = "?";
			for (int colIndex = 0; colIndex < 76; colIndex++) row += ", ?";
			row = "INSERT INTO people VALUES (" + row + ")";
			// sql���̒l���m�肷��
			try
			{
				SQLite::Statement query(*database, row);
				for (size_t personIndex = 0; personIndex < imageInfo.people.size(); personIndex++)
				{
					query.reset();
					query.bind(1, (long long)imageInfo.frameNumber);
					query.bind(2, (long long)personIndex);
					for (size_t nodeIndex = 0; nodeIndex < imageInfo.people[personIndex].size(); nodeIndex++)
					{
						query.bind(3 + nodeIndex * 3 + 0, imageInfo.people[personIndex][nodeIndex].x);
						query.bind(3 + nodeIndex * 3 + 1, imageInfo.people[personIndex][nodeIndex].y);
						query.bind(3 + nodeIndex * 3 + 2, imageInfo.people[personIndex][nodeIndex].confidence);
					}
					(void)query.exec();
				}
			}
			catch (const std::exception& e) {
				std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
				return 1;
			}
		}

		return 0;
	}
	void recieveErrors(const std::vector<std::string>& errors) override final
	{
		for (auto error : errors)
			std::cout << error << std::endl;
	}
	std::pair<op::PoseModel, op::Point<int>> selectOpenposeMode() override final
	{
		return std::pair<op::PoseModel, op::Point<int>>(
			op::PoseModel::BODY_25, op::Point<int>(-1, 368)
			);
	}
};
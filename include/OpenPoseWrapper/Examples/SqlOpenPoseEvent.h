#pragma once

#include <OpenPoseWrapper/OpenPoseEvent.h>
#include <Utils/Database.h>

class SqlOpenPoseEvent : public OpenPoseEvent, public Database
{
private:
	std::string sqlPath;
	long long saveFreq = 0;
	size_t saveCountDown = 1;
public:
	SqlOpenPoseEvent(const std::string& sqlPath, long long saveFreq = 0) :
		sqlPath(sqlPath), saveFreq(saveFreq){}
	virtual ~SqlOpenPoseEvent() {};
	int init() override final
	{
		// sql�̐���
		try
		{
			// �t�@�C���̃I�[�v��
			create(
				sqlPath,
				SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
			);

			// people�e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
			std::string row_title = u8"frame INTEGER, people INTEGER";
			for (int i = 0; i < 25; i++)
			{
				row_title += u8", joint" + std::to_string(i) + u8"x REAL";
				row_title += u8", joint" + std::to_string(i) + u8"y REAL";
				row_title += u8", joint" + std::to_string(i) + u8"confidence REAL";
			}
			if (createTableIfNoExist(u8"people", row_title)) return 1;
			if (createIndexIfNoExist(u8"people", u8"frame", false)) return 1;
			if (createIndexIfNoExist(u8"people", u8"people", false)) return 1;
			if (createIndexIfNoExist(u8"people", u8"frame", u8"people", true)) return 1;

			// timestamp�e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
			if (createTableIfNoExist(u8"timestamp", u8"frame INTEGER PRIMARY KEY, timestamp INTEGER")) return 1;
			if (createIndexIfNoExist(u8"timestamp", u8"frame", true)) return 1;
		}
		catch (const std::exception& e)
		{
			std::cout << u8"error : " << __FILE__ << u8" : L" << __LINE__ << u8"\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}
	void exit() override final
	{
		commit();
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		try
		{
			// SQL�Ƀ^�C���X�^���v�����݂����ꍇ�͂��̎����̍��i�f�[�^���g�p����
			if (isDataExist(u8"timestamp", u8"frame", imageInfo.frameNumber))
			{
				imageInfo.needOpenposeProcess = false;
				SQLite::Statement peopleQuery(*database, u8"SELECT * FROM people WHERE frame=?");
				peopleQuery.bind(1, (long long)imageInfo.frameNumber);
				while (peopleQuery.executeStep())
				{
					size_t index = (size_t)peopleQuery.getColumn(1).getInt64();
					std::vector<ImageInfo::Node> nodes;
					for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
					{
						nodes.push_back(ImageInfo::Node{
							(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
							(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
							(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 2).getDouble()
						});
					}
					imageInfo.people[index] = std::move(nodes);
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << u8"error : " << __FILE__ << u8" : L" << __LINE__ << u8"\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		try
		{
			// SQL�Ƀ^�C���X�^���v�����݂��Ȃ������ꍇ��SQL�Ƀf�[�^��ǉ�����
			if (!isDataExist(u8"timestamp", u8"frame", imageInfo.frameNumber))
			{
				// people�e�[�u���̍X�V
				std::string row = u8"?";
				for (int colIndex = 0; colIndex < 76; colIndex++) row += u8", ?";
				row = u8"INSERT INTO people VALUES (" + row + u8")";
				SQLite::Statement peopleQuery(*database, row);
				for (auto person = imageInfo.people.begin(); person != imageInfo.people.end(); person++)
				{
					peopleQuery.reset();
					peopleQuery.bind(1, (long long)imageInfo.frameNumber);
					peopleQuery.bind(2, (long long)person->first);
					for (size_t nodeIndex = 0; nodeIndex < person->second.size(); nodeIndex++)
					{
						peopleQuery.bind(3 + nodeIndex * 3 + 0, (double)person->second[nodeIndex].x);
						peopleQuery.bind(3 + nodeIndex * 3 + 1, (double)person->second[nodeIndex].y);
						peopleQuery.bind(3 + nodeIndex * 3 + 2, (double)person->second[nodeIndex].confidence);
					}
					(void)peopleQuery.exec();
				}

				// timestamp�e�[�u���̍X�V
				row = u8"INSERT INTO timestamp VALUES (?, ?)";
				SQLite::Statement timestampQuery(*database, row);
				timestampQuery.reset();
				timestampQuery.bind(1, (long long)imageInfo.frameNumber);
				timestampQuery.bind(2, (long long)imageInfo.frameTimeStamp);
				(void)timestampQuery.exec();
			}

			// sql�̃Z�[�u
			if ((saveFreq > 0) && (--saveCountDown <= 0))
			{
				saveCountDown = saveFreq;
				commit();
			}
		}
		catch (const std::exception& e) {
			std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}
};
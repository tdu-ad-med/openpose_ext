#pragma once

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/SqlOpenPose.h>
#include <Utils/Database.h>
#include <optional>

class Tracking
{
private:
	using People = MinOpenPose::People;
	using Node = MinOpenPose::Node;

public:
	// (numberFramesToLost - 1)�t���[���O���猻�݂̃t���[���܂ł̊ԂŌ��o���ꂽ�ł��V�����S�Ă̍��i
	People currentPeople;

	// 1�t���[���O��currentPeople�̕���
	People backPeople;

	// currentPeople��������backPeople�ɑ��݂��鍜�i�����߂ĉ�ʂɉf�肱�񂾂Ƃ��̍��i
	People firstPeople;

	// ���݂̃t���[���Ŏ擾�ł����S�Ă̍��i
	People latestPeople;

	// backPeople�ɂ͑��݂��邪currentPeople�ɂ͑��݂��Ȃ��l�S�ẴC���f�b�N�X
	std::vector<size_t> untrackedPeopleIndex;

	/**
	 * OpenPose�ł̓t���[�����Ƃɐl��ID���ϓ����邽�߁A���̃N���X�ł�OpenPose�œ���ꂽ���i�̃g���b�L���O���s��
	 * @param confidenceThreshold �֐߂̐M���l�����̒l�ȉ��ł���ꍇ�́A�֐߂����݂��Ȃ����̂Ƃ��ď�������
	 * @param numberNodesToTrust �M���l��confidenceThreshold���傫���֐߂̐������̒l�����ł���ꍇ�́A���̐l�����Ȃ����̂Ƃ��ď�������
	 * @param numberFramesToLost ��x�g���b�L���O���O�ꂽ�l�����̃t���[�������o�߂��Ă��Ĕ�������Ȃ��ꍇ�́A�����������̂Ƃ��ď�������
	 * @param distanceThreshold �g���b�L���O���̐l��1�t���[���i�񂾂Ƃ��A�ړ����������̒l�����傫���ꍇ�͓���l���̌�₩��O��
	 */
	Tracking(
		float confidenceThreshold = 0.5f,
		uint64_t numberNodesToTrust = 5,
		uint64_t numberFramesToLost = 10,
		float distanceThreshold = 50.0f
	) :
		confidenceThreshold{ confidenceThreshold },
		numberNodesToTrust{ numberNodesToTrust },
		numberFramesToLost{ numberFramesToLost },
		distanceThreshold{ distanceThreshold },
		currentPeople{},
		backPeople{},
		firstPeople{},
		untrackedPeopleIndex{}
	{
	}

	virtual ~Tracking() {};

	/**
	 * �g���b�L���O���s��
	 * @param sql SqlOpenPose�̃C���X�^���X������
	 * @param frameNumber ���ݍĐ����̓���̃t���[���ԍ����w�肷��
	 * @param People�̃C���X�^���X������
	 */
	std::optional<People> tracking(const People& people, SqlOpenPose& sql, const size_t frameNumber)
	{
		// people_with_tracking�e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
		std::string row_title = u8"frame INTEGER, people INTEGER";
		for (int i = 0; i < 25; i++)
		{
			row_title += u8", joint" + std::to_string(i) + u8"x REAL";
			row_title += u8", joint" + std::to_string(i) + u8"y REAL";
			row_title += u8", joint" + std::to_string(i) + u8"confidence REAL";
		}
		if (sql.createTableIfNoExist(u8"people_with_tracking", row_title)) return std::nullopt;

		// SQL�̌��������������邽�߂�Index���쐬
		if (sql.createIndexIfNoExist(u8"people_with_tracking", u8"frame", false)) return std::nullopt;
		if (sql.createIndexIfNoExist(u8"people_with_tracking", u8"people", false)) return std::nullopt;
		if (sql.createIndexIfNoExist(u8"people_with_tracking", u8"frame", u8"people", true)) return std::nullopt;

		try
		{
			// SQL����K�v�ȍ��i�����擾
			if (getPeopleFromSql(sql, frameNumber)) return std::nullopt;

			// ���o���ꂽ���i���Ȃ���ΏI��
			if (people.size() == 0) return people;

			// ���ł�SQL�ɍ��i�f�[�^�����݂���ΏI��
			if (isDataExist(sql, frameNumber)) return currentPeople;

			// ���݂̃t���[���̐l�������[�v
			std::map<size_t, bool> usedIndex;
			for (auto currentPerson = people.begin(); currentPerson != people.end(); currentPerson++)
			{
				// ���i�f�[�^�̐M���x��臒l�����ł���΃X�L�b�v
				auto&& currentNodes = currentPerson->second;
				uint64_t confidenceCount = 0;
				for (auto&& node : currentNodes) { if (node.confidence > confidenceThreshold) confidenceCount++; }
				if (confidenceCount < numberNodesToTrust) continue;

				// �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���擾
				uint64_t nearestPeopleIndex = 0;  // �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���i�[����ꎞ�ϐ�
				float nearestLength = -1.0f;  // �O�t���[���̒��ň�ԋ������߂������������i�[����ꎞ�ϐ�
				bool lostFlag = true;  // �O�t���[���ň�ԋ������߂������l�����o�ł��Ȃ������ꍇ��True�ɂȂ�t���O

				// �O�t���[���̐l�������[�v
				for (auto backPerson = backPeople.begin(); backPerson != backPeople.end(); backPerson++)
				{
					uint64_t index = backPerson->first;  // �C���f�b�N�X�̎擾

					if (usedIndex.count(index)) continue;  // �C���f�b�N�X�����Ɏg�p����Ă����ꍇ�̓X�L�b�v

					auto&& backNodes = backPerson->second;  // ���i���̎擾

					// �O�t���[������̈ړ��������Z�o
					float distance = getDistance(backNodes, currentNodes);

					// �ړ�����������ɎZ�o�ł��Ȃ������A��������distanceThreshold���傫���l�ł������ꍇ�͏��O
					if ((distance < 0.0f) || (distance > distanceThreshold)) continue;

					// �L�^�X�V����
					if (lostFlag || (distance < nearestLength))
					{
						nearestPeopleIndex = index;  // ��ԋ������߂��l�̃C���f�b�N�X���X�V
						nearestLength = distance;  // ���̋������X�V
						lostFlag = false;  // �t���O��܂�
					}
				}

				// �O�t���[���ň�ԋ������߂������l�����o�ł����ꍇ�͂��̐l�̃C���f�b�N�X�����߂�
				uint64_t addIndex = nearestPeopleIndex;

				// �O�t���[���ň�ԋ������߂������l�����o�ł��Ȃ������ꍇ�͐V�����C���f�b�N�X�����߂�
				if (lostFlag)
				{
					// ����SQL�ɓo�^���ꂽ�l�̑������擾
					SQLite::Statement peopleCountQuery(*(sql.database), "SELECT COUNT(DISTINCT people) from people_with_tracking");
					(void)peopleCountQuery.executeStep();
					addIndex = peopleCountQuery.getColumn(0).getInt();
				}

				// �g�p�ς݃C���f�b�N�X�֒ǉ�
				usedIndex[addIndex] = true;

				// SQL���̐���
				std::string row = u8"?";
				for (int colIndex = 0; colIndex < 76; colIndex++) row += u8", ?";
				row = u8"INSERT INTO people_with_tracking VALUES (" + row + u8")";
				SQLite::Statement insertQuery(*(sql.database), row);

				// ���݂̃t���[���Ō��o���ꂽ�S�Ă̍��i�f�[�^��SQL�ɒǋL
				insertQuery.reset();
				insertQuery.bind(1, (long long)frameNumber);
				insertQuery.bind(2, (long long)addIndex);
				for (size_t nodeIndex = 0; nodeIndex < currentNodes.size(); nodeIndex++)
				{
					insertQuery.bind(3 + nodeIndex * 3 + 0, (double)currentNodes[nodeIndex].x);
					insertQuery.bind(3 + nodeIndex * 3 + 1, (double)currentNodes[nodeIndex].y);
					insertQuery.bind(3 + nodeIndex * 3 + 2, (double)currentNodes[nodeIndex].confidence);
				}
				(void)insertQuery.exec();
			}

			// �ēxSQL����K�v�ȍ��i�����擾
			if (getPeopleFromSql(sql, frameNumber)) return std::nullopt;
		}
		catch (const std::exception& e)
		{
			std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
			return std::nullopt;
		}

		return currentPeople;
	}

	// �w�肳�ꂽ�t���[���ԍ��̃f�[�^��SQL�ɑ��݂��邩�ǂ���
	bool isDataExist(const SqlOpenPose& sql, size_t frame) const {
		return sql.isDataExist(u8"people_with_tracking", u8"frame", frame);
	}

	// ���i�̏d�S���擾����
	static Node getJointAverage(const std::vector<Node>& person)
	{
		Node result{ 0.0f, 0.0f, 1.0f };
		float confidenceSum = 0.0f;
		for (auto node : person)
		{
			// �M���l��0�̍��i�͍��W��(0, 0)�ɂȂ��Ă��邽�ߏ��O����
			if (node.confidence == 0.0f) continue;
			result.x += node.x * node.confidence;
			result.y += node.y * node.confidence;
			confidenceSum += node.confidence;
		}
		// 0����������
		if (confidenceSum == 0.0f) return Node{ 0.0f, 0.0f, 0.0f };
		result.x /= confidenceSum;
		result.y /= confidenceSum;
		return result;
	}

private:
	// �֐߂̐M���l�����̒l�ȉ��ł���ꍇ�́A�֐߂����݂��Ȃ����̂Ƃ��ď�������
	float confidenceThreshold;

	// �M���l��confidenceThreshold���傫���֐߂̐������̒l�����ł���ꍇ�́A���̐l�����Ȃ����̂Ƃ��ď�������
	uint64_t numberNodesToTrust;

	// ��x�g���b�L���O���O�ꂽ�l�����̃t���[�������o�߂��Ă��Ĕ�������Ȃ��ꍇ�́A�����������̂Ƃ��ď�������
	uint64_t numberFramesToLost;

	// �g���b�L���O���̐l��1�t���[���i�񂾂Ƃ��A�ړ����������̒l�����傫���ꍇ�͓���l���̌�₩��O��
	float distanceThreshold;

	// SQL�ɕۑ�����Ă���firstFrameNumber�t���[������endFrameNumber�t���[���܂ł̊ԂɌ��o���ꂽ�ł��V�����S�Ă̍��i��people�ɑ��
	int getLatestPeopleFromSql(const SqlOpenPose& sql, std::map<size_t, std::vector<Node>>& people, int64_t firstFrameNumber, int64_t endFrameNumber)
	{
		try
		{
			firstFrameNumber = (firstFrameNumber < 0) ? 0 : firstFrameNumber;
			endFrameNumber = (endFrameNumber < 0) ? 0 : endFrameNumber;
			SQLite::Statement peopleQuery(*(sql.database), u8"SELECT * FROM people_with_tracking WHERE ? <= frame AND frame <= ? GROUP BY people HAVING frame = MAX(frame)");
			if (sql.bindAll(peopleQuery, firstFrameNumber, endFrameNumber)) return 1;
			while (peopleQuery.executeStep())
			{
				size_t index = (size_t)peopleQuery.getColumn(1).getInt64();
				for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
				{
					people[index].push_back(Node{
						(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
						(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
						(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 2).getDouble()
					});
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}

	// SQL�ɕۑ�����Ă���firstFrameNumber�t���[������endFrameNumber�t���[���܂ł̊ԂɌ��o���ꂽ�S�Ă̍��i�́A�ŏ��ɉ�ʂɉf�������̍��i��people�ɑ��
	int getOldestPeopleFromSql(const SqlOpenPose& sql, std::map<size_t, std::vector<Node>>& people, int64_t firstFrameNumber, int64_t endFrameNumber)
	{
		try
		{
			firstFrameNumber = (firstFrameNumber < 0) ? 0 : firstFrameNumber;
			endFrameNumber = (endFrameNumber < 0) ? 0 : endFrameNumber;
			SQLite::Statement peopleQuery(*(sql.database), u8"SELECT * FROM people_with_tracking WHERE people IN (SELECT people FROM people_with_tracking WHERE ? <= frame AND frame <= ?) GROUP BY people HAVING frame=MIN(frame)");
			if (sql.bindAll(peopleQuery, firstFrameNumber, endFrameNumber)) return 1;
			while (peopleQuery.executeStep())
			{
				size_t index = (size_t)peopleQuery.getColumn(1).getInt64();
				for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
				{
					people[index].push_back(Node{
						(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
						(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
						(float)peopleQuery.getColumn(2 + nodeIndex * 3 + 2).getDouble()
					});
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
			return 1;
		}

		return 0;
	}

	// SQL����currentPeople, backPeople, currentPeopleFirst, untrackedPeopleId���擾
	int getPeopleFromSql(const SqlOpenPose& sql, size_t frame)
	{
		// ������
		backPeople.clear();
		currentPeople.clear();
		firstPeople.clear();
		latestPeople.clear();
		untrackedPeopleIndex.clear();

		// numberFramesToLost�t���[���O����1�t���[���O�܂ł̊ԂŌ��o���ꂽ�ł��V�������i��S�Ď擾
		if (getLatestPeopleFromSql(
			sql, backPeople,
			(int64_t)frame - (int64_t)numberFramesToLost,
			(int64_t)frame - 1
		)) return 1;

		// (numberFramesToLost - 1)�t���[���O���猻�݂̃t���[���܂ł̊ԂŌ��o���ꂽ�ł��V�������i��S�Ď擾
		if (getLatestPeopleFromSql(
			sql, currentPeople,
			(int64_t)frame - ((int64_t)numberFramesToLost - 1),
			(int64_t)frame
		)) return 1;

		// numberFramesToLost�t���[���O���猻�݂̃t���[���܂ł̊ԂŌ��o���ꂽ�ł��Â����i��S�Ď擾
		if (getOldestPeopleFromSql(
			sql, firstPeople,
			(int64_t)frame - (int64_t)numberFramesToLost,
			(int64_t)frame
		)) return 1;

		// ���݂̃t���[���Ŏ擾�ł����l�S�ẴC���f�b�N�X���擾
		if (getLatestPeopleFromSql(sql, latestPeople, (int64_t)frame, (int64_t)frame)) return 1;

		// backPeople�ɂ͑��݂��邪currentPeople�ɂ͑��݂��Ȃ��l�S�ẴC���f�b�N�X���擾
		for (auto backPerson = backPeople.begin(); backPerson != backPeople.end(); backPerson++)
		{
			if (currentPeople.count(backPerson->first) == 0) untrackedPeopleIndex.push_back(backPerson->first);
		}

		return 0;
	}

	// 2�̍��i�̊e�֐߂̋������̕��ς��擾(�M���x��confidenceThreshold�ȉ��̊֐߂͌v�Z���珜�O�����)
	// ���������0.0f�ȏ�̒l���Ԃ����
	// �S�Ă̊֐߂̐M���x��confidenceThreshold�ȉ��������ꍇ��-1.0f���Ԃ����
	float getDistance(const std::vector<Node>& nodes1, const std::vector<Node>& nodes2)
	{
		uint64_t samples = 0;  // �L���Ȋ֐߂̃T���v����
		float distance = 0.0f;  // �L���ȑS�֐߂̈ړ��ʂ̕���
		for (uint64_t index = 0; index < nodes1.size(); index++)
		{
			// 臒l�ȉ��̊֐߂͖���
			if ((nodes1[index].confidence <= confidenceThreshold) || (nodes2[index].confidence <= confidenceThreshold)) continue;
			float x = nodes1[index].x - nodes2[index].x;
			float y = nodes1[index].y - nodes2[index].y;
			distance += std::sqrtf((x * x) + (y * y));
			samples++;
		}
		return (samples == 0) ? (-1.0f) : (distance / (float)samples);
	}
};
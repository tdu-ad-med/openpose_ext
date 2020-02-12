#pragma once

#include <OpenPoseWrapper/OpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <Utils/Database.h>
#include <Utils/Vector.h>

class PeopleCounterOpenPoseEvent : public OpenPoseEvent
{
public:
	std::shared_ptr<TrackingOpenPoseEvent> tracker;
	std::shared_ptr<SqlOpenPoseEvent> sql;

	PeopleCounterOpenPoseEvent(
		const std::shared_ptr<TrackingOpenPoseEvent>& tracker,
		float lineStartX, float lineStartY, float lineEndX, float lineEndY, float lineWeigth,
		bool drawInfomation
	) :
		tracker{ tracker }, sql{ tracker->sql }, drawInfomation{ drawInfomation }
	{
		float lineVecX = lineEndX - lineStartX;
		float lineVecY = lineEndY - lineStartY;
		float lineVecLength = std::sqrt(lineVecX * lineVecX + lineVecY * lineVecY);
		float lineNormalX = -lineVecY / lineVecLength;
		float lineNormalY = lineVecX / lineVecLength;
		float moveX = lineNormalX * lineWeigth * 0.5f;
		float moveY = lineNormalY * lineWeigth * 0.5f;
		lines.clear();
		lines.push_back(Line{ lineStartX - moveX, lineStartY - moveY, lineEndX - moveX, lineEndY - moveY });
		lines.push_back(Line{ lineStartX + moveX, lineStartY + moveY, lineEndX + moveX, lineEndY + moveY });
	}
	virtual ~PeopleCounterOpenPoseEvent() {};

	int init() override final
	{
		if (checkError()) return 1;

		// people_count�e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
		std::string row_title = u8"frame INTEGER PRIMARY KEY, static_up INTEGER, static_down INTEGER, dynamic_up INTEGER, dynamic_down INTEGER";
		if (sql->createTableIfNoExist(u8"people_count", row_title)) return 1;
		if (sql->createIndexIfNoExist(u8"people_count", u8"frame", true)) return 1;

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (checkError()) return 1;
		return 0;
	}
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (checkError()) return 1;

		try
		{
			// SQL����f�[�^���擾
			auto&& sqlData = getCountFromSql(imageInfo.frameNumber);

			// SQL�Ƀf�[�^�����݂��Ȃ������ꍇ�͐V���ɃJ�E���g���擾����
			if (std::get<0>(sqlData) == -1)
			{
				// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^�����Z�b�g
				dynamicUpCount = 0;
				dynamicDownCount = 0;

				// �g���b�L���O���O��Ă��Ȃ��l�̈ړ������J�E���g
				for (auto currentPerson = tracker->currentPeople.begin(); currentPerson != tracker->currentPeople.end(); currentPerson++)
				{
					auto e = judgeUpOrDown(tracker->firstPeople[currentPerson->first], currentPerson->second, lines);
					if (e == Event::UP) dynamicUpCount++;
					if (e == Event::DOWN) dynamicDownCount++;
				}

				// �g���b�L���O���O�ꂽ�l�̈ړ������J�E���g
				for (auto&& index : tracker->untrackedPeopleIndex)
				{
					auto e = judgeUpOrDown(tracker->firstPeople[index], tracker->backPeople[index], lines);
					if (e == Event::UP) staticUpCount++;
					if (e == Event::DOWN) staticDownCount++;
				}

				// SQL�Ƀf�[�^��ǉ�
				SQLite::Statement insertQuery(*(sql->database), u8"INSERT INTO people_count VALUES(?, ?, ?, ?, ?)");
				sql->bindAllAndExec(
					insertQuery,
					(int64_t)imageInfo.frameNumber,
					(int64_t)staticUpCount,
					(int64_t)staticDownCount,
					(int64_t)dynamicUpCount,
					(int64_t)dynamicDownCount
				);
			}
			else
			{
				staticUpCount = std::get<0>(sqlData);
				staticDownCount = std::get<1>(sqlData);
				dynamicUpCount = std::get<2>(sqlData);
				dynamicDownCount = std::get<3>(sqlData);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
			return 1;
		}
		

		// �g���b�L���O�̕`����s��Ȃ��ꍇ�͂����ŏI��
		if (!drawInfomation) return 0;

		// ����̕`��
		for (auto line : lines)
		{
			cv::line(
				imageInfo.outputImage,
				{ (int)line.lineStartX, (int)line.lineStartY },
				{ (int)line.lineEndX, (int)line.lineEndY },
				cv::Scalar{ 255.0, 255.0, 255.0 }, 2
			);
		}

		// �g���b�L���O�̎n�_�ƏI�_�����Ԓ�����`��
		for (auto currentPerson = tracker->currentPeople.begin(); currentPerson != tracker->currentPeople.end(); currentPerson++)
		{
			size_t index = currentPerson->first;
			auto&& firstPosition = TrackingOpenPoseEvent::getJointAverage(tracker->firstPeople[index]);
			auto&& currentPosition = TrackingOpenPoseEvent::getJointAverage(currentPerson->second);

			// �����̕`��
			cv::line(
				imageInfo.outputImage,
				{ (int)firstPosition.x, (int)firstPosition.y },
				{ (int)currentPosition.x, (int)currentPosition.y },
				cv::Scalar{
					(double)((int)((std::sin(((double)index) * 463763.0) + 1.0) * 100000.0) % 120 + 80),
					(double)((int)((std::sin(((double)index) * 1279.0) + 1.0) * 100000.0) % 120 + 80),
					(double)((int)((std::sin(((double)index) * 92763.0) + 1.0) * 100000.0) % 120 + 80)
				}, 2.0
			);

			// id�̕`��
			gui::text(
				imageInfo.outputImage, std::to_string(index),
				{ (int)currentPosition.x, (int)currentPosition.y },
				gui::CENTER_CENTER, 0.5
			);
		}

		// �J�E���g��\��
		gui::text(imageInfo.outputImage, std::string("up : ") + std::to_string(getUpCount()), { 20, 200 });
		gui::text(imageInfo.outputImage, std::string("down : ") + std::to_string(getDownCount()), { 20, 230 });

		return 0;
	}

	// �����������Ɉړ������l�̃J�E���g���擾
	inline uint64_t getUpCount() { return staticUpCount + dynamicUpCount; }

	// ������������Ɉړ������l�̃J�E���g���擾
	inline uint64_t getDownCount() { return staticDownCount + dynamicDownCount; }

private:
	// �g���b�L���O���O�ꂽ�l�̃J�E���^(�ŏI�I�Ȉړ��������m�肵�Ă���)
	uint64_t staticUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
	uint64_t staticDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g

	// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^
	uint64_t dynamicUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
	uint64_t dynamicDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g

	// �J�E���g������ʂɕ`�悷�邩�ǂ���
	bool drawInfomation;

	int checkError()
	{
		if (!tracker)
		{
			std::cout
				<< "TrackingOpenPoseEvent�����w��ł��B\n"
				<< "�R���X�g���N�^�Ő������l���w�肵�Ă��������B"
				<< std::endl;
			return 1;
		}
		return 0;
	}

	// �����̎n�_�ƏI�_
	struct Line {
		float lineStartX, lineStartY, lineEndX, lineEndY;
	};

	// �l���J�E���g���s�����
	std::vector<Line> lines;

	enum Event { UP, DOWN, NOTHING };

	// p1Start����p1End�܂ł����Ԓ�����p2Start����p2End�܂ł����Ԓ������������Ă��邩�ǂ������擾
	bool isCross(vt::Vector4& p1Start, vt::Vector4& p1End, vt::Vector4& p2Start, vt::Vector4& p2End)
	{
		// p1Start����p1End�ւ̒�����p2Start����p2End�ւ̒������������Ă��邩�ǂ��������߂�
		// �Q�l : https://imagingsolution.blog.fc2.com/blog-entry-137.html
		double s1 = ((p2End.x - p2Start.x) * (p1Start.y - p2Start.y) - (p2End.y - p2Start.y) * (p1Start.x - p2Start.x)) / 2.0;
		double s2 = ((p2End.x - p2Start.x) * (p2Start.y - p1End.y) - (p2End.y - p2Start.y) * (p2Start.x - p1End.x)) / 2.0;
		if (s1 + s2 == 0.0) return false;
		double p = s1 / (s1 + s2);
		return (0.0 <= p && p <= 1.0);
	}

	// peopleStart����peopleEnd�܂ł����Ԓ�����line�ƌ������Ă��邩�ǂ����擾
	// ����ɁA�������Ă���ꍇ�͂ǂ���̕����Ɍ������Ă���̂����擾
	Event judgeUpOrDown(std::vector<ImageInfo::Node>& peopleStart, std::vector<ImageInfo::Node>& peopleEnd, Line& line)
	{
		auto startPos = TrackingOpenPoseEvent::getJointAverage(peopleStart);  // ���s�҂̃g���b�L���O���J�n�����_
		auto endPos = TrackingOpenPoseEvent::getJointAverage(peopleEnd);  // ���s�҂̃g���b�L���O���I�������_
		auto vecLine = vt::Vector4((double)line.lineEndX - (double)line.lineStartX, (double)line.lineEndY - (double)line.lineStartY);  // �l���J�E���g���s������̃x�N�g��
		auto vecStart = vt::Vector4((double)startPos.x - (double)line.lineStartX, (double)startPos.y - (double)line.lineStartY);  // �l���J�E���g���s������̎n�_����startPos�ւ̃x�N�g��
		auto vecEnd = vt::Vector4((double)endPos.x - (double)line.lineStartX, (double)endPos.y - (double)line.lineStartY);  // �l���J�E���g���s������̎n�_����vecEnd�ւ̃x�N�g��
		if (!isCross(vt::Vector4(0.0, 0.0), vecLine, vecStart, vecEnd)) return Event::NOTHING;  // start����end�����Ԓ�����vecLine�̏��ʉ߂��Ă��Ȃ��ꍇ
		// �uvecLine��90�x��]���������v�ƁuvecStart�v�Ƃ̓��ς����ƂɁAstartPos��vecLine�̏㑤�ɂ��邩�ǂ����𔻒�
		bool startIsUp = vecStart.x * vecLine.y > vecStart.y * vecLine.x;
		// �uvecLine��90�x��]���������v�ƁuvecEnd�v�Ƃ̓��ς����ƂɁAendPos��vecLine�̏㑤�ɂ��邩�ǂ����𔻒�
		bool endIsUp = vecEnd.x * vecLine.y > vecEnd.y * vecLine.x;
		if ((!startIsUp) && endIsUp) return Event::UP;  // ���s�҂̃g���b�L���O���l���J�E���g���s������𒴂��ď�Ɉړ����Ă����ꍇ
		if (startIsUp && (!endIsUp)) return Event::DOWN;  // ���s�҂̃g���b�L���O���l���J�E���g���s������𒴂��ĉ��Ɉړ����Ă����ꍇ
		return Event::NOTHING;  // ����ȊO
	}

	// peopleStart����peopleEnd�܂ł����Ԓ������S�Ă�lines�Ɠ�������Ɍ������Ă��邩�ǂ����擾
	// ����ɁA�������Ă���ꍇ�͂ǂ���̕����Ɍ������Ă���̂����擾
	Event judgeUpOrDown(std::vector<ImageInfo::Node>& peopleStart, std::vector<ImageInfo::Node>& peopleEnd, std::vector<Line>& lines)
	{
		Event e = Event::NOTHING;
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (i == 0) e = judgeUpOrDown(peopleStart, peopleEnd, lines[i]);
			else if (e != judgeUpOrDown(peopleStart, peopleEnd, lines[i])) return Event::NOTHING;
		}
		return e;
	}

	// �w�肳�ꂽ�t���[���ԍ��̃f�[�^��SQL����擾����
	// ���݂��Ȃ��ꍇ��-1�Ŗ������ꂽtuple��Ԃ�
	std::tuple<int, int, int, int> getCountFromSql(size_t frame)
	{
		if (!sql->isDataExist(u8"people_count", u8"frame", frame)) return std::make_tuple(-1, -1, -1, -1);
		SQLite::Statement countQuery(*(sql->database), u8"SELECT * FROM people_count WHERE frame=?");
		countQuery.bind(1, (long long)frame);
		(void)countQuery.executeStep();

		return std::make_tuple(
			countQuery.getColumn(1).getInt(),
			countQuery.getColumn(2).getInt(),
			countQuery.getColumn(3).getInt(),
			countQuery.getColumn(4).getInt()
		);
	}
};
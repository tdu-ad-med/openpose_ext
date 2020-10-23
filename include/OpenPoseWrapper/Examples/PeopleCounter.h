#pragma once

#include <OpenPoseWrapper/Examples/Tracking.h>
#include <Utils/Database.h>
#include <Utils/Vector.h>

class PeopleCounter
{
public:
	/**
	* ��ʏ�ɒ����������A���̏���ǂ���̕����ɉ��l���ʉ߂��������J�E���g����N���X
	* @param lineStartX �n�_��X���W
	* @param lineStartY �n�_��Y���W 
	* @param lineEndX �I�_��X���W
	* @param lineEndY �I�_��Y���W
	* @param lineWeigth �����̑���
	*/
	PeopleCounter(
		float lineStartX, float lineStartY, float lineEndX, float lineEndY, float lineWeigth
	)
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

	virtual ~PeopleCounter() {};

	void update(const Tracking& tracker, const size_t frameNumber)
	{
		// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^�����Z�b�g
		dynamicUpCount = 0;
		dynamicDownCount = 0;

		// �g���b�L���O���O��Ă��Ȃ��l�̈ړ������J�E���g
		for (auto currentPerson = tracker.currentPeople.begin(); currentPerson != tracker.currentPeople.end(); currentPerson++)
		{
			auto e = judgeUpOrDown(tracker.firstPeople.at(currentPerson->first), currentPerson->second, lines);
			if (e == Event::UP) dynamicUpCount++;
			if (e == Event::DOWN) dynamicDownCount++;
		}

		// �g���b�L���O���O�ꂽ�l�̈ړ������J�E���g
		for (auto&& index : tracker.untrackedPeopleIndex)
		{
			auto e = judgeUpOrDown(tracker.firstPeople.at(index), tracker.backPeople.at(index), lines);
			if (e == Event::UP) staticUpCount++;
			if (e == Event::DOWN) staticDownCount++;
		}
	}

	void drawInfo(cv::Mat& frame, const Tracking& tracker)
	{
		// ����̕`��
		for (auto line : lines)
		{
			cv::line(
				frame,
				{ (int)line.lineStartX, (int)line.lineStartY },
				{ (int)line.lineEndX, (int)line.lineEndY },
				cv::Scalar{ 255.0, 255.0, 255.0 }, 2
			);
		}

		// �g���b�L���O�̎n�_�ƏI�_�����Ԓ�����`��
		for (auto currentPerson = tracker.currentPeople.begin(); currentPerson != tracker.currentPeople.end(); currentPerson++)
		{
			size_t index = currentPerson->first;
			auto&& firstPosition = Tracking::getJointAverage(tracker.firstPeople.at(index));
			auto&& currentPosition = Tracking::getJointAverage(currentPerson->second);

			// �����̕`��
			cv::line(
				frame,
				{ (int)firstPosition.x, (int)firstPosition.y },
				{ (int)currentPosition.x, (int)currentPosition.y },
				cv::Scalar{
					(double)((int)((std::sin(((double)index) * 463763.0) + 1.0) * 100000.0) % 120 + 80),
					(double)((int)((std::sin(((double)index) * 1279.0) + 1.0) * 100000.0) % 120 + 80),
					(double)((int)((std::sin(((double)index) * 92763.0) + 1.0) * 100000.0) % 120 + 80)
				}, 2.0
			);
		}

		// �J�E���g��\��
		gui::text(frame, std::string("up : ") + std::to_string(getUpCount()), { 20, 200 });
		gui::text(frame, std::string("down : ") + std::to_string(getDownCount()), { 20, 230 });
	}

	// �����������Ɉړ������l�̃J�E���g���擾
	inline uint64_t getUpCount() const { return staticUpCount + dynamicUpCount; }

	// ������������Ɉړ������l�̃J�E���g���擾
	inline uint64_t getDownCount() const { return staticDownCount + dynamicDownCount; }

private:
	using People = MinOpenPose::People;
	using Person = MinOpenPose::Person;
	using Node = MinOpenPose::Node;

	// �g���b�L���O���O�ꂽ�l�̃J�E���^
	uint64_t staticUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
	uint64_t staticDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g

	// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^
	uint64_t dynamicUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
	uint64_t dynamicDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g

	// �����̎n�_�ƏI�_
	struct Line {
		float lineStartX, lineStartY, lineEndX, lineEndY;
	};

	// �l���J�E���g���s�����
	std::vector<Line> lines;

	enum class Event { UP, DOWN, NOTHING };

	// p1Start����p1End�܂ł����Ԓ�����p2Start����p2End�܂ł����Ԓ������������Ă��邩�ǂ������擾
	bool isCross(vt::Vector4& p1Start, vt::Vector4& p1End, vt::Vector4& p2Start, vt::Vector4& p2End) const
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
	Event judgeUpOrDown(const Person& peopleStart, const Person& peopleEnd, Line& line) const
	{
		auto startPos = Tracking::getJointAverage(peopleStart);  // ���s�҂̃g���b�L���O���J�n�����_
		auto endPos = Tracking::getJointAverage(peopleEnd);  // ���s�҂̃g���b�L���O���I�������_
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
	Event judgeUpOrDown(const Person& peopleStart, const Person& peopleEnd, std::vector<Line>& lines) const
	{
		Event e = Event::NOTHING;
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (i == 0) e = judgeUpOrDown(peopleStart, peopleEnd, lines[i]);
			else if (e != judgeUpOrDown(peopleStart, peopleEnd, lines[i])) return Event::NOTHING;
		}
		return e;
	}
};
#pragma once

#include <openpose/core/common.hpp>

#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <openpose/utilities/openCv.hpp>
#include <Utils/Vector.h>
#include <OpenPoseWrapper/OpenPoseEvent.h>

namespace op
{
	struct Node
	{
	public:
		float x;
		float y;
		float confidence;
		float confidenceThreshold;

		Node();

		Node(
			float x,
			float y,
			float confidence,
			float confidenceThreshold
		);

		Node(const Node& node);

		Node& operator = (const Node& node)
		{
			x = node.x;
			y = node.y;
			confidence = node.confidence;
			confidenceThreshold = node.confidenceThreshold;
			return *this;
		}

		bool isTrusted();

		float distanceFrom(const Node& node);
	};

	struct Tree
	{
	public:
		std::vector<Node> nodes;
		uint64_t frameNumber;
		uint64_t numberNodesToTrust;
		float confidenceThreshold;

		Tree();

		Tree(
			std::vector<Node> nodes,
			uint64_t frameNumber,
			uint64_t numberNodesToTrust,
			float confidenceThreshold,
			bool valid
		);

		Tree(const Tree& tree);

		Tree& operator = (const Tree& tree)
		{
			nodes = tree.nodes;
			frameNumber = tree.frameNumber;
			numberNodesToTrust = tree.numberNodesToTrust;
			confidenceThreshold = tree.confidenceThreshold;
			valid = tree.valid;
			return *this;
		}

		bool isTrusted();

		float distanceFrom(std::vector<Node>& nodes);

		Node average();

		bool isValid();

	private:
		bool valid;
	};

	struct PeopleList
	{
	public:
		uint64_t numberNodesToTrust;
		float confidenceThreshold;
		uint64_t numberFramesToLost;
		float distanceThreshold;

		PeopleList();

		PeopleList(
			uint64_t numberNodesToTrust,
			float confidenceThreshold,
			uint64_t numberFramesToLost,
			float distanceThreshold
		);

		// ���i�f�[�^���X�V����
		void addFrame(ImageInfo& imageInfo);

		// ���݂̃t���[���ɉf���Ă��邷�ׂĂ̐l��ID��z��Ŏ擾����
		std::vector<uint64_t> getCurrentIndices();

		// 1�t���[���O�ɉf���Ă��邷�ׂĂ̐l��ID��z��Ŏ擾����
		std::vector<uint64_t> getBackIndices();

		// 1�t���[���O�ɂ͉f���Ă������A���݂̃t���[���Ńg���b�L���O���O�ꂽ�l��ID��z��Ŏ擾����
		std::vector<uint64_t> getLostIndices();

		// �w�肳�ꂽID�̐l�̌��݂̃t���[���̍��i���擾
		// �������A����ID��getCurrentIndices()�ŕԂ����z��ɑ��݂��Ȃ��ꍇ�͖����ȍ��i�f�[�^���Ԃ����
		Tree getCurrentTree(uint64_t index);

		// �w�肳�ꂽID�̐l��1�t���[���O�̍��i���擾
		// �������A����ID��getBackIndices()�ŕԂ����z��ɑ��݂��Ȃ��ꍇ�͖����ȍ��i�f�[�^���Ԃ����
		Tree getBackTree(uint64_t index);

		// �w�肳�ꂽID�̐l�����߂ĉf�肱�񂾂Ƃ��̍��i���擾
		// �������A����ID��getCurrentIndices()�ŕԂ����z���getBackIndices()�ŕԂ����z��̂ǂ���ɂ����݂��Ȃ��ꍇ�͖����ȍ��i�f�[�^���Ԃ����
		Tree getFirstTree(uint64_t index);

	private:
		std::map<uint64_t, Tree> currentTrees;
		std::map<uint64_t, Tree> backTrees;
		std::map<uint64_t, Tree> firstTrees;
		uint64_t sumOfPeople = 0;
	};

	// �����̏�����l�̐l���A�ǂ���̕����Ɉړ����������J�E���g����N���X
	class PeopleLineCounter
	{
	private:
		// �g���b�L���O���O�ꂽ�l�̃J�E���^(�ŏI�I�Ȉړ��������m�肵�Ă���)
		uint64_t staticUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
		uint64_t staticDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g
		// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^
		uint64_t dynamicUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
		uint64_t dynamicDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g
		// �l���J�E���g���s������̎n�_�ƏI�_
		struct Line {
			float lineStartX, lineStartY, lineEndX, lineEndY;
		};
		std::vector<Line> lines;

		enum Event { UP, DOWN, NOTHING };

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

		Event judgeUpOrDown(op::Tree& peopleStart, op::Tree& peopleEnd, Line& line)
		{
			auto startPos = peopleStart.average();  // ���s�҂̃g���b�L���O���J�n�����_
			auto endPos = peopleEnd.average();  // ���s�҂̃g���b�L���O���I�������_
			auto vecLine = vt::Vector4((double)line.lineEndX - (double)line.lineStartX, (double)line.lineEndY - (double)line.lineStartY);  // �l���J�E���g���s������̃x�N�g��
			auto vecStart = vt::Vector4((double)startPos.x - (double)line.lineStartX, (double)startPos.y - (double)line.lineStartY);  // �l���J�E���g���s������̎n�_����startPos�ւ̃x�N�g��
			auto vecEnd = vt::Vector4((double)endPos.x - (double)line.lineStartX, (double)endPos.y - (double)line.lineStartY);  // �l���J�E���g���s������̎n�_����vecEnd�ւ̃x�N�g��
			if (!isCross(vt::Vector4(0.0, 0.0), vecLine, vecStart, vecEnd)) return Event::NOTHING;  // start����end�����Ԓ�����vecLine�̏��ʉ߂��Ă��Ȃ��ꍇ
			// �uvecLine��90�x��]���������v�ƁuvecStart�v�Ƃ̓��ς����ƂɁAstartPos��vecLine�̏㑤�ɂ��邩�ǂ����𔻒�
			bool startIsUp = vecStart.x * vecLine.y > vecStart.y* vecLine.x;
			// �uvecLine��90�x��]���������v�ƁuvecEnd�v�Ƃ̓��ς����ƂɁAendPos��vecLine�̏㑤�ɂ��邩�ǂ����𔻒�
			bool endIsUp = vecEnd.x * vecLine.y > vecEnd.y* vecLine.x;
			if ((!startIsUp) && endIsUp) return Event::UP;  // ���s�҂̃g���b�L���O���l���J�E���g���s������𒴂��ď�Ɉړ����Ă����ꍇ
			if (startIsUp && (!endIsUp)) return Event::DOWN;  // ���s�҂̃g���b�L���O���l���J�E���g���s������𒴂��ĉ��Ɉړ����Ă����ꍇ
			return Event::NOTHING;  // ����ȊO
		}

		Event judgeUpOrDown(op::Tree& peopleStart, op::Tree& peopleEnd)
		{
			Event e = Event::NOTHING;
			for (size_t i = 0; i < lines.size(); i++)
			{
				if (i == 0) e = judgeUpOrDown(peopleStart, peopleEnd, lines[i]);
				else if (e != judgeUpOrDown(peopleStart, peopleEnd, lines[i])) return Event::NOTHING;
			}
			return e;
		}

	public:
		PeopleLineCounter(float startX = 0.0f, float startY = 0.0f, float endX = 1.0f, float endY = 1.0f)
		{
			setLine(startX, startY, endX, endY);
		}

		// ���i���X�V����
		void update(PeopleList& peopleList)
		{
			// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^�����Z�b�g
			dynamicUpCount = 0;
			dynamicDownCount = 0;
			// �g���b�L���O���O��Ă��Ȃ��l�̈ړ������J�E���g
			const std::vector<uint64_t> currentIndex = peopleList.getCurrentIndices();
			for (size_t index : currentIndex)
			{
				auto e = judgeUpOrDown(peopleList.getFirstTree(index), peopleList.getCurrentTree(index));
				if (e == Event::UP) dynamicUpCount++;
				if (e == Event::DOWN) dynamicDownCount++;
			}
			// �g���b�L���O���O�ꂽ�l�̈ړ������J�E���g
			const std::vector<uint64_t> lostIndex = peopleList.getLostIndices();
			for (size_t index : lostIndex)
			{
				auto e = judgeUpOrDown(peopleList.getFirstTree(index), peopleList.getBackTree(index));
				if (e == Event::UP) staticUpCount++;
				if (e == Event::DOWN) staticDownCount++;
			}
		}

		// �J�E���g�̃��Z�b�g
		inline void resetCount()
		{
			staticUpCount = 0;
			staticDownCount = 0;
			dynamicUpCount = 0;
			dynamicDownCount = 0;
		}

		// �J�E���g�̊���̈ʒu�ݒ�(�n�_X, �n�_Y, �I�_X, �I�_Y)
		inline void setLine(float lineStartX, float lineStartY, float lineEndX, float lineEndY)
		{
			this->lines.clear();
			this->lines.push_back(Line{ lineStartX, lineStartY, lineEndX, lineEndY });
		}

		// �J�E���g�̊���̈ʒu�ݒ�(�n�_X, �n�_Y, �I�_X, �I�_Y, ���̑���)
		inline void setLine(float lineStartX, float lineStartY, float lineEndX, float lineEndY, float lineWeigth)
		{

			float lineVecX = lineEndX - lineStartX;
			float lineVecY = lineEndY - lineStartY;
			float lineVecLength = std::sqrt(lineVecX * lineVecX + lineVecY * lineVecY);
			float lineNormalX = -lineVecY / lineVecLength;
			float lineNormalY = lineVecX / lineVecLength;
			float moveX = lineNormalX * lineWeigth * 0.5f;
			float moveY = lineNormalY * lineWeigth * 0.5f;
			this->lines.clear();
			this->lines.push_back(Line{ lineStartX - moveX, lineStartY - moveY, lineEndX - moveX, lineEndY - moveY });
			this->lines.push_back(Line{ lineStartX + moveX, lineStartY + moveY, lineEndX + moveX, lineEndY + moveY });
		}

		// �����������Ɉړ������l�̃J�E���g���擾
		inline uint64_t getUpCount() { return staticUpCount + dynamicUpCount; }
		// �����������Ɉړ������l�̃J�E���g���擾
		inline uint64_t getDownCount() { return staticDownCount + dynamicDownCount; }

		// ����̕`��
		void drawLine(cv::Mat& mat)
		{
			for (auto line : lines)
				cv::line(mat, { (int)line.lineStartX, (int)line.lineStartY }, { (int)line.lineEndX, (int)line.lineEndY }, cv::Scalar{ 255.0, 255.0, 255.0 }, 2);
		}
	};
}
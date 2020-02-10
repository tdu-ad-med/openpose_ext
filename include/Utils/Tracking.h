#pragma once

#include <openpose/core/common.hpp>

#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <OpenPoseWrapper/OpenPoseEvent.h>
#include <Utils/Vector.h>
#include <Utils/Gui.h>
#include <Utils/Database.h>

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
		void addFrame(ImageInfo& imageInfo, std::shared_ptr<Database>& database);

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

		float getDistance(const std::vector<ImageInfo::Node>& nodes1, const std::vector<ImageInfo::Node>& nodes2);
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
		// 1�t���[���O��static�J�E���g��dynamic�J�E���g�̘a
		uint64_t preUpCount = 0;  // �����̏���㑤�Ɉړ������l�̃J�E���g
		uint64_t preDownCount = 0;  // �����̏�������Ɉړ������l�̃J�E���g
		// �l���J�E���g���s������̎n�_�ƏI�_
		struct Line {
			float lineStartX, lineStartY, lineEndX, lineEndY;
		};
		std::vector<Line> lines;

		enum Event { UP, DOWN, NOTHING };

		bool isCross(vt::Vector4& p1Start, vt::Vector4& p1End, vt::Vector4& p2Start, vt::Vector4& p2End);

		Event judgeUpOrDown(op::Tree& peopleStart, op::Tree& peopleEnd, Line& line);

		Event judgeUpOrDown(op::Tree& peopleStart, op::Tree& peopleEnd);

	public:
		PeopleLineCounter(float startX = 0.0f, float startY = 0.0f, float endX = 1.0f, float endY = 1.0f);

		// ���i���X�V����
		void updateCount(PeopleList& people);

		// �J�E���g�̃��Z�b�g
		void resetCount();

		// �J�E���g�̊���̈ʒu�ݒ�(�n�_X, �n�_Y, �I�_X, �I�_Y)
		void setLine(float lineStartX, float lineStartY, float lineEndX, float lineEndY);

		// �J�E���g�̊���̈ʒu�ݒ�(�n�_X, �n�_Y, �I�_X, �I�_Y, ���̑���)
		void setLine(float lineStartX, float lineStartY, float lineEndX, float lineEndY, float lineWeigth);

		// �����������Ɉړ������l�̃J�E���g���擾
		inline uint64_t getUpCount() { return staticUpCount + dynamicUpCount; }
		// �����������Ɉړ������l�̃J�E���g���擾
		inline uint64_t getDownCount() { return staticDownCount + dynamicDownCount; }

		// 1�t���[���O�Ɣ�ׁA�J�E���g�ɕω������������ǂ������擾
		inline bool isChanged() { return ((preUpCount != getUpCount()) || preDownCount != getDownCount()); }

		// ����̕`��
		void drawJudgeLine(cv::Mat& mat);

		// �l�X�̎n�_�ƏI�_�����Ԓ����̕`��
		void drawPeopleLine(cv::Mat& mat, PeopleList& people, bool drawId);
	};
}
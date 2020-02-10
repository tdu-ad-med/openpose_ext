#include <Utils/Tracking.h>

namespace op
{
	Node::Node() :
		x{ 0.0f },
		y{ 0.0f },
		confidence{ 0.0f },
		confidenceThreshold{ 0.5f }
	{
	}

	Node::Node(
		float x,
		float y,
		float confidence = 0.0f,
		float confidenceThreshold = 0.5f
	) :
		x{ x },
		y{ y },
		confidence{ confidence },
		confidenceThreshold{ confidenceThreshold }
	{
	}

	Node::Node(const Node& node) :
		x{ node.x },
		y{ node.y },
		confidence{ node.confidence },
		confidenceThreshold{ node.confidenceThreshold }
	{
	}

	bool Node::isTrusted()
	{
		return confidence > confidenceThreshold;
	}

	float Node::distanceFrom(const Node& node)
	{
		float x = this->x - node.x;
		float y = this->y - node.y;
		return std::sqrtf((x * x) + (y * y));
	}

	Tree::Tree() :
		frameNumber{ 0 },
		numberNodesToTrust{ 5 },
		confidenceThreshold{ 0.5 },
		valid{ false }
	{
	}

	Tree::Tree(
		std::vector<Node> nodes,
		uint64_t frameNumber = 0,
		uint64_t numberNodesToTrust = 5,
		float confidenceThreshold = 0.5,
		bool valid = true
	) :
		nodes{ nodes },
		frameNumber{ frameNumber },
		numberNodesToTrust{ numberNodesToTrust },
		confidenceThreshold{ confidenceThreshold },
		valid{ valid }
	{
	}

	Tree::Tree(const Tree& tree) :
		nodes{ tree.nodes },
		frameNumber{ tree.frameNumber },
		numberNodesToTrust{ tree.numberNodesToTrust },
		confidenceThreshold{ tree.confidenceThreshold },
		valid{ tree.valid }
	{
	}

	bool Tree::isTrusted()
	{
		uint64_t count = 0;
		for (auto node : nodes)
		{
			if (node.isTrusted()) count++;
		}
		return count >= numberNodesToTrust;
	}

	float Tree::distanceFrom(std::vector<Node>& nodes)
	{
		uint64_t samples = 0;
		float distance = 0.0f;
		for (uint64_t index = 0; index < nodes.size(); index++)
		{
			if (this->nodes[index].isTrusted() && nodes[index].isTrusted())
			{
				distance += this->nodes[index].distanceFrom(nodes[index]);
				samples++;
			}
		}
		return (samples == 0) ? (-1.0) : (distance / samples);
	}

	Node Tree::average()
	{
		Node result{ 0.0f, 0.0f, 1.0f };
		uint64_t samples = 0;
		for (auto node : nodes)
		{
			if (!node.isTrusted()) continue;
			result.x += node.x;
			result.y += node.y;
			samples++;
		}
		if (samples == 0) return Node{ 0.0f, 0.0f, 0.0f };
		result.x /= (float)samples;
		result.y /= (float)samples;
		return result;
	}

	bool Tree::isValid() {
		return valid;
	}

	PeopleList::PeopleList() :
		numberNodesToTrust{ 5 },
		confidenceThreshold{ 0.5f },
		numberFramesToLost{ 10 },
		distanceThreshold{ 50.0f }
	{
	}

	PeopleList::PeopleList(
		uint64_t numberNodesToTrust = 5,
		float confidenceThreshold = 0.5f,
		uint64_t numberFramesToLost = 10,
		float distanceThreshold = 50.0f
	) :
		numberNodesToTrust{ numberNodesToTrust },
		confidenceThreshold{ confidenceThreshold },
		numberFramesToLost{ numberFramesToLost },
		distanceThreshold{ distanceThreshold }
	{
	}

	void PeopleList::addFrame(ImageInfo& imageInfo)
	{
		backTrees = currentTrees;  // 1�t���[���O�ɉf���Ă����l�̂��ׂĂ̍��i�����ꎞ�ۑ�
		currentTrees.clear();  // ���ׂĂ̐l�̍��i����������
		// ���݂̃t���[���̐l�������[�v
		for (auto person = imageInfo.people.begin(); person != imageInfo.people.end(); person++)
		{
			std::vector<Node> nodes;  // 1�l���̍��i�����i�[����z��
			// poseKeypoints����nodes�֍��i�����R�s�[
			for (auto node : person->second)
			{
				nodes.push_back(Node(
					node.x, node.y, node.confidence,
					confidenceThreshold
				));
			}
			Tree tree = Tree(
				nodes,
				imageInfo.frameNumber,
				numberNodesToTrust,
				confidenceThreshold
			);
			if (tree.average().confidence == 0.0f) continue;
			// �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���擾
			uint64_t nearestPeopleIndex = 0;  // �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���i�[����ꎞ�ϐ�
			float nearestLength = -1.0f;  // �O�t���[���̒��ň�ԋ������߂������������i�[����ꎞ�ϐ�
			bool lostFlag = true;  // �O�t���[���ň�ԋ������߂������l�����o�ł��Ȃ������ꍇ��True�ɂȂ�t���O
			// �O�t���[���̐l�������[�v
			for (auto treeItr = backTrees.begin(); treeItr != backTrees.end(); treeItr++)
			{
				uint64_t index = treeItr->first;  // �C���f�b�N�X�̎擾
				Tree tree_ = treeItr->second;  // ���i���̎擾
				// ���i���numberFramesToLost�t���[���ȏ�O�̃t���[���ł���Ώ��O����
				if (imageInfo.frameNumber - tree_.frameNumber > numberFramesToLost) continue;
				// ���i���(numberFramesToLost+1)�t���[���ȏ�O�̃t���[���ł����firstTrees���珜������
				if (imageInfo.frameNumber - tree_.frameNumber > (numberFramesToLost + 1))
				{
					if (firstTrees.count(index) != 0) firstTrees.erase(index);
				}
				// ��U���i�������݂̃t���[���ɒǉ�����
				if (currentTrees.count(index) == 0) currentTrees[index] = tree_;
				// �O�t���[������̈ړ��������Z�o
				float distance = tree_.distanceFrom(tree.nodes);
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
			// �O�t���[���ň�ԋ������߂������l�����o�ł����ꍇ�͂��̐l�̃C���f�b�N�X���A�����łȂ��ꍇ�͐V�����C���f�b�N�X�����߂�
			uint64_t addIndex = lostFlag ? (sumOfPeople++) : nearestPeopleIndex;
			// ���݂̃t���[���ɐl��ǉ�
			currentTrees[addIndex] = tree;
			// ���߂ēo�ꂷ��l��firstTree�ɂ��ǉ�
			if (lostFlag) firstTrees[addIndex] = currentTrees[addIndex];
		}
	}
	void PeopleList::addFrame(ImageInfo& imageInfo, std::shared_ptr<Database>& database)
	{
		// ���݂̃t���[���̍��i���o����0�Ȃ�I��
		if (imageInfo.people.size() == 0) return;

		// ���݂̃t���[���̃f�[�^���Ɋ���sql�ɑ��݂��Ă���ΏI��
		if (database->isDataExist(u8"people_with_tracking", u8"frame", imageInfo.frameNumber)) return;

		// 1�t���[���O����numberFramesToLost�t���[���O�܂ł̍ŐV�̍��i���擾
		SQLite::Statement query(*(database->database), u8"SELECT * FROM people_with_tracking WHERE ? <= frame AND frame < ? GROUP BY people HAVING frame = MAX(frame)");
		database->bindAll(query, (int64_t)imageInfo.frameNumber - (int64_t)numberFramesToLost, (int64_t)imageInfo.frameNumber);
		std::map<size_t, std::vector<ImageInfo::Node>> backPeople;
		while (query.executeStep())
		{
			size_t index = (size_t)query.getColumn(1).getInt64();
			for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
			{
				backPeople[index].push_back(ImageInfo::Node{
					(float)query.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
					(float)query.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
					(float)query.getColumn(2 + nodeIndex * 3 + 2).getDouble()
				});
			}
		}

		// SQL���̐���
		std::string row = u8"?";
		for (int colIndex = 0; colIndex < 76; colIndex++) row += u8", ?";
		row = u8"INSERT INTO people_with_tracking VALUES (" + row + u8")";
		SQLite::Statement pwtQuery(*(database->database), row);

		// ���݂̃t���[���̐l�������[�v
		for (auto currentPerson = imageInfo.people.begin(); currentPerson != imageInfo.people.end(); currentPerson++)
		{
			// ���i�f�[�^�̐M���x��臒l�����ł���΃X�L�b�v
			auto&& currentNodes = currentPerson->second;
			uint64_t confidenceCount = 0;
			for (auto node : currentNodes) { if (node.confidence > confidenceThreshold) confidenceCount++; }
			if (confidenceCount < numberNodesToTrust) continue;

			// �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���擾
			uint64_t nearestPeopleIndex = 0;  // �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���i�[����ꎞ�ϐ�
			float nearestLength = -1.0f;  // �O�t���[���̒��ň�ԋ������߂������������i�[����ꎞ�ϐ�
			bool lostFlag = true;  // �O�t���[���ň�ԋ������߂������l�����o�ł��Ȃ������ꍇ��True�ɂȂ�t���O

			// �O�t���[���̐l�������[�v
			for (auto&& backPerson = backPeople.begin(); backPerson != backPeople.end(); backPerson++)
			{
				uint64_t index = backPerson->first;  // �C���f�b�N�X�̎擾
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
				SQLite::Statement peopleCountQuery(*(database->database), "SELECT COUNT(DISTINCT people) from people_with_tracking");
				(void)peopleCountQuery.executeStep();
				addIndex = peopleCountQuery.getColumn(0).getInt();
			}

			// ���݂̃t���[���ɐl��ǉ�
			pwtQuery.reset();
			pwtQuery.bind(1, (long long)imageInfo.frameNumber);
			pwtQuery.bind(2, (long long)addIndex);
			for (size_t nodeIndex = 0; nodeIndex < currentNodes.size(); nodeIndex++)
			{
				pwtQuery.bind(3 + nodeIndex * 3 + 0, (double)currentNodes[nodeIndex].x);
				pwtQuery.bind(3 + nodeIndex * 3 + 1, (double)currentNodes[nodeIndex].y);
				pwtQuery.bind(3 + nodeIndex * 3 + 2, (double)currentNodes[nodeIndex].confidence);
			}
			(void)pwtQuery.exec();
		}
	}

	std::vector<uint64_t> PeopleList::getCurrentIndices()
	{
		std::vector<uint64_t> result;
		for (auto treeItr = currentTrees.begin(); treeItr != currentTrees.end(); treeItr++)
		{
			result.push_back(treeItr->first);
		}
		return result;
	}

	std::vector<uint64_t> PeopleList::getBackIndices()
	{
		std::vector<uint64_t> result;
		for (auto treeItr = backTrees.begin(); treeItr != backTrees.end(); treeItr++)
		{
			result.push_back(treeItr->first);
		}
		return result;
	}

	std::vector<uint64_t> PeopleList::getLostIndices()
	{
		std::vector<uint64_t> result;
		for (auto backTreeItr = backTrees.begin(); backTreeItr != backTrees.end(); backTreeItr++)
		{
			bool lostFlag = true;
			for (auto currentTreeItr = currentTrees.begin(); currentTreeItr != currentTrees.end(); currentTreeItr++)
			{
				if (backTreeItr->first == currentTreeItr->first)
				{
					lostFlag = false;
					break;
				}
			}
			if (lostFlag) result.push_back(backTreeItr->first);
		}
		return result;
	}

	Tree PeopleList::getCurrentTree(uint64_t index) {
		if (currentTrees.count(index) == 0) return Tree();
		return currentTrees[index];
	}
	Tree PeopleList::getBackTree(uint64_t index) {
		if (backTrees.count(index) == 0) return Tree();
		return backTrees[index];
	}
	Tree PeopleList::getFirstTree(uint64_t index) {
		if (firstTrees.count(index) == 0) return Tree();
		return firstTrees[index];
	}
	float PeopleList::getDistance(const std::vector<ImageInfo::Node>& nodes1, const std::vector<ImageInfo::Node>& nodes2)
	{
		uint64_t samples = 0;  // �L���Ȋ֐߂̃T���v����
		float distance = 0.0f;  // �L���ȑS�֐߂̈ړ��ʂ̕���
		for (uint64_t index = 0; index < nodes1.size(); index++)
		{
			// 臒l�ȉ��̊֐߂͖���
			if ((nodes1[index].confidence <= confidenceThreshold) && (nodes2[index].confidence <= confidenceThreshold)) continue;
			float x = nodes1[index].x - nodes2[index].x;
			float y = nodes1[index].y - nodes2[index].y;
			distance += std::sqrtf((x * x) + (y * y));
			samples++;
		}
		return (samples == 0) ? (-1.0) : (distance / samples);
	}

	bool PeopleLineCounter::isCross(vt::Vector4& p1Start, vt::Vector4& p1End, vt::Vector4& p2Start, vt::Vector4& p2End)
	{
		// p1Start����p1End�ւ̒�����p2Start����p2End�ւ̒������������Ă��邩�ǂ��������߂�
		// �Q�l : https://imagingsolution.blog.fc2.com/blog-entry-137.html
		double s1 = ((p2End.x - p2Start.x) * (p1Start.y - p2Start.y) - (p2End.y - p2Start.y) * (p1Start.x - p2Start.x)) / 2.0;
		double s2 = ((p2End.x - p2Start.x) * (p2Start.y - p1End.y) - (p2End.y - p2Start.y) * (p2Start.x - p1End.x)) / 2.0;
		if (s1 + s2 == 0.0) return false;
		double p = s1 / (s1 + s2);
		return (0.0 <= p && p <= 1.0);
	}

	PeopleLineCounter::Event PeopleLineCounter::judgeUpOrDown(op::Tree& peopleStart, op::Tree& peopleEnd, Line& line)
	{
		auto startPos = peopleStart.average();  // ���s�҂̃g���b�L���O���J�n�����_
		auto endPos = peopleEnd.average();  // ���s�҂̃g���b�L���O���I�������_
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

	PeopleLineCounter::Event PeopleLineCounter::judgeUpOrDown(op::Tree& peopleStart, op::Tree& peopleEnd)
	{
		Event e = Event::NOTHING;
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (i == 0) e = judgeUpOrDown(peopleStart, peopleEnd, lines[i]);
			else if (e != judgeUpOrDown(peopleStart, peopleEnd, lines[i])) return Event::NOTHING;
		}
		return e;
	}

	PeopleLineCounter::PeopleLineCounter(float startX, float startY, float endX, float endY)
	{
		setLine(startX, startY, endX, endY);
	}

	// ���i���X�V����
	void PeopleLineCounter::updateCount(PeopleList& people)
	{
		// 1�t���[���O�̏����L��
		preUpCount = getUpCount();
		preDownCount = getDownCount();

		// �g���b�L���O���O��Ă��Ȃ��l�̃J�E���^�����Z�b�g
		dynamicUpCount = 0;
		dynamicDownCount = 0;

		// �g���b�L���O���O��Ă��Ȃ��l�̈ړ������J�E���g
		const std::vector<uint64_t> currentIndex = people.getCurrentIndices();
		for (size_t index : currentIndex)
		{
			auto e = judgeUpOrDown(people.getFirstTree(index), people.getCurrentTree(index));
			if (e == Event::UP) dynamicUpCount++;
			if (e == Event::DOWN) dynamicDownCount++;
		}

		// �g���b�L���O���O�ꂽ�l�̈ړ������J�E���g
		const std::vector<uint64_t> lostIndex = people.getLostIndices();
		for (size_t index : lostIndex)
		{
			auto e = judgeUpOrDown(people.getFirstTree(index), people.getBackTree(index));
			if (e == Event::UP) staticUpCount++;
			if (e == Event::DOWN) staticDownCount++;
		}
	}

	// �J�E���g�̃��Z�b�g
	void PeopleLineCounter::resetCount()
	{
		staticUpCount = 0;
		staticDownCount = 0;
		dynamicUpCount = 0;
		dynamicDownCount = 0;
	}

	// �J�E���g�̊���̈ʒu�ݒ�(�n�_X, �n�_Y, �I�_X, �I�_Y)
	void PeopleLineCounter::setLine(float lineStartX, float lineStartY, float lineEndX, float lineEndY)
	{
		this->lines.clear();
		this->lines.push_back(Line{ lineStartX, lineStartY, lineEndX, lineEndY });
	}

	// �J�E���g�̊���̈ʒu�ݒ�(�n�_X, �n�_Y, �I�_X, �I�_Y, ���̑���)
	void PeopleLineCounter::setLine(float lineStartX, float lineStartY, float lineEndX, float lineEndY, float lineWeigth)
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

	// ����̕`��
	void PeopleLineCounter::drawJudgeLine(cv::Mat& mat)
	{
		for (auto line : lines)
			cv::line(mat, { (int)line.lineStartX, (int)line.lineStartY }, { (int)line.lineEndX, (int)line.lineEndY }, cv::Scalar{ 255.0, 255.0, 255.0 }, 2);
	}

	// �l�X�̎n�_�ƏI�_�����Ԓ����̕`��
	void PeopleLineCounter::drawPeopleLine(cv::Mat& mat, PeopleList& people, bool drawId)
	{
		// �g���b�L���O�̎n�_�ƏI�_�����Ԓ�����`��
		for (size_t index : people.getCurrentIndices())
		{
			auto firstTree = people.getFirstTree(index);
			auto currentTree = people.getCurrentTree(index);
			if ((!firstTree.isValid()) || (!currentTree.isValid())) continue;

			// �����̕`��
			cv::line(mat, { (int)firstTree.average().x, (int)firstTree.average().y }, { (int)currentTree.average().x, (int)currentTree.average().y }, cv::Scalar{
				(double)((int)((std::sin((double)index * 463763.0) + 1.0) * 100000.0) % 120 + 80),
				(double)((int)((std::sin((double)index * 1279.0) + 1.0) * 100000.0) % 120 + 80),
				(double)((int)((std::sin((double)index * 92763.0) + 1.0) * 100000.0) % 120 + 80)
				}, 2.0);

			// id�̕`��
			if (drawId) gui::text(
				mat, std::to_string(index),
				{ (int)currentTree.average().x, (int)currentTree.average().y },
				gui::CENTER_CENTER, 0.5
			);
		}
	}
}
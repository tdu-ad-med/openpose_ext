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
			// �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���擾
			uint64_t nearestPeopleIndex = 0;  // �O�t���[���̒��ň�ԋ������߂������l�̃C���f�b�N�X���i�[����ꎞ�ϐ�
			float nearestLength = -1.0f;  // �O�t���[���̒��ň�ԋ������߂������������i�[����ꎞ�ϐ�
			bool lostFlag = true;  // �O�t���[���ň�ԋ������߂������l�����o�ł��Ȃ������ꍇ��True�ɂȂ�t���O
			// �O�t���[���̐l�������[�v
			for (auto treeItr = backTrees.begin(); treeItr != backTrees.end(); treeItr++)
			{
				uint64_t index = treeItr->first;  // �C���f�b�N�X�̎擾
				Tree tree = treeItr->second;  // ���i���̎擾
				// ���i���numberFramesToLost�t���[���ȏ�O�̃t���[���ł���Ώ��O����
				if (imageInfo.frameNumber - tree.frameNumber > numberFramesToLost) continue;
				// ���i���(numberFramesToLost+1)�t���[���ȏ�O�̃t���[���ł����firstTrees���珜������
				if (imageInfo.frameNumber - tree.frameNumber > (numberFramesToLost + 1))
				{
					if (firstTrees.count(index) != 0) firstTrees.erase(index);
				}
				// ��U���i�������݂̃t���[���ɒǉ�����
				if (currentTrees.count(index) == 0) currentTrees[index] = tree;
				// �O�t���[������̈ړ��������Z�o
				float distance = tree.distanceFrom(nodes);
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
			currentTrees[addIndex] = Tree(
				nodes,
				imageInfo.frameNumber,
				numberNodesToTrust,
				confidenceThreshold
			);
			// ���߂ēo�ꂷ��l��firstTree�ɂ��ǉ�
			if (lostFlag) firstTrees[addIndex] = currentTrees[addIndex];
		}

		imageInfo.people.clear();
		for (auto person = currentTrees.begin(); person != currentTrees.end(); person++)
		{
			std::vector<ImageInfo::Node> nodes;
			for (auto node : person->second.nodes)
			{
				nodes.push_back(ImageInfo::Node{
					node.x, node.y, node.confidence
				});
			}
			imageInfo.people[person->first] = std::move(nodes);
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
}
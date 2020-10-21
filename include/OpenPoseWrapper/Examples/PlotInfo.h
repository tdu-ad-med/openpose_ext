#pragma once

#include <Utils/Gui.h>
#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/Video.h>

#include <chrono>
#include <string>

class PlotInfo
{
private:
	using clock = std::chrono::high_resolution_clock;
	clock::time_point start, end;
	bool displayInfomation, displayJoints, displayId;

	void renderKeypoints(cv::Mat& cvFrame, const MinOpenPose::People& people, const MinOpenPose& mop)
	{
		if (people.size() == 0) return;

		op::Array<float> keypoints({ (int)people.size(), (int)((people.begin())->second.size()), 3 });
		for (int person_index = 0; person_index < people.size(); person_index++)
		{
			const MinOpenPose::Person person = people.at((size_t)person_index);
			for (int node_index = 0; node_index < person.size(); node_index++)
			{
				const MinOpenPose::Node node = person[node_index];
				keypoints[{person_index, node_index, 0}] = node.x;
				keypoints[{person_index, node_index, 1}] = node.y;
				keypoints[{person_index, node_index, 2}] = node.confidence;
			}
		}
		auto conf = mop.getConfig();

		const std::vector<unsigned int>& pairs = op::getPoseBodyPartPairsRender(conf.poseModel);
		const std::vector<float> colors = op::getPoseColors(conf.poseModel);
		const float thicknessCircleRatio = 1.f / 75.f;
		const float thicknessLineRatioWRTCircle = 0.75f;
		const std::vector<float>& poseScales = getPoseScales(conf.poseModel);
		const float threshold = conf.renderThreshold;

        // Get frame channels
        const auto width = cvFrame.size[1];
        const auto height = cvFrame.size[0];
        const auto area = width * height;

        // Parameters
        const auto lineType = 8;
        const auto shift = 0;
        const auto numberColors = colors.size();
        const auto numberScales = poseScales.size();
        const float thresholdRectangle = 0.1f;
        const auto numberKeypoints = keypoints.getSize(1);

		// Keypoints
        for (auto person = 0; person < keypoints.getSize(0); person++)
        {
			const auto personRectangle = op::getKeypointsRectangle(keypoints, person, thresholdRectangle);
            if (personRectangle.area() > 0)
            {
                const auto ratioAreas = op::fastMin(
					1.0f, op::fastMax(
                        personRectangle.width / (float)width, personRectangle.height / (float)height));
                // Size-dependent variables
                const auto thicknessRatio = op::fastMax(
                    op::positiveIntRound(std::sqrt(area) * thicknessCircleRatio * ratioAreas), 2);
                // Negative thickness in cv::circle means that a filled circle is to be drawn.
                const auto thicknessCircle = op::fastMax(1, (ratioAreas > 0.05f ? thicknessRatio : -1));
                const auto thicknessLine = op::fastMax(
                    1, op::positiveIntRound(thicknessRatio * thicknessLineRatioWRTCircle));
                const auto radius = thicknessRatio / 2;

                // Draw lines
                for (auto pair = 0u; pair < pairs.size(); pair += 2)
                {
                    const auto index1 = (person * numberKeypoints + pairs[pair]) * keypoints.getSize(2);
                    const auto index2 = (person * numberKeypoints + pairs[pair + 1]) * keypoints.getSize(2);
                    if (keypoints[index1 + 2] > threshold && keypoints[index2 + 2] > threshold)
                    {
                        const auto thicknessLineScaled = op::positiveIntRound(
                            thicknessLine * poseScales[pairs[pair + 1] % numberScales]);
                        const auto colorIndex = pairs[pair + 1] * 3; // Before: colorIndex = pair/2*3;
                        const cv::Scalar color{
                            colors[(colorIndex + 2) % numberColors],
                            colors[(colorIndex + 1) % numberColors],
                            colors[colorIndex % numberColors]
                        };
                        const cv::Point keypoint1{
                            op::positiveIntRound(keypoints[index1]), op::positiveIntRound(keypoints[index1 + 1]) };
                        const cv::Point keypoint2{
                            op::positiveIntRound(keypoints[index2]), op::positiveIntRound(keypoints[index2 + 1]) };
                        cv::line(cvFrame, keypoint1, keypoint2, color, thicknessLineScaled, lineType, shift);
					}
				}
            }
        }
    }

public:
	/**
	 * OpenPose�̏�Ԃ��摜�ɏ㏑���`�悷��N���X
	 * @param displayInfomation �t���[�����[�g�Ɠ���̍Đ����Ԃ�\�����邩�ǂ���
	 * @param displayJoints ���i��\�����邩�ǂ���
	 * @param displayId �l��ID��\�����邩�ǂ���
	 */
	PlotInfo(bool displayInfomation, bool displayJoints, bool displayId) :
		displayInfomation(displayInfomation), displayJoints(displayJoints), displayId(displayId)
	{
		start = clock::now();
	}

	virtual ~PlotInfo() {};

	// �t���[�����[�g�ƃt���[���ԍ��̕`��
	void plotFrameInfo(cv::Mat& frame, const Video& video)
	{
		// �t���[�����󂩂��m�F����
		if (frame.empty()) return;

		// fps�̑���
		end = clock::now();
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		float fps = 1000.0f / (float)time;
		start = end;

		// ����̍Đ����̎擾
		Video::VideoInfo videoInfo = video.getInfo();

		// fps�Ɠ���̍Đ����ԁA�t���[���ԍ��̕\��
		cv::Size ret{ 0, 0 }; int height = 20;
		ret = gui::text(frame, "fps : " + std::to_string(fps), cv::Point{ 20, height }); height += ret.height + 10;
		ret = gui::text(frame, "time : " + std::to_string(videoInfo.frameTimeStamp), cv::Point{ 20, height }); height += ret.height + 10;
		ret = gui::text(frame, "frame : " + std::to_string(videoInfo.frameNumber) + " / " + std::to_string(videoInfo.frameSum), cv::Point{ 20, height }); height += ret.height + 10;
	}

	// ���i��ID�̕`��
	void plotBone(cv::Mat& frame, const MinOpenPose& mop, const MinOpenPose::People& people)
	{
		// �t���[�����󂩂��m�F����
		if (frame.empty()) return;

		// ���i�̕\��
		renderKeypoints(frame, people, mop);

		// �l��ID�����i�̏d�S�ʒu�ɕ\��
		for (auto person = people.begin(); person != people.end(); person++)
		{
			cv::Point p; size_t enableNodeSum = 0;
			// �֐߂̐��������[�v���� (BODY25���f�����g���ꍇ��25��)
			for (auto node : person->second)
			{
				// �M���l�� 0 �̊֐߂͍��W�� (0, 0) �ɂȂ��Ă��邽�ߏ��O����
				if (node.confidence == 0.0f) break;
				// ���Z
				p.x += (int)node.x; p.y += (int)node.y; enableNodeSum++;
			}
			// ���i�̏d�S���v�Z
			p.x /= enableNodeSum; p.y /= enableNodeSum;
			// ID�̕\��
			gui::text(frame, std::to_string(person->first), p, gui::CENTER_CENTER, 0.5);
		}
	}
};
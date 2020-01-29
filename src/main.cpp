#include <MinOpenPose.h>

int main(int argc, char* argv[])
{
	// Web�J�����ɐڑ��J�n
	cv::VideoCapture cap(0);
	if (!cap.isOpened()) return -1;

	// OpenPose�̋N��
	MinimumOpenPose mop;
	mop.startup();
	
	// OpenPose�̃C�x���g���[�v����
	while (true)
	{
		// OpenPose �̏�����Ԃ̊m�F
		switch (mop.getProcessState())
		{
		case MinimumOpenPose::ProcessState::WaitInput: // ���͑ҋ@
		{
			auto frame = std::make_unique<cv::Mat>();
			cap.read(*frame);
			mop.pushImage(std::move(frame)); // OpenPose �ɉ摜��n��
			if (cv::waitKey(1) == 0x1b) {
				mop.shutdown();
				return 0;
			}
		}
		break;
		case MinimumOpenPose::ProcessState::Processing: // ������
			break;
		case MinimumOpenPose::ProcessState::Finish: // �����̏I��
		{
			auto results = mop.getResultsAndReset(); // �o�͂��ꂽ�f�[�^�̎擾
			if (!static_cast<bool>(results)) break;
			for (auto result : *results)
			{
				cv::imshow("result", result->cvOutputData);
			}
		}
		break;
		case MinimumOpenPose::ProcessState::Shutdown: // �N�����Ă��Ȃ�
			break;
		case MinimumOpenPose::ProcessState::Error: // �G���[����
		{
			auto errors = mop.getErrors();
			for (auto error : errors) std::cout << error << std::endl;
			mop.resetErrors();
			return -1;
		}
		break;
		}
	}

	return 0;
}

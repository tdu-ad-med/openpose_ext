#include <OpenPoseWrapper/MinimumOpenPose.h>

int main(int argc, char* argv[])
{
	MinimumOpenPose mop;

	ExampleOpenPoseEvent cope {R"(G:\�v���o\Dropbox\Dropbox\SDK\openpose\���������������f�[�^\openpose\video\58��.mp4)"};
	int ret = mop.startup(cope);

	return ret;
}

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>

int main(int argc, char* argv[])
{
	MinimumOpenPose mop;

	SqlOpenPoseEvent cope {R"(G:\�v���o\Dropbox\Dropbox\SDK\openpose\���������������f�[�^\openpose\video\58��.mp4)", false};
	int ret = mop.startup(cope);

	return ret;
}

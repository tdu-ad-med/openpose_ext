#pragma once

#include <opencv2/opencv.hpp>

#define M_PI 3.14159265358979

// Vector Tools
namespace vt
{
	class FisheyeToFlat
	{
	private:
		// �����o�ϐ�(fx, fy, cx, cy, k1, k2, k3, k4)�̈Ӗ��ɂ��Ă͈ȉ���URL���Q��
		// http://opencv.jp/opencv-2.1/cpp/camera_calibration_and_3d_reconstruction.html
		float fx = 0.0, fy = 0.0;  // �J�����̓����p�����[�^�s��̏œ_����
		float cx = 0.0, cy = 0.0;  // �J�����̓����p�����[�^�s��̎�_
		float k1 = 0.0, k2 = 0.0, k3 = 0.0, k4 = 0.0;  // �J�����̘c�݌W��(distortion coefficients)
		float cam_width = 0.0, cam_height = 0.0;  // �J�����L�����u���[�V�����ɗp�����摜�̉𑜓x
		float input_width = 0.0, input_height = 0.0;  // ���͉摜�̉𑜓x
		float output_scale = 1.0;  // �o�͉摜�̊g�嗦
		cv::Mat map1, map2;  // �c�ݕ␳��̃s�N�Z���̈ړ��ʒu��ێ�����z��
		bool change_param = false;  // �p�����[�^�ύX�t���O

	public:
		bool is_init = false;
		FisheyeToFlat();
		virtual ~FisheyeToFlat();
		void setParams(
			float cam_width, float cam_height, float output_scale,
			float fx, float fy, float cx, float cy,
			float k1 = 0.0, float k2 = 0.0, float k3 = 0.0, float k4 = 0.0
		);
		cv::Point2f translate(cv::Point2f p, float cols, float rows) const;
		cv::Point2f translate(cv::Point2f p, const cv::Mat& src) const;
		cv::Mat translateMat(const cv::Mat& src);
	};

	/**
	 * ���̃N���X�ł͉�ʏ�̎w�肳�ꂽ4�̓_�𒷕��`�ɂȂ�悤�Ɉ����L�΂��������s��
	 * ����ɂ��A�J�����̉摜���u�n�ʂ��ォ�猩���悤�ȉ摜�v�ɕϊ�����
	 * �܂��A�J�����̉f�������჌���Y�ȂǂŘc��ł���ꍇ�͘c�ݕ␳�������ɍs�����Ƃ��ł���
	 */
	class ScreenToGround
	{
	private:
		// ���[�U�[��`�p�����[�^�[
		float cam_w = 1.0f, cam_h = 1.0f;  // �J�����̉𑜓x
		cv::Point2f p1_, p2_, p3_, p4_;  // �c�ݕ␳�O�̃X�N���[�����W (����, �E��, �E��, ����)
		cv::Point2f p1, p2, p3, p4;  // �c�ݕ␳��̃X�N���[�����W (����, �E��, �E��, ����)
		cv::Point2f rect_size;  // p1����p4���͂���`�̃T�C�Y (p1 ���� p2 �܂ł̋���, p2 ���� p3 �܂ł̋���)
		cv::Mat perspectiveTransformMatrix;  // �����ϊ��s��
		FisheyeToFlat fisheyeToFlat;  // ���჌���Y�̘c�ݕ␳���s���N���X

	public:
		ScreenToGround();

		virtual ~ScreenToGround();

		/**
		 * �ˉe�ϊ��ɕK�v�ȃp�����[�^�[����͂���֐�
		 * @param cam_w, cam_h ���͉摜�̉𑜓x
		 * @param x1, y1 �J�����Ɏʂ��Ă���n�ʂ̔C�ӂ̓_1 (����)
		 * @param x2, y2 �J�����Ɏʂ��Ă���n�ʂ̔C�ӂ̓_2 (�E��)
		 * @param x3, y3 �J�����Ɏʂ��Ă���n�ʂ̔C�ӂ̓_3 (�E��)
		 * @param x4, y4 �J�����Ɏʂ��Ă���n�ʂ̔C�ӂ̓_4 (����)
		 * @param rect_width (x1, y1) ���� (x2, y2) �܂ł̒������w�肷�� (�P�ʂ͔C��)
		 * @param rect_height (x2, y2) ���� (x3, y3) �܂ł̒������w�肷�� (�P�ʂ͔C��)
		 * @note
		 * ���̃N���X�ł́Ax1��y1�ȂǂŎw�肵��4�̓_�𒷕��`�ɂȂ�悤�Ɉ����L�΂��������s���B
		 * ����ɂ��A�J�����̉摜���ォ�猩���悤�ȉ摜�ɕϊ�����B
		 */
		void setParams(
			float cam_w, float cam_h,
			float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
			float rect_width = 1.0, float rect_height = 1.0
		);

		/**
		 * �J�����̘c�ݕ␳���s���p�����[�^�[����͂���֐�
		 * @param cam_width, cam_heigth �J�����L�����u���[�V�������s�������̃J�����̉𑜓x
		 * @param output_scale �c�ݕ␳��̉摜�̊g�嗦
		 * @param fx, fy �J���������p�����[�^�̏œ_���� (�s�N�Z���P��)
		 * @param cx, cy �J���������p�����[�^�̎�_�ʒu (�s�N�Z���P��)
		 * @param k1, k2, k3, k4 ���a�����̘c�݌W��
		 * @note
		 * �œ_�������_�ʒu�A���a�����̘c�݌W���ɂ��ẮA�J�����L�����u���[�V�����œ���ꂽ�l����͂��Ă��������B
		 * �J�����L�����u���[�V�����ɂ��Ă͈ȉ��̃T�C�g���Q�l�ɂȂ�܂��B
		 * http://opencv.jp/opencv-2.1/cpp/camera_calibration_and_3d_reconstruction.html
		 * ��̓I�ȕ��@�ɂ��Ă͈ȉ��̃T�C�g���Q�l�ɂȂ�܂��B
		 * https://medium.com/@kennethjiang/calibrate-fisheye-lens-using-opencv-part-2-13990f1b157f
		 * 
		 * Todo: �J�����L�����u���[�V�����ɂ��Ă̐����������ƕ�����₷������
		 */
		void setCalibration(
			float cam_width, float cam_heigth, float output_scale,
			float fx, float fy, float cx, float cy, float k1, float k2, float k3, float k4
		);

		/**
		 * �w�肵��4�_�̏c���T�C�Y���擾���� (setParams �Ŏw�肵�� rect_width �� rect_height ���A���Ă���)
		 * @return rect_width �ɉ��� rect_height �ɏc������������
		 */
		cv::Point2f getRectSize() const;

		/**
		 * 1�̓_�����W�ϊ�����
		 * @param p �摜��̔C�ӂ̍��W
		 * @return �ϊ���̍��W
		 */
		cv::Point2f translate(cv::Point2f p) const;

		/**
		 * �摜��ϊ�����
		 * @param src ���͂���摜
		 * @param zoom �g�嗦
		 * @param drawLine true�ɂ����setParams�Ŏw�肵��4�_�ɒ�����`����s��
		 * @return �ϊ���̉摜
		 */
		cv::Mat ScreenToGround::translateMat(const cv::Mat& src, float zoom = 1.0f, bool drawLine = false);

		/**
		 * translate�ŕϊ��������W��translateMat�ŕ\���������W�ɕϊ�����
		 * @param p translate�ŕϊ��������W
		 * @return translateMat�ŕ\���������W
		 */
		cv::Point2f plot(cv::Point2f p, const cv::Mat& src, float zoom) const;

		/**
		 * ���W�ϊ��Řc�ݕ␳�݂̂��s��
		 * @param �摜��̔C�ӂ̍��W (x, y, z, w �̓� x, y �݂̂�����)
		 * @return �ϊ���̍��W
		 */
		cv::Point2f ScreenToGround::onlyFlat(cv::Point2f p);

		/**
		 * �摜�ϊ��Řc�ݕ␳�݂̂��s��
		 * @param ���͂���摜
		 * @return �ϊ���̉摜
		 */
		cv::Mat ScreenToGround::onlyFlatMat(const cv::Mat& src);
	};
};
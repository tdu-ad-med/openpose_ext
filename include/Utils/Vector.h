#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>

#include <opencv2/opencv.hpp>

#define M_PI 3.14159265358979

// Vector Tools
namespace vt
{
	// 4x4�s��̍\����
	struct Matrix4
	{
		double
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33;
		bool isEnable;  // 
		Matrix4(bool isEnable = true);
		Matrix4(
			double m00, double m01, double m02, double m03,
			double m10, double m11, double m12, double m13,
			double m20, double m21, double m22, double m23,
			double m30, double m31, double m32, double m33
		);
		Matrix4(const Matrix4& src);
		Matrix4& operator=(const Matrix4& src);
		Matrix4 getInvert();
		static Matrix4 cross(const Matrix4& left, const Matrix4& right)
		{
			return Matrix4(
				left.m00 * right.m00 + left.m01 * right.m10 + left.m02 * right.m20 + left.m03 * right.m30,
				left.m00 * right.m01 + left.m01 * right.m11 + left.m02 * right.m21 + left.m03 * right.m31,
				left.m00 * right.m02 + left.m01 * right.m12 + left.m02 * right.m22 + left.m03 * right.m32,
				left.m00 * right.m03 + left.m01 * right.m13 + left.m02 * right.m23 + left.m03 * right.m33,
				left.m10 * right.m00 + left.m11 * right.m10 + left.m12 * right.m20 + left.m13 * right.m30,
				left.m10 * right.m01 + left.m11 * right.m11 + left.m12 * right.m21 + left.m13 * right.m31,
				left.m10 * right.m02 + left.m11 * right.m12 + left.m12 * right.m22 + left.m13 * right.m32,
				left.m10 * right.m03 + left.m11 * right.m13 + left.m12 * right.m23 + left.m13 * right.m33,
				left.m20 * right.m00 + left.m21 * right.m10 + left.m22 * right.m20 + left.m23 * right.m30,
				left.m20 * right.m01 + left.m21 * right.m11 + left.m22 * right.m21 + left.m23 * right.m31,
				left.m20 * right.m02 + left.m21 * right.m12 + left.m22 * right.m22 + left.m23 * right.m32,
				left.m20 * right.m03 + left.m21 * right.m13 + left.m22 * right.m23 + left.m23 * right.m33,
				left.m30 * right.m00 + left.m31 * right.m10 + left.m32 * right.m20 + left.m33 * right.m30,
				left.m30 * right.m01 + left.m31 * right.m11 + left.m32 * right.m21 + left.m33 * right.m31,
				left.m30 * right.m02 + left.m31 * right.m12 + left.m32 * right.m22 + left.m33 * right.m32,
				left.m30 * right.m03 + left.m31 * right.m13 + left.m32 * right.m23 + left.m33 * right.m33
			);
		}
		void print();
	};
	// 4�����x�N�g���̍\����
	struct Vector4
	{
		double x, y, z, w;
		bool isEnable;
		Vector4(bool isEnable = true);
		Vector4(double x, double y, double z = 0.0, double w = 1.0);
		Vector4(const Vector4& src);
		Vector4(const cv::Point& src);
		Vector4(const cv::Point2f& src);
		Vector4(const cv::Point2d& src);
		Vector4& operator=(const Vector4& src);
		bool operator==(const Vector4& src) const;
		bool operator!=(const Vector4& src) const;
		Vector4 operator+(const Vector4& src);
		Vector4 operator+(const double num);
		Vector4 operator-(const Vector4& src);
		Vector4 operator-(const double num);
		Vector4 operator*(const Vector4& src);
		Vector4 operator*(const double num);
		Vector4 operator/(const Vector4& src);
		Vector4 operator/(const double num);
		operator cv::Point() const;
		operator cv::Point2f() const;
		operator cv::Point2d() const;
		double length();
		Vector4 normal();
		Vector4 worldToScreen(Matrix4 projModelView);
		Vector4 screenToWorld(Matrix4 proj, double wTmp, double hTmp);
		double dot(const Vector4& left, const Vector4& right);
		static Vector4 cross(const Vector4& left, const Vector4& right)
		{
			return Vector4(
				left.y * right.z - left.z * right.y,
				left.z * right.x - left.x * right.z,
				left.x * right.y - left.y * right.x,
				left.w * right.w
			);
		}
		static Vector4 cross(const Matrix4& mat, const Vector4& vec)
		{
			return Vector4(
				mat.m00 * vec.x + mat.m01 * vec.y + mat.m02 * vec.z + mat.m03 * vec.w,
				mat.m10 * vec.x + mat.m11 * vec.y + mat.m12 * vec.z + mat.m13 * vec.w,
				mat.m20 * vec.x + mat.m21 * vec.y + mat.m22 * vec.z + mat.m23 * vec.w,
				mat.m30 * vec.x + mat.m31 * vec.y + mat.m32 * vec.z + mat.m33 * vec.w
			);
		}
		void print();
	};
	// �N�H�[�^�j�I����]
	Matrix4 getRotateMatrix(Vector4 axis, double rad);
	double deg2rad(double deg);
	double rad2deg(double rad);
	class FisheyeToFlat
	{
	private:
		// �����o�ϐ�(fx, fy, cx, cy, k1, k2, k3, k4)�̈Ӗ��ɂ��Ă͈ȉ���URL���Q��
		//   http://opencv.jp/opencv-2.1/cpp/camera_calibration_and_3d_reconstruction.html
		bool is_init = false;
		double fx = 0.0, fy = 0.0;  // �J�����̓����p�����[�^�s��̏œ_����
		double cx = 0.0, cy = 0.0;  // �J�����̓����p�����[�^�s��̎�_
		double k1 = 0.0, k2 = 0.0, k3 = 0.0, k4 = 0.0;  // �J�����̘c�݌W��(distortion coefficients)
		double cam_width = 0.0, cam_height = 0.0;  // �J�����L�����u���[�V�����ɗp�����摜�̉𑜓x
		double input_width = 0.0, input_height = 0.0;  // ���͉摜�̉𑜓x
		double output_scale = 1.0;  // �o�͉摜�̊g�嗦
		cv::Mat map1, map2;  // �L�����u���[�V������̃s�N�Z���̈ړ��ʒu��ێ�����z��
		bool change_param = false;  // �p�����[�^�ύX�t���O

	public:
		FisheyeToFlat();
		virtual ~FisheyeToFlat();
		void setParams(
			double cam_width, double cam_height, double output_scale,
			double fx, double fy, double cx, double cy,
			double k1 = 0.0, double k2 = 0.0, double k3 = 0.0, double k4 = 0.0
		);
		Vector4 translate(Vector4 p, double cols, double rows);
		Vector4 translate(Vector4 p, const cv::Mat& src);
		// �L�����u���[�V������̃J�����̐�����p���v�Z����(�]�����܂߂���p)
		double calcCamWFov(double cam_w_fov, double cols, double rows);
		double calcCamWFov(double cam_w_fov, const cv::Mat& src);
		// �L�����u���[�V������̃J�����̐�����p���v�Z����(�]�����܂߂���p)
		double calcCamHFov(double cam_h_fov, double cols, double rows);
		double calcCamHFov(double cam_h_fov, const cv::Mat& src);
		cv::Mat translateMat(const cv::Mat& src);
	};
	class ScreenToGround
	{
	private:
		// ���[�U�[��`�p�����[�^�[
		double cam_w, cam_h;  // �J�����̉𑜓x
		double cam_h_fov;  // �J�����̐�����p(deg)
		double cam_pos_h;  // �J�����̒n�ʂ���̍���(m)
		Vector4 p1_, p2_, p3_, p4_;  // �J�����L�����u���[�V�����O�̃X�N���[�����W(1�_��, 2�_��, 3�_��, 4�_��)
		Vector4 p1, p2, p3, p4;  // �J�����L�����u���[�V������̃X�N���[�����W(1�_��, 2�_��, 3�_��, 4�_��)
		FisheyeToFlat fisheyeToFlat;  // ���჌���Y�̘c�ݕ␳���s���N���X

		// �v�Z�ɂ�苁�܂�p�����[�^�[
		double cam_l;  // �J�������W�̌��_�����ʂ̒��S�܂ł̋���(�s�N�Z���P��)
		double cam_w_fov;  // �J�����̐�����p
		Matrix4 r2;  // ���f���r���[�s��̋t�s��
		Vector4 m_cam_pos;  // �}�[�J�[��Ԃł̃J�������W
		double m_cam_h;  // �}�[�J�[��Ԃł̃J�����̍����̌v�Z

		void calcParams();
	public:
		ScreenToGround();
		virtual ~ScreenToGround();
		void setParams(
			double cam_w_tmp, double cam_h_tmp, double cam_h_fov_tmp, double cam_pos_h_tmp,
			Vector4 p1_tmp, Vector4 p2_tmp, Vector4 p3_tmp, Vector4 p4_tmp
		);
		void setParams(
			double cam_w_tmp, double cam_h_tmp, double cam_h_fov_tmp, double cam_pos_h_tmp,
			double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4
		);
		void setCalibration(
			double cam_width, double cam_heigth, double output_scale,
			double fx, double fy, double cx, double cy, double k1, double k2, double k3, double k4
		);
		Vector4 translate(Vector4 p);
		void drawAreaLine(cv::Mat& mat, uint8_t mode);
		cv::Mat ScreenToGround::translateMat(const cv::Mat& src, float zoom = 1.0f);
		Vector4 ScreenToGround::onlyFlat(Vector4 p);
		cv::Mat ScreenToGround::onlyFlatMat(const cv::Mat& src);
	};
};
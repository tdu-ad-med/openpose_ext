#pragma once

#include <Utils/Vector.h>

namespace vt
{
	Matrix4::Matrix4(bool isEnable) :
		m00(0.0), m01(0.0), m02(0.0), m03(0.0),
		m10(0.0), m11(0.0), m12(0.0), m13(0.0),
		m20(0.0), m21(0.0), m22(0.0), m23(0.0),
		m30(0.0), m31(0.0), m32(0.0), m33(0.0),
		isEnable(isEnable)
	{}
	Matrix4::Matrix4(
		double m00, double m01, double m02, double m03,
		double m10, double m11, double m12, double m13,
		double m20, double m21, double m22, double m23,
		double m30, double m31, double m32, double m33
	) :
		m00(m00), m01(m01), m02(m02), m03(m03),
		m10(m10), m11(m11), m12(m12), m13(m13),
		m20(m20), m21(m21), m22(m22), m23(m23),
		m30(m30), m31(m31), m32(m32), m33(m33),
		isEnable(true)
	{}
	Matrix4::Matrix4(const Matrix4& src) :
		m00(src.m00), m01(src.m01), m02(src.m02), m03(src.m03),
		m10(src.m10), m11(src.m11), m12(src.m12), m13(src.m13),
		m20(src.m20), m21(src.m21), m22(src.m22), m23(src.m23),
		m30(src.m30), m31(src.m31), m32(src.m32), m33(src.m33),
		isEnable(src.isEnable)
	{}
	Matrix4& Matrix4::operator=(const Matrix4& src)
	{
		m00 = src.m00; m01 = src.m01; m02 = src.m02; m03 = src.m03;
		m10 = src.m10; m11 = src.m11; m12 = src.m12; m13 = src.m13;
		m20 = src.m20; m21 = src.m21; m22 = src.m22; m23 = src.m23;
		m30 = src.m30; m31 = src.m31; m32 = src.m32; m33 = src.m33;
		isEnable = src.isEnable;
		return *this;
	}
	Matrix4 Matrix4::getInvert()
	{
		const double m[16] = {
			m00, m10, m20, m30,
			m01, m11, m21, m31,
			m02, m12, m22, m32,
			m03, m13, m23, m33
		};

		double invOut[16];

		double inv[16], det;
		int i;

		inv[0] = m[5] * m[10] * m[15] -
			m[5] * m[11] * m[14] -
			m[9] * m[6] * m[15] +
			m[9] * m[7] * m[14] +
			m[13] * m[6] * m[11] -
			m[13] * m[7] * m[10];

		inv[4] = -m[4] * m[10] * m[15] +
			m[4] * m[11] * m[14] +
			m[8] * m[6] * m[15] -
			m[8] * m[7] * m[14] -
			m[12] * m[6] * m[11] +
			m[12] * m[7] * m[10];

		inv[8] = m[4] * m[9] * m[15] -
			m[4] * m[11] * m[13] -
			m[8] * m[5] * m[15] +
			m[8] * m[7] * m[13] +
			m[12] * m[5] * m[11] -
			m[12] * m[7] * m[9];

		inv[12] = -m[4] * m[9] * m[14] +
			m[4] * m[10] * m[13] +
			m[8] * m[5] * m[14] -
			m[8] * m[6] * m[13] -
			m[12] * m[5] * m[10] +
			m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] +
			m[1] * m[11] * m[14] +
			m[9] * m[2] * m[15] -
			m[9] * m[3] * m[14] -
			m[13] * m[2] * m[11] +
			m[13] * m[3] * m[10];

		inv[5] = m[0] * m[10] * m[15] -
			m[0] * m[11] * m[14] -
			m[8] * m[2] * m[15] +
			m[8] * m[3] * m[14] +
			m[12] * m[2] * m[11] -
			m[12] * m[3] * m[10];

		inv[9] = -m[0] * m[9] * m[15] +
			m[0] * m[11] * m[13] +
			m[8] * m[1] * m[15] -
			m[8] * m[3] * m[13] -
			m[12] * m[1] * m[11] +
			m[12] * m[3] * m[9];

		inv[13] = m[0] * m[9] * m[14] -
			m[0] * m[10] * m[13] -
			m[8] * m[1] * m[14] +
			m[8] * m[2] * m[13] +
			m[12] * m[1] * m[10] -
			m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] -
			m[1] * m[7] * m[14] -
			m[5] * m[2] * m[15] +
			m[5] * m[3] * m[14] +
			m[13] * m[2] * m[7] -
			m[13] * m[3] * m[6];

		inv[6] = -m[0] * m[6] * m[15] +
			m[0] * m[7] * m[14] +
			m[4] * m[2] * m[15] -
			m[4] * m[3] * m[14] -
			m[12] * m[2] * m[7] +
			m[12] * m[3] * m[6];

		inv[10] = m[0] * m[5] * m[15] -
			m[0] * m[7] * m[13] -
			m[4] * m[1] * m[15] +
			m[4] * m[3] * m[13] +
			m[12] * m[1] * m[7] -
			m[12] * m[3] * m[5];

		inv[14] = -m[0] * m[5] * m[14] +
			m[0] * m[6] * m[13] +
			m[4] * m[1] * m[14] -
			m[4] * m[2] * m[13] -
			m[12] * m[1] * m[6] +
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] +
			m[1] * m[7] * m[10] +
			m[5] * m[2] * m[11] -
			m[5] * m[3] * m[10] -
			m[9] * m[2] * m[7] +
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] -
			m[0] * m[7] * m[10] -
			m[4] * m[2] * m[11] +
			m[4] * m[3] * m[10] +
			m[8] * m[2] * m[7] -
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] +
			m[0] * m[7] * m[9] +
			m[4] * m[1] * m[11] -
			m[4] * m[3] * m[9] -
			m[8] * m[1] * m[7] +
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] -
			m[0] * m[6] * m[9] -
			m[4] * m[1] * m[10] +
			m[4] * m[2] * m[9] +
			m[8] * m[1] * m[6] -
			m[8] * m[2] * m[5];

		det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
			return Matrix4(false);

		det = 1.0 / det;

		for (i = 0; i < 16; i++)
			invOut[i] = inv[i] * det;

		return Matrix4(
			invOut[0], invOut[4], invOut[8], invOut[12],
			invOut[1], invOut[5], invOut[9], invOut[13],
			invOut[2], invOut[6], invOut[10], invOut[14],
			invOut[3], invOut[7], invOut[11], invOut[15]
		);
	}
	void Matrix4::print()
	{
		std::cout << std::fixed << std::setprecision(5) <<
			m00 << " " << m01 << " " << m02 << " " << m03 << "\n" <<
			m10 << " " << m11 << " " << m12 << " " << m13 << "\n" <<
			m20 << " " << m21 << " " << m22 << " " << m23 << "\n" <<
			m30 << " " << m31 << " " << m32 << " " << m33 << "\n" <<
			std::endl;
	}

	Vector4::Vector4(bool isEnable) :
		x(0.0), y(0.0), z(0.0), w(0.0), isEnable(isEnable) {}
	Vector4::Vector4(double x, double y, double z, double w) :
		x(x), y(y), z(z), w(w), isEnable(true) {}
	Vector4::Vector4(const Vector4& src) :
		x(src.x), y(src.y), z(src.z), w(src.w), isEnable(src.isEnable) {}
	Vector4::Vector4(const cv::Point& src) :
		x((double)src.x), y((double)src.y), z(0.0), w(0.0), isEnable(true) {}
	Vector4::Vector4(const cv::Point2f& src) :
		x((double)src.x), y((double)src.y), z(0.0), w(0.0), isEnable(true) {}
	Vector4::Vector4(const cv::Point2d& src) :
		x(src.x), y(src.y), z(0.0), w(0.0), isEnable(true) {}
	Vector4& Vector4::operator=(const Vector4& src)
	{
		x = src.x; y = src.y; z = src.z; w = src.w; isEnable = src.isEnable;
		return *this;
	}
	bool Vector4::operator==(const Vector4& src) const
	{
		return ((x == src.x) && (y == src.y) && (z == src.z) && (w == src.w) && (isEnable == src.isEnable));
	}
	bool Vector4::operator!=(const Vector4& src) const
	{
		return !((*this) == src);
	}
	Vector4 Vector4::operator+(const Vector4& src)
	{
		return Vector4(
			x + src.x,
			y + src.y,
			z + src.z,
			(w + src.w) / 2.0
		);
	}
	Vector4 Vector4::operator+(const double num)
	{
		return Vector4(
			x + num,
			y + num,
			z + num,
			w
		);
	}
	Vector4 Vector4::operator-(const Vector4& src)
	{
		return Vector4(
			x - src.x,
			y - src.y,
			z - src.z,
			(w + src.w) / 2.0
		);
	}
	Vector4 Vector4::operator-(const double num)
	{
		return Vector4(
			x - num,
			y - num,
			z - num,
			w
		);
	}
	Vector4 Vector4::operator*(const Vector4& src)
	{
		return Vector4(
			x * src.x,
			y * src.y,
			z * src.z,
			(w + src.w) / 2.0
		);
	}
	Vector4 Vector4::operator*(const double num)
	{
		return Vector4(
			x * num,
			y * num,
			z * num,
			w
		);
	}
	Vector4 Vector4::operator/(const Vector4& src)
	{
		return Vector4(
			x / src.x,
			y / src.y,
			z / src.z,
			(w + src.w) / 2.0
		);
	}
	Vector4 Vector4::operator/(const double num)
	{
		return Vector4(
			x / num,
			y / num,
			z / num,
			w
		);
	}
	Vector4::operator cv::Point() const
	{
		return cv::Point{ (int)x, (int)y };
	}
	Vector4::operator cv::Point2f() const
	{
		return cv::Point2f{ (float)x, (float)y };
	}
	Vector4::operator cv::Point2d() const
	{
		return cv::Point2d{ x, y };
	}
	double Vector4::length()
	{
		return std::sqrt(std::pow(x, 2.0) + std::pow(y, 2.0) + std::pow(z, 2.0));
	}
	Vector4 Vector4::normal()
	{
		return (*this) / length();
	}
	Vector4 Vector4::worldToScreen(Matrix4 projModelView)
	{
		Vector4 v = cross(projModelView, *this);
		return v / v.w;
	}
	Vector4 Vector4::screenToWorld(Matrix4 proj, double wTmp, double hTmp)
	{
		Vector4 p = Vector4(0, 0, 0);
		p.x = (x * 2.0 / wTmp) - 1.0;
		p.y = (y * 2.0 / hTmp) - 1.0;
		p.z = z;
		p.x = p.x * p.z / proj.m00;
		p.y = p.y * p.z / proj.m11;
		return p;
	}
	double Vector4::dot(const Vector4& left, const Vector4& right)
	{
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}
	void Vector4::print()
	{
		std::cout << std::fixed << std::setprecision(5) <<
			x << " " << y << " " << z << " " << w <<
			std::endl;
	}

	Matrix4 getRotateMatrix(Vector4 axis, double rad) {
		Vector4 naxis = axis.normal();
		double x = naxis.x * std::sin(rad / 2.0);
		double y = naxis.y * std::sin(rad / 2.0);
		double z = naxis.z * std::sin(rad / 2.0);
		double w = std::cos(rad / 2.0);
		return Matrix4(
			1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + w * z), 2.0 * (x * z - w * y), 0.0,
			2.0 * (x * y - w * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + w * x), 0.0,
			2.0 * (x * z + w * y), 2.0 * (y * z - w * x), 1.0 - 2.0 * (x * x + y * y), 0.0,
			0.0, 0.0, 0.0, 1.0
		);
	}
	double deg2rad(double deg)
	{
		return deg * (2.0 * M_PI) / 360.0;
	}
	double rad2deg(double rad)
	{
		return rad * 360.0 / (2.0 * M_PI);
	}

	void ScreenToGround::calcParams()
	{
		// �J�������W�̌��_�����ʂ̒��S�܂ł̋���(�s�N�Z���P��)
		cam_l = (0.5 * cam_h) / std::tan(0.5 * deg2rad(cam_h_fov));
		// �J�����̐�����p
		cam_w_fov = rad2deg(2.0 * std::atan((0.5 * cam_w) / cam_l));

		// �r���[�s��̌v�Z
		// ��ʏ�̃}�[�J�[�̊e�ӂ̃x�N�g���̌v�Z
		Vector4 line_top = Vector4(p2.x - p1.x, p2.y - p1.y);
		Vector4 line_bottom = Vector4(p3.x - p4.x, p3.y - p4.y);
		Vector4 line_left = Vector4(p4.x - p1.x, p4.y - p1.y);
		Vector4 line_right = Vector4(p3.x - p2.x, p3.y - p2.y);
		// �J�������W����}�[�J�[�̊e�_�܂ł̃x�N�g���̌v�Z
		Vector4 left_top = Vector4(p1.x - cam_w / 2.0, p1.y - cam_h / 2.0, cam_l);
		Vector4 right_top = Vector4(p2.x - cam_w / 2.0, p2.y - cam_h / 2.0, cam_l);
		Vector4 right_bottom = Vector4(p3.x - cam_w / 2.0, p3.y - cam_h / 2.0, cam_l);
		Vector4 left_bottom = Vector4(p4.x - cam_w / 2.0, p4.y - cam_h / 2.0, cam_l);
		// �ӂƃJ�������W��ʂ�ʂ���ʏ�̃}�[�J�[�̊e�ӂŌv�Z
		Vector4 surface_top = Vector4::cross(line_top, left_top).normal();
		Vector4 surface_bottom = Vector4::cross(line_bottom, left_bottom).normal();
		Vector4 surface_left = Vector4::cross(line_left, left_top).normal();
		Vector4 surface_right = Vector4::cross(line_right, right_top).normal();
		// �㉺�̖ʂ̌���ƍ��E�̖ʂ̌�����v�Z
		Vector4 x_vec = Vector4::cross(surface_top, surface_bottom).normal();
		Vector4 z_vec = Vector4::cross(surface_left, surface_right).normal();
		Vector4 y_vec = Vector4::cross(x_vec, z_vec).normal() * -1.0;
		// �J�����̍��W
		Vector4 camera_position = Vector4(0, 0, -cam_l * 4);
		// �v���W�F�N�V�����s��̌v�Z
		Matrix4 projection = Matrix4(
			cam_l, 0, 0, 0,
			0, cam_l, 0, 0,
			0, 0, 1, 0,
			0, 0, 1, 0
		);
		// ��]�s��̌v�Z
		Matrix4 rotate_matrix = Matrix4(
			x_vec.x, y_vec.x, z_vec.x, 0,
			x_vec.y, y_vec.y, z_vec.y, 0,
			x_vec.z, y_vec.z, z_vec.z, 0,
			0, 0, 0, 1
		);
		// ���s�ړ��s��̌v�Z
		Matrix4 translate_matrix = Matrix4(
			1, 0, 0, -camera_position.x - (p1.x - cam_w / 2.0) * camera_position.z / cam_l,
			0, 1, 0, -camera_position.y - (p1.y - cam_h / 2.0) * camera_position.z / cam_l,
			0, 0, 1, -camera_position.z,
			0, 0, 0, 1
		);
		// view�s��̌v�Z
		Matrix4 view_matrix = Matrix4::cross(translate_matrix, rotate_matrix);

		// MVP�s��̌v�Z
		Matrix4 mvp = Matrix4::cross(projection, view_matrix);

		// ���f���r���[�s��̋t�s����v�Z
		r2 = view_matrix.getInvert();

		// �}�[�J�[��Ԃł̃J�������W�̌v�Z
		m_cam_pos = Vector4::cross(r2, Vector4(0, 0, 0));
		// �}�[�J�[��Ԃł̃J�����̍����̌v�Z
		m_cam_h = m_cam_pos.y;
	}
	ScreenToGround::ScreenToGround() :
		cam_w(0.0), cam_h(0.0), cam_h_fov(0.0), cam_pos_h(0.0),
		cam_l(0.0), cam_w_fov(0.0), r2(), m_cam_pos(), m_cam_h(0.0)
	{}
	ScreenToGround::~ScreenToGround() {}
	void ScreenToGround::setParams(
		double cam_w_tmp, double cam_h_tmp, double cam_h_fov_tmp, double cam_pos_h_tmp,
		const Vector4& p1_tmp, const Vector4& p2_tmp, const Vector4& p3_tmp, const Vector4& p4_tmp
	)
	{
		// �p�����[�^�ɕύX���Ȃ���Όv�Z���s��Ȃ��悤�ɂ���
		if (
			(cam_w != cam_w_tmp) ||
			(cam_h != cam_h_tmp) ||
			(cam_h_fov != cam_h_fov_tmp) ||
			(cam_pos_h != cam_pos_h_tmp) ||
			(p1 != p1_tmp) ||
			(p2 != p2_tmp) ||
			(p3 != p3_tmp) ||
			(p4 != p4_tmp)
			)
		{
			cam_w = cam_w_tmp;
			cam_h = cam_h_tmp;
			cam_h_fov = cam_h_fov_tmp;
			cam_pos_h = cam_pos_h_tmp;
			p1 = p1_tmp; p2 = p2_tmp; p3 = p3_tmp; p4 = p4_tmp;
			calcParams();
		}
	}
	void ScreenToGround::setParams(
		double cam_w_tmp, double cam_h_tmp, double cam_h_fov_tmp, double cam_pos_h_tmp,
		double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4
	)
	{
		setParams(
			cam_w_tmp, cam_h_tmp, cam_h_fov_tmp, cam_pos_h_tmp,
			Vector4(x1, y1), Vector4(x2, y2), Vector4(x3, y3), Vector4(x4, y4)
		);
	}
	Vector4 ScreenToGround::translate(Vector4 p)
	{
		// ��ʏ�̍��Wp���}�[�J�[���W�ɕϊ�
		Vector4 m_p_pos = Vector4::cross(r2, Vector4(p.x - cam_w / 2.0, p.y - cam_h / 2.0, cam_l));
		// m_cam_pos ���� m_p_pos �ւ̃x�N�g��
		Vector4 mmcvec = m_p_pos - m_cam_pos;
		// (m_cam_pos + m_p_pos * ___).y = 0 �ƂȂ�___��������
		double k = m_cam_pos.y / mmcvec.y;
		// �}�[�J�[���ʂł̍��Wp�̌v�Z
		Vector4 p_p_pos = m_cam_pos - (mmcvec * k);
		// �}�[�J�[���ʂł̍��Wp�����[�g���P�ʂɕϊ�
		Vector4 p_p_pos_m = p_p_pos * cam_pos_h / m_cam_h;
		return Vector4(p_p_pos_m.x, p_p_pos_m.z);
	}
	void ScreenToGround::drawAreaLine(cv::Mat& mat)
	{
		cv::circle(mat, cv::Point(p1.x, p1.y), 5, cv::Scalar(255, 0, 0), -1);
		cv::circle(mat, cv::Point(p2.x, p2.y), 5, cv::Scalar(255, 0, 0), -1);
		cv::circle(mat, cv::Point(p3.x, p3.y), 5, cv::Scalar(255, 0, 0), -1);
		cv::circle(mat, cv::Point(p4.x, p4.y), 5, cv::Scalar(255, 0, 0), -1);
		cv::line(mat, { (int)p1.x, (int)p1.y }, { (int)p2.x, (int)p2.y }, cv::Scalar{ 0, 0, 255 }, 2.0);
		cv::line(mat, { (int)p2.x, (int)p2.y }, { (int)p3.x, (int)p3.y }, cv::Scalar{ 0, 0, 255 }, 2.0);
		cv::line(mat, { (int)p3.x, (int)p3.y }, { (int)p4.x, (int)p4.y }, cv::Scalar{ 0, 0, 255 }, 2.0);
		cv::line(mat, { (int)p4.x, (int)p4.y }, { (int)p1.x, (int)p1.y }, cv::Scalar{ 0, 0, 255 }, 2.0);
	}
	cv::Mat ScreenToGround::perspective(const cv::Mat& src, float zoom)
	{
		// �w�肵��4�_�̃X�N���[�����W�ƌ����̍��W
		std::vector<cv::Point2f> srcPoint{ p1, p2, p3, p4 };
		std::vector<cv::Point2f> dstPoint;

		// �w�肵��4�_�̌����̍��W��dstPoint�ɑ�����A������x���Ay���̍ő�l�ƍŏ��l���擾
		cv::Point2f min, max;
		for (int i = 0; i < srcPoint.size(); i++)
		{
			auto p = translate(srcPoint[i]);
			dstPoint.push_back(p);
			if (i == 0) { min = p; max = p; }
			else
			{
				min.x = (p.x < min.x) ? p.x : min.x;
				min.y = (p.y < min.y) ? p.y : min.y;
				max.x = (p.x > max.x) ? p.x : max.x;
				max.y = (p.y > max.y) ? p.y : max.y;
			}
		}

		// �������W�̒l����ʂɒ��x���܂�悤�Ɋg��k���A�ړ�
		cv::Point2f size{ max.x - min.x, max.y - min.y };
		for (auto&& p : dstPoint) { p.x -= min.x; p.y -= min.y; }
		float rate = (size.x / size.y) / ((float)src.cols, (float)src.rows);
		float scale = (rate > 1.0f) ? ((float)src.cols / size.x) : ((float)src.rows / size.y);
		size.x *= scale; size.y *= scale;
		cv::Point2f move = (rate > 1.0f) ? cv::Point2f{ 0.0f, ((float)src.rows - size.y) / 2.0f } : cv::Point2f{ ((float)src.cols - size.x) / 2.0f, 0.0f };
		for (auto&& p : dstPoint) { p.x *= scale; p.y *= scale; p.x += move.x; p.y += move.y; }

		// �g��k��
		for (auto&& p : dstPoint)
		{
			p.x -= (float)src.cols / 2.0f; p.y -= (float)src.rows / 2.0f;
			p.x *= zoom; p.y *= zoom;
			p.x += (float)src.cols / 2.0f; p.y += (float)src.rows / 2.0f;
		}

		//�ϊ��s��쐬
		cv::Mat r_mat = cv::getPerspectiveTransform(srcPoint, dstPoint);

		//�}�`�ϊ�����
		cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, src.type());
		cv::warpPerspective(src, dst, r_mat, dst.size(), cv::INTER_LINEAR);

		return dst;
	}
};
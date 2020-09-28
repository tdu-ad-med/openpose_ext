#pragma once

#include <Utils/Vector.h>

namespace vt
{
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
		Vector4 p1_tmp, Vector4 p2_tmp, Vector4 p3_tmp, Vector4 p4_tmp
	)
	{
		// ���჌���Y�̘c�ݕ␳���l������������p���v�Z
		cam_h_fov_tmp = fisheyeToFlat.calcCamHFov(cam_h_fov_tmp, cam_w_tmp, cam_h_tmp);
		p1_tmp = fisheyeToFlat.translate(p1_tmp, cam_w_tmp, cam_h_tmp);
		p2_tmp = fisheyeToFlat.translate(p2_tmp, cam_w_tmp, cam_h_tmp);
		p3_tmp = fisheyeToFlat.translate(p3_tmp, cam_w_tmp, cam_h_tmp);
		p4_tmp = fisheyeToFlat.translate(p4_tmp, cam_w_tmp, cam_h_tmp);
		
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
	void ScreenToGround::setCalibration(
		double cam_width, double cam_heigth, double output_scale,
		double fx, double fy, double cx, double cy, double k1, double k2, double k3, double k4
	) {
		fisheyeToFlat.setParams(
			cam_width, cam_heigth, output_scale, fx, fy, cx, cy, k1, k2, k3, k4
		);
	}
	Vector4 ScreenToGround::translate(Vector4 p, bool to_flat)
	{
		if (to_flat) p = fisheyeToFlat.translate(p, cam_w, cam_h);
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
	cv::Mat ScreenToGround::translateMat(const cv::Mat& src, float zoom)
	{
		// �w�肵��4�_�̃X�N���[�����W�ƌ����̍��W
		std::vector<cv::Point2f> srcPoint{ p1, p2, p3, p4 };
		std::vector<cv::Point2f> dstPoint;

		// �w�肵��4�_�̌����̍��W��dstPoint�ɑ�����A������x���Ay���̍ő�l�ƍŏ��l���擾
		cv::Point2f min(FLT_MAX, FLT_MAX), max(FLT_MIN, FLT_MIN);
		for (int i = 0; i < srcPoint.size(); i++)
		{
			const cv::Point2f p = translate(srcPoint[i], false);
			dstPoint.push_back(p);
			min.x = (p.x < min.x) ? p.x : min.x;
			min.y = (p.y < min.y) ? p.y : min.y;
			max.x = (p.x > max.x) ? p.x : max.x;
			max.y = (p.y > max.y) ? p.y : max.y;
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
		cv::Mat dst1 = fisheyeToFlat.translateMat(src);
		cv::Mat dst2 = cv::Mat::zeros(src.rows, src.cols, src.type());
		cv::warpPerspective(dst1, dst2, r_mat, dst2.size(), cv::INTER_LINEAR);

		return dst2;
	}
	Vector4 ScreenToGround::onlyFlat(Vector4 p) {
		return fisheyeToFlat.translate(p, cam_w, cam_h);
	}
	cv::Mat ScreenToGround::onlyFlatMat(const cv::Mat& src) {
		return fisheyeToFlat.translateMat(src);
	}
};
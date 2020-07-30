#include <cmath>
#include<iostream>
#include <limits>
#include <cstdlib>
#include "camera.h"

#define inf 1e4
using namespace std;
fstream f("txt_out.txt", ios::out);


camera::camera(void) {
	eye_pos << 0, 0, 3, 1;
	center << 0, 0, 0, 1;
	//eye_direction << 0-eye_pos(0), 0 - eye_pos(1), 0 - eye_pos(2), 0;
	up << 0, 1, 0;
	eye_direction.normalize();
	up.normalize();
}

void camera::lookat() {//����任���˲����Ƶ������ʼ�
	//Vector4d center3 = center / center(3);
	Vector3d minus_g = (eye_pos-center).normalized().head(3);
	Vector3d gxt = up.cross(minus_g).normalized();//������ʼ�տ������ģ�������up=(0,0,1)�Ϳ������3��������ע���˷���
	Vector3d t = minus_g.cross(gxt).normalized();
	Vector3d eye_pos3 = eye_pos.head(3);
	//cout << t <<endl<<minus_g<<endl<<gxt<< endl;
	Matrix4d R = MatrixXd::Identity(4, 4);
	Matrix4d T = MatrixXd::Identity(4, 4);
	//cout << R << endl << T << endl << ModelView << endl;
	for (int i = 0; i < 3; i++) {
		R(0, i) = gxt(i);
		R(1, i) = t(i);
		R(2, i) = minus_g(i);
		T(i, 3) = -eye_pos3(i);
	}
	ModelView = R * T;
	//	cout << R << endl << T << endl << ModelView << endl;
}

void camera::viewtrans(int x, int y, int width, int height, int depth=2) {//�ӿڱ任��ӳ�䵽��Ļ�ռ�
	ViewPort(0, 0) = width / 2.f;
	ViewPort(0, 3) = x + width / 2.f;
	ViewPort(1, 1) = height / 2.f;
	ViewPort(1, 3) = y + height / 2.f;
	ViewPort(2, 2) = depth / 2.f;
	ViewPort(2, 3) = depth / 2.f;
}

void camera::viewproject1(double n,double f) {//͸�ӱ任��ֻ����Զ��������
	//Persp2ortho(0, 0) = -n;
	//Persp2ortho(1, 1) = -n;
	//Persp2ortho(2, 2) = n + f;
	//Persp2ortho(2, 3) = -n*f;
	//Persp2ortho(3, 2) = 1;
	double len = (eye_pos - center).norm();
	if (len != 0) {
		Persp2ortho(0, 0) = 3.f / len;
		Persp2ortho(1, 1) = 3.f / len;
	}
	else {
		Persp2ortho(0, 0) = 0;
		Persp2ortho(1, 1) = 0;
		Persp2ortho(2, 2) = 0;
	}
}

void camera::viewproject2(double r, double l, double t, double b, double nn, double ff) {//�����ж����Ƶ�-1��1���������У�δʹ��
	//double r = *max_element(x_prime.begin(), x_prime.end());
	//double l = *min_element(x_prime.begin(), x_prime.end());
	//double t = *max_element(y_prime.begin(), y_prime.end());
	//double b = *min_element(y_prime.begin(), y_prime.end());
	//double nn = *max_element(z_prime.begin(), z_prime.end());
	//double ff = *min_element(z_prime.begin(), z_prime.end());
	Ortho(0, 0) = (r != l) ? 2 / (r - l) : 0;//Ϊƽ��ֱ�ӷŵ�����
	Ortho(0, 3) = (r != l) ? -(r + l) / (r - l) : 0;
	Ortho(1, 1) = (t != b) ? 2 / (t - b) : 0;
	Ortho(1, 3) = (t != b) ? -(t + b) / (t - b) : 0;
	Ortho(2, 2) = (nn != ff) ? 2 / (nn - ff) : 0;
	Ortho(2, 3) = (nn != ff) ? -(nn + ff) / (nn - ff) : 0;
}

void camera::vertex_change(Model *model, vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace,int width,int height) {
	AllFace.clear();
	lookat();
	viewproject1(1,10);
	for (int i = 0; i < model->nfaces(); i++) {
		//cout << i << endl;
		
		Matrix3d o_vt = MatrixXd::Zero(3, 3);
		for (int j = 0; j < 3; j++) {//�����޳������ж��淨�ߺ����߼н��Ƿ�С��90�ȣ�������˵�����濴����ֱ������
			Vec3f worldc = model->vert(i, j);
			o_vt.col(j) << worldc.x, worldc.y, worldc.z;
		}
		Vector3d face_norm = (o_vt.col(0) - o_vt.col(1)).cross(o_vt.col(0) - o_vt.col(2)).normalized();
		Vector3d eye_dir = (eye_pos - center).normalized().head(3);
		if (face_norm.dot(eye_dir) <= 0)
			continue;
		
		FaceInfo nowface;
		nowface.nface = i;
		bool flag = true;
		for (int j = 0; j < 3; j++) {
			Vec3f vt=model->vert(i, j);	
			Vector4d hc_vt;
			hc_vt << vt.x ,vt.y , vt.z , 1;
			hc_vt = ModelView * hc_vt;//�任���������ϵ��
			if (hc_vt(2) >= 0) {
				flag = false;
				break;
			}//�����������������ĵ�(z>0)����ô�Ͳ����������
			hc_vt = Persp2ortho*hc_vt;//͸�ӱ任
			hc_vt = hc_vt / hc_vt(3);//��������׼��
			nowface.vt[j] = hc_vt;
			nowface.uv[j] = model->uv(i, j);
			//nowface.uv[j] << model->uv(i, j).x, model->uv(i, j).y;
			Vec3f normal = model->normal(i, j);//����Ҳ��䣬һ����ٸ�	
			nowface.normal[j] << normal.x, normal.y, normal.z, 0;
			
		}
		//cout << flag << endl;
		if (flag) {
			AllFace.push_back(nowface);	//�����������������������
			/*
			for (int j = 0; j < 3; j++) {		
				//cout << nowface.vt[j] << endl;
				if (nowface.vt[j](0) < l)
					l = nowface.vt[j](0);
				else if (nowface.vt[j](0) > r)
					r = nowface.vt[j](0);
				if (nowface.vt[j](1) < b)
					b = nowface.vt[j](1);
				else if (nowface.vt[j](1) > t)
					t = nowface.vt[j](1);
				if (nowface.vt[j](2) < ff)
					ff = nowface.vt[j](2);
				else if (nowface.vt[j](2) > nn)
					nn = nowface.vt[j](2);
			}
			*/
		}
	}
	
	//viewproject2(r, l, t, b, nn, ff);
	viewtrans(width/8, height/8, 3*width/4, 3*height/4, 255);
	//Matrix4d n_trans = (ViewPort * Persp2ortho*ModelView);
	Matrix4d n_trans = (ViewPort * Persp2ortho*ModelView).inverse().transpose();
	for (int i = 0; i < AllFace.size(); i++) {//�����ӿڱ任��Ķ������ꡢ������
		for (int j = 0; j < 3; j++) {
			AllFace[i].vt[j] = ViewPort*AllFace[i].vt[j];
			AllFace[i].normal[j] = (n_trans *AllFace[i].normal[j]).normalized();
		}
	}
}

void camera::cam_change(int zoom , int rot1 , int rot2) {
	if (!zoom&&!rot1&&!rot2)
		return;
	if (zoom == 1) {
		if ((center - eye_pos).norm() >= 1+0.25) {
			Vector4d dir = (center - eye_pos).normalized();
			eye_pos = eye_pos + 0.25*dir;//������ƽ��0.1����λ��������
		}
		return;
	}
	else if (zoom == 2) {
		if ((center - eye_pos).norm() <= 8-0.25) {
			Vector4d dir = (center - eye_pos).normalized();
			eye_pos = eye_pos - 0.25*dir;//����ƽ��0.1����λ��������
		}
		return;
	}
	Matrix4d R1 = MatrixXd::Identity(4, 4), R2 = MatrixXd::Identity(4, 4);
	if (rot1) {//�����y����ת
		//Vector4d v = eye_pos - center;
		double alpha = M_PI / 10;
		if (rot1 == 2)
			alpha = -alpha;
		R1(0, 0) = cos(alpha); R1(0, 2) = sin(alpha); R1(2, 0) = -sin(alpha); R1(2, 2) = cos(alpha);
		//R1 = T2 * R1*T1;
	}
	if (rot2) {//��������Ĵ�ֱxzƽ����ת����ת��z�ᣬ����yz����ת������ٴ�z��ת�أ�
		double alpha = -M_PI / 10;
		Matrix4d T1 = Matrix4d::Identity(4, 4),T2= Matrix4d::Identity(4, 4);
		//f << eye_pos(0) / sqrt(pow(eye_pos(0), 2) + pow(eye_pos(2), 2)) << endl;
		double cosy= sqrt(pow(eye_pos(0), 2) + pow(eye_pos(2), 2))/ sqrt(pow(eye_pos(0), 2) + pow(eye_pos(1), 2) + pow(eye_pos(2), 2));//��y��нǣ�̫С�Ͳ���ת��
		bool flag = true;
		if (rot2 == 1) {
			if (eye_pos(1) > 0 && cosy <= cos(0.95*M_PI / 2 + alpha))
				flag = false;
		}
		else {
			alpha = -alpha;
			if (eye_pos(1) < 0 && cosy <= cos(0.95*M_PI / 2 - alpha))
				flag = false;
		}
		if (flag) {
			double beta = asin(eye_pos(0) / sqrt(pow(eye_pos(0), 2) + pow(eye_pos(2), 2)));
			beta = -beta;
			f << beta << endl << endl;
			if (eye_pos(2) < 0 && eye_pos(0) >= 0)
				beta = M_PI - beta;
			else if (eye_pos(2) < 0 && eye_pos(0) < 0)
				beta = -(M_PI + beta);
			T1(0, 0) = cos(beta); T1(0, 2) = sin(beta); T1(2, 0) = -sin(beta); T1(2, 2) = cos(beta);

			T2(1, 1) = cos(alpha); T2(1, 2) = -sin(alpha); T2(2, 1) = sin(alpha); T2(2, 2) = cos(alpha);
			R2 = T1.transpose()*T2*T1;
		}	
	}
	//f << R2 << endl << endl;
	eye_pos = R2*R1*eye_pos;
}
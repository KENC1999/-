#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "tgaimage.h"
#include"model.h"
#include "geometry.h"
#include"MyShader.h"
#include<iostream>
#include<graphics.h>
#include<vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
using namespace Eigen;
using namespace std;
/*����࣬ʵ�ֶ��㡢����������任���������*/

class camera {
protected:
	Vector4d eye_pos;//���λ��
	Vector4d eye_direction = VectorXd::Zero(4, 1);//������߷��򣨲�δʹ�õ���ֱ��������ָ�����Ĵ��棩
	Vector4d center = VectorXd::Zero(4, 1);
	Vector3d up;//����Ϸ���
	Matrix4d ViewPort = MatrixXd::Identity(4, 4);//�ӿڱ任����
	Matrix4d Persp2ortho = MatrixXd::Identity(4, 4);//͸�ӱ任���󣬿��ǵ�͸�ӵĹ��ڳ������ֻ���˽���ԶС������
	Matrix4d Ortho = MatrixXd::Identity(4, 4);//δʹ��
	Matrix4d ModelView = MatrixXd::Identity(4, 4);//����任����

	void lookat();//����任
	void viewtrans(int x, int y, int width, int height, int depth);//�ӿڱ任
	void viewproject1(double n = 1, double f = 10);//͸�ӱ任(��ѹ)
	void viewproject2(double r, double l, double t, double b, double nn, double ff);//͸�ӱ任(����)��δʹ��

public:
	camera();	
	void vertex_change(Model *model, vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace, int width, int height);//����任
	void cam_change(int zoom=0,int rot1=0,int rot2=0);
	Vector4d get_cam_prime() {//���ر任��Ĺ�����������ɫ��
		//return (ViewPort*Persp2ortho*ModelView)*eye_pos;
		return (ViewPort*Persp2ortho*ModelView).inverse().transpose()*eye_pos;
		//return eye_pos;
	}
	Vector4d get_cam() {
		return eye_pos;
	}
};

#endif // !
#pragma once

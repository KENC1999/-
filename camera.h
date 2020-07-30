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
/*相机类，实现顶点、法向量坐标变换和相机操作*/

class camera {
protected:
	Vector4d eye_pos;//相机位置
	Vector4d eye_direction = VectorXd::Zero(4, 1);//相机视线方向（并未使用到，直接令视线指向中心代替）
	Vector4d center = VectorXd::Zero(4, 1);
	Vector3d up;//相机上方向
	Matrix4d ViewPort = MatrixXd::Identity(4, 4);//视口变换矩阵
	Matrix4d Persp2ortho = MatrixXd::Identity(4, 4);//透视变换矩阵，考虑到透视的过于抽象，最后只做了近大远小的缩放
	Matrix4d Ortho = MatrixXd::Identity(4, 4);//未使用
	Matrix4d ModelView = MatrixXd::Identity(4, 4);//相机变换矩阵

	void lookat();//相机变换
	void viewtrans(int x, int y, int width, int height, int depth);//视口变换
	void viewproject1(double n = 1, double f = 10);//透视变换(挤压)
	void viewproject2(double r, double l, double t, double b, double nn, double ff);//透视变换(正交)，未使用

public:
	camera();	
	void vertex_change(Model *model, vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace, int width, int height);//顶点变换
	void cam_change(int zoom=0,int rot1=0,int rot2=0);
	Vector4d get_cam_prime() {//返回变换后的光线向量给着色器
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

#ifndef __MyShader_H__
#define __MyShader_H__
#include "tgaimage.h"
#include "geometry.h"
#include"model.h"
#include<iostream>
#include<graphics.h>
#include<conio.h>
#include<vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
using namespace Eigen;
using namespace std;
/*MyShader中实现了片元着色和光栅化的功能*/
struct FaceInfo{
	int nface;
	Vector4d vt[3];
	Vector4d normal[3];
	Vec2f uv[3];
};//FaceInfo存储需要绘制的面的信息，包括：面序号、顶点坐标（变换后）、顶点法向量（变换后）、纹理坐标

struct Frame_Buffer {
	int x;
	int y;
	Vector3d color;
};//帧缓存，光栅化时用于读取像素位置及像素颜色，代码里没用这个，直接用数组做帧缓存了

class MyShader {//Shader每次处理一个三角形面时，将读取该面的信息（顶点、法向量、纹理坐标等）到类中
protected:
	Matrix3d vt = MatrixXd::Zero(3, 3);//3个顶点，每一列为一个顶点的数据
	Vec2f uv[3];
	Matrix3d normal = MatrixXd::Zero(3, 3);
	Vector3d g_color;
	Vector3d color;
	Vector3d lit_dir;
	Vector3d lit_dir_prime;
	Vector3d lit_pos;
	int faceid;
	virtual bool fragment(Vector3d bc, Model *model,bool tmap,bool nmap) = 0;//片元着色为虚函数，继承时重写

	void triangle(int width, int height, float* zbuffer, Model *model,TGAImage &image, \
		vector<Frame_Buffer, aligned_allocator<Frame_Buffer>>& FBuffer, Vector3d* fbuffer, bool tmap, bool nmap);
	Vector3d barycentric(Vector3d p);//计算给定位置的三角形质心坐标
	void set(vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace, int index);//更新Shader数据
public:
	
	MyShader();
	// ~MyShader();
	void draw(int width, int height, float* zbuffer, vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace,Model *model,TGAImage &image,\
		vector<Frame_Buffer, aligned_allocator<Frame_Buffer>>& FBuffer, Vector3d* fbuffer, DWORD* pMem, bool tmap, bool nmap);//光栅化
	//virtual Vec4f vertex(int iface, int nthvert) = 0;
	void change_lit(Vector3d new_lit) {//从相机中读取变换后的光线向量
		lit_dir = new_lit;
	}
	void change_lit_prime(Vector3d new_lit) {
		lit_dir_prime = new_lit;
	}

};

class FlatShader :public MyShader {//平面着色，计算三角形两边的外积作为法向量
	virtual bool fragment(Vector3d bc, Model *model, bool tmap, bool nmap) {
		Matrix3d o_vt = MatrixXd::Zero(3, 3);
		for (int i = 0; i < 3; i++) {
			Vec3f worldc = model->vert(faceid, i);
			o_vt.col(i) << worldc.x, worldc.y, worldc.z;
		}

		Vector3d face_norm = (o_vt.col(0) - o_vt.col(1)).cross(o_vt.col(0) - o_vt.col(2));
		face_norm.normalize();
		float intensity = face_norm.dot(lit_dir);//求光线和法线的内积
		if (intensity > 1e-2) {
		color= intensity*g_color;
		return false;
		}
		return true;
	}
};

class G_Shader :public MyShader {
	virtual bool fragment(Vector3d bc, Model *model, bool tmap, bool nmap);
};

class BP_Shader :public MyShader {
	virtual bool fragment(Vector3d bc, Model *model, bool tmap, bool nmap);
};


#endif
#pragma once

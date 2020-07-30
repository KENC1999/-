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
/*MyShader��ʵ����ƬԪ��ɫ�͹�դ���Ĺ���*/
struct FaceInfo{
	int nface;
	Vector4d vt[3];
	Vector4d normal[3];
	Vec2f uv[3];
};//FaceInfo�洢��Ҫ���Ƶ������Ϣ������������š��������꣨�任�󣩡����㷨�������任�󣩡���������

struct Frame_Buffer {
	int x;
	int y;
	Vector3d color;
};//֡���棬��դ��ʱ���ڶ�ȡ����λ�ü�������ɫ��������û�������ֱ����������֡������

class MyShader {//Shaderÿ�δ���һ����������ʱ������ȡ�������Ϣ�����㡢����������������ȣ�������
protected:
	Matrix3d vt = MatrixXd::Zero(3, 3);//3�����㣬ÿһ��Ϊһ�����������
	Vec2f uv[3];
	Matrix3d normal = MatrixXd::Zero(3, 3);
	Vector3d g_color;
	Vector3d color;
	Vector3d lit_dir;
	Vector3d lit_dir_prime;
	Vector3d lit_pos;
	int faceid;
	virtual bool fragment(Vector3d bc, Model *model,bool tmap,bool nmap) = 0;//ƬԪ��ɫΪ�麯�����̳�ʱ��д

	void triangle(int width, int height, float* zbuffer, Model *model,TGAImage &image, \
		vector<Frame_Buffer, aligned_allocator<Frame_Buffer>>& FBuffer, Vector3d* fbuffer, bool tmap, bool nmap);
	Vector3d barycentric(Vector3d p);//�������λ�õ���������������
	void set(vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace, int index);//����Shader����
public:
	
	MyShader();
	// ~MyShader();
	void draw(int width, int height, float* zbuffer, vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace,Model *model,TGAImage &image,\
		vector<Frame_Buffer, aligned_allocator<Frame_Buffer>>& FBuffer, Vector3d* fbuffer, DWORD* pMem, bool tmap, bool nmap);//��դ��
	//virtual Vec4f vertex(int iface, int nthvert) = 0;
	void change_lit(Vector3d new_lit) {//������ж�ȡ�任��Ĺ�������
		lit_dir = new_lit;
	}
	void change_lit_prime(Vector3d new_lit) {
		lit_dir_prime = new_lit;
	}

};

class FlatShader :public MyShader {//ƽ����ɫ���������������ߵ������Ϊ������
	virtual bool fragment(Vector3d bc, Model *model, bool tmap, bool nmap) {
		Matrix3d o_vt = MatrixXd::Zero(3, 3);
		for (int i = 0; i < 3; i++) {
			Vec3f worldc = model->vert(faceid, i);
			o_vt.col(i) << worldc.x, worldc.y, worldc.z;
		}

		Vector3d face_norm = (o_vt.col(0) - o_vt.col(1)).cross(o_vt.col(0) - o_vt.col(2));
		face_norm.normalize();
		float intensity = face_norm.dot(lit_dir);//����ߺͷ��ߵ��ڻ�
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

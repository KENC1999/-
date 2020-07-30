#include"MyShader.h"
#include "tgaimage.h"
#include"model.h"
#include "geometry.h"
#include<iostream>
#include<graphics.h>
#include<vector>
#include <Eigen/Dense>
#define inf 1e4
using namespace Eigen;
using namespace std;
fstream f2("txt_out.txt", ios::out);//debug用

MyShader::MyShader() {//初始化
   lit_dir << 0, 1, 1;
   lit_dir_prime << 0, 1, 1;
   lit_pos << 1, 1, 1;
   g_color << 255, 255, 255;//不贴图默认白色
   color << 0, 0, 0;
   lit_dir.normalize();
   lit_dir_prime.normalize();
}

void MyShader::set(vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace, int index) {
	for (int i = 0; i < 3; i++) {
		uv[i] = AllFace[index].uv[i];
		vt.col(i) = AllFace[index].vt[i].head(3);
		normal.col(i) = AllFace[index].normal[i].head(3);
		faceid = AllFace[index].nface;
	}
	//face_norm = (vt.col(0) - vt.col(1)).cross(vt.col(0) - vt.col(2));
	//float intensity = face_norm.dot(lit_dir);
	//if (intensity > 1e-2) {
	//	color= intensity*g_color;
	//}
}
Vector3d MyShader::barycentric(Vector3d p) {//计算质心坐标，推导见笔记
	Vector3d V[2],AB,AC,PA;
	AB = vt.col(1) - vt.col(0);
	AC = vt.col(2) - vt.col(0);
	PA = vt.col(0) - p;
	for (int i = 0; i < 2; i++) {
		V[i](0) = AB(i);
		V[i](1) = AC(i);
		V[i](2) = PA(i);
	}
	Vector3d u = V[0].cross(V[1]);
	Vector3d bc;
	if (abs(u(2)) < 1) {
		bc << -1, 1, 1;
		return bc;
	}
	bc << 1.f - (u(0) + u(1)) / u(2), u(0) / u(2), u(1) / u(2);
	return bc;
}

void MyShader::triangle(int width, int height, float* zbuffer, Model *model,TGAImage &image,\
	vector<Frame_Buffer, aligned_allocator<Frame_Buffer>>& FBuffer, Vector3d* fbuffer, bool tmap, bool nmap) {
	int xmin = inf, ymin = inf, xmax = -inf, ymax = inf;
	for (int i = 0; i < 3; i++) {//找当前三角形的包围盒
		xmin = max(0,min(xmin, vt(0, i)));
		xmax = min(width, max(xmax, vt(0, i)));
		ymin = max(0, min(ymin, vt(1, i)));
		ymax = min(height, max(ymax, vt(1, i)));
	}
	for (int x = xmin; x <= xmax; x++) {
		for (int y = ymin; y <= ymax; y++) {
			Vector3d bc = barycentric(Vector3d(x,y,1));
			float z = bc.dot(vt.row(2));
			if (bc(0) < 0 || bc(1) < 0 || bc(2) < 0 || zbuffer[x + y * width] > z)//点在三角形外或被遮挡
				continue;
			bool discard = fragment(bc, model,tmap,nmap);//着色，设置color的值
			if (!discard) {
				zbuffer[x + y * width] = z;
				fbuffer[x + y * width] = color;
				//image.set(x, y, TGAColor(color(0), color(1), color(2),255));
				//Frame_Buffer F;
				//F.color = color;
				//F.x = x;
				//F.y = height - y;
				//FBuffer.push_back(F);
			}
		}
	}
}

void MyShader::draw(int width,int height, float* zbuffer, vector<FaceInfo, aligned_allocator<FaceInfo>>& AllFace, Model *model, TGAImage &image,\
	vector<Frame_Buffer, aligned_allocator<Frame_Buffer>>& FBuffer, Vector3d* fbuffer, DWORD* pMem, bool tmap, bool nmap){
	for (int i = 0; i < AllFace.size(); i++) {
		//cout << i << endl;
		set(AllFace, i);
		triangle(width, height, zbuffer, model,image,FBuffer,fbuffer,tmap,nmap);//先计算着色存到帧缓存中
	}
	/*
	for (int i = 0; i < FBuffer.size(); i++) {
		Frame_Buffer Fn = FBuffer[i];
		putpixel(Fn.x, Fn.y, RGB(Fn.color(0), Fn.color(1), Fn.color(2)));
	}*/
	cleardevice();//进行光栅化
	for (int x = 0; x < width; x++)
		for(int y=0;y<height;y++)
			pMem[x+y*width]= BGR(RGB(fbuffer[x + (height-y) * width](0), fbuffer[x + (height - y) * width](1), fbuffer[x + (height - y) * width](2)));
	FlushBatchDraw();

}

bool G_Shader::fragment(Vector3d bc, Model *model, bool tmap, bool nmap) {//高洛德Shader类，重写fragment
	g_color << 255, 255, 255;
	Vector3d face_norm = normal * bc;//插值计算法线
	face_norm.normalize();
	Vec2f uv_p = uv[0] * bc(0) + uv[1] * bc(1) + uv[2] * bc(2);//插值计算纹理坐标
	Vector3d now_lit = lit_dir_prime;
	if (tmap) {//需要纹理贴图就用贴图里的颜色，否则为白色
		TGAColor color_p = model->diffuse(uv_p);
		g_color(0) = color_p.bgra[2];//rgb分配
		g_color(1) = color_p.bgra[1];
		g_color(2) = color_p.bgra[0];
	}
	if (nmap) {//法线贴图，涉及到切线空间的坐标变换，详见笔记
		/*
		MatrixXd uv_M(3, 2), Q_M(3, 3);
		uv_M(0, 0) = uv[1].x - uv[0].x; uv_M(1, 0) = uv[2].x - uv[0].x; uv_M(2, 0) = 0;
		uv_M(0, 1) = uv[1].y - uv[0].y; uv_M(1, 1) = uv[2].y - uv[0].y; uv_M(2, 1) = 0;
		Q_M.row(0) = vt.col(1) - vt.col(0);
		Q_M.row(1) = vt.col(2) - vt.col(0);
		Q_M.row(2) = face_norm;		
		MatrixXd Q_M_inv=Q_M.inverse();
		Vector3d I = Q_M_inv * uv_M.col(0);
		Vector3d J = Q_M_inv * uv_M.col(1);
		MatrixXd TB(3, 3);
		TB.col(0) = I.normalized(); TB.col(1) = J.normalized(); TB.col(2) = face_norm;
		*/
		MatrixXd uv_M(2, 2), Q_M(2, 3);
		uv_M(0, 0) = uv[1].x - uv[0].x; uv_M(1, 0) = uv[2].x - uv[0].x; 
		uv_M(0, 1) = uv[1].y - uv[0].y; uv_M(1, 1) = uv[2].y - uv[0].y; 
		Q_M.row(0) = vt.col(1) - vt.col(0);
		Q_M.row(1) = vt.col(2) - vt.col(0);
		//Matrix2d uv_M_inv;
		//uv_M_inv(0, 0) = uv[2].y - uv[0].y; uv_M_inv(1, 0) = uv[0].y - uv[1].y;
		//uv_M_inv(0, 1) = uv[0].x - uv[2].x; uv_M_inv(1, 1) = uv[1].x - uv[0].x;
		//uv_M_inv = (1 / (uv_M(0, 0)*uv_M(1, 1) - uv_M(1, 0)*uv_M(0, 1)))*uv_M_inv;
		MatrixXd TB = (uv_M.inverse()*Q_M).normalized();
		MatrixXd TBN(3, 3);
		TBN.col(0) = TB.row(0);
		TBN.col(1) = TB.row(1);
		TBN.col(2) = face_norm;
		Vec3f normal0 = model->normal(uv_p);
		Vector3d map_normal;
		map_normal(0) = normal0.x;
		map_normal(1) = normal0.y;
		map_normal(2) = normal0.z;
		face_norm = (TBN * map_normal).normalized();
	}
	float intensity = face_norm.dot(now_lit);
	if (intensity > 1e-2) {
		color = intensity * g_color;
		return false;
	}
	return true;
}

bool BP_Shader::fragment(Vector3d bc, Model *model, bool tmap, bool nmap) {////BlinnPhong Shader类，重写fragment
	g_color << 255, 255, 255;
	Vector3d face_norm = normal * bc;
	face_norm.normalize();
	Vec2f uv_p = uv[0] * bc(0) + uv[1] * bc(1) + uv[2] * bc(2);
	Vector3d now_lit = lit_dir_prime.normalized();
	if (tmap) {
		TGAColor color_p = model->diffuse(uv_p);
		g_color(0) = color_p.bgra[2];//rgb分配
		g_color(1) = color_p.bgra[1];
		g_color(2) = color_p.bgra[0];
	}
	if (nmap) {
		/*
		MatrixXd uv_M(3, 2), Q_M(3, 3);
		uv_M(0, 0) = uv[1].x - uv[0].x; uv_M(1, 0) = uv[2].x - uv[0].x; uv_M(2, 0) = 0;
		uv_M(0, 1) = uv[1].y - uv[0].y; uv_M(1, 1) = uv[2].y - uv[0].y; uv_M(2, 1) = 0;
		Q_M.row(0) = vt.col(1) - vt.col(0);
		Q_M.row(1) = vt.col(2) - vt.col(0);
		Q_M.row(2) = face_norm;
		MatrixXd Q_M_inv=Q_M.inverse();
		Vector3d I = Q_M_inv * uv_M.col(0);
		Vector3d J = Q_M_inv * uv_M.col(1);
		MatrixXd TB(3, 3);
		TB.col(0) = I.normalized(); TB.col(1) = J.normalized(); TB.col(2) = face_norm;
		*/
		MatrixXd uv_M(2, 2), Q_M(2, 3);
		uv_M(0, 0) = uv[1].x - uv[0].x; uv_M(1, 0) = uv[2].x - uv[0].x;
		uv_M(0, 1) = uv[1].y - uv[0].y; uv_M(1, 1) = uv[2].y - uv[0].y;
		Q_M.row(0) = vt.col(1) - vt.col(0);
		Q_M.row(1) = vt.col(2) - vt.col(0);
		//Matrix2d uv_M_inv;
		//uv_M_inv(0, 0) = uv[2].y - uv[0].y; uv_M_inv(1, 0) = uv[0].y - uv[1].y;
		//uv_M_inv(0, 1) = uv[0].x - uv[2].x; uv_M_inv(1, 1) = uv[1].x - uv[0].x;
		//uv_M_inv = (1 / (uv_M(0, 0)*uv_M(1, 1) - uv_M(1, 0)*uv_M(0, 1)))*uv_M_inv;
		MatrixXd TB = (uv_M.inverse()*Q_M).normalized();
		MatrixXd TBN(3, 3);
		TBN.col(0) = TB.row(0);
		TBN.col(1) = TB.row(1);
		TBN.col(2) = face_norm;
		Vec3f normal0 = model->normal(uv_p);
		Vector3d map_normal;
		map_normal(0) = normal0.x;
		map_normal(1) = normal0.y;
		map_normal(2) = normal0.z;
		face_norm = (TBN * map_normal).normalized();
	}
	Vector3d r = (face_norm*(face_norm.dot(now_lit)*2.f) - now_lit).normalized(); // 反射光
	float spec = pow(max(r.dot(now_lit), 0.0f), model->specular(uv_p)); // 高光项
	float diff = max(0.f, face_norm.dot(now_lit));//漫反射项
	for (int i = 0; i < 3; i++) {
		color(i) = min(5 + g_color(i)*(diff + .6*spec), 255);//按BlinnPhong反射模型设置颜色
	}
	return false;
}


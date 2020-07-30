// InteractiveSoftwareRasterizer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<algorithm>
#include <graphics.h>
#include<conio.h>
#include<io.h>
#include<vector>
#include <direct.h>
#include"model.h"
#include"camera.h"
#include"MyShader.h"
#define MAX_PATH 80
#include <Eigen/Dense>
#include <Eigen/StdVector>
using namespace Eigen;
using namespace std;

char pbuf[MAX_PATH];
string name;

const int width = 600;
const int height = 600;

Model *model = NULL;
camera MyCam;
float* zbuffer = new float[(2*width + 2)*(2*height + 2)];
Vector3d* fbuffer = new Vector3d[(2 * width + 2)*(2 * height + 2)];
FlatShader FlatS;
G_Shader GS;
BP_Shader GBP;
MyShader* now_S;
vector<FaceInfo, aligned_allocator<FaceInfo>> AllFace;
vector<Frame_Buffer, aligned_allocator<Frame_Buffer>> FBuffer;
TGAImage image(width, height, TGAImage::RGB);
MOUSEMSG m_act;
//fstream f1("txt_out.txt", ios::out);
bool bmap = true, tmap = true,bp=true,hold=false;



void getFiles(string path, vector<string>& files);//读取目录下文件名
bool choose_model();//选择模型
void make_3d(DWORD* pMem,bool paint=true);//在屏幕上显示模型
bool get_action();//获取动作
std::vector<std::string> splitWithStl(const std::string &str, const std::string &pattern);//字符串分割
int main()
{	
	while (1) {
		choose_model();
		if (hold) {
			initgraph(width, height);
			DWORD* pMem = GetImageBuffer();
			bool paint = true;
			while (hold) {
				make_3d(pMem, paint);
				paint = get_action();
			}
			closegraph();
		}
	}
	return 0;
}
bool choose_model() {
	_getcwd(pbuf, sizeof(pbuf));
	cout << "Input obj's name:";
	cin >> name;
	name += ".obj";
	vector<string> files;
	string filePath = string(pbuf) + "\\obj";
	//获取该路径下的所有文件  
	getFiles(filePath, files);
	char str[30];
	int size = files.size();
	for (int i = 0; i < size; i++)
	{
		if (splitWithStl(files[i], "\\").back() == name) {
			cout << files[i] << endl;
			model = new Model(files[i].c_str());
			hold = true;
			return 1;
		}
		else if (i == size - 1) {
			cout << "Cannot find this obj, please try again." << endl;
			return 0;
		}
	}
}

void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	//long   hFile = 0;
	_int64 hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之,如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

std::vector<std::string> splitWithStl(const std::string &str, const std::string &pattern)
{
	std::vector<std::string> resVec;

	if ("" == str)
	{
		return resVec;
	}
	//方便截取最后一段数据
	std::string strs = str + pattern;

	size_t pos = strs.find(pattern);
	size_t size = strs.size();

	while (pos != std::string::npos)
	{
		std::string x = strs.substr(0, pos);
		resVec.push_back(x);
		strs = strs.substr(pos + 1, size);
		pos = strs.find(pattern);
	}

	return resVec;
}


void make_3d(DWORD* pMem,bool paint) {
	if (!paint)
		return;
	if (bp)
		now_S = &GBP;
	else
		now_S = &GS;
	now_S->change_lit(MyCam.get_cam().head(3).normalized());
	now_S->change_lit_prime(MyCam.get_cam_prime().head(3).normalized());
	fill(zbuffer, zbuffer + (2*width + 2)*(2*height + 2), -0x3f3f3f3f);
	fill(fbuffer, fbuffer + (2 * width + 2)*(2 * height + 2),Vector3d::Zero(3,1));
	FBuffer.clear();
	MyCam.vertex_change(model, AllFace, width, height);
	now_S->draw(width, height, zbuffer, AllFace, model, image,FBuffer,fbuffer,pMem,tmap,bmap);
	//image.flip_vertically();
	//image.write_tga_file("o5.tga");
	TCHAR wc[MAX_PATH]; string info;
	info = "mode: ";
	info.append(bp ? "Blinn-Phong":"Gouraud");
	_stprintf_s(wc, MAX_PATH, _T("%S"), info.c_str());
	outtextxy(10, 20, wc);
	info = "texture: ";
	info.append(tmap ? "on" : "off");
	_stprintf_s(wc, MAX_PATH, _T("%S"), info.c_str());
	outtextxy(10, 40, wc);
	info = "norm_map: ";
	info.append(bmap ? "on" : "off");
	_stprintf_s(wc, MAX_PATH, _T("%S"), info.c_str());
	outtextxy(10, 60, wc);
	//info = "mid->alter_mode  shift+left->on/off_texture  shift+right->on/off_norm_map";
	//_stprintf_s(wc, MAX_PATH, _T("%S"), info.c_str());
	//outtextxy(10, height-40, wc);
}

bool get_action() {//左键、CTRL左键、右键、CTRL右键旋转视角，滚轮缩放，SHIFT左键开关纹理，SHIFT右键开关法线贴图，中键选择着色模式，CTRL+SHIFT+中键退出
	FlushMouseMsgBuffer();
	m_act=GetMouseMsg();
	switch (m_act.uMsg)
	{
	case WM_MBUTTONDOWN:
		if (m_act.mkShift&&m_act.mkCtrl) {
			hold = false;
			return false;
		}
			bp = !bp;
			return true;
	case WM_MOUSEWHEEL:
		if (m_act.wheel > 0)
			MyCam.cam_change(1,0,0);
		else
			MyCam.cam_change(2,0,0);
		return true;
	case WM_LBUTTONDOWN:
		if (m_act.mkShift) {
			tmap = !tmap;
			return true;
		}
		else if (m_act.mkCtrl)
			MyCam.cam_change(0, 2, 0);
		else
			MyCam.cam_change(0, 1, 0);
		return true;
	case WM_RBUTTONDOWN:
		if (m_act.mkShift) {
			bmap = !bmap;
			return true;
		}
		else if (m_act.mkCtrl)
			MyCam.cam_change(0, 0, 2);
		else
			MyCam.cam_change(0, 0, 1);
		return true;
	default:
		break;
	}
	return false;
}
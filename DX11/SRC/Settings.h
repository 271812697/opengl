#pragma once
#include<string>
#include<iostream>
class Settings
{
public:
	//ѵ���Ľ��ģ��·��
	std::string resultmodelpath;
	//ѵ���Ľ��poi·��
	std::string resultpoipath;
	//��ע�����ŵ�·���� ��Ӧ�ڱ�ע����
	std::string markpoipath;
	//ԭ�������ݼ����˹���ע��ģ�ͺ���Ȥ������ѵ�����Եģ�·��
	std::string originpath;
	std::string correct_poi_path;


	void load();
	void save();
};


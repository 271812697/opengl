#pragma once
#include<string>
#include<iostream>
class Settings
{
public:
	//训练的结果模型路径
	std::string resultmodelpath;
	//训练的结果poi路径
	std::string resultpoipath;
	//标注结果存放的路径， 对应于标注功能
	std::string markpoipath;
	//原本的数据集（人工标注的模型和兴趣点用于训练测试的）路径
	std::string originpath;
	std::string correct_poi_path;


	void load();
	void save();
};


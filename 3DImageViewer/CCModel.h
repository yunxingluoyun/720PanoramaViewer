#pragma once
#include "stdafx.h"

//模型类
class CCModel
{
public:
	CCModel();
	~CCModel();

	//刷新模型向量
	void UpdateVectors();
	//获取模型变换矩阵
	glm::mat4 GetMatrix();
	//重置模型位置和旋转
	void Reset();

	//模型位置
	glm::vec3 Positon = glm::vec3(0.0f);
	//模型旋转
	glm::vec3 Rotation = glm::vec3(0.0f);

private:
	glm::vec3 Front = glm::vec3(0.0f);
	glm::vec3 Right = glm::vec3(0.0f);
	glm::vec3 Up = glm::vec3(0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};


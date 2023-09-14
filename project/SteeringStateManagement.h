#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"

class SteeringStateManagement
{
public:

	SteeringStateManagement(IExamInterface* pExamInterFace, SteeringPlugin_Output* pSteering);
	~SteeringStateManagement() = default;


	void Seek(const Elite::Vector2 target);
	void FaceAndSeek(const Elite::Vector2 target);
	void Flee(const Elite::Vector2 target);
	
	void Wander();
	void Sprint(bool sprintActive);
	bool Face(const Elite::Vector2 target);
	void SpinAround();




private:
	IExamInterface* m_pInterface = nullptr;
	SteeringPlugin_Output* m_pSteering = nullptr;
};


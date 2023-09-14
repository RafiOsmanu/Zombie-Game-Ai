#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include "EBlackboard.h"

class ExploreControl
{

public:
	ExploreControl( IExamInterface* pInterface, int partitions);
	~ExploreControl() = default;
	void DrawDebugCells();
	void Update();
	int PositionToIndex(const Elite::Vector2 pos) const;
	Elite::Vector2 GetNextExplorationPoint(Elite::Blackboard* pBlackboard);

private:

	struct CellInfo
	{
		Elite::Vector2 leftBottom;
		Elite::Vector2 center;
		float width;
		float height;

		bool hasBeenVisited;
		bool IsExplorationPath;

	};

	IExamInterface* m_pInterFace;
	std::vector<CellInfo> m_Cells;
	std::vector<CellInfo> m_ExplorePath;
	WorldInfo m_World;
	int m_CellMeasure;
	int m_Partitions;
	bool ExploreControl::isPointInRect(Elite::Vector2 agentPos, Elite::Vector2 cellLeftBottom, int cellWidth, int cellHeight) const;
};


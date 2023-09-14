#include "stdafx.h"
#include "ExploreControl.h"

ExploreControl::ExploreControl(IExamInterface* pInterface, int partitions)
	:m_pInterFace{pInterface}
	,m_World{pInterface->World_GetInfo()}
{

	for (auto& cell : m_Cells)
	{
		//Initialize cell
		cell.width = m_World.Dimensions.x / partitions;
		cell.height = m_World.Dimensions.y / partitions;
		cell.hasBeenVisited = false;
		cell.IsExplorationPath = false;
	}

	float left{ m_World.Center.x - (m_World.Dimensions.x / 2.f) };
	float bottom{ m_World.Center.y - (m_World.Dimensions.y / 2.f) };
	CellInfo cell{};

	cell.width = m_World.Dimensions.x / partitions;
	cell.height = m_World.Dimensions.y / partitions;

	float cellWidth{ cell.width };
	float cellHeight{ cell.height };

	cell.hasBeenVisited = false;

	Elite::Vector2 cellCenter = { (left + cellWidth / 2.f), (bottom + cellHeight / 2.f) };

	for (int row{}; row < partitions; ++row)
	{
		for (int coll{}; coll < partitions; ++coll)
		{
			//assign current cell
			cell.leftBottom = {left, bottom};
			cell.center = cellCenter;

			m_Cells.emplace_back(cell);

			//one Coll RIGHT
			left += cellWidth;
			cellCenter.x += cellWidth;
		}
		//one row UP
		bottom += cellHeight;
		cellCenter.y += cellHeight;

		//back to leftSide
		left -= (m_World.Dimensions.x);
		cellCenter.x -= (m_World.Dimensions.x);
	}

	m_CellMeasure = static_cast<int>(cell.width);
	m_Partitions = partitions;

	//initialize explore path
	for (int row{}; row < partitions; ++row)
	{
		for (int coll{}; coll < partitions; ++coll)
		{
			// Left BLocks
			//-----------------------------------------------

			//Left top Block
			if (row >= 6 && row <= 7 && coll >= 14 && coll <= 16)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//Left Bottom Block
			if (row >= 6 && row <= 7 && coll >= 5 && coll <= 7)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			// Middle BLocks
			//-----------------------------------------------

			//middle center Block
			else if (coll >= 9 && coll <= 11 && row >= 9 && row <= 11)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			// Right BLocks
			//-----------------------------------------------
			
			//right Top Block
			else if (coll >= 14 && coll <= 16 && row >= 14 && row <= 15)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//right bottom Block
			else if (coll >= 5 && coll <= 7 && row >= 14 && row <= 15)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//  Corner connections / sides
			//-----------------------------------------------
			
			//Left Coll
			if (row == 6 && coll >= 8 && coll <= 13)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			// right Coll
			else if (row == 15 && coll >= 8 && coll <= 13)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//Top row
			else if (coll == 16 && row >= 8 && row <= 13)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//extra top
			else if (coll == 15 && row >= 10 && row <= 11)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//Bottom Row
			else if (coll == 5 && row >= 8 && row <= 13)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			//extra bottom
			else if (coll == 6 && row >= 10 && row <= 11)
			{
				m_ExplorePath.emplace_back(m_Cells[coll * m_Partitions + row]);
			}

			

		}
	}

}

void ExploreControl::DrawDebugCells()
{
	//for (const auto& cell : m_Cells)
	//{
	//	Elite::Vector2 cellSquarePoints[squarePoints];
	//	Elite::Vector3 cellColor{};

	//	Elite::Vector2 leftBottom{cell.leftBottom};
	//	Elite::Vector2 leftTop{leftBottom.x, leftBottom.y + cell.height};
	//	Elite::Vector2 rightTop{ leftTop.x + cell.width, leftTop.y };
	//	Elite::Vector2 rightBottom{rightTop.x, rightTop.y - cell.height};

	//	cellSquarePoints[0] = leftBottom;
	//	cellSquarePoints[1] = leftTop;
	//	cellSquarePoints[2] = rightTop;
	//	cellSquarePoints[3] = rightBottom;

	//	if (!cell.hasBeenVisited)
	//	{
	//		//blue
	//		cellColor = { 0.f, 0.f, 1.0f };
	//		m_pInterFace->Draw_Polygon(cellSquarePoints, 4, cellColor, 0.f);
	//	}
	//	else
	//	{
	//		//green
	//		cellColor = { .0f, 1.0f, .0f };
	//		m_pInterFace->Draw_Polygon(cellSquarePoints, 4, cellColor, -1.f);
	//	}
	//	
	//}

	const int squarePoints{ 4 };

	//explorePath
	for (const auto& cell : m_ExplorePath)
	{
		Elite::Vector2 cellSquarePoints[squarePoints];
		Elite::Vector3 cellColor{};

		Elite::Vector2 leftBottom{ cell.leftBottom };
		Elite::Vector2 leftTop{ leftBottom.x, leftBottom.y + cell.height };
		Elite::Vector2 rightTop{ leftTop.x + cell.width, leftTop.y };
		Elite::Vector2 rightBottom{ rightTop.x, rightTop.y - cell.height };

		cellSquarePoints[0] = leftBottom;
		cellSquarePoints[1] = leftTop;
		cellSquarePoints[2] = rightTop;
		cellSquarePoints[3] = rightBottom;

		
		if (!cell.hasBeenVisited)
		{
			//blue
			cellColor = { 1.f, 0.f, .0f };
			m_pInterFace->Draw_Polygon(cellSquarePoints, 4, cellColor, 0.f);
		}
		else
		{
			//blue
			cellColor = { 0.f, 1.f, .0f };
			m_pInterFace->Draw_Polygon(cellSquarePoints, 4, cellColor, -1.f);
		}

	}
}

void ExploreControl::Update()
{
	Elite::Vector2 agentPos{ m_pInterFace->Agent_GetInfo().Position };
	int posCellIdx{ PositionToIndex(agentPos) };

	m_ExplorePath[posCellIdx].hasBeenVisited = true;
}

int ExploreControl::PositionToIndex(const Elite::Vector2 pos) const
{

	for (size_t i{}; i < m_ExplorePath.size(); ++i)
	{
		if (isPointInRect(pos, m_ExplorePath[i].leftBottom, m_CellMeasure, m_CellMeasure))
		{

			return i;
		}
	}

	return 0;

}

Elite::Vector2 ExploreControl::GetNextExplorationPoint(Elite::Blackboard* pBlackboard)
{
	Elite::Vector2 agentPos{ m_pInterFace->Agent_GetInfo().Position };
	float closestDistance{99999.f};
	Elite::Vector2 nextExplorationPoint{};
	bool allCellsVisited{false};

	for (size_t i{}; i < m_ExplorePath.size(); ++i)
	{
		if (m_ExplorePath[i].hasBeenVisited) continue;
		allCellsVisited = true;

		if (agentPos.DistanceSquared(m_ExplorePath[i].center) < closestDistance)
		{
			closestDistance = agentPos.DistanceSquared(m_ExplorePath[i].center);
			nextExplorationPoint = m_ExplorePath[i].center;
		}

	}

	if (!allCellsVisited)
	{
		for (size_t i{}; i < m_ExplorePath.size(); ++i)
		{
			m_ExplorePath[i].hasBeenVisited = false;
			std::vector<HouseInfo> CheckedHouses{};
			pBlackboard->ChangeData("CheckedHouses", CheckedHouses);
		}
	}
	return nextExplorationPoint;
}


bool ExploreControl::isPointInRect(Elite::Vector2 agentPos, Elite::Vector2 cellLeftBottom, int cellWidth, int cellHeight) const
{
	return 
	{
		//agentPos.x >= left of cell
		agentPos.x >= cellLeftBottom.x

		//agentPos.x < left + width of cell
		&& agentPos.x < cellLeftBottom.x + cellWidth

		//agentPos.y >= bottom of cell
		&& agentPos.y >= cellLeftBottom.y

		//agentPos.y < bottom + height of cell
		&& agentPos.y < cellLeftBottom.y + cellHeight
	};
}

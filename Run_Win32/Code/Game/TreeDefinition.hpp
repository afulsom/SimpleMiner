#pragma once
#include "Engine\Math\IntVector3.hpp"
#include "BlockDefinition.hpp"
#include <vector>


enum TreeType
{
	TREE_TYPE_OAK,
	TREE_TYPE_WILLOW,
	TREE_TYPE_PINE,
	TREE_TYPE_WIMBA,
	NUM_TREE_TYPES
};

struct TreeBlockDefinition
{
	IntVector3 offsetFromBase;
	BlockType blockType;

	TreeBlockDefinition(const IntVector3& offset, BlockType type)
		: offsetFromBase(offset)
		, blockType(type)
	{
	}
};

class TreeDefinition
{
private:
	std::vector<TreeBlockDefinition> m_treeBlocks;


public:
	static TreeDefinition* s_treeDefinitions[NUM_TREE_TYPES];



	TreeDefinition(const std::vector<TreeBlockDefinition>& treeBlocks);

	const std::vector<TreeBlockDefinition> GetTreeBlocks() const;
};



inline const std::vector<TreeBlockDefinition> TreeDefinition::GetTreeBlocks() const
{
	return m_treeBlocks;
}

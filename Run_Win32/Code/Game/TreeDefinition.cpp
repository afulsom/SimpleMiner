#include "Game/TreeDefinition.hpp"

TreeDefinition* TreeDefinition::s_treeDefinitions[];

TreeDefinition::TreeDefinition(const std::vector<TreeBlockDefinition>& treeBlocks)
	: m_treeBlocks(treeBlocks)
{

}


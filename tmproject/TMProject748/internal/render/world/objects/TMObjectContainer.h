#pragma once

#include "TreeNode.h"

class TMGround;
class TMLight;

// Fixed limits kept from the legacy contract of the regional container.
constexpr auto MAX_OBJECT_LIST = 4096;
constexpr auto MAX_LIGHT_CONTAINER = 6;

/**
 * Groups the visual objects loaded for a world region.
 *
 * The scene tree remains responsible for children registered under this node.
 * `m_pGround` and the entries in `m_pObjectList` are non-owning references
 * used for fast access. The terrain effect container is attached as a child,
 * while lights retain the legacy lifecycle managed by this class.
 */
class TMObjectContainer : public TreeNode
{
public:
    TMGround* m_pGround; // External terrain associated with this region.
    int m_nLightIndex;
    TMLight* m_pLightContainer[MAX_LIGHT_CONTAINER];
    TreeNode* m_pGroundEffectContainer; // Child owned by the scene tree.
    int m_nObjectIndex;
    TMObject* m_pObjectList[MAX_OBJECT_LIST]; // Non-owning index of the children.

public:
    /** Creates the aggregate for `pGround` without taking ownership of it. */
    TMObjectContainer(TMGround* pGround);
    ~TMObjectContainer();

    /** Loads region objects from a file and returns the legacy operation status. */
    int Load(const char* szFileName);

    /** Updates children using the authoritative time received from the scene. */
    int FrameMove(unsigned int dwServerTime);
};

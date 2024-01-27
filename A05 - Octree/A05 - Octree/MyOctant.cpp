#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

void MyOctant::Init()
{
	m_uID = m_uOctantCount;
	m_uLevel = 0;
	m_uChildren = 0;
	m_fSize = 0.0f;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (uint i = 0; i < 8; i++) 
	{
		m_pChild[i] = nullptr;
	}
}

MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount) 
{
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax;

	uint nObjects = m_pEntityMngr->GetEntityCount();

	for (int i = 0; i < nObjects; i++) 
	{
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();
		lMinMax.push_back(pRigidBody->GetMinGlobal());
		lMinMax.push_back(pRigidBody->GetMaxGlobal());
	}
	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	vector3 vHalfWidth = pRigidBody->GetHalfWidth();
	float fMax = vHalfWidth.x;
	for (int i = 1; i < 3; i++) 
	{
		if (fMax < vHalfWidth[i]) 
		{
			fMax = vHalfWidth[i];
		}
	}

	vector3 v3Center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	m_fSize = fMax * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{

	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

MyOctant::MyOctant(MyOctant const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Max = other.m_v3Max;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;


	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (int i = 0; i < 8; i++) 
	{
		m_pChild[i] = other.m_pChild[i];
	}
}

MyOctant& MyOctant::operator=(MyOctant const& other) 
{
	if (this != &other) 
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

MyOctant::~MyOctant() 
{
	Release();
}

void MyOctant::Release()
{
	if (m_uLevel == 0) 
	{
		KillBranches();
	}

	m_uOctantCount = 0;
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_lEntityList.clear();
	m_lChild.clear();
}

void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (int i = 0; i < 8; i++) 
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}

float MyOctant::GetSize() 
{
	return m_fSize;
}
vector3 MyOctant::GetCenterGlobal() 
{
	return m_v3Center;
}
vector3 MyOctant::GetMinGlobal() 
{
	return m_v3Min;
}
vector3 MyOctant::GetMaxGlobal() 
{
	return m_v3Max;
}
uint MyOctant::GetOctantCount()
{
	return m_uOctantCount;
}

MyOctant* MyOctant::GetParent()
{
	return m_pParent;
}

bool MyOctant::IsLeaf()
{
	return m_uChildren == 0;
}

bool MyOctant::IsColliding(uint a_uRBIndex) 
{
	int nObjectCount = m_pEntityMngr->GetEntityCount();
	if (a_uRBIndex >= nObjectCount) 
	{
		return false;
	}

	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3MinO = pRigidBody->GetMinGlobal();
	vector3 v3MaxO = pRigidBody->GetMaxGlobal();

	//AABB
	if (m_v3Max.x < v3MinO.x) 
	{
		return false;
	}
	if (m_v3Min.x > v3MaxO.x) 
	{
		return false;
	}

	if (m_v3Max.y < v3MinO.y) 
	{
		return false;
	}
	if (m_v3Min.y > v3MaxO.y) 
	{
		return false;
	}

	if (m_v3Max.z < v3MinO.z) 
	{
		return false;
	}
	if (m_v3Min.z > v3MaxO.z) 
	{
		return false;
	}

	return true;
}

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color) 
{
	if (m_uID == a_nIndex) 
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	for (int i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->Display(a_nIndex);
	}
}

void MyOctant::Display(vector3 a_v3Color) 
{
	for (int i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::DisplayLeafs(vector3 a_v3Color) 
{
	int nLeafs = m_lChild.size();
	for (int i = 0; i < nLeafs; i++) 
	{
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::Subdivide() 
{
	if (m_uLevel >= m_uMaxLevel || m_uChildren != 0) 
	{
		return;
	}
	m_uChildren = 8;

	float childHalf = m_fSize / 4.0f;
	float childSize = m_fSize / 2.0f;
	vector3 center = m_v3Center;
	center.x -= childHalf;
	center.y -= childHalf;
	center.z -= childHalf;

	//bacon lettuce bacon
	m_pChild[0] = new MyOctant(center, childSize);

	//bacon ranch bacon
	center.x += childSize;
	m_pChild[1] = new MyOctant(center, childSize);

	//bacon ranch flatbread
	center.z += childSize;
	m_pChild[2] = new MyOctant(center, childSize);

	//bacon lettuce flatbread
	center.x -= childSize;
	m_pChild[3] = new MyOctant(center, childSize);

	//tomato lettuce flatbread
	center.y += childSize;
	m_pChild[4] = new MyOctant(center, childSize);

	//tomato lettuce bacon
	center.z -= childSize;
	m_pChild[5] = new MyOctant(center, childSize);

	//tomato ranch bacon
	center.x += childSize;
	m_pChild[6] = new MyOctant(center, childSize);

	//tomato ranch flatbread
	center.z += childSize;
	m_pChild[7] = new MyOctant(center, childSize);

	for (int i = 0; i < 8; i++) 
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		m_pChild[i]->m_uIdealEntityCount = m_uIdealEntityCount;

		if (m_pChild[i]->CheckObjectCount(m_uIdealEntityCount) && m_pChild[i]->m_uLevel < m_uMaxLevel) 
		{
			m_pChild[i]->Subdivide();
		}
	}
}

MyOctant* MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7) 
	{
		return nullptr;
	}
	return m_lChild[a_nChild];
}

bool MyOctant::CheckObjectCount(uint a_nEntities)
{
	int count = 0;
	int entityCount = m_pEntityMngr->GetEntityCount();
	for (int i = 0; i < entityCount; i++) 
	{
		if (IsColliding(i)) 
		{
			count++;
		}

		if (count > a_nEntities) 
		{
			return true;
		}

	}
	return false;
}

void MyOctant::KillBranches() 
{
	//remove all child branches
	for (int i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->KillBranches();
		SafeDelete(m_pChild[i]);
	}
	m_uChildren = 0;
}

void MyOctant::ClearEntityList()
{
	for (int i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->ClearEntityList();
	}
	m_lEntityList.clear();
}

void MyOctant::AssignIDtoEntity() 
{
	for (int i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	if (IsLeaf()) 
	{
		int nEntities = m_pEntityMngr->GetEntityCount();
		for (int i = 0; i < nEntities; i++) 
		{
			if (IsColliding(i)) 
			{
				m_lEntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

void MyOctant::ConstructTree(uint a_nMaxLevel) 
{
	if (m_uLevel != 0) 
	{
		return;
	}
	m_uMaxLevel = a_nMaxLevel;

	ClearEntityList();
	KillBranches();
	m_lChild.clear();

	Subdivide();
	AssignIDtoEntity();
	ConstructList();
}

void MyOctant::ConstructList()
{
	for (int i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->ConstructList();
	}

	if (m_lEntityList.size() > 0) 
	{
		m_pRoot->m_lChild.push_back(this);
	}
}
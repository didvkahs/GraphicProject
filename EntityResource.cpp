#include "GLTFTypes.h"

#include "EntityResource.h"

#include "GLTFReader.h"


EntityResource::EntityResource(void) {}
EntityResource::EntityResource(ModelType_e model)
{
	m_reader = new GLTFReader();

	if (!m_reader->Initialize(m_modelInfos[model].modelPath))
	{
#ifdef _DEBUG
		__debugbreak();
#endif
		fprintf(stderr, "entityResource initialize failed with error while reading %s\n", m_modelInfos[model].modelPath);
		exit(1);
	}

	m_reader->GetMeshes(meshList);
	m_reader->GetNodes(nodeList);

	if (m_modelInfos[model].hasSkin)
	{
		m_reader->GetSkins(skin);
	}

	m_model = model;
}

EntityResource::~EntityResource(void)
{
	if (meshList.list != nullptr)
	{
		delete[] meshList.list;
		meshList.list = nullptr;
		meshList.count = 0;
	}

	if (nodeList.list != nullptr)
	{
		delete[] nodeList.list;
		nodeList.list = nullptr;
		nodeList.count = 0;
	}

	if (skin.jointNodeIndices != nullptr)
	{
		delete[] skin.jointNodeIndices;
		delete[] skin.invBindPoses;

		skin.jointNodeIndices = nullptr;
		skin.invBindPoses = nullptr;
		skin.jointNodeCount = 0;
	}
}

bool EntityResource::HasSkin(void)
{
	return m_modelInfos[m_model].hasSkin;
}
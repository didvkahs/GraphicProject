#include "GLTFTypes.h"

#include "EntityResource.h"

#include "GLTFReader.h"



EntityResource::EntityResource(void) {}
EntityResource::EntityResource(const std::string& filePath)
{
	m_reader = new GLTFReader();

	if (!m_reader->Initialize(filePath))
	{
#ifdef _DEBUG
		__debugbreak();
#endif
		fprintf(stderr, "entityResource initialize failed with error while reading %s\n", filePath);
		exit(1);
	}

	m_reader->GetMeshes(meshList);
	m_reader->GetNodes(nodeList);
}

EntityResource::~EntityResource(void)
{
	if (m_reader)
	{
		delete[] m_reader;
		m_reader = nullptr;
	}

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
}
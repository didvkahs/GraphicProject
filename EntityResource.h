#pragma once

class GLTFReader;

// TODO : (IMP) If the data contains SKIN or ANIMATION, set it as a CharacterResource.
//              else set as a EntityResource

class EntityResource
{
public:
	EntityResource(void);
	EntityResource(const std::string& filePath);
	virtual ~EntityResource(void);

	MeshList_s meshList;
	NodeList_s nodeList;

protected:
	GLTFReader* m_reader = nullptr;
};
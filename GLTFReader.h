#pragma once

class GLTFReader
{
public:
	GLTFReader(void);
	~GLTFReader(void);

	bool Initialize(const std::string& filePath);
	
	// NOTICE : Read according to function call priority
	bool GetSkins(Skin_s& skin);
	bool GetNodes(NodeList_s& outList);
	bool GetMeshes(MeshList_s& outList);

private:
	
	bool m_hasSkin = false;
};
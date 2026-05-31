#pragma once

class GLTFReader
{
public:
	GLTFReader(void);
	~GLTFReader(void);

	bool Initialize(const std::string& filePath);
	
	bool GetSkins(Skin_s& skin);
	bool GetNodes(NodeList_s& outList);
	bool GetMeshes(MeshList_s& outList);

	bool HasSkin(void);
	bool HasAnimation(void);

private:
	
	bool m_hasAnimation = false;
	bool m_hasSkin = false;
};
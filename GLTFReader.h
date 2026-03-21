#pragma once

class GLTFReader
{
public:
	GLTFReader(void);
	~GLTFReader(void);
	GLTFReader(WCHAR* filePath);

	bool LoadGLTF(void);
	
	void GetJoint(JointList_s& outData);
	void GetMesh(MeshList_s& outData);
	void GetTex(Texture_s& outData);

private:

	Microsoft::glTF::Document document;
	std::unique_ptr<Microsoft::glTF::GLTFResourceReader> resourceReader;
};
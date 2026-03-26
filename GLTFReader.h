#pragma once

class GLTFReader
{
public:
	GLTFReader(void);
	~GLTFReader(void);

	bool Initialize(const std::string& filePath);
	bool GetMeshes(MeshList_s& outMesh, bool& hasOut);
	bool GetSkinedMesh(MeshList_s& outMesh, bool& hasOut);
	bool GetSkeletons(Skeleton_s& outSkel, bool& hasOut);
	bool GetTextures(TextureList_s& outTex, bool& hasOut);
	bool GetMaterials(MaterialList_s& outMat, bool& hasOut);

private:
	std::string getAccIDX(const std::string& attr, MeshPrimitive& prim);
	uint8_t* getRawData(const Accessor& acc);

private:

	Microsoft::glTF::Document document;
	std::unique_ptr<Microsoft::glTF::GLTFResourceReader> resourceReader;
};
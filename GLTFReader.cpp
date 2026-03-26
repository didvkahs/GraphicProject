#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/IStreamReader.h>

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/queue.h>

#include <DirectXTex.h>

#include <fstream>
#include <string>
#include <memory>
#include <cassert>

#include "GLTFTypes.h"
#include "GLTFReader.h"

#include <windows.h>


using namespace DirectX;
using namespace Microsoft::glTF;

class StreamReader : public IStreamReader
{
public:
	std::shared_ptr<std::istream> GetInputStream(const std::string& path) const override
	{
		return std::make_shared<std::ifstream>(path, std::ios::binary);
	}
};


GLTFReader::GLTFReader(void) {}
GLTFReader::~GLTFReader(void) {}


void convertUTF8ToWChar(const std::string& inputString, Texture_s& outTex);


bool GLTFReader::Initialize(const std::string& filePath)
{
	auto streamReader = std::make_shared<StreamReader>();
	resourceReader = std::make_unique<GLTFResourceReader>(streamReader);

	std::ifstream file(filePath);

	if (!file.is_open())
	{
		fprintf(stderr, "readFileContent file open failure : INVALID_FILE_NAME | FILE_NOT_EXSIST\n");
		return false;
	}

	int size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string content(size, '\0');

	if (!file.read(&content[0], size))
	{
		fprintf(stderr, "read failed with error : INVALID_FILE_VALUE | CAN_NOT_READ_FILE\n");
		return false;
	}

	document = Deserialize(content);

	return true;
}



bool GLTFReader::GetMeshes(MeshList_s& outMesh, bool& hasOut)
{
	// TODO : (IMP) code after understand file structure
	// TODO : (IMP) read gltf instruction
	
	int nodeCount = document.nodes.Size();
		


	return true;
}


bool GLTFReader::GetSkinedMesh(MeshList_s& outMesh, bool& hasOut)
{
	if (!document.skins.Size())
	{
		fprintf(stderr, "skin size is 0 : NOT_SUPPORT_SKIN | HAS_NO_SKELETON\n");
		hasOut = false;
		return false;
	}

	const Skin& skin = document.skins.Get(0);
	
	int nodeCount = document.nodes.Size();
	std::vector<const Node*> skinedNode;
	skinedNode.reserve(nodeCount);

	for (int i = 0; i < nodeCount; ++i)
	{
		const Node& node = document.nodes.Get(i);

		if (node.skinId == skin.id)
		{
			outMesh.count++;
			skinedNode.push_back(&node);
		}
	}


	outMesh.list = new Mesh_s[outMesh.count];

	for (int i = 0; i < outMesh.count; ++i)
	{
		const Node* node = skinedNode[i];
		const Mesh& mesh = document.meshes.Get(node->meshId);
	}


	return true;
}



bool GLTFReader::GetSkeletons(Skeleton_s& outSkel, bool& hasOut)
{
	// TODO : (IMP) change method to support all gltf files
	// TODO : (IMP) read gltf tutorial instruction to understand gltf structure

	if (document.skins.Size() == 0)
	{
		printf("document Skin Size is 0 : NO_JOINTS | NO_BONS\n");
		hasOut = false;
		return true;
	}

	const auto& skin = document.skins.Get(0);
	std::vector<float> invBinMats;

	if (!skin.inverseBindMatricesAccessorId.empty())
	{
		const Accessor& accessor = document.accessors.Get(skin.inverseBindMatricesAccessorId);
		invBinMats = resourceReader->ReadFloatData(document, accessor);
	}

	outSkel.count = skin.jointIds.size();
	outSkel.list = new Joint_s[outSkel.count];

	eastl::map<std::string, int> jointHierachy;

	for (int i = 0; i < outSkel.count; ++i)
	{
		const Node& node = document.nodes.Get(skin.jointIds[i]);
		Joint_s& joint = outSkel.list[i];

		if (invBinMats.size() >= (size_t)(i + 1) * 16)
		{
			memcpy(&joint.inveBindPose, &invBinMats[i * 16], sizeof(XMFLOAT4X4));
		}
		else
		{
			memcpy(&joint.inveBindPose, &XMMatrixIdentity(), sizeof(XMFLOAT4X4));
		}
		
		jointHierachy[skin.jointIds[i]] = i;
		joint.jointId = skin.jointIds[i];
		joint.nodeId = node.id;
		joint.jointName = node.name;
		joint.parentIDX = 0;

		joint.localPose.translation = { node.translation.x, node.translation.y, node.translation.z };
		joint.localPose.rotation = { node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w };
		joint.localPose.scale = node.scale.x;
	}

	Joint_s* list = outSkel.list;
	for (int i = 0; i < outSkel.count; ++i)
	{
		const Node& node = document.nodes.Get(skin.jointIds[i]);
		
		for (const auto& childNodeId : node.children)
		{
			Joint_s* child = &list[jointHierachy[childNodeId]];
			child->parentIDX = i;
		}
	}

	return true;
}



bool GLTFReader::GetTextures(TextureList_s& outTex, bool& hasOut)
{
	// TODO : (IMP) seperate texture and material reading method
	// TODO : (IMP) find no uri image texture -> Read my past parser_gltf code

	if (document.images.Size() == 0)
	{
		printf("document image size is 0 : NO_TEXTURE | NO_IMAGE \n");
		hasOut = false;
		return true;
	}

	int imageCount = document.images.Size();

	outTex.list = new Texture_s[imageCount];
	outTex.count = imageCount;

	for (int i = 0; i < imageCount; ++i)
	{
		const auto& image = document.images.Get(i);
		Texture_s& texNode = outTex.list[i];

		if (!image.uri.empty())
		{
			texNode.hasUri = true;
			convertUTF8ToWChar(image.uri, texNode);
		}
		else if (!image.bufferViewId.empty())
		{
			std::vector<uint8_t> binImageData = resourceReader->ReadBinaryData(document, image);
			
			texNode.hasUri = false;
			texNode.dataSize = binImageData.size();
			texNode.data = new uint8_t[texNode.dataSize];
			
			memcpy(texNode.data, binImageData.data(), texNode.dataSize);
		}

	}

	return true;
}

bool GLTFReader::GetMaterials(MaterialList_s& outMat, bool& hasOut)
{
	if (document.materials.Size() == 0)
	{
		printf("materials size is 0 : NO_MATERIAL\n");
		hasOut = false;
		return true;
	}

	int matCount = document.materials.Size();
	outMat.count = matCount;
	outMat.list = new Material_s[matCount];

	for (int i = 0; i < matCount; ++i)
	{
		const auto& material = document.materials.Get(i);
		Material_s& matNode = outMat.list[i];

		matNode.baseColor[0] = material.metallicRoughness.baseColorFactor.r;
		matNode.baseColor[1] = material.metallicRoughness.baseColorFactor.g;
		matNode.baseColor[2] = material.metallicRoughness.baseColorFactor.b;
		matNode.baseColor[3] = material.metallicRoughness.baseColorFactor.a;

		matNode.metallicFactor = material.metallicRoughness.metallicFactor;
		matNode.emissiveFactor[0] = material.emissiveFactor.r;
		matNode.emissiveFactor[1] = material.emissiveFactor.g;
		matNode.emissiveFactor[2] = material.emissiveFactor.b;
	}
}










std::string GLTFReader::getAccIDX(const std::string& attr, MeshPrimitive& primi)
{
	auto target = primi.attributes.find(attr);
	if (target == primi.attributes.end()) return nullptr;
	else return target->second;
}

uint8_t* GLTFReader::getRawData(const Accessor& acc)
{
	const BufferView& bv = document.bufferViews.Get(acc.bufferViewId);
	const Buffer& b = document.buffers.Get(bv.bufferId);
	const auto binrayStream = resourceReader->ReadBinaryData<float>(document, acc);
}



void convertUTF8ToWChar(const std::string& inputString, Texture_s& outTex)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, inputString.c_str(), inputString.length(), nullptr, 0);

	outTex.fileName = new WCHAR[size];

	MultiByteToWideChar(CP_UTF8, 0, inputString.c_str(), inputString.length(), outTex.fileName, size);
	outTex.fileName[size] = L'\0';
}

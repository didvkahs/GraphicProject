#include "GLTFTypes.h"
#include "GLTFReader.h"

#undef min
#undef max

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/IStreamReader.h>

#include <EASTL/map.h>
#include <EASTL/vector.h>

#include <fstream>
#include <memory>
#include <cassert>

using namespace DirectX;
using namespace Microsoft::glTF;
using namespace std;

Microsoft::glTF::Document g_Document;
std::unique_ptr<Microsoft::glTF::GLTFResourceReader> g_ResourceReader;

const std::string BIN_PREFIX = ".//assets//Models//";

// TODO : (LATER) basic reader handles all gltf files ( only read mesh )
//         - Create seperate reader for character with skin and animation data ( read according to function call priority )



class StreamReader : public IStreamReader
{
public:

	StreamReader(const std::string& fullPath) {
		size_t slashPos = fullPath.find("//", BIN_PREFIX.size());
		std::string modelType = fullPath.substr(BIN_PREFIX.size(), slashPos - BIN_PREFIX.size());

		m_baseModelPath = BIN_PREFIX + modelType + "//";
	}

	std::shared_ptr<std::istream> GetInputStream(const std::string& path) const override
	{
		std::string finalModelPath = m_baseModelPath + path;
		return std::make_shared<std::ifstream>(finalModelPath, std::ios::binary);
	}

private:

	std::string m_baseModelPath;
};


GLTFReader::GLTFReader(void) {}
GLTFReader::~GLTFReader(void) {}



bool GLTFReader::Initialize(const std::string& filePath)
{

	// TODO : (IMP) change bin path -> its only has file name not a path

	auto streamReader = std::make_shared<StreamReader>(filePath);
	g_ResourceReader = std::make_unique<GLTFResourceReader>(streamReader);

	std::ifstream file(filePath);

	if (!file.is_open())
	{
		fprintf(stderr, "readFileContent file open failure : INVALID_FILE_NAME | FILE_NOT_EXSIST\n");
		return false;
	}

	file.seekg(0, std::ios::end);
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string content(size, '\0');

	if (!file.read(&content[0], size))
	{
		fprintf(stderr, "read failed with error : INVALID_FILE_VALUE | CAN_NOT_READ_FILE\n");
		return false;
	}

	g_Document = Deserialize(content);

	return true;
}






bool GLTFReader::GetNodes(NodeList_s& outList)
{
	if (!g_Document.nodes.Size())
	{
		fprintf(stderr, "GetNodes failed with error : NODE_SIZE_ZERO | INVALID_FILE\n");
		return false;
	}

	int nodeCount = g_Document.nodes.Size();

	outList.count = nodeCount;
	outList.list = new Node_s[nodeCount];
	eastl::map<std::string, int> nodeMap;

	for (int i = 0; i < nodeCount; ++i)
	{
		const Node& node = g_Document.nodes.Get(i);
		Node_s& dstNode = outList.list[i];

		dstNode.name = node.name.c_str();
		nodeMap[node.id] = i;

		if (!node.meshId.empty())
		{
			dstNode.meshIDX = node.meshId.c_str();
		}

		if (!node.skinId.empty())
		{
			dstNode.skinIDX = node.skinId.c_str();
		}

		if (node.HasValidTransformType())
		{
			JointPose_s& pose = dstNode.localPose;

			pose.translation = { node.translation.x, node.translation.y,  node.translation.z };
			pose.rotation = { node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w };
			pose.scale = node.scale.x;

			dstNode.hasLocalPose = true;
		}
	}

	for (int i = 0; i < nodeCount; ++i)
	{
		const Node& node = g_Document.nodes.Get(i);

		if (!node.children.empty())
		{
			int childCount = node.children.size();

			for (int j = 0; j < childCount; ++j)
			{
				int childIDX = nodeMap[node.children[j]];

				outList.list[childIDX].parentIDX = i;
			}
		}
	}


	return true;
}


bool GLTFReader::GetMeshes(MeshList_s& outList)
{
	if (!g_Document.meshes.Size())
	{
		fprintf(stderr, "GetMeshes failed with error : MESH_SIZE_ZERO | INVALID_FILE\n");
		return false;
	}

	// TODO : (IMP) fix error when read uint12 and uint32

	int meshCount = g_Document.meshes.Size();

	outList.count = meshCount;
	outList.list = new Mesh_s[meshCount];


	std::vector<float> norData;
	std::vector<float> posData;
	std::vector<float> texData;
	std::vector<float> weightData;

	std::vector<uint16_t> inputIndices16;
	std::vector<uint32_t> inputIndices32;
	std::vector<uint16_t> jointsData;

	for (int i = 0; i < meshCount; ++i)
	{
		const Mesh& mesh = g_Document.meshes.Get(i);
		Mesh_s& dstMesh = outList.list[i];

		const auto& primitive = mesh.primitives;
		const auto& attributes = primitive[0].attributes;

		dstMesh.materialIDX = primitive[0].materialId.c_str();
		

		if (!primitive[0].indicesAccessorId.empty())
		{
			auto& indAcc = g_Document.accessors.Get(primitive[0].indicesAccessorId);

			int indCount = indAcc.count;

			dstMesh.indexCount = indCount;
			dstMesh.indices = new uint32_t[indCount];

			if (indAcc.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
			{
				inputIndices16 = g_ResourceReader->ReadBinaryData<uint16_t>(g_Document, indAcc);
				std::copy(inputIndices16.begin(), inputIndices16.end(), dstMesh.indices);
			}
			else
			{
				inputIndices32 = g_ResourceReader->ReadBinaryData<uint32_t>(g_Document, indAcc);
				std::copy(inputIndices32.begin(), inputIndices32.end(), dstMesh.indices);
			}
		}

		auto pPos = attributes.find("POSITION");
		auto pNor = attributes.find("NORMAL");
		auto pTex = attributes.find("TEXCOORD_0");
		auto pWei = attributes.find("WEIGHTS_0");
		auto pJoi = attributes.find("JOINTS_0");

		if (pPos != attributes.end())
		{
			auto& posAcc = g_Document.accessors.Get(pPos->second);
			posData = g_ResourceReader->ReadFloatData(g_Document, posAcc);

			const int vertexCount = posAcc.count;

			dstMesh.vertexCount = vertexCount;
			dstMesh.vertices = new ModelVertex_s[vertexCount];
	
		}

		if (pNor != attributes.end())
		{
			auto& norAcc = g_Document.accessors.Get(pNor->second);
			norData = g_ResourceReader->ReadFloatData(g_Document, norAcc);
		}

		if (pTex != attributes.end())
		{
			auto& texAcc = g_Document.accessors.Get(pTex->second);
			texData = g_ResourceReader->ReadFloatData(g_Document, texAcc);
		}

		if (pJoi != attributes.end())
		{
			auto& joiAcc = g_Document.accessors.Get(pJoi->second);
			jointsData = g_ResourceReader->ReadBinaryData<uint16_t>(g_Document, joiAcc);
		}

		if (pWei != attributes.end())
		{
			auto& weiAcc = g_Document.accessors.Get(pWei->second);
			weightData = g_ResourceReader->ReadFloatData(g_Document, weiAcc);
		}



		for (int i = 0; i < dstMesh.vertexCount; ++i)
		{
			ModelVertex_s& vertex = dstMesh.vertices[i];

			if (!posData.empty())
			{
				vertex.position = { posData.at(i * 3), posData.at(i * 3 + 1), posData.at(i * 3 + 2), 1.0f };
			}

			if (!norData.empty())
			{
				vertex.normal = { norData.at(i * 3), norData.at(i * 3 + 1), norData.at(i * 3 + 2), 0.0f };
			}

			if (!texData.empty())
			{
				vertex.uv = { texData.at(i * 2), texData.at(i * 2 + 1) };
			}

			if (!jointsData.empty())
			{
				vertex.joints = { jointsData.at(i * 3), jointsData.at(i * 3 + 1), jointsData.at(i * 3 + 2), 1 };
			}
		}

		inputIndices16.clear();
		inputIndices32.clear();
		weightData.clear();
		posData.clear();
		norData.clear();
		texData.clear();
	}


	return true;
}


bool GLTFReader::GetSkins(Skin_s& outList)
{
	if (!g_Document.skins.Size())
	{
		fprintf(stderr, "skin size is 0 : FILE_NOT_CONTAIN_SKIN | NO_JOINT \n");
		return false;
	}

	const Skin& skin = g_Document.skins.Get(0);
	std::vector<float> invMatData;

	if (!skin.inverseBindMatricesAccessorId.empty())
	{
		const std::string invMatID = skin.inverseBindMatricesAccessorId;
		const Accessor& invAcc = g_Document.accessors.Get(invMatID);

		invMatData = g_ResourceReader->ReadFloatData(g_Document, invAcc);
	}
	
	const int jointCount = skin.jointIds.size();
	outList.jointNodeCount = jointCount;
	outList.jointNodeIndices = new int[outList.jointNodeCount];
	outList.invBindPoses = new XMFLOAT4X4[outList.jointNodeCount];

	for (int i = 0; i < jointCount; ++i)
	{
		XMFLOAT4X4& dstInvMat = outList.invBindPoses[i];
		int& dstJointNode = outList.jointNodeIndices[i];

		if (!invMatData.empty())
		{
			memcpy(&dstInvMat, &invMatData.at(i * 16), sizeof(float) * 16);
		}

		dstJointNode = std::stoi(skin.jointIds.at(i));
	}

	return true;
}
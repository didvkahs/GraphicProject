#pragma once

class GLTFReader;


enum ModelType_e
{
	MODEL_CHISA,
	MODEL_DEKU,
	MODEL_MUDA,
	MODEL_COUNT,
	MODEL_NON
};


class EntityResource
{
public:
	EntityResource(void);
	EntityResource(ModelType_e model);
	~EntityResource(void);

	bool HasSkin(void);

	MeshList_s meshList;
	NodeList_s nodeList;
	Skin_s skin;

private:

	struct ModelInfo_s
	{
		std::string modelPath;
		bool hasSkin = false;
	};

private:
	ModelType_e m_model = MODEL_NON;

	ModelInfo_s m_modelInfos[MODEL_COUNT] =
	{
		{".//assets//Models//chisa//scene.gltf", false},
		{".//assets//Models//deku//vigilante-deku.gltf", true},
		{".//assets//Models//Muda//scene.gltf", false}
	};

	GLTFReader* m_reader = nullptr;
};
#include <string>

#include "ResourceRegistery.h"

#include "GLTFTypes.h"
#include "GLTFReader.h"
#include "EntityResource.h"
#include "CharacterResource.h"

#include <EASTL/string.h>
#include <EASTL/unordered_map.h>


namespace
{
	eastl::unordered_map<eastl::string, EntityResource*> g_Registery;
}

ResourceRegistery::ResourceRegistery(void)
{
	// TODO : (LATER) remove initialize model by RegsiterModel, Change method to read file
	
	RegisterModel("chisa", ".//assets//Models//chisa//scene.gltf");
	RegisterModel("muda", ".//assets//Models//Muda//scene.gltf");
	RegisterModel("skeleton", ".//assets//Models//skeleton//scene.gltf");
}

ResourceRegistery::~ResourceRegistery(void)
{
	for (auto& model : g_Registery)
	{
		delete model.second;
	}
}



bool ResourceRegistery::RegisterModel(const std::string& modelKey, const std::string& filePath)
{
	GLTFReader reader;
	reader.Initialize(filePath.c_str());

	bool hasAnimation = reader.HasAnimation();
	EntityResource* temp = nullptr;

	if (hasAnimation)
	{
		temp = new CharacterResource(filePath);
	}
	else
	{
		temp = new EntityResource(filePath);
	}


	g_Registery.emplace(modelKey.c_str(), temp);
	
	return true;
}


EntityResource* ResourceRegistery::GetResource(const std::string& modelKey)
{
	auto target = g_Registery.find(modelKey.c_str());

	if (target != g_Registery.end())
	{
		return target->second;
	}

	return nullptr;
}

bool ResourceRegistery::HasAnimation(const std::string& modelKey)
{
	EntityResource* target = GetResource(modelKey);
	CharacterResource* child = dynamic_cast<CharacterResource*>(target);

	if (child != nullptr)
	{
		return true;
	}

	return false;
}
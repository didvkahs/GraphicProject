#pragma once

class EntityResource;
class CharacterResource;

class ResourceRegistery
{
public:
	ResourceRegistery(void);
	~ResourceRegistery(void);

	bool RegisterModel(const std::string& modelKey, const std::string& filePath);

	EntityResource* GetResource(const std::string& modelKey);
	bool HasAnimation(const std::string& modelKey);
};
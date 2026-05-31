#pragma once

// TODO : (IMP) 
// CharacterResource: Normalize character height to a fixed range.
// Ensure an absolute height value is defined to serve as the standard reference.


class CharacterResource : public EntityResource
{
public:

	CharacterResource(void);
	CharacterResource(const std::string& filePath);
	virtual ~CharacterResource(void) override;

	Skin_s skins;
	DirectX::XMFLOAT4X4 scale;

private:
	void normalizeHeight(void);

private:
	static const int NORMALIZED_CHARACTER_HEIGHT = 3;
	float m_normalHiehtScale = 0.0f;

};
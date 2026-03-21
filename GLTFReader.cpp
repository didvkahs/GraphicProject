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

#include "CharactorTypes.h"
#include "GLTFReader.h"


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
~GLTFReader::GLTFReader(void) {}
GLTFReader::GLTFReader(WCHAR* fileName)
{

}
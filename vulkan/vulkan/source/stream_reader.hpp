#pragma once
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

#include <iostream>
#include <fstream>
#include <filesystem>

class StreamReader : public Microsoft::glTF::IStreamReader
{
public:
	StreamReader(std::filesystem::path pathBase) : m_pathBase(std::move(pathBase)) {}

	virtual std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
	{
		auto streamPath = m_pathBase / std::filesystem::u8path(filename);
		auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);
		if (!stream || !(*stream))
		{
			throw std::runtime_error("Unable to create valid input stream.");
		}
		return stream;
	}


private:
	std::filesystem::path m_pathBase;
};
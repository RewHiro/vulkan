#include "model_app.hpp"

#include "stream_reader.hpp"

namespace app
{
	void ModelApp::prepare()
	{
		auto modelFilePath = std::filesystem::path("alicia-solid.vrm");
		if (modelFilePath.is_relative())
		{
			auto current = std::filesystem::current_path();
			current /= modelFilePath;
			current.swap(modelFilePath);
		}

		auto reader = std::make_unique<StreamReader>(modelFilePath.parent_path());
		auto glbStream = reader->GetInputStream(modelFilePath.filename().u8string());
		auto glbResourceReader = std::make_shared<Microsoft::glTF::GLBResourceReader>(std::move(reader), std::move(glbStream));
		auto document = Microsoft::glTF::Deserialize(glbResourceReader->GetJson());
	}
}
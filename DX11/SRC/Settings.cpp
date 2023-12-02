#include "Settings.h"
#include"sajson.h"
static const char* fileName = "settings.ini";
static bool sReadFile(char*& data, int& size, const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (file == nullptr)
	{
		return false;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (size == 0)
	{
		return false;
	}

	data = (char*)malloc(size + 1);
	fread(data, size, 1, file);
	fclose(file);
	data[size] = 0;

	return true;
}
void Settings::load()
{

	char* data = nullptr;
	int size = 0;
	bool found = sReadFile(data, size, fileName);
	if (found == false)
	{
		return;
	}

	const sajson::document& document = sajson::parse(sajson::dynamic_allocation(), sajson::mutable_string_view(size, data));
	if (document.is_valid() == false)
	{
		return;
	}

	sajson::value root = document.get_root();
	int fieldCount = int(root.get_length());
	for (int i = 0; i < fieldCount; ++i)
	{
		sajson::string fieldName = root.get_object_key(i);
		sajson::value fieldValue = root.get_object_value(i);

		if (strncmp(fieldName.data(), "resultmodelpath", fieldName.length()) == 0)
		{
			if (fieldValue.get_type() == sajson::TYPE_STRING)
			{
				resultmodelpath = fieldValue.as_string();
			}
			continue;
		}
		if (strncmp(fieldName.data(), "correct_poi_path", fieldName.length()) == 0)
		{
			if (fieldValue.get_type() == sajson::TYPE_STRING)
			{
				correct_poi_path = fieldValue.as_string();
			}
			continue;
		}

		if (strncmp(fieldName.data(), "resultpoipath", fieldName.length()) == 0)
		{
			if (fieldValue.get_type() == sajson::TYPE_STRING)
			{
				resultpoipath = fieldValue.as_string();
			}
			continue;
		}

		if (strncmp(fieldName.data(), "markpoipath", fieldName.length()) == 0)
		{
			if (fieldValue.get_type() == sajson::TYPE_STRING)
			{
				markpoipath = fieldValue.as_string();
			}
			continue;
		}

		if (strncmp(fieldName.data(), "originpath", fieldName.length()) == 0)
		{
			if (fieldValue.get_type() == sajson::TYPE_STRING)
			{
				originpath = fieldValue.as_string();
			}
			continue;
		}
	}

	free(data);
}

void Settings::save()
{
}

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>

namespace Enjon { namespace Utils { 

	inline std::string read_file(const char* filePath)
	{
		//Create file pointer to filepath
		//FILE* file = fopen(filePath, "rt"); 
		FILE* file;
		fopen_s(&file, filePath, "rt"); 

		//Seek to end of file
		fseek(file, 0, SEEK_END);

		//Get length of file
		unsigned long length = ftell(file);

		//Create buffer to be read into
		char* data = new char[length + 1];

		//Set data to 0, since ftell is broken...
		memset(data, 0, length + 1);

		//Seek back to beginning of file
		fseek(file, 0, SEEK_SET);

		//Read into data
		fread(data, 1, length, file);

		//close file
		fclose(file);

		//Create return string
		std::string result(data);

		//Free data
		delete[] data;

		return result; 
	}

	inline void ConvertToWchar(char* buffer, wchar_t* wcstring)
	{
		 size_t origsize = strlen(buffer) + 1;
		//const size_t newsize = 100;
		size_t convertedChars = 0;
		//wchar_t string[newsize];
		mbstowcs_s(&convertedChars, wcstring, origsize, buffer, _TRUNCATE);
		//wcscpy_s(wcstring, string);
	}

	inline void parse_JSON(const char* filePath)
	{
		std::string JSON_file = read_file(filePath); 
		// How exactly do I want to parse and store the data of the JSON object? 
		// Should I create a JSON struct and use that?
	}
	
	inline int convert_buffer_to_int(char* buffer, int digits)
	{
		int d = digits - 1, num = 0, index = 0, mult = 1;
						
		while (d > 0)
		{
			mult *= 10;
			d--;
		}

		while (buffer[index] != '\0')
		{
			int n = buffer[index] - '0';
			num += mult * n;
			mult /= 10;
			index++;
		}

		return num;
	}

	inline float convert_buffer_to_float(char* buffer, int size, int decimalIndex)
	{
		int d = size - 2, multleft = 1, decimal = decimalIndex, i = 0;
		float lod = 0.0f, rod = 0.0f, multright = 0.1f; 

		// Get digits to left
		while (i < decimalIndex - 1)
		{
			multleft *= 10;	
			i++;
		} 

		// Get first number
		i = 0;
		while (buffer[i] != '.')
		{
			float n = (float)(buffer[i] - '0');
			lod += (float)(multleft * n);
			multleft /= 10;
			i++;
		}

		// Get past decimal
		i++;

		// Get second number
		while (buffer[i] != '\0')
		{
			float n = (float)(buffer[i] - '0');
			rod += (float)(multright * n);
			i++;
			multright /= 10.0f;
		}

		return lod + rod; 
	}

}} 

#endif
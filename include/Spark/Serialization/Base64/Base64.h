#pragma once
#include <Spark/Serialization/SerializationExport.h>


namespace Spark::Serialization
{
void SERIALIZATION_EXPORTS Base64Encode(const unsigned char* src, int srcLen, unsigned char* dest, int& destLen);
void SERIALIZATION_EXPORTS Base64Decode(const unsigned char* src, int srcLen, unsigned char* dest, int& destLen);
}

#include <Spark/Network/Protocol/ProtocolUtility.h>


namespace spark::network
{
short CalculateSum(unsigned char* buff, int len)
{
	int sum = 0;
	for (auto i = 0; i < len; ++i)
	{
		sum += buff[i];
	}
	return sum % 256;
}
}

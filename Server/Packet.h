#ifndef PACKET_H
#define PAKCET_H
#define WIN32_LEAN_AND_MEAN

#endif

class Packet
{
public:
	void Clear();
	void Append(const void* data, uint32_t size);

	//insert operation
	Packet& operator << (uint32_t data);
	//extract operation
	Packet& operator >> (uint32_t& data);

	uint32_t extractionOffset = 0;
	std::vector<char> buffer;


};
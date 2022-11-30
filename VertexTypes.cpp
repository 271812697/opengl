#include"VertexTypes.h"
unsigned Opengl::VertexPositionNormalTexture::Elementnum = 3;
Opengl::IpnutLayoutDesc Opengl::VertexPositionNormalTexture::inputelements[6] = {
	{0, 3, 0x1406, 0, 8 * sizeof(float), (void*)0},
	{1, 3, 0x1406, 0, 8 * sizeof(float), (void*)12},
	{2, 2, 0x1406, 0, 8 * sizeof(float), (void*)24}
};


#ifndef _Light_
#define _Light_

struct Light
{
	D3DXVECTOR3 dir;
	float specularPower;
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
} light;

#endif
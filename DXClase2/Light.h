struct Light
{
	D3DXVECTOR3 direction;
	float specularPower;
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR4 specular;
} light;

float g_Time = 0.0f;
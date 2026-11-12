struct Light
{
	D3DXVECTOR3 direction;
	float specularPower;
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR4 specular;
} light;

float g_Time = 0.0f;

// Angulo del sol para el cielo (lo controla el horario de entrenamiento:
// mañana, dia, tarde o noche). El shader del cielo lo lee de "pad.x".
float g_DayAngle = 1.4f;
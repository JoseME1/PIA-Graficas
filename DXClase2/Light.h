struct Light
{
	D3DXVECTOR3 direction;
	float specularPower;
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR4 specular;
};

extern Light light;

extern float g_Time;

// Angulo del sol para el cielo (lo controla el horario de entrenamiento:
// mañana, dia, tarde o noche). El shader del cielo lo lee de "pad.x".
extern float g_DayAngle;
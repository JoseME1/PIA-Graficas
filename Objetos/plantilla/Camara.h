#ifndef _Camara_
#define _Camara_

class Camara
{
public:
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Target;
	D3DXVECTOR3 Up;

	D3DXMATRIX View;
	D3DXMATRIX Projection;

	Camara(D3DXVECTOR3 Position, D3DXVECTOR3 Target, D3DXVECTOR3 Up, int Width, int Height)
	{
		this->Position = Position;
		this->Target = Target;
		this->Up = Up;

		//Set the View matrix 
		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);

		//Set the Projection matrix
		D3DXMatrixPerspectiveFovLH(&Projection, 
			D3DXToRadian(45), (float)Width / Height, 
			1.0f, 1000.0f);
	}
};

#endif
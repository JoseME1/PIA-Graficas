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

	D3DXVECTOR3 DefaultForward;
	D3DXVECTOR3 DefaultRight;
	D3DXVECTOR3 Forward;
	D3DXVECTOR3 Right;

	D3DXMATRIX camRotationMatrix;
	D3DXMATRIX groundWorld;

	float Yaw;
	float Pitch;

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

		DefaultForward = D3DXVECTOR3(0.0f,0.0f,1.0f);
		DefaultRight = D3DXVECTOR3(1.0f,0.0f,0.0f);
		Forward = D3DXVECTOR3(0.0f,0.0f,1.0f);
		Right = D3DXVECTOR3(1.0f,0.0f,0.0f);

		Yaw = 0.0f;
		Pitch = 0.0f;
	}

	void UpdateCamera(float moveBackForward, float moveLeftRight, float yaw, float pitch)
	{
		Yaw += yaw;
		Pitch += pitch;

		D3DXMatrixRotationYawPitchRoll(&camRotationMatrix, Yaw, Pitch, 0);
		D3DXVec3TransformCoord(&Target, &DefaultForward, &camRotationMatrix);
		D3DXVec3Normalize(&Target, &Target);

		D3DXMATRIX RotateYTempMatrix;
		D3DXMatrixRotationY(&RotateYTempMatrix, Yaw);

		D3DXVec3TransformCoord(&Right, &DefaultRight, &RotateYTempMatrix);
		D3DXVec3TransformCoord(&Up, &Up, &RotateYTempMatrix);
		D3DXVec3TransformCoord(&Forward, &DefaultForward, &RotateYTempMatrix);

		Position += moveLeftRight*Right;
		Position += moveBackForward*Forward;

		moveLeftRight = 0.0f;
		moveBackForward = 0.0f;

		Target = Position + Target;

		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);
	}

	void setSkyCamara(Camara *camara)
	{
		D3DXMatrixRotationYawPitchRoll(&camRotationMatrix, camara->Yaw, camara->Pitch, 0);
		D3DXVec3TransformCoord(&Target, &DefaultForward, &camRotationMatrix);
		D3DXVec3Normalize(&Target, &Target);

		Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);
	}

	D3DXVECTOR2 getXZ(){
		return D3DXVECTOR2(Position.x, Position.z);
	}
};

#endif
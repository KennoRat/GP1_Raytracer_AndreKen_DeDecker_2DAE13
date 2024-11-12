#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"
#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};

		float fovAngle{ 90.f };

		Vector3 forward{ Vector3 {Vector3::UnitZ} }; // Test Vector3 {0.266f, - 0.453f, 0.860f}
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Vector3 WorldUp{ Vector3::UnitY };

			right = Vector3::Cross(WorldUp, forward);
			right.Normalize();

			up = Vector3::Cross(forward, right);
			up.Normalize();

			Vector4 xRow{ right.x, right.y, right.z, 0 };
			Vector4 yRow{ up.x, up.y, up.z, 0};
			Vector4 zRow{ forward.x, forward.y, forward.z, 0 };
			Vector4 tRow{ origin.x, origin.y, origin.z, 1 };

			return { Matrix(xRow, yRow, zRow, tRow) };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float cameraSpeed{10.f};
			const float cameraRotation{2.f};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Movement WASD
			if (pKeyboardState[SDL_SCANCODE_W]) origin += forward * cameraSpeed * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_S]) origin -= forward * cameraSpeed * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_A]) origin -= right * cameraSpeed * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_D]) origin += right * cameraSpeed * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_E]) fovAngle -= cameraSpeed * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_R]) fovAngle += cameraSpeed * deltaTime;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);


			//LMB + RMB + Y-axis = Up or Down
			if (mouseState == (SDL_BUTTON(1) + SDL_BUTTON(3)) && mouseY > 0) origin -= up * cameraSpeed * deltaTime;
			else if (mouseState == (SDL_BUTTON(1) + SDL_BUTTON(3)) && mouseY < 0) origin += up * cameraSpeed * deltaTime;

			//LMB + Y-axis = forward or backwards
			if (mouseState == SDL_BUTTON(1) && mouseY > 0) origin -= forward * cameraSpeed * deltaTime;
			else if (mouseState == SDL_BUTTON(1) && mouseY < 0) origin += forward * cameraSpeed * deltaTime;

			// Rotate left and right (Yaw)
			if (mouseState == SDL_BUTTON(1) && mouseX > 0) totalYaw += cameraRotation * deltaTime;
			else if (mouseState == SDL_BUTTON(1) && mouseX < 0) totalYaw -= cameraRotation * deltaTime;

			if (mouseState == SDL_BUTTON(3) && mouseX > 0) totalYaw += cameraRotation * deltaTime;
			else if (mouseState == SDL_BUTTON(3) && mouseX < 0) totalYaw -= cameraRotation * deltaTime;

			// Rotate up and down (Pitch)
			if (mouseState == SDL_BUTTON(3) && mouseY > 0) totalPitch -= cameraRotation * deltaTime;
			else if (mouseState == SDL_BUTTON(3) && mouseY < 0) totalPitch += cameraRotation * deltaTime;

			Matrix finalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw)};

			forward = finalRotation.TransformVector(Vector3::UnitZ);

			forward.Normalize();

			//todo: W2
			//throw std::runtime_error("Not Implemented Yet");
		}
	};
}

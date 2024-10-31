//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = float(m_Width) / float(m_Height) ;
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();
	
	//Field of view
	float FOV = tan((camera.fovAngle * (float(M_PI) / 180.f)) / 2.f);
	float rayZValue{ 1.f };

	for (int px{}; px < m_Width; ++px)
	{
		float rayXValue{ ((2.f * ((float(px) + 0.5f) / m_Width)) - 1.f) * m_AspectRatio * FOV };

		for (int py{}; py < m_Height; ++py)
		{
			float rayYValue{ (1.f - (2.f * ((float(py) + 0.5f) / m_Height))) * FOV };
			

			Vector3 rayDirection{ rayXValue, rayYValue, rayZValue };
				
			rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection).Normalized();

			Ray viewRay{ camera.origin , rayDirection};

			ColorRGB finalColor{};

			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				const float occluderShadow{ 0.5 };
				Ray shadowRay{};
				shadowRay.origin = closestHit.origin;
				shadowRay.min = 0.00001f;

				float lambertsLaw{};

				for (const Light& lightsIdx : lights)
				{
					bool occluder{ false };
					shadowRay.max = LightUtils::GetDirectionToLight(lightsIdx, shadowRay.origin).Magnitude();
					shadowRay.direction = LightUtils::GetDirectionToLight(lightsIdx, shadowRay.origin) / shadowRay.max;


					if (pScene->DoesHit(shadowRay) && m_ShadowsEnabled)
					{
						occluder = true;
					}
					else
					{
						lambertsLaw = Vector3::Dot(closestHit.normal, shadowRay.direction);
						if (lambertsLaw > 0)
						{
							switch (m_CurrentLightMode)
							{
							case dae::Renderer::LightingMode::ObservedArea:
								finalColor += ColorRGB{ lambertsLaw,  lambertsLaw , lambertsLaw };
								break;
							case dae::Renderer::LightingMode::Radiance:
								finalColor += LightUtils::GetRadiance(lightsIdx, shadowRay.origin);
								break;
							case dae::Renderer::LightingMode::BRDF:
								finalColor += materials[closestHit.materialIndex]->Shade(closestHit, -shadowRay.direction, rayDirection);
								break;
							case dae::Renderer::LightingMode::Combined:
								finalColor += (LightUtils::GetRadiance(lightsIdx, shadowRay.origin) * materials[closestHit.materialIndex]->Shade(closestHit, -shadowRay.direction, rayDirection) * lambertsLaw);
								break;
							}
						}
					}	
				}
			}
			else finalColor = colors::Black;

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));			
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	switch (m_CurrentLightMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightMode = Renderer::LightingMode::Radiance;
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightMode = Renderer::LightingMode::BRDF;
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightMode = Renderer::LightingMode::Combined;
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightMode = Renderer::LightingMode::ObservedArea;
		break;
	}
}
